#pragma once

#include "serialize.hpp"
#include <unordered_map>
#include <string>

struct Renum {
  item fileValue = 0;
  item value = 0;
  char* code = 0;
};

struct RenumTable {
  vector<Renum> entries;
  unordered_map<string, item> byCode;
  vector<item> fromFileValue;
};

void pushRenum(RenumTable* table, item val, const char* code);
void pushRenum(RenumTable* table, const char* code);
item renum(RenumTable* table, item fromFile);
void renumClear(RenumTable* table);
void renumLegacyStart(RenumTable* table);
void renumLegacy(RenumTable* table, const char* code);
void renumRead(RenumTable* table, FileBuffer* file);
void renumWrite(RenumTable* table, FileBuffer* file);

