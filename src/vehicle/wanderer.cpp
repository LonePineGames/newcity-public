#include "wanderer.hpp"

#include "model.hpp"
#include "vehicle.hpp"

#include "../building/building.hpp"
#include "../building/design.hpp"
#include "../draw/texture.hpp"
#include "../draw/image.hpp"
#include "../game/game.hpp"
#include "../land.hpp"
#include "../platform/lua.hpp"
#include "../selection.hpp"
#include "../time.hpp"
#include "../util.hpp"
#include "../weather.hpp"

#include "spdlog/spdlog.h"

const float pi_o2 = pi_o*2;

struct SpawnPoint {
  item buildingNdx;
  vec3 location;
  float yaw;
};

vector<vector<SpawnPoint>> spawnPoints;

void resetWanderers_g() {
  spawnPoints.clear();
}

void readdSpawnPoints(item buildingNdx) {
  removeSpawnPoints(buildingNdx);
  addSpawnPoints(buildingNdx);
}

void addSpawnPoints(item buildingNdx) {
  Building* b = getBuilding(buildingNdx);
  Design* d = getDesign(b->design);
  int decoS = d->decos.size();
  //SPDLOG_INFO("addSpawnPoints {} {} decos:{}", buildingNdx, b->design, decoS);

  for (int i = 0; i < decoS; i++) {
    Deco deco = d->decos[i];
    if (deco.decoType < numLegacyDecoTypes) continue;
    DecoType* decoType = getDecoType(deco.decoType);

    //SPDLOG_INFO("deco {} spawns {}", decoType->code, decoType->spawns);

    if (decoType->spawns <= 0) continue;

    vec3 location = b->location;
    vec3 norm = normalize(b->normal);
    vec3 decoOff = vec3(norm.x,norm.y,0)*deco.location.x +
      vec3(-norm.y,norm.x,0)*deco.location.y + vec3(0,0,deco.location.z);
    location += decoOff;

    vec3 decoNorm = vec3(norm.x,norm.y,0)*cos(deco.yaw) +
      vec3(-norm.y,norm.x,0)*sin(deco.yaw);
    float yaw = atan2(-decoNorm.x,-decoNorm.y);

    if (spawnPoints.size() <= decoType->spawns) {
      spawnPoints.resize(decoType->spawns+1);
    }
    SpawnPoint point = {buildingNdx, location, yaw};
    spawnPoints[decoType->spawns].push_back(point);
  }
}

void removeSpawnPoints(item buildingNdx) {
  //SPDLOG_INFO("removeSpawnPoints {}", buildingNdx);
  for (int i = 0; i < spawnPoints.size(); i++) {
    vector<SpawnPoint>* points = &spawnPoints[i];
    for (int k = points->size()-1; k >= 0; k--) {
      if (points->at(k).buildingNdx == buildingNdx) {
        points->erase(points->begin()+k);
      }
    }
  }
}

float targetWandererCount(item modelNdx) {
  VehicleModel* model = getVehicleModel(modelNdx);
  if (!(model->flags & _modelWanderer)) return 0;
  bool isDesigner = getGameMode() == ModeBuildingDesigner;
  bool isSpawned = model->flags & _modelSpawned;

  if (isSpawned) {
    if (spawnPoints.size() <= modelNdx) return 0;
    vector<SpawnPoint>* points = &spawnPoints[modelNdx];
    if (isDesigner) {
      return points->size();
    } else {
      return points->size() * model->density;
    }
  } else {
    //if (isDesigner) {
      //return 0;
    //} else {
      float mapSize = getMapSize() / 1000;
      return mapSize * mapSize * model->density;
    //}
  }
}

