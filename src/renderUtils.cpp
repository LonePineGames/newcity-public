#include "renderUtils.hpp"

#include "icons.hpp"
#include "util.hpp"
#include <glm/gtx/rotate_vector.hpp>

static vec3 up = vec3(0, 0, 1);

void makeTriangle(Mesh* mesh, vec3 p0, vec3 p1, vec3 p2, vec3 xp) {
  vec3 n0 = cross(p0-p1, p0-p2);
  Triangle t1 = {{
    {p0, n0, xp},
    {p1, n0, xp},
    {p2, n0, xp},
  }};
  pushTriangle(mesh, t1);
}

void makeTriangle(Mesh* mesh, vec3 p0, vec3 p1, vec3 p2,
    vec3 xp0, vec3 xp1, vec3 xp2) {
  vec3 n0 = cross(p0-p1, p0-p2);
  Triangle t1 = {{
    {p0, n0, xp0},
    {p1, n0, xp1},
    {p2, n0, xp2},
  }};
  pushTriangle(mesh, t1);
}

void makeQuad(Mesh* mesh, vec3 topRight, vec3 topLeft,
  vec3 bottomRight, vec3 bottomLeft,
  vec3 xs, vec3 xe) {

  vec3 n0 = cross(bottomRight - topRight, bottomRight - topLeft);
  vec3 n1 = cross(topLeft - bottomLeft, topLeft - bottomRight);

  Triangle triangle1 = {{
    {bottomRight, n0, vec3(xs.x, xe.y, xe.z)},
    {topRight,    n0, xs},
    {topLeft,     n0, vec3(xe.x, xs.y, xs.z)},
  }};
  pushTriangle(mesh, triangle1);

  Triangle triangle2 = {{
    {topLeft,     n1, vec3(xe.x, xs.y, xs.z)},
    {bottomLeft,  n1, xe},
    {bottomRight, n1, vec3(xs.x, xe.y, xe.z)},
  }};
  pushTriangle(mesh, triangle2);
}

void makeQuad(Mesh* mesh, vec3 topRight, vec3 topLeft,
  vec3 bottomRight, vec3 bottomLeft,
  vec3 trn, vec3 tln, vec3 brn, vec3 bln,
  vec3 xs, vec3 xe) {

  Triangle triangle1 = {{
    {bottomRight, brn, vec3(xs.x, xe.y, xe.z)},
    {topRight,    trn, xs},
    {topLeft,     tln, vec3(xe.x, xs.y, xs.z)},
  }};
  pushTriangle(mesh, triangle1);

  Triangle triangle2 = {{
    {topLeft,     tln, vec3(xe.x, xs.y, xs.z)},
    {bottomLeft,  bln, xe},
    {bottomRight, brn, vec3(xs.x, xe.y, xe.z)},
  }};
  pushTriangle(mesh, triangle2);
}

void makeQuad(Mesh* mesh, vec3 topRight, vec3 topLeft,
  vec3 bottomRight, vec3 bottomLeft,
  vec3 trx, vec3 tlx, vec3 brx, vec3 blx) {

  vec3 n0 = cross(bottomRight - topRight, bottomRight - topLeft);
  vec3 n1 = cross(topLeft - bottomLeft, topLeft - bottomRight);

  Triangle triangle1 = {{
    {bottomRight, n0, brx},
    {topRight,    n0, trx},
    {topLeft,     n0, tlx},
  }};
  pushTriangle(mesh, triangle1);

  Triangle triangle2 = {{
    {topLeft,     n1, tlx},
    {bottomLeft,  n1, blx},
    {bottomRight, n1, brx},
  }};
  pushTriangle(mesh, triangle2);
}

void makeQuad(Mesh* mesh, vec3 topRight, vec3 topLeft,
  vec3 bottomRight, vec3 bottomLeft,
  vec3 trn, vec3 tln, vec3 brn, vec3 bln,
  vec3 trx, vec3 tlx, vec3 brx, vec3 blx) {

  Triangle triangle1 = {{
    {bottomRight, brn, brx},
    {topRight,    trn, trx},
    {topLeft,     tln, tlx},
  }};
  pushTriangle(mesh, triangle1);

  Triangle triangle2 = {{
    {topLeft,     tln, tlx},
    {bottomLeft,  bln, blx},
    {bottomRight, brn, brx},
  }};
  pushTriangle(mesh, triangle2);
}

