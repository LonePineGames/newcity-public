//-----------------------------------------------------------------------------
// binaryfilereader.cpp - A static class for reading binary
// data files as groups of sectors with informative headers
//-----------------------------------------------------------------------------

#include "binaryfilereader.hpp"


// BinaryFileReader definitions
static FileBuffer bfr_fileBuffer;
static bool bfr_activeFile;
static bool bfr_verbose = false;
static std::string bfr_activeFileName;
static int32_t bfr_pos; // Position; index

void BinaryFileReader::setVerbose(bool v) {
  bfr_verbose = v;
}

bool BinaryFileReader::getVerbose() {
  return bfr_verbose;
}

void BinaryFileReader::reset() {
  if (bfr_fileBuffer.data != NULL) {
    freeBuffer(&bfr_fileBuffer);
  }
  bfr_fileBuffer = makeFileBuffer();

  bfr_activeFile = false;
  bfr_activeFileName = "";
  bfr_pos = 0;
}

// Loads a file into the static FileBuffer
bool BinaryFileReader::startRead(std::string filename) {
  if (filename.length() == 0) {
    SPDLOG_ERROR("BinaryFileReader: Null length filename, can't read");
    return false;
  }

  if (bfr_activeFile) {
    SPDLOG_WARN("BinaryFileReader: Tried to read file {} into BinaryFileReader while another file ({}) was active", filename, bfr_activeFileName);
    return false;
  }

  bfr_fileBuffer = readFromFile(filename.c_str());

  if (bfr_fileBuffer.length > 0) {
    SPDLOG_INFO("BinaryFileReader: Loaded file {} into buffer", filename);
    bfr_activeFile = true;
    bfr_activeFileName = filename;
    bfr_pos = 0;
    return true;
  }

  SPDLOG_ERROR("BinaryFileReader: Unable to read file {}", filename);
  return false;
}

bool BinaryFileReader::finishRead() {
  if (!bfr_activeFile) {
    if (bfr_verbose) {
      SPDLOG_WARN("BinaryFileReader: Called finish with no active file");
    }
    return false;
  }

  if (bfr_verbose) {
    SPDLOG_INFO("BinaryFileReader: Unloading {} from reader and resetting...", bfr_activeFileName);
  }
  BinaryFileReader::reset();
  return true;
}

bool BinaryFileReader::readPadding() {
  int32_t chkNum = 0;
  ubyte chkByte = 0;

  if (bfr_verbose) {
    SPDLOG_INFO("BinaryFileReader: Traversing padding...");
  }
  // Look for four consecutive 0xff bytes (0xffffffff); that's the padding
  while (bfr_pos < bfr_fileBuffer.length && chkNum < 4) {
    chkByte = fread_uint8_t(&bfr_fileBuffer);
    if (chkByte == fileSectorHeaderStart) {
      chkNum++;
    } else {
      chkNum = 0;
    }
    bfr_pos++;
  }

  if (chkNum < 4) {
    SPDLOG_ERROR("BinaryFileReader: Couldn't find reasonable end of padding");
    return false;
  }

  if (bfr_verbose) {
    SPDLOG_INFO("BinaryFileReader: Found end of padding at position {}", bfr_pos);
  }
  return true;
}

bool BinaryFileReader::readInteger(FileSector<int32_t> &sector) {
  if (!bfr_activeFile) {
    SPDLOG_ERROR("BinaryFileReader: No active file");
    return false;
  }

  if (bfr_fileBuffer.length <= 0) {
    SPDLOG_ERROR("BinaryFileReader: File buffer is invalid for file {}; try resetting and reading again", bfr_activeFileName);
    return false;
  }

  int32_t i = fread_int32_t(&bfr_fileBuffer);

  return sector.push(i);
}

bool BinaryFileReader::readLong(FileSector<int64_t> &sector) {
  if (!bfr_activeFile) {
    SPDLOG_ERROR("BinaryFileReader: No active file");
    return false;
  }

  if (bfr_fileBuffer.length <= 0) {
    SPDLOG_ERROR("BinaryFileReader: File buffer is invalid for file {}; try resetting and reading again", bfr_activeFileName);
    return false;
  }

  int64_t l = fread_int64_t(&bfr_fileBuffer);

  return sector.push(l);
}

bool BinaryFileReader::readFloat(FileSector<float> &sector) {
  if (!bfr_activeFile) {
    SPDLOG_ERROR("BinaryFileReader: No active file");
    return false;
  }

  if (bfr_fileBuffer.length <= 0) {
    SPDLOG_ERROR("BinaryFileReader: File buffer is invalid for file {}; try resetting and reading again", bfr_activeFileName);
    return false;
  }

  float f = fread_float(&bfr_fileBuffer);

  return sector.push(f);
}

