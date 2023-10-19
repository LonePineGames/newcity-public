#include "box.hpp"

#include "util.hpp"

#include <stdio.h>

vector<vec2> boxCorners(Box b) {
  vec2 corner = b.corner;
  vec2 axis0 = b.axis0;
  vec2 axis1 = b.axis1;
  vec2 tl = corner+axis0;
  vector<vec2> result;
  result.push_back(corner);
  result.push_back(tl);
  result.push_back(corner+axis1);
  result.push_back(tl+axis1);
  return result;
}

void assignBoxLines(Box* b) {
  vec3 corner = vec3(b->corner, 0);
  vec3 axis0 = vec3(b->axis0, 0);
  vec3 axis1 = vec3(b->axis1, 0);
  vec3 tr = corner;
  vec3 tl = corner+axis0;
  vec3 br = corner+axis1;
  vec3 bl = tl+axis1;

  b->lines[0] = line(tr, tl);
  b->lines[1] = line(br, bl);
  b->lines[2] = line(tr, br);
  b->lines[3] = line(tl, bl);
}

Box box(vec2 corner, vec2 axis0, vec2 axis1) {
  Box result;
  result.corner = corner;
  result.axis0 = axis0;
  result.axis1 = axis1;
  assignBoxLines(&result);
  return result;
}

Box box(Line l, float width) {
  Box result;
  result.axis0 = vec2(l.end-l.start);
  result.axis1 = normalize(vec2(-result.axis0.y, result.axis0.x))*width*2.f;
  result.corner = vec2(l.start)-result.axis1*.5f;
  assignBoxLines(&result);
  return result;
}

Box box(vec2 center, float radius) {
  Box result;
  result.axis0 = vec2(radius*2, 0);
  result.axis1 = vec2(0, radius*2);
  result.corner = center - .5f*result.axis0 - .5f*result.axis1;
  assignBoxLines(&result);
  return result;
}

Box alignedBox(Line l) {
  return box(vec2(l.start),
    vec2(l.end.x-l.start.x, 0),
    vec2(0, l.end.y-l.start.y));
}

float boxSizeSqrd(Box b) {
  vec2 diag = b.axis0 + b.axis1;
  return diag.x*diag.x + diag.y*diag.y;
  //return b.axis0.x*b.axis0.x + b.axis0.y*b.axis0.y;
}

bool isLeft(Line l, vec3 c) {
  vec3 a = l.start;
  vec3 b = l.end;
  return (b.x - a.x)*(c.y - a.y) > (b.y - a.y)*(c.x - a.x);
}

vec2 boxCenter(Box b) {
  return b.corner + (b.axis0 + b.axis1)*.5f;
}

bool boxIntersect(Box b0, Box b1) {
  float size = sqrt(boxSizeSqrd(b0)) + sqrt(boxSizeSqrd(b1));
  if (distance2DSqrd(boxCenter(b0), boxCenter(b1)) > size*size*0.25f) {
    return false;
  }

  Line* l0 = &b0.lines[0];
  Line* l1 = &b1.lines[0];

  vec3 c0 = vec3(b0.corner, 0);
  if ((isLeft(l1[0], c0) != isLeft(l1[1], c0)) &&
        (isLeft(l1[2], c0) != isLeft(l1[3], c0))) {
    return true;
  }

  vec3 c1 = vec3(b1.corner, 0);
  if ((isLeft(l0[0], c1) != isLeft(l0[1], c1)) &&
        (isLeft(l0[2], c1) != isLeft(l0[3], c1))) {
    return true;
  }

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (line2Dintersect(l0[i], l1[j])) {
        return true;
      }
    }
  }

  return false;
}

float boxDistance(const Box b, vec3 p) {
  p.z = 0;
  const Line* l = &b.lines[0];
  const bool insidex = isLeft(l[0], p) != isLeft(l[1], p);
  const bool insidey = isLeft(l[2], p) != isLeft(l[3], p);

  if (insidex && insidey) {
    return 0;

  } else if (insidey) {
    const float d0 = pointLineDistance2DSqrd(p, l[0]);
    const float d1 = pointLineDistance2DSqrd(p, l[1]);
    return sqrt(d0 < d1 ? d0 : d1);

  } else if (insidex) {
    const float d0 = pointLineDistance2DSqrd(p, l[2]);
    const float d1 = pointLineDistance2DSqrd(p, l[3]);
    return sqrt(d0 < d1 ? d0 : d1);

  } else {
    const float d0 = distance2DSqrd(p, l[0].start);
    const float d1 = distance2DSqrd(p, l[0].end);
    const float d2 = distance2DSqrd(p, l[1].start);
    const float d3 = distance2DSqrd(p, l[1].end);
    const float da = d0 < d1 ? d0 : d1;
    const float db = d2 < d3 ? d2 : d3;
    return sqrt(da < db ? da : db);
  }

  /*
  if ((isLeft(l[0], p) != isLeft(l[1], p)) &&
        (isLeft(l[2], p) != isLeft(l[3], p))) {
    return 0;
  }

  float min = pointLineDistanceSqrd(p, l[0]);
  for (int i = 1; i < 4; i++) {
    float dist = pointLineDistanceSqrd(p, l[i]);
    if (dist < min) {
      min = dist;
    }
  }

  return sqrt(min);
  */
}

Box growBox(Box b, float growth) {
  vec2 u0 = normalize(b.axis0);
  vec2 u1 = normalize(b.axis1);
  b.corner -= u0*growth + u1*growth;
  b.axis0 += u0*growth*2.f;
  b.axis1 += u1*growth*2.f;
  Box result = box(b.corner, b.axis0, b.axis1);
  return result;
}

vector<vec3> boxLineIntersect(const Box b, Line l) {
  vector<vec3> results;
  for (int i = 0; i < 4; i++) {
    Line bl = b.lines[i];
    if (line2Dintersect(l, bl)) {
      vec3 p = pointOfIntersection(l, bl);
      results.push_back(p);
    }
  }
  return results;
}

