#include "broker.hpp"

#include "../cup.hpp"
#include "../game/constants.hpp"
#include "../land.hpp"
#include "../util.hpp"

#include "spdlog/spdlog.h"

#include <algorithm>

Cup<item> supplyTable_p;
Cup<item> laneBlkToChunk_p;
Cup<Cup<item>> knownRoutes_g;
Cup<Cup<item>> blocksInChunk_p;
item numBrokerChunks = 0;
float chunkSizeInv = 1;
Cup<item> searchPattern;

struct ChunkDistance {
  item chunkNdx = 0;
  float distance = 0;
};

item supplyTableNdx(item dest, Supply supplies) {
  return dest*numSupplies + supplies;
}

void resetRouteBroker_g() {
  for (int i = 0; i < knownRoutes_g.size(); i++) {
    knownRoutes_g.get(i)->clear();
  }
  knownRoutes_g.clear();

  for (int i = 0; i < blocksInChunk_p.size(); i++) {
    blocksInChunk_p.get(i)->clear();
  }
  blocksInChunk_p.clear();

  supplyTable_p.clear();
  supplyTable_p.reserve(100000);
  knownRoutes_g.reserve(100000);

  numBrokerChunks = 0;
  chunkSizeInv = 1;
}

item locToChunk(vec3 loc) {
  float x = loc.x * chunkSizeInv;
  float y = loc.y * chunkSizeInv;
  item landSize = getLandSize();
  if (x < 0) x = -1;
  if (y < 0) y = -1;
  if (x > landSize) x = landSize;
  if (y > landSize) y = landSize;
  item chunkNdx = floor(x)+1 + (floor(y)+1)*(landSize+2) + 1;
  if (chunkNdx > numBrokerChunks) handleError("chunkNdx out of bounds");
  if (chunkNdx < 0) handleError("chunkNdx out of bounds");
  return chunkNdx;
}

struct compareDistance : binary_function <ChunkDistance, ChunkDistance, bool> {
  bool operator() (ChunkDistance const& x, ChunkDistance const& y) const {
    return x.distance < y.distance;
  }
};

void initRouteBroker_g() {
  float chunkSize = getChunkSize() * tileSize;
  chunkSizeInv = 1.0 / chunkSize;
  item landSize = getLandSize();
  landSize += 2;
  numBrokerChunks = landSize*landSize;
  blocksInChunk_p.resize(numBrokerChunks+2);
  for (int i = 0; i < blocksInChunk_p.size(); i++) {
    blocksInChunk_p.get(i)->clear();
  }

  float mapSize = getMapSize();
  vec3 mapCenter = vec3(mapSize*.5f, mapSize*.5f, 0);
  item centerNdx = locToChunk(mapCenter);

  vector<ChunkDistance> distances;
  for (int x = -c(CBrokerSearchDistance); x <= c(CBrokerSearchDistance); x++) {
    for (int y = -c(CBrokerSearchDistance);
        y <= c(CBrokerSearchDistance); y++) {
      vec3 loc = mapCenter + vec3(x*chunkSize, y*chunkSize, 0);
      ChunkDistance distance = {
        locToChunk(loc),
        length(loc - mapCenter)
      };
      distances.push_back(distance);
    }
  }

  compareDistance comparator;
  sort(distances.begin(), distances.end(), comparator);
  searchPattern.clear();
  for (int i = 0; i < distances.size(); i++) {
    item chunkNdxDiff = distances[i].chunkNdx - centerNdx;
    searchPattern.push_back(chunkNdxDiff);
    //SPDLOG_INFO("search pattern: {} @ {}m",
        //chunkNdxDiff, distances[i].distance);
  }
}

item routeBroker_p(item source, Supply supplies) {
  source /= 10;
  if (source == 0 || source >= knownRoutes_g.size()) return 0;

  Cup<item>* routes = knownRoutes_g.get(source);
  size_t routeS = routes->size();
  if (routeS == 0) return 0;
  size_t offset = randItem(routeS);
  item supplyTable_pS = supplyTable_p.size();

  // See if there is any known routes
  for (size_t i = 0; i < routeS; i++) {
    item j = (i + offset) % routeS;
    item dest = routes->at(j);
    item tableNdx = supplyTableNdx(dest, supplies);
    if (tableNdx < 0 || tableNdx >= supplyTable_pS) continue;
    item result = supplyTable_p[tableNdx];
    if (result != 0) return result;
  }

  // Search nearby chunks in a spiral
  item sourceChunkNdx = laneBlkToChunk_p[source];
  if (sourceChunkNdx <= 0) return 0;
  if (sourceChunkNdx > numBrokerChunks) return 0;

  item searchPatternS = searchPattern.size();
  for (int i = 0; i < searchPatternS; i++) {
    item searchChunkNdx = sourceChunkNdx + searchPattern[i];
    if (searchChunkNdx <= 0) continue;
    if (searchChunkNdx > numBrokerChunks) continue;

    Cup<item>* blocks = blocksInChunk_p.get(searchChunkNdx);
    item blocksS = blocks->size();
    for (int j = 0 ; j < blocksS; j++) {
      //SPDLOG_INFO("source{} sourceChunkNdx{} searchChunkNdx{} numBrokerChunks{} i{} j{}",
          //source, sourceChunkNdx, searchChunkNdx, numBrokerChunks, i, j);
      item blk = blocks->at(j);
      item tableNdx = supplyTableNdx(blk, supplies);
      if (tableNdx < 0 || tableNdx >= supplyTable_pS) continue;
      item result = supplyTable_p[tableNdx];
      if (result != 0) return result;
    }
  }

  return 0;
}

