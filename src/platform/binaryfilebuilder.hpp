//-----------------------------------------------------------------------------
// binaryfilebuilder.hpp - A static class for assembling binary
// data files as groups of sectors with informative headers
//-----------------------------------------------------------------------------

#pragma once

#include "binaryfileshared.hpp"


// BinaryFileBuilder: Used for assembling binary files
// with a consistent header/sector approach.
class BinaryFileBuilder {
private:
  BinaryFileBuilder() {}; // Don't want to instantiate this class
public:
  static void setVerbose(bool v);
  static bool getVerbose();
  static void reset();
  static bool startWrite(std::string filename);
  static bool finishWrite();
  static bool writePadding();

  // Specific write methods
  // Should be one-to-one with FileDataType
  static bool writeInteger(int32_t i);
  static bool writeLong(int64_t l);
  static bool writeFloat(float f);
  static bool writeSByte(sbyte s);
  static bool writeUByte(ubyte b);
  static bool writeChar(char c);
  static bool writeString(std::string s);
  static bool writeKeyBind(KeyBind k);

  // General abstracted write methods
  static bool writeHeader(FileSectorHeader* header);
  static bool writeSectorKeyBind(FileSector<KeyBind>* sector);
};