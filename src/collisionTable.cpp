#include "collisionTable.hpp"

#include "cup.hpp"
#include "land.hpp"
#include "util.hpp"

#include "spdlog/spdlog.h"
#include <set>

struct Collider {
  Box b;
  item it;
};

static Cup<Collider>* collisionTable[numCollisionTables] = {0};
static Box* chunkBoxes = 0;

void resetCollisionTables() {
  int landSize = getLandSize();
  if (chunkBoxes != 0) {
    free(chunkBoxes);
    chunkBoxes = 0;
  }

  for (int i = 0; i < numCollisionTables; i++) {
    if (collisionTable[i] != 0) {
      for (int x = 0; x < landSize; x++) {
        int xOff = x*landSize;
        for (int y = 0; y < landSize; y++) {
          collisionTable[i][xOff + y].clear();
        }
      }
      free(collisionTable[i]);
      collisionTable[i] = 0;
    }
  }
}

void initCollisionTables() {
  int landSize = getLandSize();
  int chunkSize = getChunkSize();
  for (int i = 0; i < numCollisionTables; i++) {
    collisionTable[i] =
      (Cup<Collider>*) calloc(landSize*landSize, sizeof(Cup<Collider>));
  }

  chunkBoxes = (Box*) calloc(landSize*landSize, sizeof(Box));
  float cs = tileSize*chunkSize;
  vec2 axis0 = vec2(0, cs);
  vec2 axis1 = vec2(cs, 0);
  for (int x = 0; x < landSize; x++) {
    int xOff = x*landSize;
    for (int y = 0; y < landSize; y++) {
      chunkBoxes[xOff + y] = box(vec2(x*cs, y*cs), axis0, axis1);
    }
  }
}

void addToCollisionTable(CollisionIndex ci, Box b, item it) {
  const int landSize = getLandSize();
  const int chunkSize = getChunkSize();
  const int ls = landSize-1;
  set<item> result;
  int minX = ls;
  int maxX = 0;
  int minY = ls;
  int maxY = 0;

  vector<vec2> corners = boxCorners(b);
  for (int i = 0; i < corners.size(); i++) {
    vec2 c = corners[i];
    float x = c.x/chunkSize/tileSize;
    float y = c.y/chunkSize/tileSize;
    if (floor(x) < minX) minX = floor(x);
    if (ceil(x) > maxX) maxX = ceil(x);
    if (floor(y) < minY) minY = floor(y);
    if (ceil(y) > maxY) maxY = ceil(y);
  }

  if (minX < 0)  minX = 0;
  if (maxX > ls) maxX = ls;
  if (minY < 0)  minY = 0;
  if (maxY > ls) maxY = ls;

  for (int x = minX; x <= maxX; x++) {
    int xOff = x*landSize;
    for (int y = minY; y <= maxY; y++) {
      Cup<Collider>* chunkTable = &collisionTable[ci][xOff+y];
      if (boxIntersect(b, chunkBoxes[xOff + y])) {
        Cup<Collider>* chunkTable = &collisionTable[ci][xOff+y];
        Collider c;
        c.b = b;
        c.it = it;
        chunkTable->push_back(c);
      }
    }
  }
}

void removeFromCollisionTable(CollisionIndex ci, item it) {
  int landSize = getLandSize();
  set<item> result;
  for (int x = 0; x < landSize; x++) {
    int xOff = x*landSize;
    for (int y = 0; y < landSize; y++) {
      Cup<Collider>* chunkTable = &collisionTable[ci][xOff+y];
      for (int i = chunkTable->size()-1; i >= 0; i--) {
        Collider* c = chunkTable->get(i);
        if(c == 0) {
          continue;
        }
        if (c->it == it) {
          chunkTable->remove(i);
        }
      }
    }
  }
}

/*
item getRandomNearby(CollisionIndex ci, vec3 p) {
  float cs = tileSize*chunkSize;
  Cup<Collider>* chunkTable =
    &collisionTable[ci][int(p.x/cs)*xOff + int(p.y/cs)];

  int tableSize = chunkTable->size();
  if (tableSize == 0) return 0;
  return chunkTable[randItem(0, tableSize)]->it;
}
*/

vector<item> getCollisions(CollisionIndex ci, Box b, item exclude) {
  const int landSize = getLandSize();
  const int chunkSize = getChunkSize();
  const int ls = landSize-1;
  set<item> result;
  int minX = ls;
  int maxX = 0;
  int minY = ls;
  int maxY = 0;

  vector<vec2> corners = boxCorners(b);
  for (int i = 0; i < corners.size(); i++) {
    vec2 c = corners[i];
    float x = c.x/chunkSize/tileSize;
    float y = c.y/chunkSize/tileSize;
    if (floor(x) < minX) minX = floor(x);
    if (ceil(x) > maxX) maxX = ceil(x);
    if (floor(y) < minY) minY = floor(y);
    if (ceil(y) > maxY) maxY = ceil(y);
  }

  if (minX < 0)  minX = 0;
  if (maxX > ls) maxX = ls;
  if (minY < 0)  minY = 0;
  if (maxY > ls) maxY = ls;

  for (int x = minX; x <= maxX; x++) {
    int xOff = x*landSize;
    for (int y = minY; y <= maxY; y++) {
      Cup<Collider>* chunkTable = &collisionTable[ci][xOff+y];
      int cts = chunkTable->size();
      if (cts > 0 && boxIntersect(b, chunkBoxes[xOff + y])) {
        for (int i = 0; i < cts; i++) {
          Collider* c = chunkTable->get(i);
          if (c->it != exclude && boxIntersect(b, c->b)) {
            result.insert(c->it);
          }
        }
      }
    }
  }

  vector<item> resultVector(result.begin(), result.end());
  return resultVector;
}

