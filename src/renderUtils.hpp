#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include "main.hpp"
#include "draw/mesh.hpp"

const float signHeight = 2.5;
const float signSize = 1.5f;
const float signPoleWidth = 0.25;

const vec4 snowColor = vec4(211/255.f, 204/255.f, 202/255.f, 1);

// Special
void mergeMeshes(Mesh* mesh, item meshNdx);

// 2D
void makeTriangle(Mesh* mesh, vec3 p0, vec3 p1, vec3 p2, vec3 xp);
void makeTriangle(Mesh* mesh, vec3 p0, vec3 p1, vec3 p2,
    vec3 xp0, vec3 xp1, vec3 xp2);
void makeQuad(Mesh* mesh, vec3 topRight, vec3 topLeft,
  vec3 bottomRight, vec3 bottomLeft,
  vec3 xs, vec3 xe);
void makeQuad(Mesh* mesh, vec3 topRight, vec3 topLeft,
  vec3 bottomRight, vec3 bottomLeft,
  vec3 trn, vec3 tln, vec3 brn, vec3 bln,
  vec3 xs, vec3 xe);
void makeQuad(Mesh* mesh, vec3 topRight, vec3 topLeft,
  vec3 bottomRight, vec3 bottomLeft,
  vec3 trn, vec3 tln, vec3 brn, vec3 bln,
  vec3 trx, vec3 tlx, vec3 brx, vec3 blx);
void makeQuad(Mesh* mesh, vec3 topRight, vec3 topLeft,
  vec3 bottomRight, vec3 bottomLeft,
  vec3 trx, vec3 tlx, vec3 brx, vec3 blx);
void makeDisc(Mesh* mesh, vec3 location,
    float width, int numSides, vec3 xp);

// Cones, Trees, Cylinders
void makeTree(Mesh* mesh, vec3 location, float height,
  float width, vec3 xp, vec3 trunkXp);
void makeSimpleTree(Mesh* mesh, vec3 location, float height,
  float width, vec3 xp);
void makeCone(Mesh* mesh, vec3 location, vec3 axis,
  float width, vec3 xp, bool includeBottom);
void makeLightCone(Mesh* mesh, vec3 location, vec3 axis,
  float width);
void makeCylinder(Mesh* mesh, vec3 location, float height,
  float width, int numSides, vec3 xp);
void makeCylinder(Mesh* mesh, vec3 location, float height,
  float width, int numSides, vec3 xpb, vec3 xpt);
void makeCylinder(Mesh* mesh, vec3 location, vec3 axis,
  float width, int numSides, vec3 xp);
void makeCylinder(Mesh* mesh, vec3 location, vec3 axis,
  float width, int numSides, vec3 xp, bool includeTop);
void makePipe(Mesh* mesh, vec3 location, vec3 axis,
  float width, float widthInner, int numSides, vec3 xp);

// Cubes etc
void makeAngledCube(Mesh* mesh, vec3 center, vec3 dir, float height,
  vec3 xp, vec3 xfs, vec3 xfe);
void makeCube(Mesh* mesh, vec3 center, vec3 size, vec3 xs,
  bool includeTop, bool rounded);
void makeFlaredCube(Mesh* mesh, vec3 center, vec3 size, float flare,
    vec3 xsb, vec3 xst);
void makeFlaredCube(Mesh* mesh, vec3 center, vec3 size, float flare,
    vec3 xsb, vec3 xst, bool rounded);
void makeAngledCube(Mesh* mesh, vec3 loc, vec3 right, vec3 along, vec3 up,
  bool includeTop, vec3 xp);
void makeIBeam(Mesh* mesh, vec3 loc, vec3 right, vec3 along, vec3 up,
    float innerSize, bool includeTop, bool includeBottom, vec3 xs);

// Signs etc
void makeIcon(Mesh* mesh, vec3 tl, vec3 right, vec3 down,
    vec3 icon);
void makeTransitSign(Mesh* mesh, vec3 loc, vec3 dir, float height,
    vec3 icon, vec3 reverse);
void makeSign(Mesh* mesh, vec3 loc, vec3 dir, vec3 icon, vec3 reverse);
void makeDoubleSign(Mesh* mesh, vec3 loc, vec3 dir, vec3 icon, vec3 reverse);

#endif
