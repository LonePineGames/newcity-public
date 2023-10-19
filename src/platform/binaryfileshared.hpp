//-----------------------------------------------------------------------------
// binaryfileshared.hpp - Shared data structures and 
// typedefs for reading and writing binary files
//-----------------------------------------------------------------------------

#pragma once

#include "file.hpp"
#include "spdlog/spdlog.h"
#include "../input.hpp"
#include "../serialize.hpp"
#include "../game/version.hpp"


typedef int8_t sbyte;
typedef uint8_t ubyte;

const uint32_t fileSectorPadding = UINT32_MAX;      // 0xffffffff; used for holding a space or padding sectors
const ubyte fileSectorHeaderStart = UINT8_MAX;      // 0xff, or 11111111
const ubyte fileSectorHeaderSize = 32;              // Bytes; must be less than the max of a signed byte
const ubyte fileSectorMaxElements = INT8_MAX+1;     // Number of elements a sector can hold
const ubyte fileSectorHeaderMaster = 1;
const ubyte fileSectorHeaderSector = 0;

// Definitions for the different bytes in a sector header
enum HeaderByte : ubyte {
  Start = 0,
  HeaderSize,
  SectorType,
  DataType,
  SectorSize,   // Number of data elements stored in sector
  VersionMajor,
  VersionMinor,
  VersionPatch,

  FreeByteA = 28,
  FreeByteB = 29,
  FreeByteC = 30,
  FreeByteD = 31,
  MaxHeaderBytes = fileSectorHeaderSize
};

// Data types that can be stored for unique handling when reading/writing
enum FileDataType : ubyte {
  DataUnspecified = 0,
  DataInteger,
  DataLong,
  DataFloat,
  DataSByte,
  DataUByte,
  DataChar,
  DataString,
  DataKeyBind,

  NumDataTypes
};

// Get file data type size in bytes
namespace BinaryFile {
  static size_t getFileDataTypeSize(FileDataType type) {
    switch (type) {
      case FileDataType::DataInteger:
        return sizeof(int32_t);
      case FileDataType::DataLong:
        return sizeof(int64_t);
      case FileDataType::DataFloat:
        return sizeof(float);
      case FileDataType::DataSByte:
        return sizeof(sbyte);
      case FileDataType::DataUByte:
        return sizeof(ubyte);
      case FileDataType::DataChar:
        return sizeof(int8_t);
      case FileDataType::DataKeyBind:
        return sizeof(KeyBind);
      default:
        return -1;
    }
  }
}

// Definition for a sector header
// Stores key info like data type, number of elements, and version
// specific info for handling.
struct FileSectorHeader {
private:
  std::unique_ptr<ubyte[]> _header; // Ptr for header data array
  int32_t _size = -1;  // Size of header; should always be fileSectorHeaderSize after initialization

public:
  // Properties
  int32_t size() { return _size; } // Size of header; should always be fileSectorHeaderSize after initialization
  bool valid() { return _header.get() != nullptr && _size >= 0; }
  bool master() { return valid() ? _header[HeaderByte::SectorType] : false; }
  int32_t dataType() { return valid() ? (int32_t)_header[HeaderByte::DataType] : -1; }

  // Member functions
  ubyte* get() { return _header.get(); }

  int32_t get(int32_t ndx) {
    if (_header.get() == nullptr) {
      return -1;
    }

    if (ndx < 0 || ndx >= _size) {
      return -1;
    }

    return (int32_t)_header[ndx];
  }

  void set(bool master, FileDataType type, ubyte size, ubyte major, ubyte minor, ubyte patch) {
    _header = std::make_unique<ubyte[]>(fileSectorHeaderSize);
    _size = fileSectorHeaderSize; // Size of header, not size of sector; stored here in data struct
    _header[HeaderByte::Start] = fileSectorHeaderStart; // First byte of header is delimiter
    _header[HeaderByte::HeaderSize] = fileSectorHeaderSize; // Second byte is size; stored here in the header for verifying size between versions
    _header[HeaderByte::SectorType] = (ubyte)master;
    _header[HeaderByte::DataType] = type;
    _header[HeaderByte::SectorSize] = size <= fileSectorMaxElements ? size : fileSectorMaxElements;
    _header[HeaderByte::VersionMajor] = major;
    _header[HeaderByte::VersionMinor] = minor;
    _header[HeaderByte::VersionPatch] = patch;
  }

  FileSectorHeader() {
    set(false, FileDataType::DataUnspecified, 0, (ubyte)majorVersion, (ubyte)minorVersion, (ubyte)patchVersion);
  }

  FileSectorHeader(bool master, FileDataType type, ubyte size) {
    set(master, type, size, (ubyte)majorVersion, (ubyte)minorVersion, (ubyte)patchVersion);
  }

  FileSectorHeader(bool master, FileDataType type, ubyte size, ubyte major, ubyte minor, ubyte patch) {
    set(master, type, size, major, minor, patch);
  }

  void clear() {
    _header.reset(nullptr);
    _size = -1;
  }

