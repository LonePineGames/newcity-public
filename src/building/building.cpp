#include "building.hpp"

#include "buildingTexture.hpp"
#include "design.hpp"
#include "renderBuilding.hpp"

#include "../amenity.hpp"
#include "../board.hpp"
#include "../box.hpp"
#include "../business.hpp"
#include "../city.hpp"
#include "../collisionTable.hpp"
#include "../draw/entity.hpp"
#include "../draw/texture.hpp"
#include "../economy.hpp"
#include "../game/game.hpp"
#include "../game/task.hpp"
#include "../graph.hpp"
#include "../heatmap.hpp"
#include "../land.hpp"
#include "../lot.hpp"
#include "../money.hpp"
#include "../name.hpp"
#include "../option.hpp"
#include "../person.hpp"
#include "../plan.hpp"
#include "../pool.hpp"
#include "../renderLand.hpp"
#include "../route/broker.hpp"
#include "../selection.hpp"
#include "../time.hpp"
#include "../util.hpp"
#include "../vehicle/wanderer.hpp"
#include "../zone.hpp"

#include "../parts/messageBoard.hpp"
#include "../tools/query.hpp"

#include "spdlog/spdlog.h"
#include <algorithm>

const float buildingGap = 0.05*tileSize;

Pool<Building>* buildings = Pool<Building>::newPool(20000);

const int maxTrackedXSize = 5;
float maxLandValues[numZoneTypes+1][maxTrackedXSize+2];
float maxDensity[numZoneTypes+1][maxTrackedXSize+2];
float maxFloors[numZoneTypes+1][maxTrackedXSize+2];
item maxLandValuesBuilding[numZoneTypes+1][maxTrackedXSize+2];
item maxDensityBuilding[numZoneTypes+1][maxTrackedXSize+2];
item maxFloorsBuilding[numZoneTypes+1][maxTrackedXSize+2];
static bool areIssuesIconsVisible = false;
static bool areBuildingsVisible = true;
item tallestBuilding = 0;
float tallestBuildingZ = -1;
item newestSkyscraper = 0;

void adjustBuildingStats(item ndx, float mult);
void repostTenancies(item bNdx);
void nameBuilding(item ndx);

vector<item> skyscrapers;

bool buildingsVisible() {
  return areBuildingsVisible;
}

bool issuesIconsVisible() {
  return areIssuesIconsVisible;
}

float getMaxLandValues(item zone, item xSize) {
  return maxLandValues[zone][xSize];
}

float getMaxDensity(item zone, item xSize) {
  return maxDensity[zone][xSize];
}

float getMaxFloors(item zone, item xSize) {
  return maxFloors[zone][xSize];
}

item getMaxLandValuesBuilding(item zone, item xSize) {
  return maxLandValuesBuilding[zone][xSize];
}

item getMaxDensityBuilding(item zone, item xSize) {
  return maxDensityBuilding[zone][xSize];
}

item getMaxFloorsBuilding(item zone, item xSize) {
  return maxFloorsBuilding[zone][xSize];
}

Building* getBuilding(item ndx) {
  if(ndx > buildings->size()) return NULL;
  return buildings->get(ndx);
}

void computeTallestBuilding() {
  item tallest = 0;
  float z = -1;
  for (int i = 1; i <= buildings->size(); i++) {
    Building* b = getBuilding(i);
    if (b == 0) continue;
    if (!(b->flags & _buildingExists)) continue;
    if (b->flags & _buildingCity) continue;
    float myZ = getDesignZ(b->design);
    if (myZ > z) {
      tallest = i;
      z = myZ;
    }
  }

  tallestBuilding = tallest;
  tallestBuildingZ = z;
}

item getTallestBuilding() {
  return tallestBuilding;
}

item addBuilding(item desgin, item edgeNdx,
  vec3 bLoc, vec3 normal, item zone);

Box getBuildingBox(item designNdx, vec3 loc, vec3 normal) {
  vec2 size = vec2(getDesign(designNdx)->size);
  //size.x *= 2;
  size -= vec2(buildingGap, buildingGap);
  vec2 normnorm = normalize(vec2(normal));
  vec2 loc2 = vec2(loc);

  if (isDesignAquatic(designNdx)) {
    float tide = getDesign(designNdx)->highTide;
    tide *= 0.5f;
    if (tide > c(CTileSize)*1) tide = c(CTileSize)*1;
    loc2 += normnorm * tide;
    size.x -= tide;
  }

  vec2 axis0 = normnorm * size.x;
  vec2 axis1 = vec2(normnorm.y, -normnorm.x) * size.y;
  vec2 corner = vec2(loc2) - axis1*.5f + normnorm * buildingGap *.5f;
  return box(corner, axis0, axis1);
}

Box getBuildingBox(item buildingNdx) {
  Building* b = getBuilding(buildingNdx);
  return getBuildingBox(b->design, b->location, b->normal);
}

void setMaxLandValues(item building) {
  if (buildings->size() < building) return;
  Building* b = getBuilding(building);
  if (b->flags & _buildingWasPlopped) return;
  if (!(b->flags & _buildingComplete)) return;
  if (b->flags & _buildingCity) return;

  Design* d = getDesign(b->design);
  if (getNumBusinesses(b->design) + getDesignNumHomes(b->design) == 0) {
    return;
  }
  float z = round(getDesignZ(b->design) / 4);
  item xSize = round(d->size.x/tileSize);
  xSize = clamp(xSize, 0, maxTrackedXSize);

  for (int i = 0; i <= xSize; i++) {
    if (d->minDensity > maxDensity[b->zone][i]) {
      maxDensity[b->zone][i] = d->minDensity;
      maxDensityBuilding[b->zone][i] = building;
    }

    if (d->minDensity > maxDensity[0][i]) {
      maxDensity[0][i] = d->minDensity;
      maxDensityBuilding[0][i] = building;
    }

    if (d->minLandValue > maxLandValues[b->zone][i]) {
      maxLandValues[b->zone][i] = d->minLandValue;
      maxLandValuesBuilding[b->zone][i] = building;
    }

    if (d->minLandValue > maxLandValues[0][i]) {
      maxLandValues[0][i] = d->minLandValue;
      maxLandValuesBuilding[0][i] = building;
    }

    if (z > maxFloors[b->zone][i]) {
      maxFloors[b->zone][i] = z;
      maxFloorsBuilding[b->zone][i] = building;
    }
  }
}

//item targetEmptyTenancies(item board) {
  //return numBuildings() * (board == Homes ? 0.01f : .005f) + 10;
//}

const char* placeBuildingLegal(item design, vec3 loc, vec3 unorm) {
  Design* d = getDesign(design);
  float mapSize = getMapSize();
  bool aquatic = isDesignAquatic(design);

  for (int y = 0; y < 2; y++) {
    for (int x = 0; x < 2; x++) {

      vec3 sideStep = ((y-.5f)*d->size.y) * zNormal(unorm);
      vec3 corner = loc + (d->size.x * x) * unorm + sideStep;
      if (corner.x < 0 || corner.y < 0 ||
          corner.x > mapSize || corner.y > mapSize) {
        return "Outside City Limits";
      }

      if (!aquatic || x == 0) {
        vec3 cornerOnLand = pointOnLand(corner);
        float cornerLandZ = cornerOnLand.z;
        if (cornerLandZ < 1) {
          return "Below Waterline";
        }

        if (abs(cornerLandZ - loc.z) > c(CZTileSize)*3) {
          return "Terrain Too Rough";
        }
      }

      if (aquatic) {
        float tideDist = x == 0 ? d->highTide : d->lowTide;
        vec3 tideCorner = loc + sideStep + (tideDist*unorm);
        vec3 cornerOnLand = pointOnLand(tideCorner);
        float landZ = cornerOnLand.z;
        if (x == 0 && landZ < -5) return "Too Close to Water";
        if (x == 1 && landZ > 2) return "Place Next to Water";
      }
    }
  }

  return NULL;
}