void makeDisc(Mesh* mesh, vec3 location,
    float width, int numSides, vec3 xp) {

  vec3 axis = vec3(0,0,1);
  vec3 normal = normalize(cross(axis, vec3(0, 1.f, 0.f))) * (width/2);
  vec3 *points1 = (vec3*)alloca(sizeof(vec3)*numSides);
  for (int i = 0; i < numSides; i ++) {
    float theta = pi_o * 2 * i / numSides;
    vec3 dir = rotate(normal, theta, axis);
    points1[i] = dir + location;
  }

  for (int i = 0; i < numSides; i ++) {
    vec3 p0 = points1[(i + 1) % numSides];
    vec3 p1 = points1[i];

    Triangle t3 = {{
      {p1, up, xp},
      {p0, up, xp},
      {location, up, xp},
    }};
    pushTriangle(mesh, t3);
  }
}

void makeTree(Mesh* mesh, vec3 location, float height, float width,
    vec3 xp, vec3 trunkXp) {

  width *= .5f;
  const float trunkHeight = 5;
  const float segmentFrac = 0.25;
  trunkXp.z = 0;
  makeCylinder(mesh, location+vec3(0,0,-trunkHeight),
      trunkHeight, width*.25f, 12, trunkXp);
  vec3 top = location + vec3(0,0,height);
  vec3 halfHeight = location + vec3(0,0,height*segmentFrac);
  xp.z = 0;
  vec3 xpt = vec3(vec2(xp), height);
  vec3 xpHalf = vec3(vec2(xp), height*.25);
  const int numSides = 12;
  vec3 points0[numSides];
  vec3 points1[numSides];
  for (int i = 0; i < numSides; i ++) {
    float theta = pi_o * 2 * i / numSides;
    vec3 dir = vec3(cos(theta)*width, sin(theta)*width, 0);
    points0[i] = dir + location;
    points1[i] = dir*(1-segmentFrac) + halfHeight;
  }

  for (int i = 0; i < numSides; i ++) {
    vec3 p0 = points1[(i + 1) % numSides];
    vec3 p1 = points1[i];
    vec3 p2 = top;
    vec3 p3 = points0[(i + 1) % numSides];
    vec3 p4 = points0[i];
    vec3 n0 = normalize(p0 - location);
    vec3 n1 = normalize(p1 - location);
    vec3 n3 = normalize(p3 - location);
    vec3 n4 = normalize(p4 - location);

    Triangle t1 = {{
      {p2, vec3(0,0,1), xpt},
      {p1, n1, xpHalf},
      {p0, n0, xpHalf},
    }};
    pushTriangle(mesh, t1);

    makeQuad(mesh, p0, p1, p3, p4,
      n0, n1, n3, n4, xpHalf, xpHalf, xp, xp);
  }
}

void makeSimpleTree(Mesh* mesh, vec3 location, float height,
    float width, vec3 xp) {

  width *= .5f;
  vec3 top = location + vec3(0,0,height);
  xp.z = 0;
  vec3 xpt = vec3(vec2(xp), height);
  const int numSides = 6;
  vec3 points0[numSides];
  for (int i = 0; i < numSides; i ++) {
    float theta = pi_o * 2 * i / numSides;
    vec3 dir = vec3(cos(theta)*width, sin(theta)*width, 0);
    points0[i] = dir + location;
  }

  for (int i = 0; i < numSides; i ++) {
    vec3 p0 = points0[(i + 1) % numSides];
    vec3 p1 = points0[i];
    vec3 p2 = top;
    vec3 n0 = normalize(p0 - location);
    vec3 n1 = normalize(p1 - location);

    Triangle t1 = {{
      {p2, vec3(0,0,1), xpt},
      {p1, n1, xp},
      {p0, n0, xp},
    }};
    pushTriangle(mesh, t1);
  }
}

