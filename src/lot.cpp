#include "lot.hpp"

#include "building/building.hpp"
#include "collisionTable.hpp"
#include "draw/entity.hpp"
#include "economy.hpp"
#include "graph.hpp"
#include "graph/graphParent.hpp"
#include "game/game.hpp"
#include "heatmap.hpp"
#include "land.hpp"
#include "money.hpp"
#include "option.hpp"
#include "pool.hpp"
#include "renderLot.hpp"
#include "time.hpp"
#include "util.hpp"
#include "zone.hpp"

#include "parts/toolbar.hpp"

#include <glm/gtx/rotate_vector.hpp>

Pool<Lot>* lots = Pool<Lot>::newPool(20000);
static vector<item> emptyLots[numZoneTypes+1];
static vector<item> orphanedLots;
static item numZonedLots = 0;
static bool lotsVisible = true;
static bool parkLotsVisible = false;
static bool lotDensityVisible = false;
static bool occupiedLotsVisible = false;
static item currentUpdateLot = 0;

void reoccupyLot(item lotNdx);
void occupyLot(item lotNdx, item building);

item addLot(vec3 loc, vec3 normal, item elem) {
  item lotNdx = lots->create();
  Lot* lot = getLot(lotNdx);
  lot->flags = _lotExists;
  lot->elem = elem;
  lot->loc = loc;
  lot->normal = normal;
  lot->zone = NoZone;
  lot->occupant = 0;
  lot->entity = 0;

  Box b = getLotBox(lotNdx);
  addToCollisionTable(LotCollisions, b, lotNdx);
  reoccupyLot(lotNdx);

  if (lot->elem == 0) {
    orphanedLots.push_back(lotNdx);
  }

  heatMapAdd(Prosperity, lot->loc, c(CRoadBuiltProsperity));
  return lotNdx;
}

Lot* getLot(item lotNdx) {
  return lots->get(lotNdx);
}

void unoccupyLot(item lotNdx) {
  Lot* lot = getLot(lotNdx);
  if (!(lot->flags & _lotExists)) return;
  if (lot->zone == GovernmentZone) lot->zone = NoZone;
  if (lot->occupant == 0) return;
  lot->occupant = 0;
  emptyLots[lot->zone].push_back(lotNdx);

  reoccupyLot(lotNdx);
}

void reoccupyLot(item lotNdx) {
  Lot* lot = getLot(lotNdx);
  vector<item> buildings = collideBuilding(getLotBox(lotNdx), 0);
  item bestBuild = 0;
  float bestDist = 10000;

  for (int i = 0; i < buildings.size(); i++) {
    item bNdx = buildings[i];
    Building* b = getBuilding(bNdx);
    if (!(b->flags & _buildingExists)) continue;
    if (!(b->flags & _buildingComplete)) continue;

    float dist = vecDistance(b->location, lot->loc);
    if (dist < bestDist) {
      bestDist = dist;
      bestBuild = bNdx;
    }
  }

  if (bestBuild != 0) {
    occupyLot(lotNdx, bestBuild);
  }

  renderLot(lotNdx);
}

void removeLotFromEmptyLots(item lotNdx) {
  Lot* lot = getLot(lotNdx);
  vector<item> *table = &emptyLots[lot->zone];
  int size = table->size();
  for (int i=size-1; i >= 0; i--) {
    if (table->at(i) == lotNdx) {
      table->erase(table->begin()+i);
    }
  }
}

void removeLot(item lotNdx) {
  Lot* lot = getLot(lotNdx);
  removeFromCollisionTable(LotCollisions, lotNdx);
  lot->flags = 0;
  if (lot->entity) {
    removeEntity(lot->entity);
    lot->entity = 0;
  }

  if (lot->occupant) {
    Building* b = getBuilding(lot->occupant);
    removeFromVector(&(b->lots), lotNdx);
  }

  if (lot->zone != 0) {
    numZonedLots--;
  }

  removeLotFromEmptyLots(lotNdx);

  lots->free(lotNdx);
}

