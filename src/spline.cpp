#include "spline.hpp"

#include "serialize.hpp"
#include "util.hpp"

#include <stdio.h>

Spline spline(Line l0, Line l1) {
  Spline s;

  s.end[0] = l0.start;
  s.end[1] = l1.start;

  float length = vecDistance(s.end[0], s.end[1]) * .75f;
  s.normal[0] = normalize(l0.start - l0.end) * length;
  s.normal[1] = normalize(l1.start - l1.end) * length;

  return s;
}

vec3 interpolateSpline(Spline s, float along) {
  float invAlong = 1 - along;

  vec3 e0 = s.end[0];
  vec3 e1 = s.end[1];
  vec3 n0 = s.normal[0];
  vec3 n1 = s.normal[1];

  float e0x = e0.x;
  float e0y = e0.y;
  float e0z = e0.z;
  float e1x = e1.x;
  float e1y = e1.y;
  float e1z = e1.z;
  float n0x = n0.x;
  float n0y = n0.y;
  float n0z = n0.z;
  float n1x = n1.x;
  float n1y = n1.y;
  float n1z = n1.z;

  float l0x = e0x + n0x*along;
  float l0y = e0y + n0y*along;
  float l0z = e0z + n0z*along;
  float l1x = e1x + n1x*invAlong;
  float l1y = e1y + n1y*invAlong;
  float l1z = e1z + n1z*invAlong;

  float r0x = l0x*invAlong + l1x*along;
  float r0y = l0y*invAlong + l1y*along;
  float r0z = l0z*invAlong + l1z*along;

  return vec3(r0x, r0y, r0z);

  // DON'T DELETE - Keep for legacy
  //vec3 l0 = s.end[0] + s.normal[0]*along;
  //vec3 l1 = s.end[1] + s.normal[1]*invAlong;
  //return l0*invAlong + l1*along;
}

Spline readSpline(FileBuffer* file, int version) {
  Spline s;
  s.end[0] = fread_vec3(file);
  s.end[1] = fread_vec3(file);
  s.normal[0] = fread_vec3(file);
  s.normal[1] = fread_vec3(file);
  return s;
}

void writeSpline(FileBuffer* file, Spline s) {
  fwrite_vec3(file, s.end[0]);
  fwrite_vec3(file, s.end[1]);
  fwrite_vec3(file, s.normal[0]);
  fwrite_vec3(file, s.normal[1]);
}