void makeCone(Mesh* mesh, vec3 location, vec3 axis,
  float width, vec3 xp, bool includeBottom) {

  vec3 normal = normalize(cross(axis, vec3(0, 1.f, 1.f))) * (width/2);
  vec3 top = location + axis;
  const int numSides = 24;
  vec3 points0[numSides];
  for (int i = 0; i < numSides; i ++) {
    float theta = pi_o * 2 * i / numSides;
    vec3 dir = rotate(normal, theta, axis);
    points0[i] = dir + location;
  }

  for (int i = 0; i < numSides; i ++) {
    vec3 p0 = points0[(i + 1) % numSides];
    vec3 p1 = points0[i];
    vec3 p2 = top;
    vec3 n0 = normalize(p0 - location);
    vec3 n1 = normalize(p1 - location);

    Triangle t1 = {{
      {p2, normalize(n0+n1), xp},
      {p1, n1, xp},
      {p0, n0, xp},
    }};
    pushTriangle(mesh, t1);

    if (includeBottom) {
      Triangle t3 = {{
        {p0, -axis, xp},
        {p1, -axis, xp},
        {location, -axis, xp},
      }};
      pushTriangle(mesh, t3);
    }
  }
}

void makeLightCone(Mesh* mesh, vec3 location, vec3 axis, float width) {
  vec3 xp = vec3(0,0,0);
  vec3 normal = normalize(cross(axis, vec3(0, 1.f, 1.f))) * (width/2);
  const int numSides = 5;
  vec3 points0[numSides];
  for (int i = 0; i < numSides; i ++) {
    float theta = pi_o * 2 * i / numSides;
    vec3 dir = rotate(normal, theta, axis);
    dir.z /= 2;
    points0[i] = location + axis + dir;
  }

  for (int i = 0; i < numSides; i ++) {
    vec3 p0 = points0[(i + 1) % numSides];
    vec3 p1 = points0[i];

    Triangle t1 = {{
      {location, location, xp},
      {p0, location, xp},
      {p1, location, xp},
    }};
    pushTriangle(mesh, t1);
  }
}

void makeCylinder(Mesh* mesh, vec3 location, float height,
  float width, int numSides, vec3 xp) {
  makeCylinder(mesh, location, height, width, numSides, xp, xp);
}

void makeCylinder(Mesh* mesh, vec3 location, float height,
  float width, int numSides, vec3 xpb, vec3 xpt) {

  vec3 top = location + vec3(0,0,height);
  vec3 *points0 = (vec3*)alloca(sizeof(vec3)*numSides);
  vec3 *points1 = (vec3*)alloca(sizeof(vec3)*numSides);
  for (int i = 0; i < numSides; i ++) {
    float theta = pi_o * 2 * i / numSides;
    vec3 dir = vec3(cos(theta)*width, sin(theta)*width, 0);
    points0[i] = dir + location;
    points1[i] = dir + top;
  }

  for (int i = 0; i < numSides; i ++) {
    vec3 p0 = points0[(i + 1) % numSides];
    vec3 p1 = points0[i];
    vec3 p2 = points1[(i + 1) % numSides];
    vec3 p3 = points1[i];

    Triangle t1 = {{
      {p2, p2-top, xpt},
      {p1, p1-location, xpb},
      {p0, p0-location, xpb},
    }};
    pushTriangle(mesh, t1);

    Triangle t2 = {{
      {p2, p2-top, xpt},
      {p3, p3-top, xpt},
      {p1, p1-location, xpb},
    }};
    pushTriangle(mesh, t2);

    Triangle t3 = {{
      {p3, up, xpt},
      {p2, up, xpt},
      {top, up, xpt},
    }};
    pushTriangle(mesh, t3);
  }
}

void makeCylinder(Mesh* mesh, vec3 location, vec3 axis,
  float width, int numSides, vec3 xp) {
  makeCylinder(mesh, location, axis, width, numSides, xp, true);
}