void zoneLot(item lotNdx, item zone, bool overzone) {
  Lot* lot = getLot(lotNdx);
  if (!(lot->flags & _lotExists)) {
    return;
  }
  if(zone != NoZone && !overzone && lot->zone != NoZone) {
    return;
  }

  if (lot->zone != zone) {
    if (lot->occupant != 0) {
      Building* b = getBuilding(lot->occupant);
      if (b->zone == GovernmentZone) {
        zone = GovernmentZone;
      //} else if (b->zone != zone) {
        //removeBuilding(lot->occupant);
      }

    }

    if (lot->zone != 0) {
      removeLotFromEmptyLots(lotNdx);
      /*
      vector<item> *table = &emptyLots[lot->zone];
      for (int i=0; i < table->size(); i++) {
        if (table->at(i) == lotNdx) {
          table->erase(table->begin()+i);
        }
      }
      */
    }

    if (lot->zone == 0 && zone != 0) {
      heatMapAdd(Prosperity, lot->loc, c(CLotZonedProsperity));
      numZonedLots ++;
    } else if (lot->zone != 0 && zone == 0) {
      heatMapAdd(Prosperity, lot->loc, -c(CLotZonedProsperity));
      numZonedLots --;
    }

    lot->zone = zone;
    if (zone != 0 && lot->occupant == 0) {
      emptyLots[zone].push_back(lotNdx);
    }
    renderLot(lotNdx);
  }
}

bool isLotVisible(item ndx) {
  Lot* lot = getLot(ndx);
  if (!(lot->flags & _lotExists)) return false;
  if (getHeatMap() == ZoneHeatMap) return true;
  return (lot->occupant == 0 || isUndergroundView()) &&
    (lotDensityVisible || lot->zone != 0);
}

item nearestLot(Line ml, bool includeInvisible) {
  float leastDist = FLT_MAX;
  item best=0;
  for (item i=1; i <= lots->size(); i++) {
    Lot* lot = getLot(i);
    if (includeInvisible || isLotVisible(i)) {
      float dist = pointLineDistance(lot->loc, ml);
      if (dist < leastDist) {
        leastDist = dist;
        best = i;
      }
    }
  }
  return best;
}

item nearestLot(Line ml) {
  return nearestLot(ml, false);
}

bool getLotSide(item lotNdx) {
  Lot* lot = getLot(lotNdx);
  if (!(lot->flags & _lotExists)) return false;
  if (lot->elem <= 0) return false;
  Line l = getLine(lot->elem);
  vec3 start = l.start;
  vec3 along = l.end - start;
  return cross(lot->loc - start, along).z > 0;
}

vector<item> getLotsByElem(item elem, bool side) {
  vector<item> result;
  Line l = getLine(elem);
  vec3 start = l.start;
  vec3 along = l.end - start;

  for (int i=1; i <= lots->size(); i++) {
    Lot* lot = getLot(i);
    if (!(lot->flags & _lotExists)) {
      continue;
    }
    if (lot->elem != elem) {
      continue;
    }
    if (elem > 0 && ((cross(lot->loc - start, along).z > 0) != side)) {
      continue;
    }
    result.push_back(i);
  }
  return result;
}

vector<item> getLotsByRad(vec3 searchOrigin, float radius) {
  vector<item> result;
  for(int i = 1; i <= lots->size(); i++) {
    Lot* lot = getLot(i);
    if(lot == 0) {
      continue;
    }
    if(distanceSqrd(lot->loc, searchOrigin) > getSqr(radius)) {
      continue;
    }
    if(!(lot->flags & _lotExists)) {
      continue;
    }
    result.push_back(i);
  }
  return result;
}

int getLotMaxDensity(item lotNdx) {
  Lot* lot = getLot(lotNdx);
  int max = (lot->flags & _lotMaxDensityMask) >> _lotMaxDensityShift;
  return 10 - max;
}

