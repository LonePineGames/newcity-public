#ifndef BOOST_ALL_NO_LIB
	#define BOOST_ALL_NO_LIB
#endif

#include "log.hpp"

#include "console/conDisplay.hpp"
#include "game/constants.hpp"
#include "game/game.hpp"
#include "game/version.hpp"
#include "error.hpp"
#include "string_proxy.hpp"
#include "option.hpp"
#include "platform/file.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/msvc_sink.h"
#include "spdlog/sinks/base_sink.h"

#include <memory>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mutex>
//#include <curl/curl.h>

#ifdef _WIN32
  #include<windows.h>
#endif

#ifdef __linux__
  #include <sys/utsname.h>
#else
 /* #include <boost/beast/core.hpp>
  #include <boost/beast/http.hpp>
  #include <boost/beast/version.hpp>
  #include <boost/asio/connect.hpp>
  #include <boost/asio/ip/tcp.hpp>
  #include <boost/stacktrace.hpp>

  namespace beast = boost::beast;     // from <boost/beast.hpp>
  namespace http = beast::http;       // from <boost/beast/http.hpp>
  namespace asio = boost::asio;        // from <boost/asio.hpp>
  using tcp = asio::ip::tcp;           // from <boost/asio/ip/tcp.hpp>*/
#endif

using namespace spdlog;
using namespace std;

const char* sentinelFilename = ".newcity.lock";
static bool forceLogUpload = false;
static bool errorOverwrite = false;
static uint16_t lastLogNum = 0;

template<typename Mutex>
class console_sink : public spdlog::sinks::base_sink <Mutex> {
  protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
      // log_msg is a struct containing the log entry info like level,
      // timestamp, thread id etc.
      // msg.raw contains pre formatted log

      // If needed (very likely but not mandatory), the sink formats
      // the message before sending it to its final destination:
      spdlog::memory_buf_t formatted;
      spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
      std::string msgStr = fmt::to_string(formatted);
      consolePrintLine(msgStr);
    }

    void flush_() override {}
};

using console_sink_mt = console_sink<std::mutex>;

static size_t read_callback(void *ptr, size_t size,
    size_t nmemb, void *stream) {
  size_t retcode = fread(ptr, size, nmemb, (FILE*) stream);
  SPDLOG_INFO("Read {} bytes from file", retcode);
  return retcode;
}

/*
void checkCurl(CURLcode res) {
  if (res != CURLE_OK) {
    SPDLOG_ERROR("Log file upload upload failed! {}", curl_easy_strerror(res));
  }
}
*/