void makeCylinder(Mesh* mesh, vec3 location, vec3 axis,
  float width, int numSides, vec3 xp, bool includeTop) {

  vec3 normal = normalize(cross(axis, vec3(0, 1.f, 1.f))) * (width/2);
  vec3 top = location + axis;
  vec3 *points0 = (vec3*)alloca(sizeof(vec3)*numSides);
  vec3 *points1 = (vec3*)alloca(sizeof(vec3)*numSides);
  for (int i = 0; i < numSides; i ++) {
    float theta = pi_o * 2 * i / numSides;
    vec3 dir = rotate(normal, theta, axis);
    points0[i] = dir + location;
    points1[i] = dir + top;
  }

  for (int i = 0; i < numSides; i ++) {
    vec3 p0 = points0[(i + 1) % numSides];
    vec3 p1 = points0[i];
    vec3 p2 = points1[(i + 1) % numSides];
    vec3 p3 = points1[i];

    Triangle t1 = {{
      {p2, p2-top, xp},
      {p1, p1-location, xp},
      {p0, p0-location, xp},
    }};
    pushTriangle(mesh, t1);

    Triangle t2 = {{
      {p2, p2-top, xp},
      {p3, p3-top, xp},
      {p1, p1-location, xp},
    }};
    pushTriangle(mesh, t2);

    if (includeTop) {
      Triangle t3 = {{
        {p3, up, xp},
        {p2, up, xp},
        {top, up, xp},
      }};
      pushTriangle(mesh, t3);
    }
  }
}

void makePipe(Mesh* mesh, vec3 location, vec3 axis,
  float width, float widthInner, int numSides, vec3 xp) {

  vec3 normal = normalize(cross(axis, vec3(0, 1.f, 1.f))) * (width/2);
  vec3 normalInner = normalize(cross(axis, vec3(0, 1.f, 1.f))) * (widthInner/2);
  vec3 top = location + axis;
  vec3 *points0 = (vec3*)alloca(sizeof(vec3)*numSides);
  vec3 *points1 = (vec3*)alloca(sizeof(vec3)*numSides);
  vec3 *points2 = (vec3*)alloca(sizeof(vec3)*numSides);
  vec3 *points3 = (vec3*)alloca(sizeof(vec3)*numSides);
  for (int i = 0; i < numSides; i ++) {
    float theta = pi_o * 2 * i / numSides;
    vec3 dir = rotate(normal, theta, axis);
    vec3 dirInner = rotate(normalInner, theta, axis);
    points0[i] = dir + location;
    points1[i] = dir + top;
    points2[i] = dirInner + location;
    points3[i] = dirInner + top;
  }

  for (int i = 0; i < numSides; i ++) {
    vec3 p0 = points0[(i + 1) % numSides];
    vec3 p1 = points0[i];
    vec3 p2 = points1[(i + 1) % numSides];
    vec3 p3 = points1[i];
    vec3 p4 = points2[(i + 1) % numSides];
    vec3 p5 = points2[i];
    vec3 p6 = points3[(i + 1) % numSides];
    vec3 p7 = points3[i];
    vec3 n0 = p2-top;
    vec3 n1 = p3-top;

    makeQuad(mesh, p1, p0, p3, p2,
      n1, n0, n1, n0, xp, xp);
    makeQuad(mesh, p3, p2, p7, p6, xp, xp);
    makeQuad(mesh, p4, p5, p6, p7,
      -n0, -n1, -n0, -n1, xp, xp);
  }
}