void setLotMaxDensity(item lotNdx, int max) {
  if(max > lotMaxDensity || max < 0) {
    SPDLOG_INFO("Tried to set lot {} max density to invalud value {}",
        lotNdx, max);
    return;
  }

  Lot* lot = getLot(lotNdx);
  if(!(lot->flags & _lotExists)) return;

  int val = 10 - max;
  lot->flags &= ~_lotMaxDensityMask;
  lot->flags |= (val << _lotMaxDensityShift) & _lotMaxDensityMask;
}

void zoneLots(vector<item> toZone, item zoneType, item densityValue) {
  bool overzone = getOverzoneMode();
  for (int i=0; i < toZone.size(); i++) {
    item ndx = toZone[i];
    zoneLot(ndx, zoneType, overzone);
    setLotMaxDensity(ndx, densityValue);
  }
}

void changeDensityLots(vector<item> toChange, int val) {
  for(int i=0; i < toChange.size(); i++) {
    setLotMaxDensity(toChange[i], val);
  }
}

Box getLotBox(vec3 loc, vec3 normal) {
  float size = tileSize - lotGap;
  vec2 normnorm = normalize(vec2(normal));
  vec2 axis1 = vec2(normnorm.y, -normnorm.x) * size;
  vec2 axis0 = normnorm * size;
  vec2 corner = vec2(loc) - axis1*.5f;
  return box(corner, axis1, axis0);
}

Box getLotBox(item ndx) {
  Lot* lot = getLot(ndx);
  return getLotBox(lot->loc, lot->normal);
}

void maybeAddLot(vec3 loc, vec3 normal, item elem) {
  float mapSize = getMapSize();
  if (loc.x < 0 || loc.x > mapSize ||
    loc.y < 0 || loc.y > mapSize) return;

  float size = tileSize - lotGap;
  Box b = getLotBox(loc, normal);
  float landZ = pointOnLand(loc).z;
  if (
      landZ > 0 &&
      abs(loc.z - landZ) < c(CZTileSize)*2 &&
      collidingLots(b).size() == 0 &&
      !graphIntersect(b, elem, false)
    ) {
    addLot(loc, normal, elem);
  }
}

void makeLotsInner(vec3 cursor, vec3 along, vec3 normal, float l, item elem) {
  cursor += along*0.5f;
  for (; l > tileSize; l -= tileSize) {
    maybeAddLot(cursor,normal,elem);
    cursor += along;
  }
}

void makeLots(Line line, float size, item element) {
  if (element == 0) return; // Element/ndx should be less than or more than 0
  Configuration config = getElementConfiguration(element);

  if (config.type != ConfigTypeRoad) return;

  float landZStart = pointOnLand(line.start).z;
  float landZEnd = pointOnLand(line.end).z;
  float diffZStart = line.start.z - landZStart;
  float diffZEnd = line.end.z - landZEnd;
  bool inboundZStart = (diffZStart >= -3.0f && diffZStart <= 3.0f);
  bool inboundZEnd = (diffZEnd >= -3.0f && diffZEnd <= 3.0f);
  // Don't build lots for any tunnels or viaducts
  // If the flag is set but the desired elevation is 0 (terrain height), then continue
  if ((config.flags & _configDontMoveEarth) && !(inboundZStart && inboundZEnd)) {
    // SPDLOG_INFO("LandZStart: {} LandZEnd: {} StartZ: {} EndZ: {}", landZStart, landZEnd, line.start.z, line.end.z);
    return;
  }

  vec3 transverse = line.end - line.start;
  float l = length(transverse);
  vec3 along = transverse * tileSize / l;
  vec3 normal = normalize(vec3(along.y, -along.x, 0));
  vec3 buildNorm = normal*buildDistance(element);
  makeLotsInner(line.start + buildNorm, along, normal, l, element);
  makeLotsInner(line.start - buildNorm, along, -normal, l, element);
}