item maybeAddBuilding(item lotNdx) {
  Lot* lot = getLot(lotNdx);
  if (!(lot->flags & _lotExists)) return 0;
  if (lot->zone == 0) return 0;
  if (lot->zone == GovernmentZone) return 0;

  vec3 bLoc = lot->loc;
  item econ = getEcon(bLoc);
  //SPDLOG_INFO("maybeAddBuilding zoneDemand({}, {}) => {}",
      //econ, zoneName[lot->zone], zoneDemand(econ, lot->zone));
  //if (zoneDemand(econ, lot->zone) < randFloat()) return 0;

  // Find the location of the building
  item elemNdx = lot->elem; //nearestEdge(bLoc, false);
  if (elemNdx == 0) return 0;
  Line l = getLine(elemNdx);
  vec3 intersection = nearestPointOnLine(bLoc, l);
  float width = elemNdx < 0 ? elementWidth(elemNdx) : 0;
  width = std::max(width, c(CBuildDistance));
  if (vecDistance(bLoc, intersection) > tileSize*.5f+width) return 0;
  vec3 normal = lot->normal;
  normal.z = 0;
  vec3 unorm = normalize(normal);
  normal = unorm * width;
  bLoc = intersection + normal;

  // Get a design
  econ = getEcon(bLoc);
  float density = getAdjustedDensity(lot->zone, bLoc);
  float maxDensity = getLotMaxDensity(lotNdx)*.1f+.01f;
  density = clamp(density, 0.f, maxDensity);
  float landValue = getAdjustedLandValue(lot->zone, bLoc);
  item design = getRandomDesign(lot->zone, econ, density, landValue);
  //SPDLOG_INFO("maybeAddBuilding design:{} {}", design, econ);
  if (design == 0) return 0;

  if (isDesignAquatic(design)) {
    bLoc.z = c(CAquaticBuildingHeight);
  } else {
    bLoc = pointOnLand(bLoc);
    if (bLoc.z < beachLine/2) {
      return 0;
    }
  }

  // Checking the flatness of the land
  if (placeBuildingLegal(design, bLoc, unorm) != NULL) {
    return 0;
  }

  // Don't intersect with road
  Box bbox = getBuildingBox(design, bLoc, normal);
  if (graphIntersect(bbox, elemNdx, false)) return 0;

  vector<item> collisions = collideBuilding(bbox, 0);
  money valueDestroyed = 0;
  for (int i = 0; i < collisions.size(); i++) {
    Building* other = getBuilding(collisions[i]);
    if (!canRemoveBuilding(collisions[i])) return 0;
    valueDestroyed += getDesignValue(other->design, other->econ,
        landValue, density)
      * (other->zone == lot->zone ? 1 : 0.2)
      * (other->flags & _buildingAbandoned ? 0.2 : 1);
  }
  if (valueDestroyed*(.9f+.3f*randFloat()) >
      getDesignValue(design, econ, landValue, density)) return 0;

  /* printf("new %s building: %s destroyed, %s anticipated value\n",
      zoneName[lot->zone],
      printMoneyString(valueDestroyed),
      printMoneyString(getDesignValue(design, landValue, density)));
      */

  for (int i = 0; i < collisions.size(); i++) {
    removeBuilding(collisions[i]);
  }

  return addBuilding(design, elemNdx, bLoc, normal, lot->zone);
}

item addBuilding(int flags, vec3 bLoc, vec3 normal, item design, item zone,
    item econ) {
  item ndx = buildings->create();
  Building* building = getBuilding(ndx);

  building->flags = _buildingExists | flags;
  building->businesses.clear();
  building->families.clear();
  building->peopleInside.clear();
  building->lots.clear();
  building->color = randItem(numBuildingColors);

  if (isDesignAquatic(design)) bLoc.z = c(CAquaticBuildingHeight);
  building->location = bLoc;
  building->normal = normal;
  building->design = design;
  building->zone = zone;
  building->plan = 0;
  building->graphLoc.lane = 0;
  building->lastUpdateTime = getCurrentDateTime();
  building->builtTime = getCurrentDateTime();
  building->value = 0;
  building->name = 0;
  building->econ = econ;

  building->entity = 0;
  building->decoEntity = 0;
  building->iconEntity = 0;

  if (!(building->flags & _buildingCity)) {
    addToCollisionTable(BuildingCollisions, getBuildingBox(ndx), ndx);
  }

  return ndx;
}

item addBuilding(vec3 bLoc, vec3 normal, item design, item zone) {
  return addBuilding(0, bLoc, normal, design, zone, getEcon(bLoc));
}

item addCityBuilding(item cityNdx, vec3 loc, vec3 normal, item zone,
    float density, float landValue, bool render) {
  City* city = getCity(cityNdx);
  item econ = city->econ;

  item designNdx = getRandomDesign(zone, econ, density, landValue);
  if (designNdx == 0) return 0;
  if (isDesignAquatic(designNdx)) return 0;
  if (getDesign(designNdx)->flags & _designIsHotel) return 0;

  item ndx = addBuilding(_buildingCity | _buildingComplete, loc, normal,
       designNdx, zone, econ);
  Building* building = getBuilding(ndx);
  GraphLocation graphLoc;
  graphLoc.lane = getCity(cityNdx)->laneBlock;
  graphLoc.dap = 0;
  building->graphLoc = graphLoc;

  adjustStat(building->econ, BuildingsConstructed, 1);
  adjustBuildingStats(ndx, 1);
  updateBuildingValue(ndx);
  repostTenancies(ndx);
  nameBuilding(ndx);
  if (render) renderBuilding(ndx);

  return ndx;
}

item addGovernmentBuilding(item lot, item designNdx) {
  vec3 loc = getLot(lot)->loc;
  item elemNdx = nearestElement(loc, false);
  if (elemNdx == 0) {
    return 0;
  }
  Line l = getLine(elemNdx);
  vec3 intersection = nearestPointOnLine(loc, l);
  //float width = elementWidth(elemNdx);
  //if (elemNdx < 0) {
    //width *= 2;
  //}
  if (vecDistance(loc, intersection) > tileSize*3) {
    return 0;
  }
  vec3 normal = loc - intersection;
  normal.z = 0;
  normal = normalize(normal) * c(CBuildDistance);
  loc = pointOnLand(intersection + normal);

  if (designNdx == 0) {
    return 0;
  }

  item ndx = addBuilding(loc, normal, designNdx, getDesign(designNdx)->zone);
  Building* building = getBuilding(ndx);
  building->graphLoc = graphLocation(loc);
  building->color = 0;
  renderBuilding(ndx);

  return ndx;
}

item addBuilding(item designNdx, item edgeNdx,
  vec3 bLoc, vec3 normal, item zone) {

  item ndx = addBuilding(bLoc, normal, designNdx, zone);

  Building* building = getBuilding(ndx);
  building->graphLoc = graphLocation(bLoc);

  completeBuilding(ndx);

  return ndx;
}

