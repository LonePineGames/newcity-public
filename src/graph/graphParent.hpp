#pragma once

#include "../item.hpp"

#include <vector>
using namespace std;

item getGraphParent(item ndx);
void setGraphParent(item child, item parent);
vector<item> getGraphChildren(item parent);
void clearGraphChildren(item ndx);
void resetGraphParents();