void makeNodeLots(vec3 loc, vec3 normal, item element) {
  if (element >= 0) return; // Element/ndx should be less than than 0
  Configuration config = getElementConfiguration(element);

  if (config.type != ConfigTypeRoad) return;

  float landZ = pointOnLand(loc).z;
  float diffZ = loc.z - landZ;
  bool inboundZ = (diffZ >= -3.0f && diffZ <= 3.0f);
  // Don't build lots for any tunnels or viaducts
  // If the flag is set but the desired elevation is 0 (terrain height), then continue
  if ((config.flags & _configDontMoveEarth) && !inboundZ) return;

  Node* node = getNode(element);
  float dist = buildDistance(element);
  normal = normalize(normal)*dist;
  for (int i=0; i < 8; i ++) {
    vec3 rotNorm = rotate(normal, float(i*pi_o/4), vec3(0,0,1.f));
    vec3 lotLoc = loc + rotNorm;
    maybeAddLot(lotLoc, normalize(rotNorm), element);
  }
}

item numEmptyLots(item zone) {
  return emptyLots[zone].size();
}

item getNumZonedLots() {
  return numZonedLots;
}

item numLots() {
  return lots->count();
}

item randomLot() {
  item size = lots->size();
  if (size == 0) return 0;
  item ndx = randItem(size);
  return ndx+1;
}

item getEmptyLot(item zone) {
  vector<item> *table = &emptyLots[zone];
  int size = table->size();
  if (size == 0) {
    return 0;
  }
  int ndx = randItem(size);
  item result = table->at(ndx);
  //table->erase(table->begin() + ndx);
  return result;
}

void removeLots(Box box) {
  vector<item> clots = collidingLots(box);
  for (int i = 0; i < clots.size(); i ++) {
    removeLot(clots[i]);
  }
}

void orphanLots(item graphNdx) {
  for (int i = 1; i <= lots->size(); i ++) {
    Lot* lot = getLot(i);
    if (lot->flags & _lotExists && lot->elem == graphNdx) {
      //lot->elem = 0;
      orphanedLots.push_back(i);
    }
  }
}

void updateLot(item ndx) {
  if (ndx <= 0 || ndx > lots->size()) return;
  Lot* lot = getLot(ndx);
  if (!(lot->flags & _lotExists)) return;

  item targetElem = lot->elem;

  bool deleted = false;
  if (targetElem > 0) {
    Edge* edge = getEdge(targetElem);
    if (!(edge->flags & _graphExists)) deleted = true;
  } else if (targetElem < 0) {
    Node* node = getNode(targetElem);
    if (!(node->flags & _graphExists)) deleted = true;
  }

  if (deleted) {
    // Elem was deleted/replaced, see if we can find it's replacement
    targetElem = 0;
    vector<item> children = getGraphChildren(lot->elem);
    float bestChildDist = tileSize*.5f;

    for (int i = 0; i < children.size(); i++) {
      item childNdx = children[i];
      if (childNdx == 0) continue;
      Configuration config = getElementConfiguration(childNdx);
      if (config.type != ConfigTypeRoad) continue;
      vec3 loc = nearestPointOnLine(lot->loc, getLine(childNdx));
      float dist = vecDistance(loc, lot->loc);
      dist -= buildDistance(childNdx);

      if (dist < bestChildDist) {
        targetElem = childNdx;
        bestChildDist = dist;
      }
    }

  } else if (targetElem != 0) {
    vec3 loc = nearestPointOnLine(lot->loc, getLine(targetElem));
    float dist = vecDistance(loc, lot->loc) - buildDistance(targetElem);
    if (dist > tileSize*.5f) targetElem = 0;
  }

  if (targetElem != 0) {
    Configuration config = getElementConfiguration(targetElem);
    if (config.type != ConfigTypeRoad) targetElem = 0;
  }

  if (targetElem == 0) {
    Configuration config;
    config.flags = 0;
    config.type = ConfigTypeRoad;
    targetElem = nearestElement(lot->loc, false, config);
  }

  if (targetElem == 0) {
    removeLot(ndx);
    return;
  }

  if (targetElem != lot->elem) {
    vec3 p = nearestPointOnLine(lot->loc, getLine(targetElem));
    vec3 norm = lot->loc - p;
    float dist = length(norm) - buildDistance(targetElem);
    if (dist > tileSize*.5f) {
      removeLot(ndx);
      return;
    }

    vec3 unorm = normalize(norm);
    vec3 finalLoc = p + unorm * buildDistance(targetElem);
    lot->loc = finalLoc;
    lot->elem = targetElem;
    lot->normal = unorm;

    // See if there are colliding lots
    vector<item> collisions = getCollisions(LotCollisions, getLotBox(ndx), ndx);
    if (collisions.size() > 0) {
      removeLot(ndx);
      return;
    }
  }
}

