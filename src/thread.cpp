#include "thread.hpp"

#include <thread>

#include "spdlog/spdlog.h"

#ifdef __linux__
  #include <sys/prctl.h>
#endif
#ifdef _WIN32
  // #include <VersionHelpers.h> // Will eventually need for testing Windows versions
  #include <processthreadsapi.h>
#endif

struct ThreadData {
  const char* name;
  ThreadCallback callback;
};

void* startThreadCallback(void* dataUntyped) {
  ThreadData* data = (ThreadData*) dataUntyped;
  const char* name = data->name;
  ThreadCallback callback = data->callback;
  free(data);
  nameThread(name);
  SPDLOG_INFO("Starting Thread {}", name);

  callback();

  SPDLOG_INFO("Ending Thread {}", name);
  return 0;
}

void startThread(const char* name, ThreadCallback callback) {
  ThreadData* data = (ThreadData*) malloc(sizeof(ThreadData));
  data->name = name;
  data->callback = callback;

  std::thread thread(startThreadCallback, data);
  thread.detach();
}

void nameThread(const char* name) {
  #ifdef __linux__
    prctl(PR_SET_NAME, name, 0, 0, 0);

  #elif _WIN32

    // Early return for right now
    return;

    // Commenting out for now
    // We will actually want to do this right in the future,
    // which will require an app manifest specifying Win10
    /*
    // Checking for Win8, because absent of a manifest Win10 will
    // still return the Win8 version
    // Version should be 6.2 if checking precisely
    
    if (!IsWindows8OrGreater()) {
      SPDLOG_INFO("Windows version does not support SetThreadDescription");
      return;
    }

    SPDLOG_INFO("Setting thread description to {}", name);

    HRESULT r;
    wchar_t* wString = new wchar_t[4096];
    MultiByteToWideChar(CP_ACP, 0, name, -1, wString, 4096);
    r = SetThreadDescription(
      GetCurrentThread(),
      wString
    );
    */
  #endif
}

void sleepMicroseconds(int sleepTime) {
  std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
}

void sleepMilliseconds(int sleepTime) {
  std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
}