void makeFlaredCube(Mesh* mesh, vec3 center, vec3 size, float flare,
    vec3 xsb, vec3 xst, bool rounded) {
  size = size * 0.5f;
  center.z += size.z;
  vec3 tlf = center + vec3( size.x, -size.y,  size.z);
  vec3 tlb = center + vec3( size.x,  size.y,  size.z);
  vec3 trf = center + vec3(-size.x, -size.y,  size.z);
  vec3 trb = center + vec3(-size.x,  size.y,  size.z);
  size += vec3(flare, flare, 0);
  vec3 blf = center + vec3( size.x, -size.y, -size.z);
  vec3 blb = center + vec3( size.x,  size.y, -size.z);
  vec3 brf = center + vec3(-size.x, -size.y, -size.z);
  vec3 brb = center + vec3(-size.x,  size.y, -size.z);

  //vec3 ntlb = vec3( 1,  1,  1);
  //vec3 ntlf = vec3( 1, -1,  1);
  //vec3 ntrb = vec3(-1,  1,  1);
  //vec3 ntrf = vec3(-1, -1,  1);
  if (rounded) {
    vec3 ntlb = vec3( 1,  1, 1);
    vec3 ntlf = vec3( 1, -1, 1);
    vec3 ntrb = vec3(-1,  1, 1);
    vec3 ntrf = vec3(-1, -1, 1);
    vec3 nblb = vec3( 1,  1, 0);
    vec3 nblf = vec3( 1, -1, 0);
    vec3 nbrb = vec3(-1,  1, 0);
    vec3 nbrf = vec3(-1, -1, 0);

    makeQuad(mesh, tlf, trf, blf, brf, ntlf, ntrf, nblf, nbrf, xst, xsb);
    makeQuad(mesh, tlb, tlf, blb, blf, ntlb, ntlf, nblb, nblf, xst, xsb);
    makeQuad(mesh, tlb, blb, trb, brb, ntlb, nblb, ntrb, nbrb, xst, xsb);
    makeQuad(mesh, brb, brf, trb, trf, nbrb, nbrf, ntrb, ntrf, xsb, xst);
    //top
    makeQuad(mesh, trf, tlf, trb, tlb, ntrf, ntlf, ntrb, ntlb, xst, xst);

  } else {
    makeQuad(mesh, tlf, trf, blf, brf, xst, xsb);
    makeQuad(mesh, tlb, tlf, blb, blf, xst, xsb);
    makeQuad(mesh, trb, tlb, brb, blb, xst, xsb);
    makeQuad(mesh, brb, brf, trb, trf, xsb, xst);
    //top
    makeQuad(mesh, trf, tlf, trb, tlb, xst, xst);
  }
}

void makeFlaredCube(Mesh* mesh, vec3 center, vec3 size, float flare,
    vec3 xsb, vec3 xst) {
  makeFlaredCube(mesh, center, size, flare, xsb, xst, false);
}

void makeCube(Mesh* mesh, vec3 center, vec3 size, vec3 xs,
  bool includeTop, bool rounded) {

  size = size * 0.5f;
  center.z += size.z;
  vec3 tlf = center + vec3( size.x, -size.y,  size.z);
  vec3 tlb = center + vec3( size.x,  size.y,  size.z);
  vec3 trf = center + vec3(-size.x, -size.y,  size.z);
  vec3 trb = center + vec3(-size.x,  size.y,  size.z);
  vec3 blf = center + vec3( size.x, -size.y, -size.z);
  vec3 blb = center + vec3( size.x,  size.y, -size.z);
  vec3 brf = center + vec3(-size.x, -size.y, -size.z);
  vec3 brb = center + vec3(-size.x,  size.y, -size.z);

  if (rounded) {
    vec3 ntlb = vec3( 1,  1,  0);
    vec3 ntlf = vec3( 1, -1,  0);
    vec3 ntrb = vec3(-1,  1,  0);
    vec3 ntrf = vec3(-1, -1,  0);
    vec3 nblb = vec3( 1,  1, -1);
    vec3 nblf = vec3( 1, -1, -1);
    vec3 nbrb = vec3(-1,  1, -1);
    vec3 nbrf = vec3(-1, -1, -1);

    makeQuad(mesh, tlf, trf, blf, brf,
      ntlf, ntrf, nblf, nbrf, xs, xs);
    makeQuad(mesh, tlb, tlf, blb, blf,
      ntlb, ntlf, nblb, nblf, xs, xs);
    makeQuad(mesh, tlb, blb, trb, brb,
      ntlb, nblb, ntrb, nbrb, xs, xs);
    makeQuad(mesh, brb, brf, trb, trf,
      nbrb, nbrf, ntrb, ntrf, xs, xs);
    if (includeTop) {
      makeQuad(mesh, trf, tlf, trb, tlb,
        ntrf, ntlf, ntrb, ntlb, xs, xs);
    }

  } else {
    makeQuad(mesh, tlf, trf, blf, brf, xs, xs);
    makeQuad(mesh, tlb, tlf, blb, blf, xs, xs);
    makeQuad(mesh, tlb, blb, trb, brb, xs, xs);
    makeQuad(mesh, brb, brf, trb, trf, xs, xs);
    if (includeTop) {
      makeQuad(mesh, trf, tlf, trb, tlb, xs, xs);
    }
  }
  /*
  makeQuad(mesh, brf, blf, trf, tlf);
  makeQuad(mesh, trf, tlf, trb, tlb);
  makeQuad(mesh, tlb, tlf, blb, blf);

  makeQuad(mesh, brb, trb, blb, tlb);
  makeQuad(mesh, brb, blb, brf, blf);
  makeQuad(mesh, brb, brf, trb, trf);
  */
}

