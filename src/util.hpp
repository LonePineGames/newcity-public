#ifndef UTIL_H
#define UTIL_H

#include "main.hpp"
#include "line.hpp"
#include "item.hpp"

#include <set>
#include <csignal>

// Generate an interrupt
#define TRIP std::raise(SIGINT);

float getSqr(float val);
float distanceSqrd(vec3 v1, vec3 v2);
float distance2D(vec3 v1, vec3 v2);
float distance2DSqrd(vec3 v1, vec3 v2);
float distance2DSqrd(vec2 v1, vec2 v2);
float vecDistance(vec3 v1, vec3 v2);
vec3 zNormal(vec3 v1);
vec3 uzNormal(vec3 v1);
void printMatrix(mat4 matrix);
void printVec(vec3 vec);

char* formatFloat(float val);

vec3 pointOfIntersection(Line l0, Line l1);
float pointLineDistance(vec3 point, Line l);
float pointLineDistanceSqrd(const vec3 p, const Line line);
float pointLineDistance2DSqrd(const vec3 p, const Line l);
float pointRayDistanceSqrd(const vec3 p, const Line l);
float lineDistance(Line l0, Line l1);
float lineDistance2D(Line l0, Line l1);
bool line2Dintersect(Line l0, Line l1);
vec3 nearestPointOnLine(vec3 p, Line l);
vec3 lineAtZ(Line l, float z);

item iclamp(item a, item b, item c);
float lerp(float a0, float a1, float w);
vec3 lerp(vec3 a0, vec3 a1, float w);
float nonZero(float val);
float slope(Line line);
float triangle_intersection(Line line,
  const vec3 v0, const vec3 v1, const vec3 v2);
bool boxLineIntersect(Line box, Line ml);
vec3 linePlaneIntersect(Line ray, vec3 p0, vec3 n);
bool isInDim(vec2 loc, Line dim);

bool removeFromVector(vector<item>* v, item n);
bool isInVector(vector<item>* v, item n);

bool validate(float f);
bool validate(Line l);
bool validate(vec3 v);

item nextRand(item* randomSeed);
float randFloat();
float randFloat(item* randomSeed);
float randFloat(float a, float b);
float randFloat(item* randomSeed, float a, float b);
item randItem(item num);
item randItem(item* randomSeed, item num);
item randItem(item a, item b);
item randItem(item* randomSeed, item a, item b);
item randInSet(set<item>* s, item num);
item randRound(float val);

void free_proxy(void* ptr);

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#endif