  // Make your choice, adventerous stranger
  // Index in and bide the danger...
  ubyte& operator[](int32_t ndx) {
    return _header[ndx];
  }

  // Make your choice, adventerous stranger
  // Index in and bide the danger...
  const ubyte& operator[](int32_t ndx) const {
    return _header[ndx];
  }
};

// Definition for a data sector in a file
// Sectors can be of variable size in terms of bytes stored, but have a defined 
// maximum number of elements, with an element being one instance of a particular 
// data type (e.g. a keybind). Each sector contains only one type of data.
template <typename T>
struct FileSector {
private:
  FileSectorHeader _header;
  std::unique_ptr<T[]> _data; // Ptr for sector data array
  int32_t _count = -1; // Count for number of elements in data array; should be non-negative after initialization

public:
  // Properties
  int32_t count() { return _count; }
  int32_t size() { return _header.valid() ? (int32_t)_header[HeaderByte::SectorSize] : -1; }
  bool valid() { return _header.get() != nullptr && _header.size() > 0 && _data.get() != nullptr && _count >= 0; }
  bool master() { return _header.master(); }
  FileSectorHeader* getHeader() { return &_header; }

  // Member functions
  void initData(ubyte size) {
    if (_data.get() != nullptr) {
      // Already initialized
      return;
    }

    if (size > fileSectorMaxElements) {
      size = fileSectorMaxElements;
    }

    _data = std::make_unique<T[]>(size);
    _count = 0;
  }

  // Change size of sector data array; will modify header SectorSize byte accordingly
  bool resize(ubyte size) {

    // This should never happen because it's a ubyte arg, but eh....
    if (size < 0) {
      SPDLOG_ERROR("Tried to resize file sector to negative value");
      return false;
    }

    if (size > fileSectorMaxElements) {
      size = fileSectorMaxElements;
    }

    if (size < _count) {
      SPDLOG_WARN("Resizing file sector to be smaller than previous count ({}->{}); data may be lost...", _count, size);
    }

    // Write new size to header if applicable
    _header[(int)HeaderByte::SectorSize] = size;

    // Check that data already exists
    if (_data.get() != nullptr) {
      std::unique_ptr<T[]> newData = std::make_unique<T[]>(size);

      // Use sector data count for copy loop iterations
      for (int i = 0; i < _count; i++) {
        newData[i] = _data[i];
      }

      _data.swap(newData);
      newData.reset(nullptr);
    }

    // Count will be greater than size if we lost data
    if (_count > size) {
      _count = size;
    }

    return true;
  }

  // Synchronize the size of the sector's data array with the header's sector size value
  bool syncWithHeader() {
    // Resize sector data array to size indicated by header
    // sector.size() returns the HeaderByte::SectorSize value in header if the header is valid
    return this->resize(this->size());
  }

  // Push an element into the sector
  bool push(T element) {
    if (_data.get() == nullptr) {
      SPDLOG_ERROR("Push to file sector failed: nullptr");
      return false;
    }

    if (_count >= size() || _count < 0) {
      SPDLOG_ERROR("Push to file sector failed: count {}, size {}", _count, size());
      return false;
    }

    _data[_count] = element;
    _count++;
    return true;
  }

  // Pop an element out of the sector
  T* pop() {
    if (_data.get() == nullptr) {
      return NULL;
    }

    if (size() <= 0 || _count <= 0) {
      return NULL;
    }

    _count--; // Decrement before indexing
    T* element = &_data[_count];

    return element;
  }

  void clear(bool all) {
    _data.reset(nullptr);
    _count = -1;

    if (all) {
      _header.clear();
    }
  }

  FileSector() {
    _header.set(false, FileDataType::DataUnspecified, 0, (ubyte)majorVersion, (ubyte)minorVersion, (ubyte)patchVersion);
    initData(0);
  }

  FileSector(bool master) {
    _header.set(master, FileDataType::DataUnspecified, 0, (ubyte)majorVersion, (ubyte)minorVersion, (ubyte)patchVersion);
    initData(0);
  }

  FileSector(bool master, FileDataType type, ubyte size) {
    _header.set(master, type, size, (ubyte)majorVersion, (ubyte)minorVersion, (ubyte)patchVersion);
    initData(size);
  }

  FileSector(bool master, FileDataType type, ubyte size, ubyte major, ubyte minor, ubyte patch) {
    _header.set(master, type, size, major, minor, patch);
    initData(size);
  }

  // Make your choice, adventerous stranger
  // Index in and bide the danger...
  T* operator[](int32_t ndx) {
    if (_data.get() == nullptr || ndx >= size()) {
      return NULL;
    }

    return &_data[ndx];
  }

  // Make your choice, adventerous stranger
  // Index in and bide the danger...
  const T* operator[](int32_t ndx) const {
    if (_data.get() == nullptr || ndx >= size()) {
      return NULL;
    }

    return &_data[ndx];
  }

  ~FileSector() {
    clear(true);
  }
};