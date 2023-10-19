#include "city.hpp"

#include "building/building.hpp"
#include "configuration.hpp"
#include "draw/entity.hpp"
#include "draw/texture.hpp"
#include "economy.hpp"
#include "game/game.hpp"
#include "graph.hpp"
#include "land.hpp"
#include "label.hpp"
#include "name.hpp"
#include "plan.hpp"
#include "pool.hpp"
#include "string.hpp"
#include "string_proxy.hpp"
#include "util.hpp"
#include "zone.hpp"

#include "spdlog/spdlog.h"

#include <algorithm>

const item citiesWavelength = 25;
//const Configuration cityConfig = {3, 4, ConfigTypeRoad, SpawnStrategy,
      //_configMedian};
const Configuration cityConfig = {5, 2, ConfigTypeRoad, SpawnStrategy,
                  0};
const Configuration railConfig = {1, 4, ConfigTypeHeavyRail,
                  TrafficLightStrategy, 0};
const Configuration stationConfig = {1, 2, ConfigTypeHeavyRail,
                  TrafficLightStrategy, _configPlatform};

Pool<City>* cities = Pool<City>::newPool(20);

void addCityBuilding(item cityNdx, bool render);

item countNeighbors() {
  return cities->count();
}

City* getCity(item ndx) {
  return cities->get(ndx);
}

void renderCity(item ndx) {
  City* city = getCity(ndx);
  if (!(city->flags & _cityExists)) return;

  if (city->entity == 0) {
    city->entity = addEntity(WSUITextShader);
  }
  Entity* textEntity = getEntity(city->entity);
  textEntity->texture = textTexture;
  setEntityBringToFront(city->entity, true);
  setEntityTransparent(city->entity, true);
  setEntityVisible(city->entity, areLabelsVisible());
  placeEntity(city->entity, city->visualCenter+vec3(0,0,400), 0.f, 0.f);
  createMeshForEntity(city->entity);
  Mesh* textMesh = getMeshForEntity(city->entity);

  float fontSize = 200 + city->buildings.size()*.25f;
  textEntity->maxCameraDistance = fontSize * 200;
  vec3 normal = vec3(0,-1,0);
  renderStringCentered(textMesh, city->name,
    vec3(0,-fontSize*.5f, 0), -zNormal(normal)*fontSize, -normal*fontSize);

  bufferMesh(textEntity->mesh);
}

void setCitiesVisible(bool viz) {
  for (int i = 1; i <= cities->size(); i ++) {
    City* city = getCity(i);
    if (city->entity == 0) continue;
    setEntityVisible(city->entity, viz);
  }
}

item addCityNode(vec3 loc, Configuration config) {
  item result = addNode(loc, config);
  Node* node = getNode(result);
  node->spawnProbability = 0.f; //0.5f;
  node->flags |= _graphCity;
  complete(result, config);
  return result;
}

item addCityEdge(item n0, item n1, Configuration config) {
  item result = addEdge(n0, n1, config);
  Edge* edge = getEdge(result);
  edge->flags |= _graphCity;
  complete(result, config);
  setupEdgeLanes(result);
  return result;
}

item addCityEdge(item n0, vec3 loc1, Configuration config) {
  item n1 = addCityNode(loc1, config);
  return addCityEdge(n0, n1, config);
}

item addCityEdge(vec3 loc0, vec3 loc1, Configuration config) {
  item n0 = addCityNode(loc0, config);
  item n1 = addCityNode(loc1, config);
  return addCityEdge(n0, n1, config);
}

item addCity(vec3 location, vec3 dir, item backNode) {
  if (location.z < 0) return 0;

  item cityIndex = cities->create();
  City* city = getCity(cityIndex);
  city->flags = _cityExists;
  city->name = randomName(CityName);
  city->buildings.clear();

  city->nodeCenter = vec3(0,0,0); //location;
  city->visualCenter = location; // + normalize(dir)*c(CCityDistance);
  city->normal = dir;
  city->entity = 0;
  city->econ = addEcon(NeighborCityEcon, location,
      city->name, nationalEconNdx());

  city->node = addCityNode(city->visualCenter, cityConfig);
  item cityEdge = addCityEdge(city->node, backNode, cityConfig);
  Edge* edge = getEdge(cityEdge);
  city->laneBlock = edge->laneBlocks[0];

  renderCity(cityIndex);
  return cityIndex;
}

