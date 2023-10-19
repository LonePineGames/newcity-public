#include "string_proxy.hpp"

#define _GNU_SOURCE /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <ctype.h>
#include <unordered_set>

#include <algorithm>
#include <cctype>

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

const int maxStringLength = 255;

std::unordered_set<std::string> stringIntern;

char* strdup_s(const char* s) {
  return sprintf_o("%s", s);
}

char* strdup_s(const char* s, unsigned int size) {
  if (s == 0) return 0;
  char* result = (char*) malloc(size+1);
  memcpy_s(result, s, size);
  result[size] = '\0';
  return result;
}

#ifdef WIN32
void vasprintf(char **strp, const char *fmt, va_list ap) {
	// _vscprintf tells you how big the buffer needs to be
	int len = _vscprintf(fmt, ap);

	if (len == -1) {
		return;
	}

	size_t size = (size_t)len + 1;
	char* str = (char*)malloc(size);

	if (!str) {
		return;
	}

	int r = vsprintf_s(str, len + 1, fmt, ap);

	if (r == -1) {
		free(str);
		return;
	}

	*strp = str;
}
#endif

char* sprintf_o(const char* format, ...) {

  /*
  char* str = (char*) malloc(maxStringLength*sizeof(char));
  va_list args;
  va_start(args, format);
  int size = stbsp_vsnprintf(str, maxStringLength, format, args) + 1;
  va_end(args);
  //if (size >= maxStringLength) {
    free(str);
    str = (char*) malloc((size+1)*sizeof(char));
    va_list args2;
    va_start(args2, format);
    stbsp_vsnprintf(str, size+1, format, args2);
    va_end(args2);
  //}
  return str;

  #ifdef __linux__
  */
    char* str;// = strdup_s("");
    va_list args;
    va_start(args, format);
    vasprintf(&str, format, args);
    va_end(args);
    return str;

    /*
  #elif _WIN32
    char* str = (char*) malloc(256*sizeof(char));
    va_list args;
    va_start(args, format);
    vsnprintf(str, 256, format, args);
    va_end(args);
    return str;
  #endif
  */
}

bool startsWith(const char* c, const char* s) {
  for (int i = 0;; i++) {
    char si = s[i];
    if (si == '\0') return true;
    if (si != c[i]) return false;
  }
}

bool endsWith(const char* c, const char* e) {
  int i;
  int j;
  for (i = 0; c[i] != '\0'; i++);
  for (j = 0; e[j] != '\0'; j++);
  if (j>i) return false;

  for (;j>=0; j--) {
    if(c[i] != e[j]) {
      return false;
    }
    i--;
  }
  return true;
}

bool streql(const char* c, const char* e) {
  if (c == 0) return e == 0;
  if (e == 0) return false;
  if (c[0] == '\0') return e[0] == '\0';
  int j = 0;
  for (; c[j] != '\0'; j++) {
    if(e[j] == '\0' || c[j] != e[j]) {
      return false;
    }
  }
  if(e[j] != '\0') return false;
  return true;
}

int strlength(const char* c) {
  int i;
  for (i = 0; c[i] != '\0'; i++);
  return i;
}

int strcmpi_s(char const *a, char const *b) {
  for (;; a++, b++) {
    int d = tolower(*a) - tolower(*b);
    if (d != 0 || !*a) {
      return d;
    }
  }
  return 0;
}

void toLower(char *a) {
  for (;*a; a++) {
    *a = tolower(*a);
  }
}

//Adapted from Stack Overflow
char* trimWhiteSpace(char* str) {
  // Trim leading space
  while(isspace((unsigned char)str[0])) {
    str++;
  }

  if(str[0] == 0) { // All spaces?
    return str;
  }

  // Trim trailing space
  char* end = str + strlength(str) - 1;
  while(end > str && isspace((unsigned char)end[0])) {
    end--;
  }

  // Write new null terminator
  end[1] = 0;

  return str;
}

void * memset_s ( void * ptr, int value, unsigned int num ) {
  return memset(ptr, value, num);
}

void* memcpy_s (void* dest, const void* source, unsigned int num) {
  return memcpy(dest, source, num);
}

const char* safe(const char* str) {
  return str == 0 ? "(null)" : str;
}

/// Try to find in the Haystack the Needle - ignore case
bool stringContainsCaseInsensitive(const std::string& haystack,
    const std::string& needle) {
  auto it = std::search(
    haystack.begin(), haystack.end(),
    needle.begin(),   needle.end(),
    [](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
  );
  return (it != haystack.end() );
}

bool stringEqualCaseInsensitive(const std::string& haystack,
    const std::string& needle) {
  auto it = std::search(
    haystack.begin(), haystack.end(),
    needle.begin(),   needle.end(),
    [](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
  );
  return (it != haystack.begin() );
}

bool iequals(const std::string& a, const std::string& b) {
  return std::equal(a.begin(), a.end(),
                    b.begin(), b.end(),
                    [](char a, char b) {
                        return tolower(a) == tolower(b);
                    });
}

char* intern(char* cstr) {
  std::string str(cstr);
  auto search = stringIntern.find(str);

  if (search == stringIntern.end()) {
    stringIntern.insert(str);
    return intern(cstr);

  } else {
    free(cstr);
    return (char*) search->c_str();
  }
}

void resetStringInternTable() {
  stringIntern.clear();
}

// trim from start (in place)
void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