void finishLots() {
  for (int i=0; i < orphanedLots.size(); i++) {
    updateLot(orphanedLots[i]);
  }
  orphanedLots.clear();
}

void updateLots(double duration) {
  // Handle Park Lots
  double yearDur = duration / gameDayInRealSeconds / oneYear;
  int parkLots = 0;
  float angle = randFloat(0, pi_o*2);
  //float mag = pow(randFloat(0, 1),0.5) * c(CParkLotThrow);
  float mag = c(CParkLotThrow);
  float control = getBudgetControl(RecreationExpenses);
  float effect = duration * control;
  vec3 thrw = vec3(sin(angle), cos(angle), 0.f) * mag;

  for (int i = 1; i <= numZoneTypes; i++) {
    for (int j = 0; j < emptyLots[i].size(); j++) {
      Lot* lot = getLot(emptyLots[i][j]);
      if (lot->occupant != 0) continue;

      vec3 loc = lot->loc + thrw;
      if (lot->zone == ParkZone && lot->occupant == 0) {
        parkLots ++;
        float z = lot->loc.z;
        float pbonus = z < 25 ? c(CParkBeachPollution) :
          z > 300 ? c(CParkHillPollution) : c(CParkLotPollution);
        float vbonus = z < 25 ? c(CParkBeachValue) :
          z > 300 ? c(CParkHillValue) : c(CParkLotValue);
        float hbonus = c(CHealthParkEffect);
        float cbonus = c(CCommunityParkEffect);
        pbonus *= effect;
        vbonus *= effect;
        hbonus *= effect;
        cbonus *= effect;
        heatMapAdd(Pollution, loc, pbonus);
        heatMapAdd(Value, loc, vbonus);
        heatMapAdd(HealthHM, loc, hbonus);
        heatMapAdd(CommunityHM, loc, cbonus);

      } else if (lot->zone != NoZone) {
        heatMapAdd(Density, lot->loc, c(CEmptyLotDensity)*duration);
      }
    }
  }

  setStat(ourCityEconNdx(), ParkLots, parkLots);

  // Reposition/reparent lots
  item numToRun = std::min(lots->size(), 1000);
  for (int i = 0; i < numToRun; i++) {
    currentUpdateLot = currentUpdateLot%lots->size() + 1;
    updateLot(currentUpdateLot);
  }
}

vector<item> collidingLots(Box b) {
  return getCollisions(LotCollisions, b, 0);
}

void occupyLot(item lotNdx, item building) {
  getLot(lotNdx)->occupant = building;
  getBuilding(building)->lots.push_back(lotNdx);
  removeLotFromEmptyLots(lotNdx);
  renderLot(lotNdx);
}

void occupyLots(vector<item> lots, item building) {
  Building* b0 = getBuilding(building);
  for (int i=0; i < lots.size(); i++) {
    item ndx = lots[i];
    Lot* lot = getLot(ndx);

    if (lot->occupant) {
      Building* b1 = getBuilding(lot->occupant);
      float lotDist0 = vecDistance(lot->loc, b0->location);
      float lotDist1 = vecDistance(lot->loc, b1->location);

      if (lotDist1 > lotDist0) {
        removeFromVector(&(b1->lots), ndx);
      } else {
        removeFromVector(&(b0->lots), ndx);
        continue;
      }
    }

    occupyLot(ndx, building);
  }
}

void resetLots() {
  lots->clear();
  orphanedLots.clear();
  for (int i=0; i < numZoneTypes; i++) {
    emptyLots[i].clear();
  }
  resetLotRender();
  numZonedLots = 0;
  lotsVisible = getOptionsFlags() & _optionHideLots;
  lotDensityVisible = false;
  parkLotsVisible = false;
}

