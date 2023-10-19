#include "serialize.hpp"

#include "board.hpp"
#include "building/building.hpp"
#include "building/design.hpp"
#include "business.hpp"
#include "city.hpp"
#include "collisionTable.hpp"
#include "draw/camera.hpp"
#include "draw/entity.hpp"
#include "error.hpp"
#include "game/constants.hpp"
#include "game/game.hpp"
#include "graph.hpp"
#include "heatmap.hpp"
#include "input.hpp"
#include "land.hpp"
#include "lot.hpp"
#include "money.hpp"
#include "person.hpp"
#include "pillar.hpp"
#include "plan.hpp"
#include "string_proxy.hpp"
#include "time.hpp"
#include "vehicle/vehicle.hpp"
#include "weather.hpp"

#include "parts/messageBoard.hpp"
#include "parts/root.hpp"
#include "parts/toolbar.hpp"

#include "spdlog/spdlog.h"
#include "lz4.h"
#include "base64.h"

#include <fstream>

#ifdef WIN32
#include <codecvt>
#endif

const int headerSize = 12;

FileBuffer makeFileBuffer() {
  FileBuffer result;
  result.flags = _fileExists;
  result.capacity = 0;
  result.length = 0;
  result.cursor = 0;
  result.data = 0;
  result.lastSegment = 0;
  result.lastSegmentName = 0;
  return result;
}

FileBuffer* makeFileBufferPtr() {
  FileBuffer* result = new FileBuffer();
  result->flags = _fileExists;
  result->capacity = 0;
  result->length = 0;
  result->cursor = 0;
  result->data = 0;
  result->lastSegment = 0;
  result->lastSegmentName = 0;
  return result;
}

void freeBuffer(FileBuffer* buf) {
  if (buf->data != 0 && buf->capacity > 0) {
    free(buf->data);
  }
  buf->data = 0;
  buf->capacity = 0;
}

size_t fread(void* data, size_t num, size_t size, FileBuffer* buffer) {
  int readSize = num*size;
  if (buffer->cursor + readSize > buffer->length) {
    if (!(buffer->flags & _fileError)) {
      buffer->flags |= _fileError;
      handleError("Overread buffer");
    }
    memset_s(data, 0, readSize);
    return num;
  }

  memcpy_s(data, &buffer->data[buffer->cursor], readSize);
  buffer->cursor += readSize;
  return num;
}

size_t fwrite(const void* data, size_t num, size_t size, FileBuffer* buffer) {
  if (num == 0 || size == 0) return num;

  int writeSize = num*size;
  if (buffer->cursor + writeSize >= buffer->capacity) {
    if (buffer->capacity == 0 || buffer->data == 0) {
      buffer->data = (char*) malloc(writeSize);
      buffer->capacity = writeSize;

    } else {
      int targetSize = buffer->capacity * 1.5f;
      int minSize = buffer->cursor + writeSize;
      if (targetSize < minSize) {
        targetSize = minSize;
      }
      buffer->data = (char*) realloc(buffer->data, targetSize);
      buffer->capacity = targetSize;
    }
  }

  memcpy_s(&buffer->data[buffer->cursor], data, writeSize);
  buffer->cursor += writeSize;
  buffer->length = std::max(buffer->cursor, buffer->length);
  return num;
}

void compressBuffer(FileBuffer* buffer) {
  int uncompSize = buffer->length;
  char* uncompData = &buffer->data[headerSize];
  int compSize = LZ4_compressBound(uncompSize-headerSize)+headerSize;
  char* compData = (char*)malloc(compSize+headerSize);
  char* compDataPtr = &compData[headerSize];

  buffer->cursor = headerSize-4;
  fwrite_int(buffer, uncompSize);

  SPDLOG_INFO("Compress {}=>{}", uncompSize, compSize);
  int finalCompSize = headerSize +
    LZ4_compress_default(uncompData, compDataPtr,
      uncompSize-headerSize, compSize-headerSize);
  if (finalCompSize <= 0) {
    handleError("Compression Error");
  }

  SPDLOG_INFO("Final Size {}", finalCompSize);
  memcpy_s(compData, buffer->data, headerSize);
  free(buffer->data);
  buffer->data = compData;
  buffer->capacity = compSize;
  buffer->length = finalCompSize;
  buffer->cursor = buffer->length;
  buffer->flags |= _fileCompressed;
}

