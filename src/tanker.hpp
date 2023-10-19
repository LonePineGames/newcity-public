#pragma once

#include "cup.hpp"

struct Tanker {
  Cup<item> elements;
  Cup<item> gaps;
  Cup<item> ndx;
  item num;
};

item randomInTanker(Tanker* tanker);
void resetTanker(Tanker* tanker);
bool insertIntoTanker(Tanker* tanker, item element);
bool removeFromTanker(Tanker* tanker, item element);