item addCityWithBuildings(vec3 location, vec3 dir, item backNode,
    item citySize) {

  item cityNdx = addCity(location, dir, backNode);

  if (cityNdx != 0) {
    City* city = getCity(cityNdx);

    for (int i = 1; i < numZoneTypes; i++) {
      if (i == GovernmentZone) {
        setStat(city->econ, GovernmentZoneDemand, 0);
        continue;
      }
      if (i == ParkZone) {
        setStat(city->econ, ParkZoneDemand, 0);
        continue;
      }

      setStat(city->econ, ResidentialZoneDemand+i-1, 1.0);
    }

    setStat(city->econ, FarmDemand, 1.0);
    setStat(city->econ, RetailDemand, 1.0);
    setStat(city->econ, FactoryDemand, 1.0);
    setStat(city->econ, OfficeDemand, 1.0);

    for (int i = 0; i < citySize; i ++) {
      addCityBuilding(cityNdx, false);
    }
    renderCity(cityNdx);
  }

  return cityNdx;
}

item addCitySmall(vec3 location, vec3 dir, item backNode) {
  return addCityWithBuildings(location, dir, backNode, randItem(10));
}

item addCityLarge(vec3 location, vec3 dir, item backNode) {
  item cityNdx = addCityWithBuildings(location, dir, backNode,
      randItem(800)+100);
  if (cityNdx != 0) {
    City* city = getCity(cityNdx);
    city->flags |= _cityMajor;
  }
  return cityNdx;
}

item nearestCity(vec3 loc) {
  float best = 0;
  float bestDist = FLT_MAX;
  for (int i = 1; i <= cities->size(); i ++) {
    City* city = getCity(i);
    if (!(city->flags & _cityExists)) continue;

    float dist = vecDistance(city->visualCenter, loc);
    if (dist < bestDist) {
      best = i;
      bestDist = dist;
    }
  }
  return best;
}

item intersectCity(Line mouseLine) {
  for (int i = 1; i <= cities->size(); i ++) {
    City* city = getCity(i);
    if (!(city->flags & _cityExists)) continue;

    float dist = pointLineDistance(city->visualCenter, mouseLine);
    if (dist < c(CTileSize)*50) {
      return i;
    }
  }
  return 0;
}

void resetCities() {
  for (int i = 1; i <= cities->size(); i ++) {
    City* city = getCity(i);
    if (city->name) free(city->name);
  }
  cities->clear();
}

void initCitiesEntities() {
  for (int i = 1; i <= cities->size(); i ++) {
    City* city = getCity(i);
    if (city->flags & _cityExists && city->entity == 0) {
      city->entity = addEntity(WSUITextShader);
      createMeshForEntity(city->entity);
    }
  }
}

void renderCities() {
  for (int i = 1; i <= cities->size(); i ++) {
    renderCity(i);
  }
}

void updateCity(item cityNdx, float duration) {
  City* city = getCity(cityNdx);

  // Add a building
  float buildingsToAdd = duration * c(CCityGrowthRate);
  if (isPresimulating()) buildingsToAdd *= 2;
  if (city->flags & _cityMajor) buildingsToAdd *= 4;
  while (buildingsToAdd > 1) {
     addCityBuilding(cityNdx, true);
     buildingsToAdd --;
  }
  if (randFloat() < buildingsToAdd) {
     addCityBuilding(cityNdx, true);
  }

  /*
  if (isPresimulating()) {
    for (int i = 0; i < 10; i++) {
      addCityBuilding(cityNdx, true);
    }
  } else if (randFloat() < duration * c(CCityGrowthRate)) {
     addCityBuilding(cityNdx, true);
  }
  */
}

