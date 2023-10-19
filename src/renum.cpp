#include "renum.hpp"

#include "string_proxy.hpp"

#include "spdlog/spdlog.h"

void renumClear(RenumTable* table) {
  int entrieS = table->entries.size();
  for (int i = 0; i < entrieS; i++) {
    free(table->entries[i].code);
  }
  table->entries.clear();
  table->byCode.clear();
  table->fromFileValue.clear();
}

void pushRenum(RenumTable* table, item val, const char* code) {
  if (val < 0) return;
  int entrieS = table->entries.size();
  if (entrieS <= val) table->entries.resize(val+1);
  Renum* entry = &table->entries[val];
  entry->value = val;
  entry->fileValue = 0;
  if (entry->code != 0) free(entry->code);
  entry->code = strdup_s(code);
  table->byCode[code] = val;
}

void pushRenum(RenumTable* table, const char* code) {
  pushRenum(table, table->entries.size(), code);
}

item renum(RenumTable* table, item fromFile) {
  if (fromFile < 0 || fromFile >= table->fromFileValue.size()) return 0;
  return table->fromFileValue[fromFile];
}

void renumLegacyStart(RenumTable* table) {
  table->fromFileValue.clear();
}

void renumLegacy(RenumTable* table, const char* code) {
  item val = table->byCode[code];
  table->fromFileValue.push_back(val);
}

void renumRead(RenumTable* table, FileBuffer* file) {
  table->fromFileValue.clear();
  int entrieS = fread_item(file, file->version);
  for (int i = 0; i < entrieS; i++) {
    char* code = fread_string(file);
    item val = table->byCode[code];
    //if (val == 0) SPDLOG_WARN("Missing: {}", code);
    table->fromFileValue.push_back(val);
    free(code);
  }
}

void renumWrite(RenumTable* table, FileBuffer* file) {
  int entrieS = table->entries.size();
  fwrite_item(file, entrieS);
  for (int i = 0; i < entrieS; i++) {
    fwrite_string(file, table->entries[i].code);
  }
}

