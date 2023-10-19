//-----------------------------------------------------------------------------
// binaryfilereader.hpp - A static class for reading binary
// data files as groups of sectors with informative headers
//-----------------------------------------------------------------------------

#pragma once

#include "binaryfileshared.hpp"


// BinaryFileReader: Used for reading binary files
// with a consistent header/sector approach.
class BinaryFileReader {
private:
  BinaryFileReader() {}; // Don't want to instantiate this class
public:
  static void setVerbose(bool v);
  static bool getVerbose();
  static void reset();
  static bool startRead(std::string filename);
  static bool finishRead();
  static bool readPadding();

  // Specific read methods
  // Should be one-to-one with FileDataType
  static bool readInteger(FileSector<int32_t> &sector);
  static bool readLong(FileSector<int64_t> &sector);
  static bool readFloat(FileSector<float> &sector);
  static bool readSByte(FileSector<sbyte> &sector);
  static bool readUByte(FileSector<ubyte> &sector);
  static bool readChar(FileSector<char> &sector);
  static bool readString(FileSector<std::string> &sector);
  static bool readKeyBind(FileSector<KeyBind> &sector);

  // General abstracted read methods
  static bool readHeader(FileSectorHeader *header);
  static bool readSectorKeyBind(FileSector<KeyBind> &sector);
};