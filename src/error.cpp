#ifndef BOOST_ALL_NO_LIB
	#define BOOST_ALL_NO_LIB
#endif

#include "spdlog/spdlog.h"

#ifdef WIN32
  #include "StackWalker.h"

  class NewCityStackWalker : public StackWalker
  {
  private:
    bool calledStacktrace = false;
    bool skipFirst = false;
    std::string stacktrace;
  public:
    NewCityStackWalker() : StackWalker (0xF) {
      stacktrace = "";
    }

    void BuildStacktraceString() {
      if(calledStacktrace) {
        return;
      }

      this->ShowCallstack();
      calledStacktrace = true;
    }

    std::string GetStacktraceString() {
      return stacktrace;
    }
  protected:
    virtual void OnOutput(LPCSTR szText) {
      if(!skipFirst) {
        skipFirst = true;
        return;
      }
      std::string addText = szText;
      size_t lastBS = addText.find_last_of('\\', std::string::npos);
      if(lastBS != std::string::npos) {
        addText = addText.substr(lastBS, std::string::npos);
      }
      stacktrace += addText;
    }
  };
#endif

#include "error.hpp"

#include "cup.hpp"
#include "game/game.hpp"
#include "option.hpp"
#include "string_proxy.hpp"
#include "serialize.hpp"
#include "sound.hpp"
#include "util.hpp"

#include "parts/root.hpp"

#include <fstream>
#include <cstdio>
#include <string.h>

//#define BOOST_STACKTRACE_USE_ADDR2LINE
#ifdef _WIN32
  #define BOOST_STACKTRACE_USE_WINDBG
#else
//  #define BOOST_STACKTRACE_USE_BACKTRACE
#endif
//#define BOOST_STACKTRACE_USE_BACKTRACE

#define BOOST_NO_EXCEPTIONS
#include <boost/throw_exception.hpp>
#ifdef _WIN32
  void __cdecl boost::throw_exception(std::exception const & e){
    //do nothing
  }

  /*
  void __cdecl boost::throw_exception(std::exception const & e, boost::source_location const & sl){
    //do nothing
  }
  */
#endif

#include <boost/stacktrace.hpp>

struct BinaryMapEntry {
  uint64_t memOffset;
  char* functionName;
};

const int32_t MAX_ERROR_WIN = 10;

static char* binaryMap = NULL;
static Cup<BinaryMapEntry> functionTable;
static bool wasAnyErrorDetected = false;


bool anyErrorDetected() {
  return wasAnyErrorDetected;
}

void parseBinaryMap() {
  std::ifstream is("binary_map", std::ifstream::binary);
  if (is) {
    // Get length of file
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);

    // Read into buffer
    binaryMap = new char[length+1];
    is.read(binaryMap, length);
    is.close();
    binaryMap[length] = '\0';

    // Build functionTable
    functionTable.reserve(15000);
    int lastBreak = 0;
    bool readingFunctionName = false;
    for (int i = 0; i < length; i++) {
      char c = binaryMap[i];

      if (readingFunctionName) {
        if (c == '\n' || c == '\r' ) {
          binaryMap[i] = '\0';
          BinaryMapEntry entry = functionTable.back();
          lastBreak = i+1;
          readingFunctionName = false;
        }

      } else if (c == ' ') {
        BinaryMapEntry entry;
        entry.functionName = &binaryMap[i+1];
        entry.memOffset = strtol(&binaryMap[lastBreak], NULL, 16);
        functionTable.push_back(entry);
        readingFunctionName = true;
      }
    }
    SPDLOG_INFO("Parsed binary_map: {} entries found", functionTable.size());
  }
}

const char* lookupFunction(uint64_t memOffset) {
  int first = 0;
  int size = functionTable.size();
  int last = size-1;
  int middle = (first + last)/2;

  while (first + 1 < last) {
    BinaryMapEntry* entry = functionTable.get(middle);
    if (entry->memOffset < memOffset) {
      first = middle;
    } else if (entry->memOffset == memOffset) {
      return entry->functionName;
    } else {
      last = middle;
    }
    middle = (first + last)/2;
    middle = middle < 0 ? 0 : middle >= size ? size-1 : middle;
  }
  return functionTable.get(middle)->functionName;
}

void resolveFunctions(char* stacktrace) {
  char* memOffsetPtr = 0;
  char* functionNamePtr = 0;

  for (int i = 0;; i++) {
    char c = stacktrace[i];

    if (c == '\n' || c == '\0') {
      if (memOffsetPtr != 0 && functionNamePtr != 0 &&
          (strstr(functionNamePtr, "newcities") ||
          strstr(functionNamePtr, "newcity"))) {
        int memOffset = strtol(memOffsetPtr, NULL, 16);
        const char* functionName = lookupFunction(memOffset);
        char* cAddr = &stacktrace[i];
        int j = 0;

        for (; &functionNamePtr[j] < &stacktrace[i] &&
            functionName[j] != '\0'; j++) {
          functionNamePtr[j] = functionName[j];
        }
        for (; &functionNamePtr[j] < &stacktrace[i]; j++) {
          functionNamePtr[j] = ' ';
        }
      }
      memOffsetPtr = 0;
      functionNamePtr = &stacktrace[i+1];
      if (c == '\0') {
        break;
      }

    } else if (memOffsetPtr == 0 && c == 'x') {
      memOffsetPtr = &stacktrace[i+1];
      functionNamePtr = &stacktrace[i-1];
    }
  }
}