void makeAngledCube(Mesh* mesh, vec3 center, vec3 dir, float height,
  vec3 xp, vec3 xfs, vec3 xfe) {

  height/=2;
  dir/=2;
  vec3 norm = zNormal(dir);
  vec3 up = vec3(0, 0, height);
  center.z += height;
  vec3 tlf = center + norm - dir + up;
  vec3 tlb = center + norm + dir + up;
  vec3 trf = center - norm - dir + up;
  vec3 trb = center - norm + dir + up;
  vec3 blf = center + norm - dir - up;
  vec3 blb = center + norm + dir - up;
  vec3 brf = center - norm - dir - up;
  vec3 brb = center - norm + dir - up;

  makeQuad(mesh, tlf, trf, blf, brf, xp, xp);
  makeQuad(mesh, tlb, tlf, blb, blf, xp, xp);
  makeQuad(mesh, trb, tlb, brb, blb, xfs, xfe);
  makeQuad(mesh, brb, brf, trb, trf, xp, xp);
  makeQuad(mesh, trf, tlf, trb, tlb, xp, xp);
}


void makeAngledCube(Mesh* mesh, vec3 loc, vec3 right, vec3 along, vec3 up,
  bool includeTop, vec3 xp) {

  vec3 blf = loc                     ;
  vec3 blb = loc         + along     ;
  vec3 brf = loc + right             ;
  vec3 brb = loc + right + along     ;
  vec3 tlf = loc                 + up;
  vec3 tlb = loc         + along + up;
  vec3 trf = loc + right         + up;
  vec3 trb = loc + right + along + up;

  makeQuad(mesh, tlf, trf, blf, brf, xp, xp);
  makeQuad(mesh, trb, tlb, brb, blb, xp, xp);
  makeQuad(mesh, tlb, tlf, blb, blf, xp, xp);
  makeQuad(mesh, brb, brf, trb, trf, xp, xp);
  if (includeTop) {
    makeQuad(mesh, trf, tlf, trb, tlb, xp, xp);
  }
}

/*
void makeCubeSides(Mesh* mesh, vec3 center, vec3 size) {
  size = size * 0.5f;
  center.z += size.z;
  vec3 tlf = center + vec3(size.x, size.y, size.z);
  vec3 tlb = center + vec3(size.x, size.y, -size.z);
  vec3 trf = center + vec3(-size.x, size.y, size.z);
  vec3 trb = center + vec3(-size.x, size.y, -size.z);
  vec3 blf = center + vec3(size.x, -size.y, size.z);
  vec3 blb = center + vec3(size.x, -size.y, -size.z);
  vec3 brf = center + vec3(-size.x, -size.y, size.z);
  vec3 brb = center + vec3(-size.x, -size.y, -size.z);

  makeQuad(mesh, trf, tlf, trb, tlb);
  makeQuad(mesh, tlb, tlf, blb, blf);

  makeQuad(mesh, brb, blb, brf, blf);
  makeQuad(mesh, brb, brf, trb, trf);
}
*/

void makeIcon(Mesh* mesh, vec3 tl, vec3 right, vec3 down,
    vec3 icon) {
  Line iconL = iconToSpritesheet(icon, 0.f);
  makeQuad(mesh, tl, tl+right, tl+down, tl+right+down,
      iconL.start, iconL.end);
}

