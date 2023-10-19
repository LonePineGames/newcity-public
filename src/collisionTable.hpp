#pragma once

#include "item.hpp"
#include "box.hpp"

enum CollisionIndex {
  BuildingCollisions, LotCollisions, GraphCollisions,
  numCollisionTables
};

void resetCollisionTables();
void initCollisionTables();
void addToCollisionTable(CollisionIndex ci, Box b, item it);
void removeFromCollisionTable(CollisionIndex ci, item it);
vector<item> getCollisions(CollisionIndex ci, Box b, item exclude);