const char* buildingLegalMessage(item ndx) {
  Building* building = getBuilding(ndx);
  if (!building->flags & _buildingExists) {
    return "Building doesn't exist";
  }
  if (building->flags & _buildingComplete) {
    return NULL;
  }

  if (building->zone == GovernmentZone) {
    const char* res = governmentBuildingLegalMessage(ndx);
    if (res != NULL) return res;
  }


  const char* res = placeBuildingLegal(building->design, building->location,
        normalize(building->normal));
  if (res != NULL) return res;

  /*
  if (!isDesignAquatic(building->design) &&
      building->location.z < beachLine/2) {
    return "Below Waterline";
  }
  */

  bool collide = true;
  if (building->plan != 0) {
    Plan* plan = getPlan(building->plan);
    if (plan->flags & _planForceDemolish) {
      collide = false;
    }
  }

  if (collide) {
    Box bbox = getBuildingBox(ndx);
    if (graphIntersect(bbox, false)) {
      return "Road in the Way";
    }

    vector<item> collisions = collideBuilding(bbox, ndx);
    for (int i = 0; i < collisions.size(); i++) {
      if (!canRemoveBuilding(collisions[i])) {
        return "Another Building is in the Way";
      }
    }
  }

  return NULL;
}

bool buildingIsSkyscraper(item ndx) {
  Building* building = getBuilding(ndx);
  if (!(building->flags & _buildingExists)) return false;
  Design* design = getDesign(building->design);
  return design->minDensity*10+0.1 > c(CSkyscraperMinDensity) && design->size.z >= c(CSkyscraperMinHeight) && (building->flags & _buildingComplete);
}

item getNewestSkyscraper() {
  return newestSkyscraper;
}

void addSkyscraper(item ndx) {
  Building* building = getBuilding(ndx);
  if (!(building->flags & _buildingExists)) return;
  if (building->flags & _buildingCity) return;
  if (!(building->flags & _buildingComplete)) return;
  Design* design = getDesign(building->design);

  // Find out if this is a new city center
  bool anyClose = false;
  for (int i = 0; i < skyscrapers.size(); i++) {
    item skyNdx = skyscrapers[i];
    if (skyNdx <= 0 || skyNdx > sizeBuildings()) continue;
    Building* other = getBuilding(skyscrapers[i]);
    if (!(other->flags & _buildingExists) || (other->flags & _buildingCity) || !(other->flags & _buildingComplete)) continue;

    float dist = distance2DSqrd(other->location, building->location);
    if (dist < c(CCityDistance) * c(CCityDistance) * 4) {
      anyClose = true;
      break;
    }
  }

  if (!anyClose) {
    adjustStat(ourCityEconNdx(), CityCenters, 1);
  }

  skyscrapers.push_back(ndx);
}

void adjustBuildingStats(item ndx, float mult) {
  Building* building = getBuilding(ndx);
  if (!(building->flags & _buildingExists)) return;
  Design* design = getDesign(building->design);
  item e = building->econ;

  if (!(building->flags & _buildingCity) &&
      (building->flags & _buildingComplete)) {
    design->numBuildings += mult;
  }

  if (building->zone == GovernmentZone) {
    item stat = NumParks + design->category*2;
    adjustStat(e, NumGovBuildings, 1*mult);
    adjustStat(e, stat, 1*mult);
    if (building->flags & _buildingEnabled) {
      adjustStat(e, NumGovBuildingsEnabled, 1*mult);
      adjustStat(e, stat+1, 1*mult);
    }

  } else {
    adjustStat(e, AggregatePropertyValue, building->value*mult);
  }

  adjustStat(e, NumResidentialBuildings + building->zone - 1, 1*mult);
  if (building->flags & _buildingAbandoned) {
    adjustStat(e, NumAbandoned, 1*mult);
    adjustStat(e, AbandonedResidentialBuildings + building->zone - 1, 1*mult);
  }

  if (isDesignEducation(building->design)) {
    //int numHomes = getDesignNumHomes(building->design);
    //adjustStat(e, NumCollegeBunks, numHomes);

  } else {
    if (design->flags & _designIsHotel) {
      adjustStat(e, NumHotelRooms, design->numFamilies*mult);
    } else {
      adjustStat(e, NumHomes, design->numFamilies*mult);
      if (design->numFamilies > 1) {
        adjustStat(e, NumMultiFam, design->numFamilies*mult);
      } else {
        adjustStat(e, NumSingleFam, design->numFamilies*mult);
      }
    }
    adjustStat(e, NumFactories, design->numBusinesses[Factory]*mult);
    adjustStat(e, NumFarms, design->numBusinesses[Farm]*mult);
    adjustStat(e, NumShops, design->numBusinesses[Retail]*mult);
    adjustStat(e, NumOffices, design->numBusinesses[Office]*mult);
  }

  if (buildingIsSkyscraper(ndx)) {
    adjustStat(e, Skyscrapers, mult);

    if (!(building->flags & _buildingCity)) {
      if (newestSkyscraper > 0 && newestSkyscraper <= sizeBuildings()) {
        Building* other = getBuilding(newestSkyscraper);
        if (!(other->flags & _buildingExists) || other->builtTime < building->builtTime) {
          newestSkyscraper = ndx;
        }
      } else {
        newestSkyscraper = ndx;
      }

      if (mult > 0) {
        addSkyscraper(ndx);
      } else {
        removeFromVector(&skyscrapers, ndx);
      }
    }
  }
}

void repostTenancies(item bNdx) {
  Building* b = getBuilding(bNdx);
  if(!(b->flags & _buildingEnabled) || (b->flags & _buildingAbandoned)) return;
  Design* design = getDesign(b->design);
  int numBiz[numBusinessTypes];

  if (isDesignEducation(b->design)) {
    int numHomes = getDesignNumHomes(b->design);
    numHomes -= int(b->families.size());
    if (numHomes > 0) {
      boardPut(b->econ, DormBunks, bNdx, numHomes);
      supplyTableSuggest_g(b->graphLoc.lane, SuppliesDorm, bNdx);
    }
    for (int i = 0; i < numBusinessTypes; i++) {
      numBiz[i] = 0;
    }
    numBiz[Retail] = design->numBusinesses[CSStorefronts];
    numBiz[Institution] = design->numBusinesses[CSColleges];

  } else {
    int numHomes = design->numFamilies - int(b->families.size());
    if (numHomes > 0) {
      if (design->flags & _designIsHotel) {
        boardPut(b->econ, HotelRooms, bNdx, numHomes);
        supplyTableSuggest_g(b->graphLoc.lane, SuppliesHotel, bNdx);
      } else {
        boardPut(b->econ, Homes, bNdx, numHomes);
        supplyTableSuggest_g(b->graphLoc.lane, SuppliesHome, bNdx);
      }
    }
    for (int i = 0; i < numBusinessTypes; i++) {
      numBiz[i] = design->numBusinesses[i];
    }
  }

  for (int i = 0; i < b->businesses.size(); i++) {
    BusinessType type = getBusinessType(b->businesses[i]);
    numBiz[type] --;
  }

  for (int i = 0; i < numBusinessTypes; i++) {
    boardPut(b->econ, RetailTenancies+i, bNdx, numBiz[i]);
  }
}

