#pragma once

#include "line.hpp"
#include "serialize.hpp"

struct Spline {
  vec3 end[2];
  vec3 normal[2];
};

Spline spline(Line l0, Line l1);
vec3 interpolateSpline(Spline s, float along);
Spline readSpline(FileBuffer* file, int version);
void writeSpline(FileBuffer* file, Spline s);