/* This code is public domain -- Will Hartung 4/9/09 */
size_t getline_o(char **lineptr, size_t *n, FILE *stream) {
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL) {
        return -1;
    }
    if (stream == NULL) {
        return -1;
    }
    if (n == NULL) {
        return -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF) {
        return -1;
    }
    if (bufptr == NULL) {
        bufptr = (char*) malloc(128);
        if (bufptr == NULL) {
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while(c != EOF) {
        if ((p - bufptr) > (size - 1)) {
            size = size + 128;
            int pDiff = p - bufptr;
            bufptr = (char*) realloc(bufptr, size);
            p = bufptr + pDiff;
            if (bufptr == NULL) {
                return -1;
            }
        }
        *p++ = c;
        if (c == '\n') {
            break;
        }
        c = fgetc(stream);
    }

    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;

    return p - bufptr - 1;
}

bool containsString(FILE* file, const char* str) {
  char* line = NULL;
  size_t  len = 0;
  int32_t read;
  while ((read = getline_o(&line, &len, file)) != -1) {
    line[strcspn(line, "\n")] = 0;
    if (strstr(line, str) != NULL) {
      return true;
    }
  }
  return false;
}

void sendLogFile() {
  if (!hasEnabledLogUpload()) {
    SPDLOG_INFO("User has opted out of log uploading");
    return;
  }
  #ifdef LP_DEBUG
    return;
  #endif

  //try {
    /*
    CURL *curl = curl_easy_init();
    if (!curl) {
      SPDLOG_ERROR("Could not init libcurl!");
      return;
    }
    */

  #ifndef __linux__
    FILE *file;
    const char* filename = "game_log.1.log";
    if (file = fopen(filename, "r")) {
      fclose(file);
      if(!forceLogUpload) return;
      /*
      SPDLOG_INFO("Setting up log file upload");
      asio::io_context ioc;
      tcp::resolver resolver(ioc);
      beast::tcp_stream stream(ioc);
      auto const host = "logs.newcities.io";
      auto const resolv = resolver.resolve(host, "80");
      stream.connect(resolv);
      http::request<http::file_body> req(http::verb::put, "/", 11);
      req.set(http::field::host, host);
      req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
      req.insert("x-newcities-version", versionStringUnderscored());

      http::file_body::value_type body;
      beast::error_code ec;
      body.open(filename, boost::beast::file_mode::read, ec);
      if (ec) {
        SPDLOG_ERROR("Error opening log file for upload! {}", ec.message());
      }
      req.body() = std::move(body);
      req.prepare_payload();

      http::write(stream, req);
      http::response<http::dynamic_body> res;
      beast::flat_buffer buffer;
      http::read(stream, buffer, res);
      stream.socket().shutdown(tcp::socket::shutdown_both, ec);
      if (res.result_int() != 200 || ec) {
        SPDLOG_ERROR("Log file upload failed! {} {}", res.result_int(),
            ec.message());
      } else {
        SPDLOG_INFO("Log file uploaded");
      }

      /*
      checkCurl(curl_easy_setopt(curl, CURLOPT_URL,
            "http://logs.newcities.io"));
      checkCurl(curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L));
      checkCurl(curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L));
      checkCurl(curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback));
      checkCurl(curl_easy_setopt(curl, CURLOPT_READDATA, file));
      #ifdef LP_DEBUG
        checkCurl(curl_easy_setopt(curl, CURLOPT_VERBOSE, true));
      #endif

      struct curl_slist *headers = NULL;
      char* versionHeader = sprintf_o("x-newcities-version: %s",
          versionStringUnderscored());
      headers = curl_slist_append(headers, versionHeader);
      headers = curl_slist_append(headers, "Transfer-Encoding: chunked");
      checkCurl(curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers));

      CURLcode res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
        SPDLOG_ERROR("Log file upload failed! {}", curl_easy_strerror(res));
      } else {
        curl_off_t speed_upload, total_time, size_upload;
        curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD_T, &size_upload);
        curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &speed_upload);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total_time);
        SPDLOG_INFO("Log file uploaded: {}kb in {}ms",
            size_upload/1000.0, total_time/1000);
      }

      curl_slist_free_all(headers);
      free(versionHeader);
      curl_easy_cleanup(curl);
      */
    }

  //} catch (const std::exception& ex) {
    //SPDLOG_ERROR("Error uploading log file: {}", ex.what());
    //logStacktrace();
  //} catch (...) {} // If you feel the need to vomit, just swallow it.
  #endif
}

void initLogging() {
  //try {
    auto stdoutSink = make_shared<sinks::stdout_color_sink_mt>();
    stdoutSink->set_level(level::info);

    // Try to create the game_log fileSink
    FILE* file = 0;
    std::string logNum = "";

    // If the sentinel file exists, we exited abnormally; create a numbered game_log backup of the previous game_log
    // If all game_log numbers are used up, we overwrite the last one
    if(fileExists(sentinelFilename)) {
        for(lastLogNum; lastLogNum < MAX_NUM_LOGS; lastLogNum++) {
          logNum = lastLogNum != 0 ? "." + std::to_string(lastLogNum) : "";
          if(!(file = fopen((LOG_FILENAME + logNum + LOG_EXT).c_str(), "r"))) break; // Found the first available game_log num
          // Close file if it was opened
          fclose(file);
          // If there are no more available game_log slots, we "overwrite" the last one
          if(lastLogNum + 1 == MAX_NUM_LOGS) {
            remove((LOG_FILENAME + logNum + LOG_EXT).c_str());
          }
        }
        // Rename the previous game_log as a numbered backup
        rename((LOG_FILENAME + LOG_EXT).c_str(), (LOG_FILENAME + logNum + LOG_EXT).c_str());
    }

    // Prepare game_log to be overwritten
    if(!(file = fopen((LOG_FILENAME + LOG_EXT).c_str(), "w"))) {
      errorOverwrite = true; // If we were not returned a pointer to the file, there was an error opening the file
    }

    // Close file when we're done
    if(file) fclose(file);

    // Always log to the current game_log
    auto fileSink = make_shared<sinks::rotating_file_sink_mt>(
        (LOG_FILENAME + LOG_EXT).c_str(), 10000000, 5);
    fileSink->set_level(level::trace);

    auto consoleSink = make_shared<console_sink_mt>();
    consoleSink->set_level(level::trace);

    vector<spdlog::sink_ptr> sinks {stdoutSink, fileSink, consoleSink};

    #ifdef _WIN32
      auto msvcSink = make_shared<sinks::msvc_sink_mt>();
      sinks.push_back(msvcSink);
    #endif

    auto mainLog = make_shared<logger>("main", sinks.begin(), sinks.end());
    mainLog->set_pattern(
        "%M:%S.%e %l [t%5t] %s:%#%^>\n          %v%$");
    mainLog->set_level(level::trace);
    mainLog->flush_on(level::warn);
    set_default_logger(mainLog);
    SPDLOG_INFO("Init NewCity, {}", versionString());

    if(errorOverwrite) {
      errorOverwrite = false;
      SPDLOG_ERROR("Overwrite issue when setting up game_log");
    }

    #ifdef _WIN32
      OSVERSIONINFOEX info;
      ZeroMemory(&info, sizeof(OSVERSIONINFOEX));
      info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
      GetVersionEx((LPOSVERSIONINFO)&info);//info requires typecasting
      SPDLOG_INFO("Windows version: {}.{}",
          info.dwMajorVersion, info.dwMinorVersion);
    #endif

    #ifdef MINGW
      SPDLOG_INFO("Built with MingW");
    #endif

    #ifdef __linux__
      struct utsname my_uname;
      if(uname(&my_uname) == -1) {
         SPDLOG_INFO("uname call failed!");
      } else {
         SPDLOG_INFO("System name:{} Nodename:{} Release:{} ",
             my_uname.sysname, my_uname.nodename, my_uname.release);
         SPDLOG_INFO("Version:{} Machine:{}",
             my_uname.version, my_uname.machine);
      }
    #endif

    mainLog->flush();

    #ifdef LP_DEBUG
      return;
    #endif

    forceLogUpload = false;
    if (fileExists(sentinelFilename)) {
      SPDLOG_ERROR("Previous run did not exit normally");
      doErrorLoad();
      forceLogUpload = true;

    } else {
      FILE *file;
      if (file = fopen(sentinelFilename, "wb")) {
        fclose(file);
        SPDLOG_INFO("Wrote sentinel file");
      } else {
        SPDLOG_ERROR("Failed to write sentinel file");
      }
    }

  //} catch (const spdlog::spdlog_ex& ex) {
    //handleError("Log init failed");
  //}
}

void removeSentinelFile() {
  if (!anyErrorDetected()) {
    SPDLOG_INFO("Removing sentinel file");
    remove(sentinelFilename);
  } else {
    SPDLOG_WARN("Not removing sentinel file, since there was an error.");
  }
}