void handleSignal(int sig) {
  std::string sigType = "";

  switch(sig) {
    case SIGABRT:
      sigType = "Abnormal Abort";
      break;
    case SIGFPE:
      sigType = "Floating-Point Exception";
      break;
    case SIGILL:
      sigType = "Illegal Instruction";
      break;
    case SIGINT:
      sigType = "Signal Interrupt";
      break;
    case SIGSEGV:
      sigType = "Segmentation Violation";
      break;
    case SIGTERM:
      sigType = "Termination Request";
      break;
    default:
      sigType = "Unknown signal type";
      break;
  }

  SPDLOG_ERROR("Raised signal: {}", sigType);

  handleError(sigType.c_str());
}

void initErrorHandlingCore() {
  /*
  std::ifstream ifs("./backtrace.dump", std::ifstream::in);
  if (ifs.good()) {
      boost::stacktrace::stacktrace st =
        boost::stacktrace::stacktrace::from_dump(ifs);
      SPDLOG_ERROR("ERROR: Previous run crashed: {}", to_string(st));
      ifs.close();
      std::remove("./backtrace.dump");
  }
  */
}

#ifdef __linux__
  #include <signal.h>
  #include <stdio.h>
  #include <stdlib.h>

  void segfault_sigaction(int signal, siginfo_t *si, void *arg) {
    handleError("SEGFAULT!");
  }

  void initErrorHandling() {
    struct sigaction sa;

    //memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_sigaction;
    sa.sa_flags   = SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);

    initErrorHandlingCore();
  }

#elif _WIN32

  // Compile with /EHa
  #include <windows.h>
  #include <eh.h>
  #include <psapi.h>
  #include <string>
  #include <sstream>

  static std::atomic<int32_t> errorNumWin = 0;

  class InfoFromSE
  {
  public:
    typedef unsigned int exception_code_t;

    static const char* opDescription(const ULONG opcode)
    {
      switch (opcode) {
      case 0: return "read";
      case 1: return "write";
      case 8: return "user-mode data execution prevention (DEP) violation";
      default: return "unknown";
      }
    }

    static const char* seDescription(const exception_code_t& code)
    {
      switch (code) {
        case EXCEPTION_ACCESS_VIOLATION:
          return "EXCEPTION_ACCESS_VIOLATION";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
          return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
        case EXCEPTION_BREAKPOINT:
          return "EXCEPTION_BREAKPOINT";
        case EXCEPTION_DATATYPE_MISALIGNMENT:
          return "EXCEPTION_DATATYPE_MISALIGNMENT";
        case EXCEPTION_FLT_DENORMAL_OPERAND:
          return "EXCEPTION_FLT_DENORMAL_OPERAND";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
          return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
        case EXCEPTION_FLT_INEXACT_RESULT:
          return "EXCEPTION_FLT_INEXACT_RESULT";
        case EXCEPTION_FLT_INVALID_OPERATION:
          return "EXCEPTION_FLT_INVALID_OPERATION";
        case EXCEPTION_FLT_OVERFLOW:
          return "EXCEPTION_FLT_OVERFLOW";
        case EXCEPTION_FLT_STACK_CHECK:
          return "EXCEPTION_FLT_STACK_CHECK";
        case EXCEPTION_FLT_UNDERFLOW:
          return "EXCEPTION_FLT_UNDERFLOW";
        case EXCEPTION_ILLEGAL_INSTRUCTION:
          return "EXCEPTION_ILLEGAL_INSTRUCTION";
        case EXCEPTION_IN_PAGE_ERROR:
          return "EXCEPTION_IN_PAGE_ERROR";
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
          return "EXCEPTION_INT_DIVIDE_BY_ZERO";
        case EXCEPTION_INT_OVERFLOW:
          return "EXCEPTION_INT_OVERFLOW";
        case EXCEPTION_INVALID_DISPOSITION:
          return "EXCEPTION_INVALID_DISPOSITION";
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
          return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
        case EXCEPTION_PRIV_INSTRUCTION:
          return "EXCEPTION_PRIV_INSTRUCTION";
        case EXCEPTION_SINGLE_STEP:
          return "EXCEPTION_SINGLE_STEP";
        case EXCEPTION_STACK_OVERFLOW:
          return "EXCEPTION_STACK_OVERFLOW";
        default: return "UNKNOWN EXCEPTION";
      }
    }

    static std::string information(struct _EXCEPTION_POINTERS* ep) {
      exception_code_t code = ep->ExceptionRecord->ExceptionCode;
      HMODULE hm;
      ::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
          static_cast<LPCTSTR>(ep->ExceptionRecord->ExceptionAddress), &hm);
      MODULEINFO mi;
      ::GetModuleInformation(::GetCurrentProcess(), hm, &mi, sizeof(mi));
      char fn[MAX_PATH];
      ::GetModuleFileNameExA(::GetCurrentProcess(), hm, fn, MAX_PATH);

      std::ostringstream oss;
      oss << "SE " << seDescription(code) << " at address 0x" << std::hex <<
        ep->ExceptionRecord->ExceptionAddress << std::dec
        << " inside " << fn << " loaded at base address 0x" << std::hex <<
        mi.lpBaseOfDll << "\n";

      if (code == EXCEPTION_ACCESS_VIOLATION ||
          code == EXCEPTION_IN_PAGE_ERROR) {
        oss << "Invalid operation: " <<
          opDescription(ep->ExceptionRecord->ExceptionInformation[0]) <<
          " at address 0x" << std::hex <<
          ep->ExceptionRecord->ExceptionInformation[1] << std::dec << "\n";
      }

      if (code == EXCEPTION_IN_PAGE_ERROR) {
        oss << "Underlying NTSTATUS code that resulted in the exception " <<
          ep->ExceptionRecord->ExceptionInformation[2] << "\n";
      }

      return oss.str();
    }
  };

  /*
  LONG WINAPI NCVectorExceptionHandler(
      struct _EXCEPTION_POINTERS *ExceptionInfo) {
    // SPDLOG_ERROR("Unhandled Exception {}",
        // InfoFromSE::information(ExceptionInfo));
    // handleError("Unhandled Exception");
    return EXCEPTION_CONTINUE_EXECUTION;
  }
  */

  LONG WINAPI TopLevelExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo) {

    if (errorNumWin >= MAX_ERROR_WIN)
      return EXCEPTION_CONTINUE_SEARCH; // Should either get handled elsewhere or crash

    SPDLOG_ERROR("Unhandled Exception {}",
        InfoFromSE::information(pExceptionInfo));
    // handleError("Unhandled Exception");
    // return EXCEPTION_CONTINUE_SEARCH;

    errorNumWin++;

    if (errorNumWin == MAX_ERROR_WIN) {
      SPDLOG_ERROR("Supressing additional Unhandled Exceptions due to maximum reached, crashes may result...");
    }

    return EXCEPTION_CONTINUE_EXECUTION; // Try to continue
  }

  void initErrorHandling() {
    // AddVectoredExceptionHandler(1, NCVectorExceptionHandler);
    SetUnhandledExceptionFilter(TopLevelExceptionHandler);
    initErrorHandlingCore();
    parseBinaryMap();

    // Set signal handling 
    ::signal(SIGABRT, handleSignal);
    ::signal(SIGFPE, handleSignal);
    ::signal(SIGILL, handleSignal);
    ::signal(SIGINT, handleSignal);
    ::signal(SIGSEGV, handleSignal);
    ::signal(SIGTERM, handleSignal);
  }