void decompressBuffer(FileBuffer* buffer) {
  int compSize = buffer->length;
  char* compData = &buffer->data[headerSize];
  buffer->cursor = headerSize-4;
  int uncompSize = fread_int(buffer);
  char* uncompData = (char*) malloc(uncompSize);
  char* uncompDataPtr = &uncompData[headerSize];

  //SPDLOG_INFO("Decompress {}=>{}", compSize, uncompSize);
  int finalUncompSize = headerSize +
    LZ4_decompress_safe(compData, uncompDataPtr,
      compSize-headerSize, uncompSize-headerSize);
  if (finalUncompSize < 0) {
    handleError("Decompression failed; corrupt file?");
  }

  memcpy_s(uncompData, buffer->data, headerSize);
  free(buffer->data);
  buffer->data = uncompData;
  buffer->capacity = uncompSize;
  buffer->length = finalUncompSize;
  buffer->cursor = headerSize;
}

void startSegment(FileBuffer* buffer, const char* segmentName) {
  if (buffer->lastSegmentName != 0) {
    item bytes = buffer->cursor - buffer->lastSegment;
    if (bytes > 1000000000) {
      SPDLOG_INFO("{:.2f} Gb    in Segment {}",
          bytes/1000.f/1000.f/1000.f, buffer->lastSegmentName);
    } else if (bytes > 1000000) {
      SPDLOG_INFO("{:.2f} Mb    in Segment {}",
          bytes/1000.f/1000.f, buffer->lastSegmentName);
    } else if (bytes > 1000) {
      SPDLOG_INFO("{:.2f} Kb    in Segment {}",
          bytes/1000.f, buffer->lastSegmentName);
    } else {
      SPDLOG_INFO("{} bytes in Segment {}",
          bytes, buffer->lastSegmentName);
    }
  }

  SPDLOG_INFO("Reading Segment {}", segmentName);
  buffer->lastSegment = buffer->cursor;
  buffer->lastSegmentName = segmentName;
}