void addCityBuilding(item cityNdx, bool update) {
  City* city = getCity(cityNdx);
  float lv = randFloat();
  float d = randFloat();
  int citySize = city->buildings.size();
  float maxD = citySize/800.f;
  item zone = 0;

  if (update) {
    item needZone = 0;
    float zoneNeed = 0;
    for (int i = 0; i < numZoneTypes; i++) {
      if (i == GovernmentZone) continue;
      float demand = zoneDemand(city->econ, i) * randFloat();
      if (demand > zoneNeed) {
        needZone = i;
        zoneNeed = demand;
      }
    }

    if (zoneNeed < c(CCityGrowthMinDemand)) return;
    zone = needZone;

  } else {
    while (zone == GovernmentZone || zone == NoZone || zone == ParkZone) {
      if (citySize < 50 && randFloat() < 0.8) {
        zone = FarmZone;
      } else if (citySize > 800 && randFloat() < 0.2) {
        zone = OfficeZone;
      } else if (randFloat() < 0.5) {
        zone = ResidentialZone;
      } else {
        zone = randItem(numZoneTypes);
      }
    }
  }

  float angle = randFloat()*pi_o*2;
  float yaw = randFloat()*pi_o*2;
  float distance = randFloat();

  if (zone == FarmZone) {
    distance += 0.4f;
    d = 0;
  } else if (zone == FactoryZone) {
    distance += 0.4f;
  } else if (zone == OfficeZone) {
    distance *= 0.5f;
    d = std::min(maxD,1.f);
  } else if (zone == ResidentialZone) {
    distance += 0.1f;
    maxD *= .5f;
    d *= d;
  }

  if (d > maxD) d = maxD;
  distance += .25f * (1-d);
  distance *= 1.2-d;
  distance *= c(CCityExtent);
  vec3 offset = vec3(sin(angle), cos(angle), 0) * distance;
  offset.z = randFloat(-20,-10);
  vec3 normal = vec3(sin(yaw), cos(yaw), 0);
  vec3 loc = city->visualCenter + offset;

  item buildingNdx =
    addCityBuilding(cityNdx, loc, normal, zone, d, lv, update);
  if (buildingNdx != 0) {
    city->buildings.push_back(buildingNdx);
  }

  if (update) {
    renderCity(cityNdx);
  }
}

void updateCities(double duration) {
  for (int i=1; i <= cities->size(); i++) {
    updateCity(i, duration);
  }
}

item addCityStructure(vec3 loc, vec3 dir) {
  float distance = randFloat(1,3) * c(CCityDistance);
  dir = normalize(dir);
  vec3 cityLoc = loc + dir * distance;
  vec3 backLoc = cityLoc + dir * c(CCityDistance);
  item backNode = addCityNode(backLoc, cityConfig);
  item smallCityNdx = addCitySmall(cityLoc, dir, backNode);
  item bigCityNdx = 0;
  vec3 dirNorm = zNormal(dir);
  vec3 otherCityLoc;

  if (randFloat() < 0.5) {
    distance = randFloat(3,10) * c(CCityDistance);
    float offset = randFloat(-1.5f,1.5f) * c(CCityDistance);
    otherCityLoc = backLoc + dir * distance + dirNorm*offset;
    bigCityNdx = addCityLarge(otherCityLoc, dir, backNode);
  } else {
    vec3 superBackLoc = backLoc + dir*200.f;
    addCityEdge(backNode, addCityNode(superBackLoc, cityConfig), cityConfig);
  }

  if (smallCityNdx != 0) {
    vec3 railBackLoc1 = cityLoc + dir*200.f + dirNorm*100.f + vec3(0,0,10);
    item railBackNode1 = addCityNode(railBackLoc1, railConfig);
    addCityEdge(getCity(smallCityNdx)->node, railBackNode1, stationConfig);
    if (bigCityNdx != 0) {
      vec3 railBackLoc2 = otherCityLoc - dir*200.f + dirNorm*100.f
        + vec3(0,0,10);
      item railBackNode2 = addCityNode(railBackLoc2, railConfig);
      addCityEdge(getCity(bigCityNdx)->node, railBackNode2, stationConfig);
      addCityEdge(railBackNode1, railBackNode2, railConfig);
    }
  }

  return backNode;
}

void makeCorner(int x, int y, item n0, item n1) {
  float mapSize = getMapSize();
  vec3 cornerLoc = vec3(x*mapSize, y*mapSize, 100);
  cornerLoc += vec3(x*2-1, y*2-1, 0) * (c(CCityDistance)*2.f);
  item corner = addCityNode(cornerLoc, cityConfig);
  addCityEdge(corner, n0, cityConfig);
  addCityEdge(corner, n1, cityConfig);
}