#elif __APPLE__
  // TODO: MacOSX error handling
#endif

static bool errorState = false;

void handleErrorInner(const char* error) {
  SPDLOG_ERROR("ERROR: {}", error);
  //boost::stacktrace::safe_dump_to("./backtrace.dump");
  logStacktrace();
  bool firstError = !errorState;
  errorState = true;
  wasAnyErrorDetected = true;
  if (firstError) {
    playErrorSound();
    setGameSpeed(0);
    setErrorMessage(strdup_s(error));
    //crashAutosave();
    #ifdef LP_DEBUG
    //if (debugMode()) {
      TRIP;
      //int i = 1 / 0;
    //}
    #endif
  }
  //throw error;
}

void handleError(const char* format, ...) {
  char* str;
  #ifdef __linux__
    va_list args;
    va_start(args, format);
    vasprintf(&str, format, args);
    va_end(args);

  #elif _WIN32
    str = (char*) malloc(256*sizeof(char));
    va_list args;
    va_start(args, format);
    vsnprintf(str, 256, format, args);
    va_end(args);
  #endif

  handleErrorInner(str);
}

void clearErrorState() {
  errorState = false;
}

template <class Allocator>
std::string to_string(const boost::stacktrace::basic_stacktrace<Allocator>& bt) {
    if (bt) {
        return boost::stacktrace::detail::to_string(&bt.as_vector()[0], bt.size());
    } else {
        return std::string();
    }
}

void logStacktrace() {
  #ifdef _WIN32
    std::string stacktraceStr = boost::stacktrace::to_string(boost::stacktrace::stacktrace());
    #if defined(__MINGW32__)
      char* stacktrace = (char*)stacktraceStr.c_str();
      resolveFunctions(stacktrace);
      SPDLOG_WARN("Stacktrace:\n{}", stacktrace);
    #else
      NewCityStackWalker ncsw;
      ncsw.BuildStacktraceString();
      SPDLOG_WARN("StackWalker Stacktrace:\n{}", ncsw.GetStacktraceString());
    #endif
  #else
    std::string stacktraceStr = boost::stacktrace::to_string(boost::stacktrace::stacktrace());
    SPDLOG_WARN("Stacktrace:\n{}", stacktraceStr);
  #endif
}

