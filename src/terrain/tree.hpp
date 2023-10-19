#pragma once

#include "../serialize.hpp"

const int _treeExists = 1 << 0;
const int _treeWind = 1 << 1;
const int _treeForAlpine = 1 << 2;
const int _treeForDesert = 1 << 3;

struct TreeType {
  uint32_t flags;
  item meshImport;
  char* code;
  float weight;
};

void initTreeCallbacks();
item sizeTreeTypes();
item randomTreeType(item ndx);
TreeType* getTreeType(item ndx);