void generateCities() {
  float mapSize = getMapSize();
  vec3 center = vec3(mapSize/2, mapSize/2, 0);
  int wl = std::min(getLandSize(), citiesWavelength);
  int chunkSize = getChunkSize();
  item corners[8];
  item lastBackNodeX = 0;
  item lastBackNodeY = 0;

  for (int i = 0; i < 1; i++) {
    for (int j = 0; j < 2; j++) {
      for (int k = wl/2; k < getLandSize(); k+=wl) {

        vec3 xLoc = vec3(mapSize*i-(i*2-1)*k*chunkSize*tileSize, mapSize*j, 0);
        vec3 yLoc = vec3(mapSize*j, mapSize*i-(i*2-1)*k*chunkSize*tileSize, 0);
        xLoc = pointOnLand(xLoc);
        yLoc = pointOnLand(yLoc);
        vec3 xDir = normalize(vec3(0,j*2-1,0));
        vec3 yDir = normalize(vec3(j*2-1,0,0));

        item xBackNode = addCityStructure(xLoc, xDir);
        item yBackNode = addCityStructure(yLoc, yDir);

        if (lastBackNodeX != 0) {
          addCityEdge(xBackNode, lastBackNodeX, cityConfig);
        }
        if (lastBackNodeY != 0) {
          addCityEdge(yBackNode, lastBackNodeY, cityConfig);
        }
        lastBackNodeX = xBackNode;
        lastBackNodeY = yBackNode;

        if (k == wl/2) {
          corners[j*4 + 0 + 0] = xBackNode;
          corners[j*4 + 0 + 1] = yBackNode;
        }

        if (k + wl > getLandSize()) {
          corners[j*4 + 2 + 0] = xBackNode;
          corners[j*4 + 2 + 1] = yBackNode;
        }
      }
      lastBackNodeX = 0;
      lastBackNodeY = 0;
    }
  }

  makeCorner(0, 0, corners[0], corners[1]);
  makeCorner(1, 0, corners[2], corners[5]);
  makeCorner(0, 1, corners[3], corners[4]);
  makeCorner(1, 1, corners[6], corners[7]);

  /*
  // Roads criss-crossing the map
  for (int i = 1; i <= 1; i++) { //cities->size(); i++) {
    for (int j = cities->size()-1; j <= cities->size(); j++) {
      if (i == j) continue;
      item n0 = getCity(i)->node;
      item n1 = getCity(j)->node;
      item edgeNdx = addEdge(n0, n1, cityConfig);
      SPDLOG_INFO("edgeNdx {} i{} j{}", edgeNdx, i, j);
      Edge* edge = getEdge(edgeNdx);
      edge->flags |= _graphCity;
      //addPlan(GraphElementPlan, edgeNdx);
      //split(edgeNdx);
      setPlan(addPlan(GraphElementPlan, edgeNdx), true);
      //if (anyIllegalPlans()) {
        //discardAllPlans();
      //} else {
        //completeAllPlans();
      //}
    }
  }
  */
}

void readCity(FileBuffer* file, item ndx, int version) {
  City* city = getCity(ndx);

  city->flags = fread_int(file);
  city->name = fread_string(file);
  if (version >= 52) {
    city->node = fread_item(file);
    city->econ = fread_item(file);
  }
  city->nodeCenter = fread_vec3(file);
  city->visualCenter = fread_vec3(file);
  city->normal = fread_vec3(file);
  fread_item_vector(file, &city->buildings, version);

  city->entity = 0;
}

void writeCity(FileBuffer* file, item ndx) {
  City* city = getCity(ndx);

  fwrite_int(file, city->flags);
  fwrite_string(file, city->name);
  fwrite_item(file, city->node);
  fwrite_item(file, city->econ);
  fwrite_vec3(file, city->nodeCenter);
  fwrite_vec3(file, city->visualCenter);
  fwrite_vec3(file, city->normal);
  fwrite_item_vector(file, &city->buildings);
}

void readCities(FileBuffer* file, int version) {
  cities->read(file, version);

  for (int i=1; i <= cities->size(); i++) {
    readCity(file, i, version);
  }
}

void writeCities(FileBuffer* file) {
  cities->write(file);

  for (int i=1; i <= cities->size(); i++) {
    writeCity(file, i);
  }
}
