#include "graphParent.hpp"

#include "../util.hpp"

#include "spdlog/spdlog.h"
#include <unordered_map>

unordered_map<item, item> parents;
unordered_map<item, vector<item>> children;

void resetGraphParents() {
  parents.clear();
  children.clear();
}

item getGraphParent(item ndx) {
  item result = ndx;
  while (true) {
    item next = parents[result];
    if (next == 0) break;
    result = next;
  }
  return result;
}

void setGraphParent(item child, item parent) {
  parents[child] = parent;
  children[parent].push_back(child);
}

vector<item> getGraphChildren(item parent) {
  return children[parent];
}

void clearGraphChildren(item ndx) {
  item parent = parents[ndx];
  if (parent != 0) {
    removeFromVector(&children[parent], ndx);
    parents.erase(ndx);
  }

  vector<item>* childs = &children[ndx];
  for (int i = 0; i < childs->size(); i++) {
    item child = childs->at(i);
    if (parents[child] == ndx) parents[child] = 0;
  }
  children.erase(ndx);
}