void rebuildBuildingStats() {
  /*
  for (int i = 1; i <= buildings->size(); i++) {
    Building* b = buildings->get(i);
    updateBuildingValue(i);
  }
  */

  skyscrapers.clear();
  newestSkyscraper = 0;
  resetStat(NumGovBuildings);
  resetStat(NumGovBuildingsEnabled);
  resetStat(NumParks);
  resetStat(NumParksEnabled);
  resetStat(NumSchools);
  resetStat(NumSchoolsEnabled);
  resetStat(NumCommServices);
  resetStat(NumCommServicesEnabled);
  resetStat(NumUniBuildings);
  resetStat(NumUniBuildingsEnabled);
  resetStat(NumHomes);
  resetStat(NumSingleFam);
  resetStat(NumMultiFam);
  resetStat(NumFactories);
  resetStat(NumFarms);
  resetStat(NumShops);
  resetStat(NumOffices);
  resetStat(AggregatePropertyValue);
  resetStat(NumHotelRooms);
  resetStat(Skyscrapers);
  resetStat(CityCenters);

  for (int i = 1; i < numZoneTypes; i++) {
    resetStat(NumResidentialBuildings+i-1);
  }

  for (int i = 1; i <= buildings->size(); i++) {
    Building* b = buildings->get(i);
    adjustBuildingStats(i, 1);
    repostTenancies(i);
  }
  rebuildAmenityStats();
}

void nameBuilding(item ndx) {
  Building* b = getBuilding(ndx);
  Design* d = getDesign(b->design);

  if (b->name != 0) {
    free(b->name);
    b->name = 0;
  }

  if (d->zone == GovernmentZone) {
    b->name = sprintf_o("%s %s", randomName(AmenityName), d->displayName);
    return;
  }

  if (b->value < c(CBuildingNameworthyValue) * getInflation()) {
    return;
  }

  const char* name = randomName(BuildingName);
  if (b->zone == ResidentialZone) {
    if (getDesignZ(b->design)/4 > 10) {
      b->name = sprintf_o("%s Tower", name);
    } else if (d->numFamilies > 3) {
      b->name = sprintf_o("%s Apartments", name);
    } else {
      b->name = sprintf_o("%s Manor", name);
    }
  } else if (b->zone == FarmZone) {
    b->name = sprintf_o("%s Ranch", name);
  } else if (d->size.x > 3*tileSize) {
    b->name = sprintf_o("%s Complex", name);
  } else {
    b->name = sprintf_o("%s Building", name);
  }
}

void completeBuilding(item ndx) {
  Building* building = getBuilding(ndx);
  if (!(building->flags & _buildingExists)) return;
  if (building->flags & _buildingComplete) return;
  building->flags |= _buildingComplete | _buildingEnabled;

  vec3 bLoc = building->location;
  vec3 normal = building->normal;
  Design* design = getDesign(building->design);
  vec3 unorm = normalize(normal);
  Box bbox = getBuildingBox(ndx);

  if (getGameMode() != ModeDesignOrganizer) {
    vector<item> collisions = collideBuilding(bbox, ndx);
    for (int i = 0; i < collisions.size(); i++) {
      removeBuilding(collisions[i]);
    }

    if (building->plan != 0) {
      Plan* p = getPlan(building->plan);
      if (p->flags & _planForceDemolish) {
        vector<item> graphCollisions = getGraphCollisions(bbox);
        for (int i = 0; i < graphCollisions.size(); i++) {
          item graphNdx = graphCollisions[i];
          if (graphNdx > 0) {
            Edge* e = getEdge(graphNdx);
            if (!(e->flags & _graphUnderground)) {
              trimEdge(graphNdx, bbox);
            }
          }
        }
      }
    }
  }

  addBuildingElevator(ndx, true);

  vector<item> lots = collidingLots(bbox);
  occupyLots(lots, ndx);
  addSpawnPoints(ndx);

  if (building->zone == GovernmentZone) {
    addBuildingEffect(building->design);
    heatMapAdd(Prosperity, building->location, design->cost/1000000 *
        c(CNewAmenityProsperity));
    addGovernmentBuilding(ndx);
  } else {
    heatMapAdd(Prosperity, bLoc, c(CNewBuildingProsperity));
  }

  if (!(building->flags & _buildingCity)) {
    float myZ = getDesignZ(building->design);
    if (myZ > tallestBuildingZ) {
      tallestBuilding = ndx;
      tallestBuildingZ = myZ;
    }
  }

  setMaxLandValues(ndx);
  adjustStat(building->econ, BuildingsConstructed, 1);
  adjustBuildingStats(ndx, 1);
  updateBuildingValue(ndx);
  repostTenancies(ndx);
  nameBuilding(ndx);
  renderBuilding(ndx);
}

void removeBuilding(item ndx) {
  Building* building = getBuilding(ndx);
  if (!building->flags & _buildingExists) {
    return;
  }

  deselect(SelectionBuilding, ndx);
  removeMessageByObject(BuildingMessage, ndx);
  supplyTableErase_g(building->graphLoc.lane, SuppliesDorm, ndx);
  supplyTableErase_g(building->graphLoc.lane, SuppliesHome, ndx);
  removeSpawnPoints(ndx);

  if (building->zone == GovernmentZone) {
    removeGovernmentBuilding(ndx);
  }

  if (building->plan != 0) {
    removePlan(building->plan);
    building->plan = 0;
  }

  for (int i = building->businesses.size()-1; i >= 0; i--) {
    removeBusiness(building->businesses[i]);
  }
  vector<item> prevFamilies(
      building->families.begin(), building->families.end());
  for (int i = building->families.size()-1; i >= 0; i--) {
    evictFamily(building->families[i], false);
  }

  vector<item> prevInside = building->peopleInside;
  for (int i = 0; i < prevInside.size(); i++) {
    removePersonFromLocation(prevInside[i]);
  }

  if (building->flags & _buildingComplete) {
    removeBuildingElevator(ndx);
    adjustBuildingStats(ndx, -1);
  }

  removeFromCollisionTable(BuildingCollisions, ndx);
  for (int i = building->lots.size()-1; i >= 0; i--) {
    unoccupyLot(building->lots[i]);
  }
  building->lots.clear();

  removeEntity(building->entity);
  removeEntity(building->decoEntity);
  if (building->iconEntity != 0) {
    removeEntity(building->iconEntity);
  }

  for (int i = 0; i < numBusinessTypes+1; i++) {
    boardClean(building->econ, i, ndx);
  }
  boardClean(building->econ, Homes, ndx);
  boardClean(building->econ, DormBunks, ndx);
  boardClean(building->econ, HotelRooms, ndx);

  building->econ = 0;
  building->flags = 0;
  building->value = 0;
  free(building->name);
  building->name = 0;
  buildings->free(ndx);

  for (int i = 0; i < prevFamilies.size(); i++) {
    handleHome(prevFamilies[i]);
  }

  if (ndx == tallestBuilding) {
    computeTallestBuilding();
  }
}