void initLotsEntities() {
  for (int i = 1; i < lots->size(); i++) {
    if (isLotVisible(i)) {
      Lot* lot = getLot(i);
      if (lot->entity == 0) {
        lot->entity = addEntity(LotShader);
      }
    }
  }

  for (int i = 0; i <= numZoneTypes; i++) {
    for (int j = 0; j <= 10; j++) {
      getLotMesh(i, j);
    }
  }
}

void renderLots() {
  for (int i = 1; i < lots->size(); i++) {
    if (getLot(i)->flags & _lotExists) {
      renderLot(i);
    }
  }
}

void setLotsVisible() {
  bool hideLotsOption = getOptionsFlags() & _optionHideLots;
  bool viz = (!isUndergroundView() && !hideLotsOption) ||
    getCurrentTool() == 3 || getHeatMap() == ZoneHeatMap;
  bool occupiedViz = isUndergroundView() || getHeatMap() == ZoneHeatMap;
  bool parkViz = viz && getGameMode() != ModeBuildingDesigner &&
    (getCurrentTool() == 3 || getHeatMap() == ZoneHeatMap);
  bool densityViz = parkViz;
  if (viz == lotsVisible && parkViz == parkLotsVisible &&
      densityViz == lotDensityVisible &&
      occupiedViz == occupiedLotsVisible) return;

  lotsVisible = viz;
  parkLotsVisible = parkViz;
  lotDensityVisible = densityViz;
  occupiedLotsVisible = occupiedViz;
  for (int i = 1; i <= lots->size(); i++) {
    renderLot(i);
    //Lot* lot = getLot(i);
    //if (lot->entity != 0) {
      //if (lot->zone == ParkZone) {
        //setEntityVisible(lot->entity, parkViz);
      //} else {
        //setEntityVisible(lot->entity, viz);
      //}
    //}
  }
}

bool areLotsVisible() {
  return lotsVisible;
}

bool areParkLotsVisible() {
  return parkLotsVisible;
}

bool isLotDensityVisible() {
  return lotDensityVisible;
}

void writeLot(FileBuffer* file, item ndx) {
  Lot* lot = getLot(ndx);

  fwrite_int(file, lot->flags);
  fwrite_item(file, lot->elem);
  fwrite_item(file, lot->zone);
  fwrite_item(file, lot->occupant);
  fwrite_vec3(file, lot->loc);
  fwrite_vec3(file, lot->normal);
}

void readLot(FileBuffer* file, int version, item ndx) {
  Lot* lot = getLot(ndx);

  lot->flags    = fread_int(file);
  lot->elem     = fread_item(file, version);
  lot->zone     = fread_item(file, version);
  lot->occupant = fread_item(file, version);
  lot->loc      = fread_vec3(file);
  lot->normal   = fread_vec3(file);
  if (version < 16) {
    fread_item(file, version); // lot size
  }

  lot->entity = 0;
  if (lot->flags & _lotExists) {
    addToCollisionTable(LotCollisions, getLotBox(ndx), ndx);
    if (lot->zone != 0) {
      numZonedLots ++;
    }
    if (lot->elem == 0) {
      orphanedLots.push_back(ndx);
    }
    if (lot->occupant == 0) {
      emptyLots[lot->zone].push_back(ndx);
    }
  }
}

void writeLots(FileBuffer* file) {
  lots->write(file);

  for (int i=1; i <= lots->size(); i++) {
    writeLot(file, i);
  }

  //for (int j = 0; j < numZoneTypes+1; j++) {
    //fwrite_item_vector(file, &emptyLots[j]);
  //}
}

void readLots(FileBuffer* file, int version) {
  lots->read(file, version);

  for (int i=1; i <= lots->size(); i++) {
    readLot(file, version, i);
  }

  if (version < 52) {
    int num = version <= 48 ? 5 : 9;
    for (int j = 0; j < num; j++) {
      vector<item> temp;
      fread_item_vector(file, &temp, version);
      //fread_item_vector(file, &emptyLots[j], version);
    }
  }
}