void updateOneWanderer_g(item ndx, float duration) {
  Vehicle* v = getVehicle(ndx);

  float age = getCurrentDateTime() - v->creationTime;
  VehicleModel* model = getVehicleModel(v->model);
  if (age > model->maxAge) {
    removeVehicle(ndx);
    return;
  }

  bool isDesigner = getGameMode() == ModeBuildingDesigner;
  if (isDesigner || age > model->maxAge*.25f) {
    float maxCount = ceil(targetWandererCount(v->model));
    if (!isDesigner) maxCount *= 2;
    if (model->count*randFloat(0,1) > maxCount) {
      removeVehicle(ndx);
      return;
    }
  }

  if ((model->flags & _modelSea) && getIceFade() > 0.5) {
    removeVehicle(ndx);
    return;
  }

  if (model->updateFunc != -1) {
    callLuaUpdateFunction(model->updateFunc, model->code, ndx, duration);
    placeVehicle(ndx);
  }

  /*
  Image blueNoise = getBlueNoiseImage();
  vec3 bnLoc = v->location / 6400.f;
  vec4 wave = samplePixel(blueNoise, bnLoc.x, bnLoc.y);

  //float pitch = randFloat(-1,1) * (pi_o*.25);
  //pitch = mix(v->pitch, pitch, 0.01f);
  //v->pitch = pitch;

  float turnSpeed = model->speed * duration * 0.01f;

  float pitch = v->pitch;
  pitch += (wave.y*2-1) * turnSpeed;
  pitch = mix(pitch, 0.f, 0.01);
  v->pitch = pitch;

  float yaw = v->yaw;
  yaw += (wave.x*2-1) * turnSpeed;
  if (yaw < 0) yaw += pi_o2;
  if (yaw > pi_o2) yaw -= pi_o2;
  v->yaw = yaw;

  float spitch = sin(pitch);
  float cpitch = cos(pitch);
  float syaw = sin(yaw) * cpitch;
  float cyaw = cos(yaw) * cpitch;
  vec3 move = (duration * model->speed) * vec3(syaw, cyaw, spitch);
  v->location += move;

  if (ndx == getSelection()) {
    SPDLOG_INFO("updateOneWanderer_g {} {} ({},{},{})",
        ndx, duration, v->location.x, v->location.y, v->location.z);
    SPDLOG_INFO("yaw:{} move:({},{},{})",
        v->yaw, move.x, move.y, move.z);
    SPDLOG_INFO("entity:{}", v->entity);
  }

  float mapSize = getMapSize();
  float buffer = 1000;
  if (v->location.x > mapSize+buffer || v->location.x < -buffer ||
    v->location.y > mapSize+buffer || v->location.y < -buffer) {
    // Rotate 180 degrees
    if (v->yaw >= pi_o) {
      v->yaw -= pi_o;
    } else {
      v->yaw += pi_o;
    }
    v->location.x = clamp(v->location.x, -buffer+2, mapSize+buffer-2);
    v->location.y = clamp(v->location.y, -buffer+2, mapSize+buffer-2);
  }
  float floor = pointOnLand(v->location).z;
  if (floor < 0) floor = 0;
  float ceiling = c(CMaxHeight) + 100.f;
  if (floor > v->location.z || v->location.z > ceiling) {
    v->location.z = clamp(v->location.z, floor + 2.f, ceiling - 2.f);
    v->pitch *= -1;
  }

  if (v->location.z < floor + 200 && v->pitch < 0.05) {
    float pitchDiff = (floor+200 - v->location.z)/200*.01f;
    pitchDiff = clamp(pitchDiff, 0.f, 0.01f);
    v->pitch += pitchDiff;
  }

  placeVehicle(ndx);
  */
}

void addWanderer_g(item modelNdx) {
  VehicleModel* model = getVehicleModel(modelNdx);
  bool isSpawned = model->flags & _modelSpawned;
  vec3 loc;
  float yaw = 0;

  if (isSpawned) {
    vector<SpawnPoint>* points = &spawnPoints[modelNdx];
    item pointS = points->size();
    if (pointS <= 0) return;
    SpawnPoint point = points->at(randItem(pointS));
    loc = point.location;
    yaw = point.yaw;

  } else {
    yaw = randFloat(0, pi_o2);
    float mapSize = getMapSize();
    for (int k = 0; k < 10; k++) {
      loc = vec3(randFloat(), randFloat(), 0) * mapSize;
      loc = pointOnLand(loc);
      if (model->flags & _modelSea && loc.z > -10) continue;
      if (loc.z < 0) loc.z = 0;
      break;
    }
  }

  item vNdx = addWanderer_g(modelNdx, loc, yaw);
  if (model->flags & _modelSea && getSelection() == 0) {
    //setSelection(SelectionVehicle, vNdx);
  }
}

void addWinWanderers_g(item buildingNdx) {
  Building* b = getBuilding(buildingNdx);
  vec3 loc = getBuildingTop(buildingNdx);
  float yaw = atan2(-b->normal.x, -b->normal.y);

  item winAnimationModel = vehicleModelByCode("VhWinAnimation");
  addWanderer_g(winAnimationModel, loc, yaw);

  for (int i = 0; i < 40; i++) {
    item balloonsModel = vehicleModelByCode("VhPartyBalloons");
    float range = 100;
    vec3 locOff = loc + vec3(randFloat(-range, range), randFloat(-range, range), randFloat(-range*2, 0));
    addWanderer_g(balloonsModel, locOff, randFloat(0, pi_o2));
  }
}

void updateWanderers_g(float duration) {
  float dayDur = duration / gameDayInRealSeconds;
  for (int i = 1; i <= vehicleModelsSize(); i++) {
    VehicleModel* model = getVehicleModel(i);
    if (!(model->flags & _modelWanderer)) continue;
    if ((model->flags & _modelSea) && getIceFade() > 0.5) continue;
    bool isDesigner = getGameMode() == ModeBuildingDesigner;
    float targetCount = targetWandererCount(i);

    float diff = targetCount - model->count;
    if (isDesigner && diff > 0) diff = 1;
    if (diff > 0 && randFloat()/diff < duration * (isDesigner ? 1 : .002f)) {
      addWanderer_g(i);
    }

    //float numToSpawn = isDesigner ? duration : dayDur/model->maxAge;
    //numToSpawn = clamp(numToSpawn, 0.f, diff);
    //SPDLOG_INFO("type:{} count:{} targetCount:{} numToSpawn:{}",
        //model->code, model->count, targetCount, numToSpawn);

    /*
    while (numToSpawn > 1) {
      addWanderer_g(i);
      numToSpawn --;
    }
    */

    //if (randFloat() < numToSpawn) {
      //addWanderer_g(i);
    //}
  }
}