void removeFamilyFromHome(item buildingNdx, item familyNdx) {
  Building* building = getBuilding(buildingNdx);
  Design* design = getDesign(building->design);
  int numHomes = getDesignNumHomes(building->design);
  bool isEdu = isDesignEducation(building->design);
  bool isHotel = design->flags & _designIsHotel;

  for (int i = building->families.size() - 1; i >= 0; i--) {
    if (building->families[i] == familyNdx) {
      building->families.erase(building->families.begin() + i);
      if (building->families.size() < numHomes) {
        boardPut(building->econ, isEdu ? DormBunks :
            isHotel ? HotelRooms : Homes, buildingNdx);
        supplyTableSuggest_g(building->graphLoc.lane,
            isEdu ? SuppliesDorm :
            isHotel ? SuppliesHotel : SuppliesHome, buildingNdx);
      }
    }
  }
}

void removeBusinessFromTenancy(item buildingNdx, item businessNdx) {
  Building* building = getBuilding(buildingNdx);
  BusinessType bType = getBusinessType(businessNdx);
  Design* design = getDesign(building->design);
  for (int i = building->businesses.size() - 1; i >= 0; i--) {
    if (building->businesses[i] == businessNdx) {
      building->businesses.erase(building->businesses.begin() + i);
      boardPut(building->econ, RetailTenancies+bType, buildingNdx);
    }
  }
}

void removeCollidingBuildings(Box b) {
  vector<item> collisions = collideBuilding(b, 0);
  for (int i = 0; i < collisions.size(); i++) {
    item bNdx = collisions[i];
    Building* b = getBuilding(bNdx);
    if (!canRemoveBuilding(bNdx)) continue;
    removeBuilding(bNdx);
  }
}

vector<item> collideBuilding(item buildingNdx) {
  return collideBuilding(getBuildingBox(buildingNdx), buildingNdx);
}

vector<item> collideBuilding(Box box0, item self) {
  return getCollisions(BuildingCollisions, box0, self);

  /*
  vector<item> result;

  for (int i=1; i <= buildings->size(); i++) {
    if (i == self) {
      continue;
    }

    Building* b = getBuilding(i);
    if (b->flags & _buildingExists) {
      Box box1 = getBuildingBox(i);
      if (boxIntersect(box0, box1)) {
        result.push_back(i);
      }
    }
  }

  return result;
  */
}

IssueIcon getBuildingIssue(item ndx) {
  Building* building = getBuilding(ndx);
  IssueIcon bestIcon = NoIssue;
  item numJobless = 0;
  item numWorkers = 0;

  for (int i = 0; i < building->families.size(); i++) {
    item famNdx = building->families[i];
    Family* fam = getFamily(famNdx);
    IssueIcon ico = getFamilyIssue(famNdx);
    if (ico > bestIcon) {
      bestIcon = ico;
    }

    /*
    for (int i = 0; i < fam->members.size(); i++) {
      Person* p = getPerson(fam->members[i]);
      if (p->flags & _personIsWorker) {
        numWorkers ++;
        if (p->employer == 0) {
          numJobless ++;
        }
      }
    }
    */
  }

  for (int i = 0; i < building->businesses.size(); i++) {
    item busNdx = building->businesses[i];
    IssueIcon ico = getBusinessIssue(busNdx);
    if (ico > bestIcon) {
      bestIcon = ico;
    }
  }

  /*
  float unemploymentRate = getStatistic(building->econ, UnemploymentRate);
  if (numJobless > 0) {
    SPDLOG_INFO("jobless {}/{} buildingUnemp {}%",
        numWorkers, numJobless, numJobless*100/numWorkers);
  }
  if ((numWorkers+10) * unemploymentRate < numJobless &&
      randFloat() < 0.25) {
  //if (numWorkers * unemploymentRate * 10 * randFloat() < numJobless) {
    bestIcon = Jobless;
  }
  */

  return bestIcon;
}

bool canRemoveBuilding(item ndx) {
  Building* building = getBuilding(ndx);
  return getGameMode() == ModeGame &&
    !(building->flags & _buildingHistorical) &&
    !(building->zone == GovernmentZone);
}

