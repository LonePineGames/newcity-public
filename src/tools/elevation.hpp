#pragma once

#include "../configuration.hpp"
#include "../parts/part.hpp"

struct Elevation {
  bool moveEarth = true;
  int zOffset = 0;
};

const float maxZOffset = 8;
const float minZOffset = -7;

bool elevationCallback(Part* part, InputEvent event);
Part* elevationWidget(vec2 loc, Elevation* elevation);
Configuration elevate(Elevation elevation, Configuration config);
void elevationSelect(bool select, Elevation* elevation);