bool supplyTableSuggest_g(item block, Supply supply, item target) {
  block /= 10;
  item tableNdx = supplyTableNdx(block, supply);
  if (tableNdx < 0) return false;
  supplyTable_p.ensureSize(tableNdx+1);
  item current = supplyTable_p[tableNdx];
  if (current != 0 && randFloat() < c(CSupplyTableUpdateRate)) return false;
  supplyTable_p.set(tableNdx, target);
  return true;
}

bool supplyTableErase_g(item block, Supply supply, item target) {
  block /= 10;
  item tableNdx = supplyTableNdx(block, supply);
  if (tableNdx < 0) return false;
  if (tableNdx >= supplyTable_p.size()) return false;
  item current = supplyTable_p[tableNdx];
  if (current == 0) return false;
  supplyTable_p.set(tableNdx, 0);
  return true;
}

bool knownRouteAdd_g(item source, item dest) {
  source /= 10;
  dest /= 10;
  if (source <= 0) return false;
  knownRoutes_g.ensureSize(source+1);

  Cup<item>* routes = knownRoutes_g.get(source);
  size_t routeS = routes->size();
  for (size_t i = 0; i < routeS; i++) {
    item destI = routes->at(i);
    if (destI == dest) return false;
  }

  routes->push_back(dest);
  return true;
}

void knownRouteErase_g(item source, item dest) {
  source /= 10;
  dest /= 10;
  if (source <= 0) return;
  if (source >= knownRoutes_g.size()) return;

  Cup<item>* routes = knownRoutes_g.get(source);
  routes->removeByValue(dest);
  return;
}

void broker_addLaneBlock_g(item blkNdx, vec3 loc) {
  blkNdx /= 10;
  item chunkNdx = locToChunk(loc);
  laneBlkToChunk_p.ensureSize(blkNdx+1);
  laneBlkToChunk_p.set(blkNdx, chunkNdx);
  blocksInChunk_p.get(chunkNdx)->push_back(blkNdx);
}

void broker_removeLaneBlock_g(item blkNdx) {
  blkNdx /= 10;
  laneBlkToChunk_p.ensureSize(blkNdx+1);
  item chunkNdx = laneBlkToChunk_p[blkNdx];
  laneBlkToChunk_p.set(blkNdx, 0);
  if (chunkNdx > 0 && chunkNdx <= numBrokerChunks) {
    blocksInChunk_p.get(chunkNdx)->removeByValue(blkNdx);
  }
}

/*
const uint32_t _suppliesRetail      = 1 << 0;
const uint32_t _suppliesAmenity     = 1 << 1;
const uint32_t _suppliesFreightNeed = 1 << 2;
const uint32_t _suppliesFriend      = 1 << 3;
const uint32_t _suppliesHome        = 1 << 4;
const uint32_t _suppliesDorm        = 1 << 5;
const uint32_t _suppliesHotel       = 1 << 6;
const uint32_t _suppliesHomeless    = 1 << 7;
const uint32_t _suppliesNoEduJob    = 1 << 8;
const uint32_t _suppliesHSEduJob    = 1 << 9;
const uint32_t _suppliesBclEduJob   = 1 << 10;
const uint32_t _suppliesPhdEduJob   = 1 << 11;

struct BrokerBuilding {
  uint32_t supplies;
  item ndx;
};

Cup<item> buildingSupplies_p;
Cup<Cup<BrokerBuilding>> laneBlockBuildings_p;
Cup<uint16_t> laneBlockSupplies_p;
Cup<item> supplyTable_p;

Cup<Cup<item>> knownRoutes;

item routeBrokerBuilding_p(item block, uint32_t supplies) {
  if (block == 0 || block >= laneBlockBuildings_p.size()) return 0;

  Cup<BrokerBuilding>* buildings = laneBlockBuildings_p.get(block);
  size_t buildingS = buildings->size();
  size_t offset = randItem(buildingS);
  for (size_t i = 0; i < buildingS; i++) {
    item j = (i + offset) % buildingS;
    BrokerBuilding building = buildings->at(j);
    if (building.supplies & supplies) {
      return building.ndx;
    }
  }
}

item supplyTableNdx(item dest, item supplies) {
  return dest*numSupplies + supplies;
}

item routeBroker_p(item source, uint32_t supplies) {
  if (source == 0 || source >= knownRoutes.size()) return 0;

  Cup<item>* routes = knownRoutes.get(source);
  size_t routeS = routes->size();
  size_t offset = randItem(routeS);
  for (size_t i = 0; i < routeS; i++) {
    item j = (i + offset) % routeS;
    item dest = routes->at(j);
    uint16_t destSupplies = laneBlockSupplies_p[dest];
    if (destSupplies & supplies) {
      item result = supplyTable_p[supplyTableNdx(dest, supplies)];
      //item result = routeBrokerBuilding_p(dest, supplies);
      if (result != 0) return result;
    }
  }

  return 0;
}

item routeBrokerFriend_g(item source) {
  item buildingNdx = routeBroker_p(source, _suppliesFriend);
  Building* building = getBuilding(buildingNdx);
  for (int i = 0; i < building->
}

item routeBrokerJob_g(item source) {
  item buildingNdx = routeBroker_p(source, _suppliesFriend);
}

item routeBroker_p(item source, uint32_t supplies) {
}
*/

