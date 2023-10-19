#ifndef BOX_H
#define BOX_H

#include "line.hpp"

struct Box {
  vec2 corner;
  vec2 axis0;
  vec2 axis1;
  Line lines[4];
};

Box box(vec2 corner, vec2 axis0, vec2 axis1);
Box box(Line l, float width);
Box alignedBox(Line l);
Box box(vec2 center, float radius);
Box growBox(Box box, float growth);
vector<vec2> boxCorners(Box b);
bool boxIntersect(Box b0, Box b1);
vector<vec3> boxLineIntersect(const Box b, Line l);
float boxDistance(Box b, vec3 p);
float boxSizeSqrd(Box b);

#endif
