#pragma once

#include "error.hpp"
#include "item.hpp"
#include "line.hpp"
#include "main.hpp"
#include "route/location.hpp"

#include <stdio.h>
#include <string>

const int _fileExists = 1 << 0;
const int _fileError = 1 << 1;
const int _fileLinux = 1 << 2;
const int _fileCompressed = 1 << 3;
const int _fileAcceleratedStart  = 1 << 4;
const int _fileLeftHandTraffic = 1 << 5;
const int _fileDifficultyShift = 16;
const int _fileDifficultyMask = 3 << _fileDifficultyShift;

#ifdef __linux__
  const int defaultFileFlags = _fileExists | _fileLinux;
#else
  const int defaultFileFlags = _fileExists;
#endif

struct FileBuffer {
  int flags;
  int version;
  int patchVersion;
  int capacity;
  int length;
  int cursor;
  int lastSegment;
  const char* lastSegmentName;
  char* data;
};

FileBuffer makeFileBuffer();
FileBuffer* makeFileBufferPtr();
void writeToFile(FileBuffer* buf, FILE* out);
FileBuffer readFromFile(const char* filename, bool winCompat = false);
void freeBuffer(FileBuffer* buf);
void compressBuffer(FileBuffer* buffer);
void decompressBuffer(FileBuffer* buffer);
void decompressBuffer(FileBuffer* buffer);
void startSegment(FileBuffer* buffer, const char* segmentName);
FileBuffer* base64ToBuffer(const char* base64);
char* bufferToBase64(FileBuffer* buffer);
FileBuffer* bufferFromString(const char* str);
void fwrite_buffer(FileBuffer* dest, FileBuffer* src);

size_t fwrite(const void* data, size_t num, size_t size, FileBuffer* buffer);
void fwrite_char  (FileBuffer* out, char c);
void fwrite_int   (FileBuffer* out, int num);
void fwrite_item  (FileBuffer* out, item ndx);
void fwrite_int32_t(FileBuffer* out, int32_t num);
void fwrite_uint8_t(FileBuffer* out, uint8_t num);
void fwrite_uint32_t(FileBuffer* out, uint32_t num);
void fwrite_int64_t(FileBuffer* out, int64_t num);
void fwrite_uint64_t(FileBuffer* out, uint64_t num);
void fwrite_vec2  (FileBuffer* out, vec2 v);
void fwrite_vec3  (FileBuffer* out, vec3 v);
void fwrite_float (FileBuffer* out, float num);
void fwrite_double(FileBuffer* out, double num);
void fwrite_string(FileBuffer* out, const char* str);
void fwrite_string(FileBuffer* out, std::string str);
void fwrite_string_no_term(FileBuffer* out, const char* str);
void fwrite_line  (FileBuffer* out, Line line);
void fwrite_item_vector(FileBuffer* in, vector<item> *list);
void fwrite_location(FileBuffer* in, Location loc);
void fwrite_location_vector(FileBuffer* in, vector<Location> *list);
void fwrite_graph_location(FileBuffer* in, GraphLocation loc);
void fputs(const char* str, FileBuffer* out);

size_t fread(void* data, size_t num, size_t size, FileBuffer* buffer);
char   fread_char  (FileBuffer* in);
int    fread_int   (FileBuffer* in);
item   fread_item  (FileBuffer* in, int version);
item   fread_item  (FileBuffer* in);
int32_t fread_int32_t(FileBuffer* in);
uint8_t fread_uint8_t(FileBuffer* in);
uint32_t fread_uint32_t(FileBuffer* in);
int64_t   fread_int64_t(FileBuffer* in);
uint64_t fread_uint64_t(FileBuffer* in);
vec3   fread_vec3  (FileBuffer* in);
vec2   fread_vec2  (FileBuffer* in);
float  fread_float (FileBuffer* in);
double fread_double(FileBuffer* in);
char*  fread_string(FileBuffer* in);
std::string  fread_cpp_string(FileBuffer* in);
char*  fread_string_until_whitespace(FileBuffer* in);
char*  fread_string_until(FileBuffer* in, char term);
Line   fread_line  (FileBuffer* in);
void   fread_item_vector(FileBuffer* in, vector<item> *result, int version);
Location fread_location(FileBuffer* in);
void   fread_location_vector(FileBuffer* in,
    vector<Location> *result, int version);
GraphLocation fread_graph_location(FileBuffer* in, int version);

template <typename T>
void fwrite_data(FileBuffer* out, T* val, int num) {
  fwrite_int(out, num);
  fwrite(val, sizeof(T), num, out);
}

template <typename T>
int fread_data(FileBuffer* in, T** result) {
  int num = fread_int(in);
  *result = (T*) malloc(sizeof(T)*num);
  fread(*result, sizeof(T), num, in);
  return num;
}