bool BinaryFileReader::readSByte(FileSector<sbyte> &sector) {
  if (!bfr_activeFile) {
    SPDLOG_ERROR("BinaryFileReader: No active file");
    return false;
  }

  if (bfr_fileBuffer.length <= 0) {
    SPDLOG_ERROR("BinaryFileReader: File buffer is invalid for file {}; try resetting and reading again", bfr_activeFileName);
    return false;
  }

  sbyte byte = fread_char(&bfr_fileBuffer);

  return sector.push(byte);
}

bool BinaryFileReader::readUByte(FileSector<ubyte> &sector) {
  if (!bfr_activeFile) {
    SPDLOG_ERROR("BinaryFileReader: No active file");
    return false;
  }

  if (bfr_fileBuffer.length <= 0) {
    SPDLOG_ERROR("BinaryFileReader: File buffer is invalid for file {}; try resetting and reading again", bfr_activeFileName);
    return false;
  }

  ubyte byte = fread_uint8_t(&bfr_fileBuffer);

  return sector.push(byte);
}

bool BinaryFileReader::readChar(FileSector<char> &sector) {
  if (!bfr_activeFile) {
    SPDLOG_ERROR("BinaryFileReader: No active file");
    return false;
  }

  if (bfr_fileBuffer.length <= 0) {
    SPDLOG_ERROR("BinaryFileReader: File buffer is invalid for file {}; try resetting and reading again", bfr_activeFileName);
    return false;
  }

  char c = fread_char(&bfr_fileBuffer);

  return sector.push(c);
}

bool BinaryFileReader::readString(FileSector<std::string> &sector) {
  if (!bfr_activeFile) {
    SPDLOG_ERROR("BinaryFileReader: No active file");
    return false;
  }

  if (bfr_fileBuffer.length <= 0) {
    SPDLOG_ERROR("BinaryFileReader: File buffer is invalid for file {}; try resetting and reading again", bfr_activeFileName);
    return false;
  }

  std::string s = fread_string(&bfr_fileBuffer);

  return sector.push(s);
}

bool BinaryFileReader::readKeyBind(FileSector<KeyBind> &sector) {
  if (!bfr_activeFile) {
    SPDLOG_ERROR("BinaryFileReader: No active file");
    return false;
  }

  if (bfr_fileBuffer.length <= 0) {
    SPDLOG_ERROR("BinaryFileReader: File buffer is invalid for file {}; try resetting and reading again", bfr_activeFileName);
    return false;
  }

  uint32_t action = fread_uint32_t(&bfr_fileBuffer);
  int32_t key = fread_int32_t(&bfr_fileBuffer);

  return sector.push(KeyBind((InputAction)action, key));
}

bool BinaryFileReader::readHeader(FileSectorHeader *header) {
  if (!bfr_activeFile) {
    SPDLOG_ERROR("BinaryFileReader: No active file");
    return false;
  }

  if (bfr_fileBuffer.length <= 0) {
    SPDLOG_ERROR("BinaryFileReader: File buffer is invalid for file {}; try resetting and reading again", bfr_activeFileName);
    return false;
  }

  BinaryFileReader::readPadding();

  if (bfr_pos >= bfr_fileBuffer.length) {
    SPDLOG_ERROR("BinaryFileReader: Could not find header start byte before EOF");
    return false;
  }

  // The first 0xff byte after the padding begins the header
  ubyte startByte = fread_uint8_t(&bfr_fileBuffer);
  bfr_pos++;

  if (startByte != fileSectorHeaderStart) {
    SPDLOG_ERROR("BinaryFileReader: Did not find header start byte after padding");
    return false;
  }

  if (bfr_verbose) {
    SPDLOG_INFO("BinaryFileReader: Found header start byte at index {}", bfr_pos);
  }

  int32_t headerSize = (int32_t)fread_uint8_t(&bfr_fileBuffer);
  if (bfr_verbose) {
    SPDLOG_INFO("BinaryFileReader: Reading header from file {} with length {}", bfr_activeFileName, headerSize);
  }
  bfr_pos++;

  std::unique_ptr<ubyte[]> headerData = std::make_unique<ubyte[]>(headerSize);
  int h = 2; // Read first two bytes already
  headerData[(int)HeaderByte::Start] = fileSectorHeaderStart;
  headerData[(int)HeaderByte::HeaderSize] = headerSize;
  for (h; h < headerSize; h++) {
    headerData[h] = fread_uint8_t(&bfr_fileBuffer);
    bfr_pos++;
  }

  header->set(headerData[(int)HeaderByte::SectorType], (FileDataType)headerData[(int)HeaderByte::DataType],
    headerData[(int)HeaderByte::SectorSize], headerData[(int)HeaderByte::VersionMajor],
    headerData[(int)HeaderByte::VersionMinor], headerData[(int)HeaderByte::VersionPatch]);

  // Retrieve free bytes
  header->operator[]((int)HeaderByte::FreeByteA) = headerData[(int)HeaderByte::FreeByteA];
  header->operator[]((int)HeaderByte::FreeByteB) = headerData[(int)HeaderByte::FreeByteB];
  header->operator[]((int)HeaderByte::FreeByteC) = headerData[(int)HeaderByte::FreeByteC];
  header->operator[]((int)HeaderByte::FreeByteD) = headerData[(int)HeaderByte::FreeByteD];

  // Check that the headers match
  for (int c = 0; c < header->size(); c++) {
    if (header->get(c) != headerData[c]) {
      SPDLOG_WARN("BinaryFileReader: Header mismatch between buffer and data structure for file {} at byte index {}:\nget() - {}, [] - {}, readSize - {}, header->size() - {}",
        bfr_activeFileName, c, header->get(c), headerData[c], headerSize, header->size());
    }
  }

  if (bfr_verbose) {
    std::string readData = "";
    for (int d = 0; d < headerSize; d++) {
      readData += std::to_string(headerData[d]);
      if (d < headerSize-1) {
        readData += ",";
      }
    }
    SPDLOG_INFO("BinaryFileReader: Successfully read {} bytes from header of size {}: \n{}", h, headerSize, readData);
  }

  return true;
}