void makeTransitSign(Mesh* mesh, vec3 loc, vec3 dir, float height,
    vec3 icon, vec3 reverse) {
  vec3 xpp = vec3(0.5/spriteSheetSize, 4.5/spriteSheetSize, 1);
  vec3 right = normalize(vec3(dir.y, -dir.x, 0))*signSize;
  vec3 down = -up*signSize;
  vec3 tl = loc + up*height*2.f - right*.25f +
    normalize(dir)*signPoleWidth/2.f;
  Line iconL = iconToSpritesheet(icon, 1.f);
  Line reverseL = iconToSpritesheet(reverse, 1.f);

  makeCylinder(mesh, loc+down, up*height*2.f-down,
    signPoleWidth, 6, xpp);
  makeQuad(mesh, tl, tl+right, tl+down, tl+right+down,
    iconL.start, iconL.end);
  makeQuad(mesh, tl+right, tl, tl+right+down, tl+down,
    reverseL.start, reverseL.end);
}

void makeSign(Mesh* mesh, vec3 loc, vec3 dir, vec3 icon, vec3 reverse) {
  vec3 xpp = vec3(0.5/spriteSheetSize, 4.5/spriteSheetSize, 1);
  vec3 right = normalize(vec3(dir.y, -dir.x, 0))*signSize;
  vec3 down = -up*signSize;
  vec3 tl = loc + up*signHeight - right/2.f + normalize(dir)*signPoleWidth/2.f;
  Line iconL = iconToSpritesheet(icon, 1.f);
  Line reverseL = iconToSpritesheet(reverse, 1.f);

  makeCylinder(mesh, loc+down, up*signHeight-down,
    signPoleWidth, 6, xpp);
  makeQuad(mesh, tl, tl+right, tl+down, tl+right+down,
    iconL.start, iconL.end);
  makeQuad(mesh, tl+right, tl, tl+right+down, tl+down,
    reverseL.start, reverseL.end);
}

void makeDoubleSign(Mesh* mesh, vec3 loc, vec3 dir,
  vec3 icon, vec3 reverse
) {
  vec3 xpp = vec3(0.5/spriteSheetSize, 4.5/spriteSheetSize, 1.);
  vec3 right = normalize(vec3(dir.y, -dir.x, 0))*signSize*2.f;
  vec3 down = -up*signSize*2.f;
  dir = normalize(dir);
  vec3 tl = loc + up*signHeight*2.f - right/2.f + dir*signPoleWidth/2.f;
  vec3 dtl = tl + right - dir*signPoleWidth;
  vec3 ll = loc-right/4.f;
  vec3 lr = loc+right/4.f;
  Line iconL = iconToSpritesheet(icon, 1.f);
  Line reverseL = iconToSpritesheet(reverse, 1.f);

  makeCylinder(mesh, lr-up*signHeight, up*signHeight*3.f,
    signPoleWidth, 6, xpp);
  makeCylinder(mesh, ll-up*signHeight, up*signHeight*3.f,
    signPoleWidth, 6, xpp);

  makeQuad(mesh, tl, tl+right, tl+down, tl+right+down,
    iconL.start, iconL.end);
  makeQuad(mesh, tl+right, tl, tl+right+down, tl+down,
    reverseL.start, reverseL.end);

  makeQuad(mesh, dtl, dtl-right, dtl+down, dtl-right+down,
    iconL.start, iconL.end);
  makeQuad(mesh, dtl-right, dtl, dtl-right+down, dtl+down,
    reverseL.start, reverseL.end);
}

void makeIBeam(Mesh* mesh, vec3 loc, vec3 right, vec3 along, vec3 up,
    float innerSize, bool includeTop, bool includeBottom, vec3 xs) {

  vec3 internal = normalize(right)*innerSize;
  vec3 unitUp = normalize(up)*0.25f;
  vec3 innerUp = up-unitUp;
  makeAngledCube(mesh, loc, right, along, unitUp, true, xs);
  makeAngledCube(mesh, loc+innerUp, right, along, unitUp, includeTop, xs);
  makeAngledCube(mesh, loc+right*.5f-internal*.5f, internal,
    along, up, false, xs);
  if (includeBottom) {
    makeQuad(mesh, loc, loc+right, loc+along, loc+right+along, xs, xs);
    makeQuad(mesh, loc+innerUp, loc+innerUp+right,
        loc+innerUp+along, loc+innerUp+right+along, xs, xs);
  }
}