void updateBuilding(item ndx) {
  Building* building = getBuilding(ndx);
  if (!(building->flags & _buildingExists)) return;
  if (!(building->flags & _buildingComplete)) return;

  bool forcePaint = false; // Should we force paint the building at the end?
  bool isGame = getGameMode() == ModeGame;
  float time = getCurrentDateTime();
  float duration = (time - building->lastUpdateTime) * gameDayInRealSeconds;
  building->lastUpdateTime = time;

  Design* design = getDesign(building->design);
  const float invTileSqrd = 1.0/tileSize/tileSize;
  float size = design->size.x * design->size.y * invTileSqrd;
  vec3 loc = building->location;
  vec3 center = getBuildingCenter(ndx);
  bool isDorm = isDesignEducation(building->design);

  if (isGame) {
    // Reparent the building
    if (isGame && randFloat() < 0.01) {
      Configuration config;
      config.flags = 0;
      config.type = ConfigTypeRoad;
      GraphLocation newLoc = graphLocation(loc, config);
      if (newLoc.lane/10 != building->graphLoc.lane/10) {
        supplyTableErase_g(building->graphLoc.lane, SuppliesHome, ndx);
        supplyTableErase_g(building->graphLoc.lane, SuppliesDorm, ndx);
      }
      building->graphLoc = newLoc;

      // See if the building is too far away from road
      if (canRemoveBuilding(ndx) && !(building->flags & _buildingCity)) {
        item elem = getElement(newLoc);
        vec3 roadLoc = getLocation(newLoc);
        float dist = vecDistance(loc, roadLoc) - buildDistance(elem);
        if (dist > tileSize || dist < -.5f*tileSize) {
          removeBuilding(ndx);
          return;
        }
      }
    }

    if (isDorm) {
      int numStudents = getDesignNumHomes(building->design);
      float pollution = numStudents * c(CStudentPollution);
      pollution += design->numBusinesses[CSStudents] * c(CStudentPollution);
      pollution += design->numBusinesses[CSStorefronts]*c(CCommercialPollution);
      pollution += design->numBusinesses[CSColleges] * c(CCommercialPollution);
      heatMapAdd(Pollution, center, duration*pollution);

    } else {
      int numHomes = getDesignNumHomes(building->design);
      float pollution = numHomes * c(CFamilyPollution);
      pollution += design->numBusinesses[Factory] * c(CFactoryPollution);
      pollution += design->numBusinesses[Farm] * c(CFarmPollution);
      pollution += design->numBusinesses[Retail] * c(CCommercialPollution);
      pollution += design->numBusinesses[Office] * c(CCommercialPollution);
      pollution += design->numBusinesses[Institution] * c(CCommercialPollution);
      heatMapAdd(Pollution, center, duration*pollution);
    }

    if (building->zone == GovernmentZone) {
      updateGovernmentBuilding(ndx, duration);
    }

    for (int i = building->businesses.size() -1; i >= 0; i--) {
      updateBusiness(building->businesses[i], duration, center);
    }
  }

  //Turn on or off lights
  bool lights = building->flags & _buildingLights;
  bool forceLights = false;
  int numBiz = getNumBusinesses(building->design);

  if (getGameMode() == ModeBuildingDesigner) {
    lights = true;
    forceLights = true;

  } else if (getGameMode() == ModeDesignOrganizer) {
    lights = getLightLevel() < 0.5;
    forceLights = true;

  } else if (building->peopleInside.size() == 0) {
    lights = false;
    forceLights = true;

  } else if (getLightLevel() < 0.6) {
    int num = building->peopleInside.size();
    if (num > 20) {
      lights = true;
      if (num > 100) forceLights = true;
    } else if (design->numFamilies <= 0 && numBiz <= 0) {
      lights = true;
    } else {
      lights = false;
      for (int i = 0; i < building->peopleInside.size(); i++) {
        Person* person = getPerson(building->peopleInside[i]);
        if (person->activity != SleepActivity || randFloat() < 0.1) {
          lights = true;
          break;
        }
      }
    }
  } else {
    lights = false;
  }

  if (isGame) {
    bool wasAbandoned = building->flags & _buildingAbandoned;
    BudgetLine budgetLine =
      getBudgetLineFromBuildingCategory(design->category);
    float controlEffect = getBudgetControlEffect(budgetLine);
    float prosp = heatMapGet(Prosperity, center);
    float designVal = design->minLandValue;
    float heatmapVal = heatMapGet(Value, center);

    bool abandoned = building->zone == GovernmentZone ?
      controlEffect <= 0.0f || !(building->flags & _buildingEnabled) :
      numPeople(ourCityEconNdx()) >= 100 &&
      (design->numFamilies > 0 || numBiz > 0) &&
      building->businesses.size() == 0 &&
      building->families.size() == 0 &&
      (wasAbandoned || randFloat() < duration*.001f);

    /*
    if (building->zone != GovernmentZone &&
        (heatmapVal < design->minLandValue-0.2 ||
         prosp < .5f*pow(randFloat(),4))
        && (wasAbandoned || randFloat() < duration*.001f)) {
      //SPDLOG_INFO("forced abandon");
      abandoned = true;
    }
    */

    item numFam = design->numFamilies;
    if ((design->flags & _designIsHotel) &&
        building->zone != GovernmentZone &&
        numFam > 0 && building->families.size() <= numFam*.5f &&
        getStatistic(building->econ, NumEmptyHotelRooms) > numFam*2) {
      abandoned = true;
    }

    if (abandoned & !wasAbandoned) {
      forcePaint = true;
      building->flags |= _buildingAbandoned;
      if (building->zone != GovernmentZone) {
        adjustStat(building->econ, NumAbandoned, 1);
        adjustStat(building->econ,
            AbandonedResidentialBuildings + building->zone - 1, 1);
      }
      for (int i = building->peopleInside.size()-1; i >= 0; i--) {
        removePersonFromLocation(building->peopleInside[i]);
      }
      for (int i = building->businesses.size()-1; i >= 0; i--) {
        removeBusiness(building->businesses[i]);
      }

    } else if (!abandoned && wasAbandoned) {
      forcePaint = true;
      building->flags &= ~_buildingAbandoned;
      if (building->zone != GovernmentZone) {
        adjustStat(building->econ, NumAbandoned, -1);
        adjustStat(building->econ,
            AbandonedResidentialBuildings + building->zone - 1, -1);
      }
    }

    float dens = 0;
    if (abandoned) {
      dens += c(CAbandonedDensity);
      heatMapAdd(Crime, center, c(CAbandonedCrime)*duration);
      forceLights = true;
      lights = false;
      heatMapAdd(Prosperity, center, c(CAbandonedProsperity) * duration);
      heatMapAdd(CommunityHM, center, c(CAbandonedCommunity) * duration);
      heatMapAdd(Value, center, c(CAbandonedBuildingValue) * duration);
      if (canRemoveBuilding(ndx) &&
          randFloat() < duration / gameDayInRealSeconds / oneYear) {
        removeBuilding(ndx);
        return;
      }
    }

    float height = getNaturalHeight(loc);
    float coastStart = c(CCommunityCoastEnd);
    float coastCommAdd = c(CCommunityCoastTarget);
    item zoneVal = building->zone;
    float community =
        zoneVal == OfficeZone ? c(CCommunityOfficeBuildingEffect) :
        zoneVal == MixedUseZone ? c(CCommunityMixedUseBuildingEffect) :
        zoneVal == FactoryZone ? c(CCommunityFactoryBuildingEffect) :
        zoneVal == FarmZone ? c(CCommunityFarmBuildingEffect) :
        zoneVal == RetailZone ? c(CCommunityRetailBuildingEffect) :
        zoneVal == ResidentialZone ? (
            c(CCommunityResidentialDens10Effect)*design->minDensity +
            c(CCommunityResidentialDens0Effect)*(1-design->minDensity)) : 0;
    if(height < coastStart) {
      community += (coastStart / height) * coastCommAdd;
    }
    if (community != 0) {
      float angle = randFloat(0, pi_o * 2);
      float mag = randFloat(0, 1);
      vec3 thrw = vec3(sin(angle), cos(angle), 0.f) * mag;

      heatMapAdd(CommunityHM, center + thrw*c(CCommunityThrow), community * duration);
    }

    if (heatmapVal < designVal-0.1 && !abandoned) {
      heatMapAdd(Value, center, duration * c(CNiceBuildingValue));
    } else if (heatmapVal > designVal+0.1) {
      heatMapAdd(Value, center, duration * c(CBadBuildingValue));
    }

    int numHomes = getDesignNumHomes(building->design);
    int empties = numBiz - building->businesses.size() +
      numHomes - building->families.size();
    dens += empties * c(CEmptyTenancyDensity);
    if (dens > 0) {
      heatMapAdd(Density, center, duration * dens);
    }

    Supply supplies = isDorm ? SuppliesDorm : SuppliesHome;
    if (empties > 0) {
      supplyTableSuggest_g(building->graphLoc.lane, supplies, ndx);
    } else {
      supplyTableErase_g(building->graphLoc.lane, supplies, ndx);
    }

    updateBuildingValue(ndx);
  }

  if (forceLights || randFloat()*oneHour*gameDayInRealSeconds < duration) {
    if (lights) {
      if (building->entity != 0) {
        setEntityIlluminated(building->entity, true);
        setEntityIlluminated(building->decoEntity, true);
      }
      building->flags |= _buildingLights;
    } else {
      if (building->entity != 0) {
        setEntityIlluminated(building->entity, false);
        setEntityIlluminated(building->decoEntity, false);
      }
      building->flags &= ~_buildingLights;
    }
  }

  if (forcePaint || randFloat()*oneHour*gameDayInRealSeconds < duration) {
    paintBuilding(ndx);
  }
}

void makeBuildingPayments(Budget* budget, double duration) {
  transaction(budget, PropertyTax,
      duration*getTaxRate(PropertyTax)*
      getStatistic(ourCityEconNdx(), AggregatePropertyValue));
  makeGovernmentBuildingPayments(budget, duration);

  /*
  for (int i = 1; i <= buildings->size(); i++) {
    Building* building = getBuilding(i);
    if (!building->flags & _buildingExists) {
      continue;
    }

    if (building->zone == Government) {
      makeGovernmentBuildingPayments(i, budget, duration);

    } else {
      transaction(budget, (BudgetLine)building->zone,
          getBuildingTaxes(i)*duration);
    }
  }
  */
}

money getCityAssetValue() {
  money result = 0;
  for (int i = 1; i <= buildings->size(); i++) {
    Building* building = getBuilding(i);
    if (!building->flags & _buildingExists) {
      continue;
    }

    if (building->zone == GovernmentZone) {
      result += building->value;
    }
  }

  return result;
}

money getBuildingTaxes(item ndx) {
  Building* b = getBuilding(ndx);
  if (b->flags & _buildingAbandoned) return 0;
  return getTaxRate(PropertyTax) * b->value;
}