// Reads a KeyBind file sector; will return sector data by reference and boolean success
bool BinaryFileReader::readSectorKeyBind(FileSector<KeyBind> &sector) {
  if (!bfr_activeFile) {
    SPDLOG_ERROR("BinaryFileReader: No active file");
    return false;
  }

  if (bfr_fileBuffer.length <= 0) {
    SPDLOG_ERROR("BinaryFileReader: File buffer is invalid for file {}; try resetting and reading again", bfr_activeFileName);
    return false;
  }

  if (bfr_verbose) {
    SPDLOG_INFO("BinaryFileReader: Reading in file buffer for file {} of length {}", bfr_activeFileName, bfr_fileBuffer.length);
  }

  // Read the header data into the sector passed by reference
  if (!BinaryFileReader::readHeader(sector.getHeader())) {
    SPDLOG_ERROR("BinaryFileReader: Error reading header for file {}", bfr_activeFileName);
    return false;
  }

  // Sync the size of the sector's data array to match what size the header says it should be
  if (!sector.syncWithHeader()) {
    SPDLOG_ERROR("BinaryFileReader: Error syncing size of sector data array with header indicated size");
    return false;
  }

  // If the sector.size is negative or 0 after syncing
  if (sector.size() <= 0) {
    SPDLOG_ERROR("BinaryFileReader: Invalid number of elements in sector ({}); can't read sector", sector.size());
    return false;
  }

  // Read padding after header
  if (!BinaryFileReader::readPadding()) {
    SPDLOG_ERROR("BinaryFileReader: Error traversing padding between header and data");
    return false;
  }

  // Read in the data elements from the sector
  int32_t numElements = sector.size();
  int32_t dataType = sector.getHeader()->dataType();
  int e = 0;
  int err = 0;
  std::string bindStr = "";
  if (bfr_verbose) {
    SPDLOG_INFO("BinaryFileReader: Preparing to read {} elements of data type {}", numElements, dataType);
  }

  for (e; e < numElements; e++) {
    if (!BinaryFileReader::readKeyBind(sector)) {
      SPDLOG_ERROR("BinaryFileReader: Error pushing data to sector at index {}", e);
      continue;
    }

    if (bfr_verbose) {
      KeyBind* chk = sector[e]; // Observer pointer to data in the sector
      if (chk == NULL) {
        SPDLOG_ERROR("BinaryFileReader: Data in sector at index {} is null", e);
        err++;
        continue;
      }
      SPDLOG_INFO("BinaryFileReader: Read out keybind as {} and {}", chk->action, chk->key);
      bindStr += "(" + std::to_string(chk->action) + "," + std::to_string(chk->key) + ")";
      if (e < numElements-1) {
        bindStr += ",\n";
      }
    }
  }

  if (err > e/2) {
    SPDLOG_WARN("BinaryFileReader: Significant errors when reading elements from sector: ({}-{}), {} successful, \n{}", e, err, e-err, bindStr);
  }

  if (bfr_verbose) {
    SPDLOG_INFO("BinaryFileReader: Successfully read {} elements ({}-{}) from sector: \n{}", e-err, e, err, bindStr);
  }

  BinaryFileReader::reset();
  return true;
}
//End BinaryFileReader definitions