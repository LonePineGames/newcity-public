//-----------------------------------------------------------------------------
// binaryfilebuilder.cpp - A static class for assembling binary
// data files as groups of sectors with informative headers
//-----------------------------------------------------------------------------

#include "binaryfilebuilder.hpp"


// BinaryFileBuilder definitions
static FILE* bfb_fileHandle;
static FileBuffer* bfb_fileBuffer;
static bool bfb_activeFile;
static bool bfb_verbose = false;
static std::string bfb_activeFileName;

void BinaryFileBuilder::setVerbose(bool v) {
  bfb_verbose = v;
}

bool BinaryFileBuilder::getVerbose() {
  return bfb_verbose;
}

void BinaryFileBuilder::reset() {
  // Reset state
  if (bfb_fileHandle != NULL) {
    fclose(bfb_fileHandle);
  }
  bfb_fileHandle = NULL;

  if (bfb_fileBuffer != NULL) {
    freeBuffer(bfb_fileBuffer);
  }
  bfb_fileBuffer = NULL;

  bfb_activeFile = false;
  bfb_activeFileName = "";
}

bool BinaryFileBuilder::startWrite(std::string filename) {
  if (filename.length() == 0) {
    SPDLOG_ERROR("BinaryFileBuilder: Null length filename in start");
    return false;
  }

  // Reset state
  BinaryFileBuilder::reset();

  if (bfb_fileHandle = fopen(filename.c_str(), "wb")) {
    bfb_fileBuffer = makeFileBufferPtr();
    bfb_activeFile = true;
    bfb_activeFileName = filename;
    return true;
  }

  // Something went wrong
  SPDLOG_ERROR("BinaryFileBuilder: Error writing to file {}", filename);
  BinaryFileBuilder::reset();
  return false;
}

bool BinaryFileBuilder::finishWrite() {
  if (!bfb_activeFile) {
    if (bfb_verbose) {
      SPDLOG_WARN("BinaryFileBuilder: Called finish with no active file");
    }
    BinaryFileBuilder::reset(); // Let's reset anyways...
    return false;
  }

  if (!bfb_fileHandle) {
    SPDLOG_ERROR("BinaryFileBuilder: Called finish with null file handle");
    return false;
  }

  if (!bfb_fileBuffer) {
    SPDLOG_ERROR("BinaryFileBuilder: Called finish with null file buffer");
    return false;
  }

  writeToFile(bfb_fileBuffer, bfb_fileHandle);
  if (bfb_verbose) {
    SPDLOG_INFO("BinaryFileBuilder: Wrote buffer to {}, cleaning up...", bfb_activeFileName);
  }
  BinaryFileBuilder::reset();
  return true;
}

bool BinaryFileBuilder::writePadding() {
  if (!bfb_activeFile) {
    SPDLOG_ERROR("BinaryFileBuilder: Tried to write to file when no file was active");
    return false;
  }

  fwrite_uint32_t(bfb_fileBuffer, fileSectorPadding);
  return true;
}

bool BinaryFileBuilder::writeInteger(int32_t i) {
  if (!bfb_activeFile) {
    SPDLOG_ERROR("BinaryFileBuilder: Tried to write to file when no file was active");
    return false;
  }

  fwrite_int32_t(bfb_fileBuffer, i);
  return true;
}

bool BinaryFileBuilder::writeLong(int64_t l) {
  if (!bfb_activeFile) {
    SPDLOG_ERROR("BinaryFileBuilder: Tried to write to file when no file was active");
    return false;
  }

  fwrite_int64_t(bfb_fileBuffer, l);
  return true;
}

bool BinaryFileBuilder::writeFloat(float f) {
  if (!bfb_activeFile) {
    SPDLOG_ERROR("BinaryFileBuilder: Tried to write to file when no file was active");
    return false;
  }

  fwrite_float(bfb_fileBuffer, f);
  return true;
}

bool BinaryFileBuilder::writeSByte(sbyte s) {
  if (!bfb_activeFile) {
    SPDLOG_ERROR("BinaryFileBuilder: Tried to write to file when no file was active");
    return false;
  }

  fwrite_char(bfb_fileBuffer, s);
  return true;
}

bool BinaryFileBuilder::writeUByte(ubyte b) {
  if (!bfb_activeFile) {
    SPDLOG_ERROR("BinaryFileBuilder: Tried to write to file when no file was active");
    return false;
  }

  fwrite_uint8_t(bfb_fileBuffer, b);
  return true;
}