money getBuildingValue(item ndx) {
  Building* b = getBuilding(ndx);
  if (!(b->flags & _buildingExists) || !(b->flags & _buildingComplete)) {
      //(b->flags & _buildingCity)) {
    return 0;
  }
  return b->value;
}

void updateBuildingValue(item buildingNdx) {
  Building* b = getBuilding(buildingNdx);
  if (!(b->flags & _buildingExists) || !(b->flags & _buildingComplete)) {
      //(b->flags & _buildingCity)) {
    b->value = 0;
    return;
  }

  /*
  if (b->businesses.size() == 0 && b->families.size() == 0) { //Abandoned
    return 0;
  }
  */

  Design* d = getDesign(b->design);
  float value = heatMapGet(Value, b->location);
  value = std::max(0.f, value);
  float designValue = getDesignValue(b->design);
  float density = heatMapGet(Density, b->location);
  float multiplier = 1;
  if (value < d->minLandValue) {// || density < design->minDensity) {
    multiplier = 0.5f;
  }

  float lastValue = b->value;
  b->value = getInflation()
    * (b->flags & _buildingAbandoned ? 0.1f : 1.f)
    //* (1 - 2.f*targetUnemploymentRate(b->econ))
    * (d->size.x * d->size.y * c(CLandValue) * value + designValue)
    * multiplier;

  adjustStat(b->econ, AggregatePropertyValue, b->value - lastValue);
}

vec3 getBuildingCenter(item ndx) {
  Building* b = getBuilding(ndx);
  Design* d = getDesign(b->design);
  return b->location + d->size.x * normalize(b->normal) * .5f;
}

vec3 getBuildingTop(item ndx) {
  Building* b = getBuilding(ndx);
  Design* d = getDesign(b->design);
  float top = getDesignZ(b->design);
  return b->location + d->size.x * normalize(b->normal) * .5f + vec3(0,0,top);
}

item currentBuilding = 0;
float buildingsToAdd = 0;
float numToProcess = 0;

void updateBuildings(double duration) {
  int numBuildings = buildings->size();
  numToProcess += duration * numBuildings / c(CBuildingUpdateTime);
  bool isDO = getGameMode() == ModeDesignOrganizer;
  if (isDO) numToProcess = duration * numBuildings;
  if (numToProcess > numBuildings) numToProcess = numBuildings;

  if (sizeBuildings() > 1000 && !isDO) {
    queueTask_g(TaskUpdate1000Buildings, duration);
    numToProcess -= 1000;
  } else {
    while (numToProcess > 0) {
      currentBuilding = currentBuilding % buildings->size() + 1;
      updateBuilding(currentBuilding);
      numToProcess --;
    }
  }

  /*
  int numThisFrame = numToProcess < c(CBuildingUpdateSpread) ?
    numToProcess : numToProcess / c(CBuildingUpdateSpread);
  numToProcess -= numThisFrame;
  for (int i = 0; i < numThisFrame; i++) {
    currentBuilding = currentBuilding % buildings->size() + 1;
    updateBuilding(currentBuilding);
  }
  */

  if (getGameMode() != ModeGame) return;

  buildingsToAdd += duration *
    (numBuildings * c(CBuildingAddRate) + c(CBuildingAddRateBase));

  while (buildingsToAdd > 1) {
    queueTask_g(TaskAddBuilding, duration);
    buildingsToAdd --;
  }
}

void update1000Buildings(double duration) {
  for (int i = 0; i < 1000; i++) {
    currentBuilding = currentBuilding % buildings->size() + 1;
    updateBuilding(currentBuilding);
  }
}

void addOneBuilding(double duration) {
  item needZone = 0;
  float zoneNeed = 0;
  for (int i = 1; i < numZoneTypes; i++) {
    float d = zoneDemand(ourCityEconNdx(), i) * randFloat();
    if (d > zoneNeed) {
      needZone = i;
      zoneNeed = d;
    }
  }

  //SPDLOG_INFO("addBuilding needZone:{} zoneNeed:{}, buildingsToAdd:{}",
      //zoneName[needZone], zoneNeed, buildingsToAdd);

  float rnd1 = randFloat();
  if (zoneNeed*zoneNeed < rnd1) return;

  //SPDLOG_INFO("updateBuildings numThisFrame {} buildingsToAdd {}",
      //numThisFrame, buildingsToAdd);

  for (int k = 0; k < 10; k++) {
    item lot = randFloat() < c(CBuildingUpgradeRate) ? randomLot() :
      getEmptyLot(needZone);
    if (lot == 0) continue;
    //if (heatMapGet(Density, getLot(lot)->loc) <= k) continue;
    item bNdx = maybeAddBuilding(lot);
    if (bNdx != 0) break;
  }
}

int numBuildings() {
  return buildings->count();
}

int sizeBuildings() {
  return buildings->size();
}

void resetBuildings() {
  areIssuesIconsVisible = alwaysShowIssuesIcons();
  areBuildingsVisible = true;
  currentBuilding = 0;
  tallestBuildingZ = -1;
  tallestBuilding = 0;
  newestSkyscraper = 0;

  for (int i = 1; i <= buildings->size(); i++) {
    Building* building = getBuilding(i);
    building->families.clear();
    building->businesses.clear();
    building->peopleInside.clear();
    building->value = 0;
    free(building->name);
    building->name = 0;
  }
  buildings->clear();

  for (int i = 0; i < numZoneTypes+1; i++) {
    for (int k = 0; k < maxTrackedXSize; k++) {
      maxLandValues[i][k] = 0;
      maxDensity[i][k] = 0;
      maxFloors[i][k] = 0;
      maxLandValuesBuilding[i][k] = 0;
      maxDensityBuilding[i][k] = 0;
      maxFloorsBuilding[i][k] = 0;
    }
  }

  resetBuildingRender();
}

void initBuildingsEntities() {
  for (int i = 1; i <= buildings->size(); i++) {
    Building* building = getBuilding(i);
    if ((building->flags & _buildingExists) && building->entity == 0) {
      building->entity = addEntity(BuildingShader);
      building->decoEntity = addEntity(DecoShader);
      IssueIcon issue = c(CShowIssueIcons) ? getBuildingIssue(i) : NoIssue;
      if (issue != NoIssue) {
        building->iconEntity = addEntity(WSUIInstancedShader);
      }

      if (getDesign(building->design)->flags & _designHasStatue) {
        Entity* entity = getEntity(building->entity);
        Entity* decoEntity = getEntity(building->decoEntity);
        entity->mesh = addMesh();
        entity->simpleMesh = addMesh();
        decoEntity->mesh = addMesh();
        decoEntity->simpleMesh = addMesh();
      }
    }
  }

  for (int i = 1; i <= sizeDesigns(); i++) {
    Design* design = getDesign(i);
    if (design->mesh == 0 && !(design->flags & _designHasStatue)) {
      design->mesh = addMesh();
      design->simpleMesh = addMesh();
      design->decoMesh = addMesh();
      design->simpleDecoMesh = addMesh();
    }
  }
}

void renderBuildings() {
  for (int i = 1; i <= sizeDesigns(); i++) {
    renderDesign(i);
  }
  for (int i = 1; i <= buildings->size(); i++) {
    renderBuilding(i);
  }
}