FileBuffer readFromFile(const char* filename, bool winCompat) {
  FileBuffer buffer = makeFileBuffer();
  buffer.capacity = 0;
  buffer.length = 0;
  buffer.cursor = 0;

  /*
  std::ifstream f(filename);
  f.seekg(0, std::ios::end);

  int64_t lSize = f.tellg();
  buffer.data = (char*)calloc(sizeof(char), lSize);

  if(buffer.data == NULL) {
    handleError("Memory Error opening %s (size %d)", filename, lSize);
    return buffer;
  }

  buffer.capacity = lSize;
  buffer.length = lSize;

  f.seekg(0);
  f.read(buffer.data, lSize);

  SPDLOG_INFO("readFromFile {} buffer.data {}", filename, buffer.data);

  return buffer;
  */

  // obtain file size:
  int64_t lSize;
  #if WIN32
    WIN32_FILE_ATTRIBUTE_DATA data;
    LARGE_INTEGER fileSize;
    // Convert const char* filename to wstring
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring fn = converter.from_bytes(std::string(filename));

    if(!GetFileAttributesExW(fn.c_str(), GetFileExInfoStandard, &data)) {
      SPDLOG_ERROR("Error getting file attributes : WIN32");
      return buffer;
    }

    fileSize.HighPart = data.nFileSizeHigh;
    fileSize.LowPart = data.nFileSizeLow;
    lSize = fileSize.QuadPart;

    if(lSize <= 0) {
      SPDLOG_ERROR("File lSize less than or eql to 0");
      return buffer;
    }

    // Prepare to read file into buffer
    HANDLE fileHandle = CreateFileW(fn.c_str(), GENERIC_READ, FILE_SHARE_READ, 
      NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if(fileHandle == INVALID_HANDLE_VALUE) {
      handleError("Error opening file handle : WIN32");
      return buffer;
    }

    // allocate memory to contain the whole file:
    buffer.data = (char*)calloc(sizeof(char), lSize);
    if(buffer.data == NULL) {
      handleError("Memory Error opening %s (size %d)", fn.c_str(), lSize);
      CloseHandle(fileHandle);
      return buffer;
    }

    buffer.capacity = lSize;
    buffer.length = lSize;
    DWORD bytesRead = 0;

    unsigned char* uCharBuf = new unsigned char[lSize];

    // lpNumberOfBytesRead parameter can be NULL only if lpOverlapped parameter is not NULL
    if(!(ReadFile(fileHandle, uCharBuf, lSize, &bytesRead, NULL))) {
      handleError("Error with ReadFile %s (bytesRead:%s)", fn.c_str(), std::to_string((unsigned long)bytesRead));
      CloseHandle(fileHandle);
      delete(uCharBuf);
      return buffer;
    }

    for (int i = 0; i < lSize; i++) {
      buffer.data[i] = (signed char)uCharBuf[i];
    }

    if ( winCompat ) {
      buffer.data[lSize-1] = 0;
      // buffer.data[lSize-2] = 0;
    }

    delete(uCharBuf);
    // Close file handle when we're done
    CloseHandle(fileHandle);

  #else
    struct stat st;
    int error = stat(filename, &st);
    lSize = st.st_size;

    if (error || lSize <= 0) {
      SPDLOG_WARN("Error opening file {}", filename);
      return buffer;
    }

    // allocate memory to contain the whole file:
    buffer.data = (char*)calloc(sizeof(char), lSize);
    if(buffer.data == NULL) {
      handleError("Memory Error opening %s (size %d)", filename, lSize);
      return buffer;
    }

    buffer.capacity = lSize;
    buffer.length = lSize;

    // copy the file into the buffer:
    FILE* in = fopen(filename, "r");
    int64_t num = fread(buffer.data, 1, lSize, in);
    fclose(in);

    if(num != lSize) {
      handleError(sprintf_o("Error reading file (num: %d)(lSize: %d)",
            num, lSize));
    }
  #endif  

  return buffer;
}

void writeToFile(FileBuffer* buf, FILE* out) {
  fwrite(buf->data, 1, buf->length, out);
}

FileBuffer* bufferFromString(const char* str) {
  FileBuffer* result = makeFileBufferPtr();
  int ln = strlength(str);
  result->length = ln;
  result->capacity = ln;
  result->cursor = 0;
  result->data = strdup_s(str);
  return result;
}

FileBuffer* base64ToBuffer(const char* base64) {
  FileBuffer* result = makeFileBufferPtr();
  int resLength = 0;
  result->data = (char*) unbase64(base64, strlength(base64), &resLength);
  result->length = resLength;
  result->capacity = resLength;
  result->cursor = 0;
  return result;
}

char* bufferToBase64(FileBuffer* buffer) {
  int resLength = 0;
  char* result = base64(buffer->data, buffer->length, &resLength);
  return result;
}

void fwrite_buffer(FileBuffer* dest, FileBuffer* src) {
  fwrite(src->data, src->length, 1, dest);
}

void fputs(const char* str, FileBuffer* out) {
  int len = strlength(str);
  fwrite(str, 1, len, out);
}

void fwrite_int32_t(FileBuffer* out, int32_t num) {
  fwrite(&num, sizeof(int32_t), 1, out);
}

int32_t fread_int32_t(FileBuffer* in) {
  int32_t result = 0;
  fread(&result, sizeof(int32_t), 1, in);
  return result;
}

void fwrite_int64_t(FileBuffer* out, int64_t num) {
  fwrite(&num, sizeof(int64_t), 1, out);
}

int64_t fread_int64_t(FileBuffer* in) {
  int64_t result = 0;
  fread(&result, sizeof(int64_t), 1, in);
  return result;
}

void fwrite_uint64_t(FileBuffer* out, uint64_t num) {
  fwrite(&num, sizeof(uint64_t), 1, out);
}

uint64_t fread_uint64_t(FileBuffer* in) {
  uint64_t result;
  fread(&result, sizeof(uint64_t), 1, in);
  return result;
}

void fwrite_uint32_t(FileBuffer* out, uint32_t num) {
  fwrite(&num, sizeof(uint32_t), 1, out);
}

uint32_t fread_uint32_t(FileBuffer* in) {
  uint32_t result;
  fread(&result, sizeof(uint32_t), 1, in);
  return result;
}

void fwrite_uint8_t(FileBuffer* out, uint8_t num) {
  fwrite(&num, sizeof(uint8_t), 1, out);
}

uint8_t fread_uint8_t(FileBuffer* in) {
  uint8_t result;
  fread(&result, sizeof(uint8_t), 1, in);
  return result;
}

void fwrite_char  (FileBuffer* out, char c) {
  fwrite(&c, sizeof(char), 1, out);
}

void fwrite_int(FileBuffer* out, int num) {
  fwrite(&num, sizeof(int), 1, out);
}

void fwrite_item(FileBuffer* out, item ndx) {
  fwrite(&ndx, sizeof(item), 1, out);
}

void fwrite_vec2 (FileBuffer* out, vec2 v) {
  fwrite(&v.x, sizeof(float), 1, out);
  fwrite(&v.y, sizeof(float), 1, out);
}

void fwrite_vec3 (FileBuffer* out, vec3 v) {
  fwrite(&v.x, sizeof(float), 1, out);
  fwrite(&v.y, sizeof(float), 1, out);
  fwrite(&v.z, sizeof(float), 1, out);
}

void fwrite_float(FileBuffer* out, float num) {
  fwrite(&num, sizeof(float), 1, out);
}

void fwrite_double(FileBuffer* out, double num) {
  fwrite(&num, sizeof(double), 1, out);
}

void fwrite_string(FileBuffer* out, const char* string) {
  if (string == 0) {
    short int length = 0;
    fwrite(&length, sizeof(short int), 1, out);

  } else {
    short int length = strlength(string);
    fwrite(&length, sizeof(short int), 1, out);
    fwrite(string, sizeof(char), length, out);
  }
}

void fwrite_string(FileBuffer* out, std::string string) {
  fwrite_string(out, string.c_str());
}

void fwrite_string_no_term(FileBuffer* out, const char* str) {
  fwrite(str, sizeof(char), strlength(str), out);
}

void fwrite_line(FileBuffer* in, Line line) {
  fwrite_vec3(in, line.start);
  fwrite_vec3(in, line.end);
}

char fread_char(FileBuffer* in) {
  char result;
  fread(&result, sizeof(char), 1, in);
  return result;
}

int fread_int(FileBuffer* in) {
  int result;
  fread(&result, sizeof(int), 1, in);
  return result;
}

item fread_item(FileBuffer* in) {
  item result;
  if (in->version < 16) {
    short temp;
    fread(&temp, sizeof(short), 1, in);
    result = temp;
  } else {
    fread(&result, sizeof(item), 1, in);
  }
  return result;
}

item fread_item(FileBuffer* in, int version) {
  return fread_item(in);
}

vec2 fread_vec2 (FileBuffer* in) {
  vec2 result;
  fread(&result.x, sizeof(float), 1, in);
  fread(&result.y, sizeof(float), 1, in);
  return result;
}

vec3 fread_vec3 (FileBuffer* in) {
  vec3 result;
  fread(&result.x, sizeof(float), 1, in);
  fread(&result.y, sizeof(float), 1, in);
  fread(&result.z, sizeof(float), 1, in);
  return result;
}

float fread_float(FileBuffer* in) {
  float result;
  fread(&result, sizeof(float), 1, in);
  return result;
}

double fread_double(FileBuffer* in) {
  double result;
  fread(&result, sizeof(double), 1, in);
  return result;
}

std::string fread_cpp_string(FileBuffer* in) {
  char* result = fread_string(in);
  std::string resultStr = result;
  free(result);
  return resultStr;
}

char* fread_string(FileBuffer* in) {
  short int length;
  fread(&length, sizeof(short int), 1, in);
  char* result = (char*) malloc(sizeof(char)*(length+1));
  fread(result, sizeof(char), length, in);
  result[length] = '\0';
  return result;
}

char* fread_string_until_whitespace(FileBuffer* in) {
  vector<char> nameBuf;
  for (;;) {
    char c = fread_char(in);
    if (isspace(c) || in->cursor >= in->length) break;
    nameBuf.push_back(c);
  }
  nameBuf.push_back('\0');
  return strdup_s(nameBuf.data());
}

char* fread_string_until(FileBuffer* in, char term) {
  vector<char> nameBuf;
  for (;;) {
    char c = fread_char(in);
    if (c == term || in->cursor >= in->length) break;
    nameBuf.push_back(c);
  }
  nameBuf.push_back('\0');
  return strdup_s(nameBuf.data());
}

void fwrite_location_vector(FileBuffer* in, vector<Location> *list) {
  fwrite_int(in, list->size());
  for (int i = 0; i < list->size(); i ++) {
    fwrite_uint32_t(in, list->at(i));
  }
}

void fread_location_vector(FileBuffer* in,
    vector<Location> *result, int version) {
  int num = fread_int(in);
  if (num > 1000000 || num < 0) {
    handleError("Error reading from file");
  }
  result->clear();
  result->reserve(num);
  for (int i = 0; i < num; i ++) {
    result->push_back(fread_uint32_t(in));
  }
}

void fwrite_item_vector(FileBuffer* in, vector<item> *list) {
  fwrite_int(in, list->size());
  for (int i = 0; i < list->size(); i ++) {
    fwrite_item(in, list->at(i));
  }
}

void fread_item_vector(FileBuffer* in, vector<item> *result, int version) {
  int num = fread_int(in);
  //if (num > 1000000 || num < 0) {
    //handleError("Error reading from file");
  //}
  result->clear();
  result->reserve(num);
  for (int i = 0; i < num; i ++) {
    result->push_back(fread_item(in, version));
  }
}

Line fread_line(FileBuffer* in) {
  Line result;
  result.start = fread_vec3(in);
  result.end = fread_vec3(in);
  return result;
}

void fwrite_graph_location(FileBuffer* in, GraphLocation loc) {
  fwrite_item(in, loc.lane);
  fwrite_float(in, loc.dap);
}

GraphLocation fread_graph_location(FileBuffer* in, int version) {
  GraphLocation result;
  result.lane = fread_item(in, version);
  result.dap = fread_float(in);
  return result;
}

void fwrite_location(FileBuffer* out, Location loc) {
  fwrite(&loc, sizeof(loc), 1, out);
}

Location fread_location(FileBuffer* in) {
  Location result;
  fread(&result, sizeof(Location), 1, in);
  return result;
}

