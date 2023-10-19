#pragma once

#include <string>

#if defined(_WIN32) || defined(_WIN64)
  #define strtok_r strtok_s
#endif

char* strdup_s(const char* s);
char* strdup_s(const char* s, unsigned int size);
char* sprintf_o(const char* format, ...);
bool endsWith(const char* c, const char* e);
bool startsWith(const char* c, const char* s);
bool streql(const char* c, const char* e);
int strlength(const char* c);
int strcmpi_s(char const *a, char const *b);
char* trimWhiteSpace(char* str);
void toLower(char *a);
void* memset_s (void* ptr, int value, unsigned int num);
void* memcpy_s (void* dest, const void* source, unsigned int num);
const char* safe(const char* str);
char* intern(char* str);
void resetStringInternTable();
bool stringContainsCaseInsensitive(const std::string& haystack,
    const std::string& needle);
bool iequals(const std::string& a, const std::string& b);
void trim(std::string &s);
void rtrim(std::string &s);