item nearestBuilding(vec3 loc) {
  float leastDist = FLT_MAX;
  item best=0;
  for (item i=1; i <= buildings->size(); i++) {
    Building* building = getBuilding(i);
    if (building->flags & _buildingExists) {
      float dist = vecDistance(getBuildingCenter(i), loc);
      if (dist < leastDist) {
        leastDist = dist;
        best = i;
      }
    }
  }
  return best;
}

item nearestBuilding(Line ml) {
  float leastDist = FLT_MAX;
  item best=0;
  for (item i=1; i <= buildings->size(); i++) {
    Building* building = getBuilding(i);
    if (!(building->flags & _buildingExists)) continue;
    bool isGov = building->zone == GovernmentZone;
    bool isHistorical = building->flags & _buildingHistorical;
    bool isViz = areBuildingsVisible || isGov || isHistorical;
    if (!isViz) continue;
    float dist = pointLineDistance(getBuildingCenter(i), ml);
    if (dist < leastDist) {
      leastDist = dist;
      best = i;
    }
  }
  return best;
}

void setIssuesIconsVisible() {
  bool val = false;
  if (alwaysShowIssuesIcons()) val = true;
  item hm = getHeatMap();
  if (isHeatMapIntense() &&
      (hm == Prosperity || hm == HealthHM || hm == CommunityHM)) val = true;
  if (getHeatMap() == Prosperity && isHeatMapIntense()) val = true;
  if (!areBuildingsVisible) val = false;
  if (areIssuesIconsVisible == val) return;
  areIssuesIconsVisible = val;

  for (item i=1; i <= buildings->size(); i++) {
    Building* building = getBuilding(i);
    if ((building->flags & _buildingExists) && building->iconEntity != 0) {
      setEntityVisible(building->iconEntity, val);
    }
  }
}

void setBuildingsVisible() {
  bool val = true;
  if (isUndergroundView()) val = false;
  if (getHeatMap() == ZoneHeatMap) val = false;
  if (getGameMode() == ModeBuildingDesigner ||
      getGameMode() == ModeDesignOrganizer) val = true;
  if (areBuildingsVisible == val) return;
  areBuildingsVisible = val;

  for (item i=1; i <= buildings->size(); i++) {
    Building* building = getBuilding(i);
    if ((building->flags & _buildingExists)) {
    bool isGov = building->zone == GovernmentZone;
    bool isHistorical = building->flags & _buildingHistorical;
    bool myVal = areBuildingsVisible || isGov || isHistorical;

      if (building->entity != 0) setEntityVisible(building->entity, myVal);
      if (building->decoEntity != 0) {
        setEntityVisible(building->decoEntity, myVal);
      }
      if (building->iconEntity != 0) {
        setEntityVisible(building->iconEntity, val && areIssuesIconsVisible);
      }
    }
  }
}

void writeBuilding(FileBuffer* file, item ndx) {
  Building* building = getBuilding(ndx);

  fwrite_vec3 (file, building->location);
  fwrite_vec3 (file, building->normal);
  fwrite_item(file, building->design);
  fwrite_item(file, building->zone);
  fwrite_item(file, building->plan);
  fwrite_item(file, building->color);
  fwrite_item(file, building->econ);
  fwrite_item_vector(file, &building->families);
  fwrite_item_vector(file, &building->businesses);
  fwrite_item_vector(file, &building->peopleInside);
  fwrite_item_vector(file, &building->lots);
  fwrite_graph_location(file, building->graphLoc);
  fwrite_int  (file, building->flags);
  fwrite_float(file, building->builtTime);
  fwrite_float(file, building->lastUpdateTime);
  fwrite_float(file, building->value);
  fwrite_string(file, building->name);
}

void readBuilding(FileBuffer* file, int version, item ndx) {
  Building* building = getBuilding(ndx);
  if(building == 0) return;

  building->location     = fread_vec3 (file);
  building->normal       = fread_vec3 (file);
  building->design       = fread_item(file, version);
  building->zone         = fread_item(file, version);
  if (version >= 18) {
    building->plan = fread_item(file, version);
  }
  if (version >= 17) {
    building->color         = fread_item(file, version);
    if (building->color >= numBuildingColors) {
      building->color = randItem(numBuildingColors);
    }
  } else {
    building->color = randItem(numBuildingColors);
  }

  if (version >= 52) {
    building->econ = fread_item(file, version);
  } else {
    building->econ = getEcon(building->location);
  }

  fread_item_vector(file, &building->families, version);
  fread_item_vector(file, &building->businesses, version);
  fread_item_vector(file, &building->peopleInside, version);
  if (version >= 7) {
    fread_item_vector(file, &building->lots, version);
  }
  building->graphLoc = fread_graph_location(file, version);
  building->flags        = fread_int  (file);
  if (version <= 17) {
    building->flags |= _buildingComplete;
  }

  if (version < 49) {
    building->builtTime = getCurrentDateTime();
  } else {
    building->builtTime = fread_float(file);
  }

  if (version >= 45) {
    building->lastUpdateTime = fread_float(file);
    building->value = fread_float(file);
  } else {
    building->lastUpdateTime = getCurrentDateTime();
    building->value = 0;
  }

  if (version >= 52) {
    building->name = fread_string(file);
    if (building->name != 0 && building->name[0] == 0) {
      free(building->name);
      building->name = 0;
    }
  }

  building->entity = 0;
  building->decoEntity = 0;
  building->iconEntity = 0;

  if (version < 25) {
    Design* design = getDesign(building->design);
    if (design->size.x > tileSize*2) {
      addBuildingElevator(ndx, false);
    }
  }

  if ((building->flags & _buildingExists) &&
      !(building->flags & _buildingCity)) {
    //building->lastUpdateTime = getCurrentDateTime();
    //building->value = 0;
    setMaxLandValues(ndx);
    addToCollisionTable(BuildingCollisions, getBuildingBox(ndx), ndx);
    addSpawnPoints(ndx);

    float myZ = getDesignZ(building->design);
    if (myZ > tallestBuildingZ) {
      tallestBuilding = ndx;
      tallestBuildingZ = myZ;
    }
  }
}

void writeBuildings(FileBuffer* file) {
  buildings->write(file);

  for (int i=1; i <= buildings->size(); i++) {
    writeBuilding(file, i);
  }
}

void readBuildings(FileBuffer* file, int version) {
  buildings->read(file, version);

  for (int i=1; i <= buildings->size(); i++) {
    readBuilding(file, version, i);
  }
}

void nameBuildings() {
  for (int i=1; i <= buildings->size(); i++) {
    nameBuilding(i);
  }
}

float getTargetIllumLevel(item buildingNdx) {
  float targetIllum = 0;
  bool designer = getGameMode() == ModeBuildingDesigner;
  bool organizer = getGameMode() == ModeDesignOrganizer;
  if (designer || organizer) {
    targetIllum = getBuildingTextureLightLevel();

  } else { //if (b->flags & _buildingLights) {
    Building* b = getBuilding(buildingNdx);
    item numInside = b->peopleInside.size();
    item capacity = 1 + getNumBusinesses(b->design) * 20 +
      getDesignNumHomes(b->design) * 1;
    targetIllum = numInside*6.f/capacity - getLightLevel()*4;
    targetIllum *= (1-1.25f*getLightLevel());
    targetIllum = sqrt(clamp(targetIllum, 0.f, 100.f));
    if (numInside > 20 && targetIllum < 1) targetIllum = 1;
  }

  if (targetIllum > 0 && targetIllum < 1) targetIllum = 1;

  return targetIllum;
}