bool BinaryFileBuilder::writeChar(char c) {
  if (!bfb_activeFile) {
    SPDLOG_ERROR("BinaryFileBuilder: Tried to write to file when no file was active");
    return false;
  }

  fwrite_char(bfb_fileBuffer, c);
  return true;
}

bool BinaryFileBuilder::writeString(std::string s) {
  if (!bfb_activeFile) {
    SPDLOG_ERROR("BinaryFileBuilder: Tried to write to file when no file was active");
    return false;
  }

  fwrite_string(bfb_fileBuffer, s.c_str());
  return true;
}

bool BinaryFileBuilder::writeKeyBind(KeyBind k) {
  if (!bfb_activeFile) {
    SPDLOG_ERROR("BinaryFileBuilder: Tried to write to file when no file was active");
    return false;
  }

  fwrite_uint32_t(bfb_fileBuffer, k.action);
  fwrite_int32_t(bfb_fileBuffer, k.key);
  return true;
}

bool BinaryFileBuilder::writeHeader(FileSectorHeader* header) {
  if (!bfb_activeFile) {
    SPDLOG_ERROR("BinaryFileBuilder: Tried to write to file when no file was active");
    return false;
  }

  if (header == NULL) {
    SPDLOG_ERROR("BinaryFileBuilder: Null header ptr");
    return false;
  }

  // Write padding at beginning of header
  BinaryFileBuilder::writePadding();

  int i = 0;
  std::string headerStr = "";
  for (i; i < header->size(); i++) {
    int32_t b = header->get(i);
    if (b < 0) {
      SPDLOG_ERROR("BinaryFileBuilder: Fetched invalid byte from header struct at index {}", i);
      continue;
    }
    ubyte toWrite = header->get()[i];
    fwrite_uint8_t(bfb_fileBuffer, toWrite);
    headerStr += std::to_string(toWrite);
    if (i < header->size()-1) {
      headerStr += ",";
    }
  }

  if (bfb_verbose) {
    SPDLOG_INFO("BinaryFileBuilder: Successfully wrote header of {} bytes to file buffer: \n{}", i, headerStr);
  }
  return true;
}

bool BinaryFileBuilder::writeSectorKeyBind(FileSector<KeyBind>* sector) {
  if (!bfb_activeFile) {
    SPDLOG_ERROR("BinaryFileBuilder: Tried to write sector when no file was active");
    return false;
  }

  if (sector == NULL) {
    SPDLOG_ERROR("BinaryFileBuilder: Null sector ptr");
    return false;
  }

  if (!sector->valid()) {
    SPDLOG_ERROR("BinaryFileBuilder: Sector is invalid");
    return false;
  }

  // Try to write the header
  if (!BinaryFileBuilder::writeHeader(sector->getHeader())) {
    SPDLOG_ERROR("BinaryFileBuilder: Error writing sector header");
    return false;
  }

  // Write padding between header and data
  if (!BinaryFileBuilder::writePadding()) {
    SPDLOG_ERROR("BinaryFileBuilder: Error writing padding between header and data");
    return false;
  }

  int i = 0;
  int err = 0;
  std::string bindStr = "";
  for (i; i < sector->size(); i++) {
    KeyBind* bind = sector->pop();
    if (bind == NULL) {
      if (bfb_verbose) {
        SPDLOG_WARN("BinaryFileBuilder: Popped null data from sector");
        bindStr += "(NULL,NULL)";
        if (i < sector->size()-1) {
          bindStr += ",\n";
        }
      }
      err++;
      continue;
    }
    if (!BinaryFileBuilder::writeKeyBind(*bind)) {
      SPDLOG_ERROR("BinaryFileBuilder: Error writing data at index {}", i);
      continue;
    }
    if (bfb_verbose) {
      bindStr += "(" + std::to_string(bind->action) + "," + std::to_string(bind->key) + ")";
      if (i < sector->size()-1) {
        bindStr += ",\n";
      }
    }
  }

  if (err > i/2) {
    SPDLOG_WARN("BinaryFileBuilder: Significant errors when writing elements to sector: ({}-{}), {} successful, \n{}", i, err, i-err, bindStr);
  }

  if (bfb_verbose) {
    SPDLOG_INFO("BinaryFileBuilder: Successfully wrote {} elements ({}-{}) into file buffer for sector: \n{}", i-err, i, err, bindStr);
  }

  return true;
}
// End BinaryFileBuilder definitions