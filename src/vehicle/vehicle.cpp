#include "vehicle.hpp"

#include "model.hpp"
#include "renderVehicle.hpp"
#include "travelGroup.hpp"
#include "physics.hpp"
#include "update.hpp"

#include "../building/building.hpp"
#include "../draw/entity.hpp"
#include "../draw/texture.hpp"
#include "../economy.hpp"
#include "../game/game.hpp"
#include "../graph.hpp"
#include "../graph/transit.hpp"
#include "../graph/stop.hpp"
#include "../heatmap.hpp"
#include "../lane.hpp"
#include "../option.hpp"
#include "../pool.hpp"
#include "../person.hpp"
#include "../route/router.hpp"
#include "../selection.hpp"
#include "../sound.hpp"
#include "../util.hpp"
#include "../time.hpp"

#include "../parts/messageBoard.hpp"

#include <algorithm>
#include <boost/dynamic_bitset.hpp>

float vehicleRate = 0;

Pool<Vehicle>* vehicles = Pool<Vehicle>::newPool(20000);
boost::dynamic_bitset<> vehicleActive_g;

float getVehicleRate();
void placeVehicleEntity(Vehicle* vehicle);
void updateVehicleHeadlights(item vNdx, float duration);
void rerouteVehicle(item ndx);

const item randomColorSet[] = {
  0, 1, 2, 3, 12, 13, 16, 17, 20, 21, 22, 23, 28, 29, 30, 31
};
const item numRandomColors = 16;

item randomVehicleColor() {
  if (randFloat() < 0.33) return 31;
  return randomColorSet[randItem(numRandomColors)];
}

item randomWandererColor() {
  return randItem(28);
}

VehicleDescription createVehicleDescription() {
  VehicleDescription result;
  result.style = randomVehicleColor() +
    randItem(numVehicleModels(VhTypePrivate)) * numVehicleColors;
  return result;
}

void writeVehicleDescription(FileBuffer* file, VehicleDescription desc) {
  fwrite_item(file, desc.style);
}

VehicleDescription readVehicleDescription(FileBuffer* file, int version) {
  VehicleDescription result;
  item style = fread_item(file, version);
  item numColors = version < 54 ? 15 : numVehicleColors;
  item model = style / numColors;
  item color = style % numColors;
  model = renumVehicleModel(model);

  if (model <= 0 || model > vehicleModelsSize()) {
    model = randomVehicleModel(VhTypePrivate);
  }

  if (version < 54 || color < 0 || color >= numVehicleColors) {
    color = randomVehicleColor();
  }

  result.style = model * numVehicleColors + color;
  return result;
}

item vehicleColor(int flags) {
  return (flags & _vehicleColorMask) >> _vehicleColorShift;
}

item vehicleColor(Vehicle* vehicle) {
  return vehicleColor(vehicle->flags);
}

bool isVehicleActive_g(item ndx) {
  if (ndx >= vehicleActive_g.size()) return false;
  return vehicleActive_g[ndx];
}

void setVehicleActive_g(item ndx) {
  if (vehicleActive_g.size() <= ndx) {
    vehicleActive_g.resize(ndx*2+1);
  }

  Vehicle* v = getVehicle(ndx);
  bool active = (v->flags & _vehicleExists) &&
    (v->flags & _vehiclePlaced) &&
    (v->pilot.lane != 0);
  vehicleActive_g[ndx] = active;
}

vec3 getVehicleCenter_g(item ndx) {
  Vehicle* v = getVehicle(ndx);
  vec3 loc = v->location;
  float num = 1;
  //while (v->trailer) {
    //v = getVehicle(v->trailer);
    //loc += v->location;
    //num ++;
  //}
  return loc / num;
}

void placeVehicleEntity(Vehicle* vehicle, item ndx) {
  if (vehicle->entity == 0) return;
  bool isWanderer = vehicle->flags & _vehicleWanderer;
  bool visible = (vehicle->laneLoc.lane != 0) || isWanderer;
  int flags = vehicle->flags;

  Entity* entity = getEntity(vehicle->entity);
  setEntityIlluminated(vehicle->entity, flags & _vehicleHeadlights);
  setEntityVisible(vehicle->entity, visible);
  placeEntity(vehicle->entity, vehicle->location,
    -vehicle->yaw, vehicle->pitch);

  if (!wasEntityCulled_g(vehicle->entity)) {
    vehicleAudible_g(vehicle->model);
  }

  if (vehicle->flags & _vehicleIsTransit) {
    entity->flags |= _entityTransit;
  } else {
    entity->flags &= ~_entityTransit;
  }

  if (vehicle->lightEntity != 0) {
    copyEntityPlacement(vehicle->entity, vehicle->lightEntity);
  }

  if (vehicle->numberEntity != 0 && isTransitVisible()) {
    vec3 loc = getVehicleCenter_g(ndx);
    placeEntity(vehicle->numberEntity, loc + vec3(0,0,20), 0, 0);
    copyEntityPlacement(vehicle->numberEntity, vehicle->numberShadowEntity);
  }
}

void routeVehicle(item ndx) {
  Vehicle* vehicle = getVehicle(ndx);
  vehicle->flags &= ~_vehicleHasRoute;
  routeVehicle_g(ndx, vehicle->pilot, vehicle->destination);
  //SPDLOG_INFO("routeVehicle {} {} {}",
   //   ndx, vehicle->routeSourceLane, vehicle->destination.lane);
}

void invalidateVehicleRoute(item ndx) {
  Vehicle* vehicle = getVehicle(ndx);
  invalidateRouteCache(&vehicle->route);
}

void rerouteVehicle(item ndx) {
  Vehicle* vehicle = getVehicle(ndx);
  vehicle->flags &= ~_vehicleHasRoute;
  invalidateRouteCache(&vehicle->route);

  if (vehicle->transitLine != 0) {
    routeTransitVehicle_g(ndx, vehicle->transitLine);
  } else {
    removeVehicle(ndx);
    //routeVehicle(ndx);
  }
}

item addTrailer(item ndx, item trailingNdx);
item addTestVehicle_g(GraphLocation start, GraphLocation dest) {
  //if (countVehicles() > 0) return 0;
  adjustStat(ourCityEconNdx(), TotalTrips, 1);
  VehicleDescription vehicleDescription = createVehicleDescription();
  //item styleType = (randFloat() < .1) ?
    //((rand()%2==0) ? BoxTruck : VhSemiTractor) : randItem(numCarStyles);
  //item styleType = Train;

  vector<item> ndxes;
  int numNdxes = 1; //styleType == VhSemiTractor ? 2 : styleType == Train ? 8 : 1;
  for (int i = 0; i < numNdxes; i++) {
    ndxes.push_back(vehicles->create());
    sort(ndxes.begin(), ndxes.end());
  }

  item ndx = ndxes[0];
  addVehicleCommand(ndx);

  Vehicle* vehicle = getVehicle(ndx);
  vehicle->flags = _vehicleExists;
  vehicle->entity = 0;
  vehicle->lightEntity = 0;
  vehicle->numberEntity = 0;
  vehicle->numberShadowEntity = 0;
  vehicle->lastNumberRendered = -1;
  //vehicle->pilotEntity = 0;
  vehicle->pilot = start;
  vehicle->laneLoc = start;
  clearRoute(&vehicle->route);
  vehicle->destination = dest;
  vehicle->vehicleAhead[0] = 0;
  vehicle->vehicleAhead[1] = 0;
  vehicle->location = getLocation(start);
  vehicle->velocity = vec3(0, 0, 0);
  vehicle->acceleration = vec2(0, 0);
  vehicle->pitch = 0;
  vehicle->distanceSinceMerge = 0;
  vehicle->aggressiveness = randFloat(0.8, 1.2);
  vehicle->creationTime = getCurrentDateTime();
  vehicle->laneBlockEnterTime = vehicle->creationTime;
  vehicle->trailer = 0;
  vehicle->trailing = 0;
  vehicle->yieldTo = 0;
  vehicle->yieldFrom = 0;
  //vehicle->routeSourceLane = vehicle->pilot.lane;

  Lane* lane = getLane(start);
  vehicle->yaw = atan2(lane->ends[1].x-lane->ends[0].x,
    lane->ends[1].y-lane->ends[0].y);

  item color = randomVehicleColor();
  vehicle->flags |= color << _vehicleColorShift;
  vehicle->model = randomVehicleModel(VhTypePrivate);

  if (vehicle->model > 0) {
    VehicleModel* model = getVehicleModel(vehicle->model);
    model->count ++;
  }

  for (int i = 1; i < ndxes.size(); i++) {
    addTrailer(ndxes[i], ndxes[i-1]);
  }

  clearRoute(&vehicle->route);
  routeVehicle(ndx);
  //setEntityVisible(vehicle->pilotEntity, false);
  renderVehicle(ndx);
  setEntityVisible(vehicle->entity, false);
  setEntityVisible(vehicle->lightEntity, false);

  return ndx;
}

item addTrailer(item ndx, item trailingNdx) {
  Vehicle* vehicle = getVehicle(ndx);
  Vehicle* trailing = getVehicle(trailingNdx);
  vehicle->flags = _vehicleExists;
  vehicle->entity = 0;
  vehicle->lightEntity = 0;
  vehicle->numberEntity = 0;
  vehicle->numberShadowEntity = 0;
  vehicle->lastNumberRendered = -1;
  vehicle->pilot = trailing->pilot;
  vehicle->laneLoc = trailing->laneLoc;
  clearRoute(&vehicle->route);
  vehicle->destination = trailing->destination;
  vehicle->vehicleAhead[0] = 0;
  vehicle->vehicleAhead[1] = 0;
  vehicle->velocity = vec3(0, 0, 0);
  vehicle->acceleration = vec2(0, 0);
  vehicle->pitch = 0;
  vehicle->distanceSinceMerge = 0;
  vehicle->aggressiveness = trailing->aggressiveness;
  vehicle->creationTime = getCurrentDateTime();
  vehicle->laneBlockEnterTime = vehicle->creationTime;
  vehicle->trailer = 0;
  vehicle->trailing = trailingNdx;
  trailing->trailer = ndx;
  vehicle->yieldTo = 0;
  vehicle->yieldFrom = 0;
  //vehicle->routeSourceLane = vehicle->pilot.lane;
  clearRoute(&vehicle->route);

  vehicle->pilot.dap -= 8;
  vehicle->laneLoc.dap -= 8;
  vehicle->yaw = trailing->yaw;
  vehicle->location = getLocation(vehicle->laneLoc);
  if (getGameMode() == ModeGame) {
    Lane* lane = getLane(vehicle->laneLoc);
    vec3 alongLane = lane->ends[1] - lane->ends[0];
    vehicle->location -= uzNormal(alongLane)*c(CLaneWidth);
  }

  item trailingType = getVehicleModel(trailing->model)->type;
  item targetType = trailingType == VhTypeTruckFront ? VhTypeTruckMiddle :
    trailingType == VhTypeTrainFront ? VhTypeTrainMiddle : trailingType;

  vehicle->model = randomVehicleModel((VehicleModelType)targetType);
  if (vehicle->model == 0) TRIP;
  if (vehicle->model > 0) {
    VehicleModel* model = getVehicleModel(vehicle->model);
    model->count ++;
  }

  item color = vehicleColor(trailing);
  vehicle->flags |= color << _vehicleColorShift;
  renderVehicle(ndx);
  setEntityVisible(vehicle->entity, false);

  addVehicleCommand(ndx);
  return ndx;
}

item addVehicle_g(item ndx, item style, Route* route, item startLane) {
  Vehicle* vehicle = getVehicle(ndx);
  vehicle->flags = _vehicleExists;
  if (route->steps.size() > 0) {
    vehicle->flags |= _vehicleHasRoute;
  }

  vehicle->entity = 0;
  vehicle->lightEntity = 0;
  vehicle->numberEntity = 0;
  vehicle->numberShadowEntity = 0;
  vehicle->lastNumberRendered = -1;
  //vehicle->pilotEntity = 0;
  vehicle->vehicleAhead[0] = 0;
  vehicle->vehicleAhead[1] = 0;
  vehicle->velocity = vec3(0, 0, 0);
  vehicle->acceleration = vec2(0, 0);
  vehicle->pitch = 0;
  vehicle->distanceSinceMerge = 0;
  vehicle->aggressiveness = randFloat(0.8, 1.2);
  vehicle->creationTime = getCurrentDateTime();
  vehicle->laneBlockEnterTime = vehicle->creationTime;
  vehicle->trailer = 0;
  vehicle->trailing = 0;
  vehicle->yieldTo = 0;
  vehicle->yieldFrom = 0;
  vehicle->travelGroups.clear();

  vehicle->pilot = getNextGraphLoc(route);
  if (startLane != 0) vehicle->pilot.lane = startLane;
  vehicle->laneLoc = vehicle->pilot;

  if (vehicle->pilot.lane == 0) {
    vehicle->pilot.lane = startLane;
  } else {
    vehicle->location = getLocation(vehicle->pilot);
    Lane* lane = getLane(vehicle->pilot.lane);
    vec3 alongLane = lane->ends[1] - lane->ends[0];
    vehicle->yaw = atan2(alongLane.x, alongLane.y);
    vehicle->location -= uzNormal(alongLane)*c(CLaneWidth);
  }

  item color = style % numVehicleColors;
  vehicle->model = style / numVehicleColors;
  if (vehicle->model <= 0) vehicle->model = randomVehicleModel(VhTypePrivate);
  if (vehicle->model > 0) {
    VehicleModel* model = getVehicleModel(vehicle->model);
    model->count ++;
  }

  vehicle->flags |= color << _vehicleColorShift;
  copyRoute(route, &vehicle->route);
  renderVehicle(ndx);
  setEntityVisible(vehicle->entity, false);
  setEntityVisible(vehicle->lightEntity, false);
  //setEntityVisible(vehicle->pilotEntity, false);

  addVehicleCommand(ndx);
  return ndx;
}

item addVehicle_g(item ndx, item style, item travelGroup, Route* route) {
  addVehicle_g(ndx, style, route, 0);
  putTravelGroupInVehicle_g(travelGroup, ndx);
  return ndx;
}

item addVehicle_g(item style, item travelGroup, Route* route) {
  return addVehicle_g(vehicles->create(), style, travelGroup, route);
}

item addFreightVehicle_g(item travelGroup, Route* route) {
  item result = 0;
  if (c(CEnableSemis) && rand()%2 == 0) {
    item ndxV = vehicles->create();
    item ndxT = vehicles->create();
    if (ndxV > ndxT) { // Force trailer to be updated after tractor
      item swap = ndxV;
      ndxV = ndxT;
      ndxT = swap;
    }

    item model = randomVehicleModel(VhTypeTruckFront);
    item style = model*numVehicleColors + randomVehicleColor();
    item result = addVehicle_g(ndxV, style, travelGroup, route);
    result = addTrailer(ndxT, result);
    //setSelection(SelectionVehicle, result);

  } else {
    item model = randomVehicleModel(VhTypeTruck);
    item style = model*numVehicleColors + randomVehicleColor();
    result = addVehicle_g(vehicles->create(), style, travelGroup, route);
  }

  if (result > 0) {
    Vehicle* v = getVehicle(result);
    v->flags |= _vehicleIsFrieght;
  }

  return result;
}

item addTransitVehicle_g(item lineNdx, Route* route) {
  TransitLine* line = getTransitLine(lineNdx);
  if (line->legs.size() == 0) return 0;
  bool hasRoute = route->steps.size() > 0;
  Stop* stop = getStop(line->legs[0].stop);
  item startLane = stop->graphLoc.lane;
  if (startLane == 0) return 0;

  vector<item> ndxes;
  int numNdxes = clamp(float(line->maxCars), 1.f, c(CMaxCarsPerVehicle));
  for (int i = 0; i < numNdxes; i++) {
    ndxes.push_back(vehicles->create());
  }
  sort(ndxes.begin(), ndxes.end());

  for (int i = 0; i < ndxes.size(); i++) {
    item ndx = ndxes[i];
    if (i == 0) {
      item model = randomVehicleModel(
          ndxes.size() == 1 ? VhTypeBus : VhTypeTrainFront);
      addVehicle_g(ndx,
          model*numVehicleColors + 2, // vBlueA
          route, startLane);
    } else {
      addTrailer(ndx, ndxes[i-1]);
    }
    Vehicle* v = getVehicle(ndx);
    v->transitLine = lineNdx;
    v->flags |= _vehicleIsTransit;
    if (hasRoute) {
      v->flags |= _vehicleHasRoute;
    }
    renderVehicle(ndx);
  }

  if (!hasRoute) {
    routeTransitVehicle_g(ndxes[0], lineNdx);
  }

  return ndxes[0];
}

item addWanderer_g(item modelNdx, vec3 loc, float yaw) {
  item ndx = vehicles->create();
  Vehicle* vehicle = getVehicle(ndx);
  vehicle->flags = _vehicleExists | _vehiclePlaced | _vehicleHasRoute |
    _vehicleWanderer | _vehicleHeadlights;

  if (modelNdx > 0) {
    VehicleModel* model = getVehicleModel(modelNdx);
    model->count ++;
  }

  vehicle->entity = 0;
  vehicle->lightEntity = 0;
  vehicle->numberEntity = 0;
  vehicle->numberShadowEntity = 0;
  vehicle->lastNumberRendered = -1;
  //vehicle->pilotEntity = 0;
  vehicle->vehicleAhead[0] = 0;
  vehicle->vehicleAhead[1] = 0;
  vehicle->velocity = vec3(0, 0, 0);
  vehicle->acceleration = vec2(0, 0);
  vehicle->location = loc;
  vehicle->pitch = 0;
  vehicle->yaw = yaw;
  vehicle->distanceSinceMerge = 0;
  vehicle->aggressiveness = randFloat(0.8, 1.2);
  vehicle->creationTime = getCurrentDateTime();
  vehicle->laneBlockEnterTime = vehicle->creationTime;
  vehicle->trailer = 0;
  vehicle->trailing = 0;
  vehicle->yieldTo = 0;
  vehicle->yieldFrom = 0;
  vehicle->travelGroups.clear();
  vehicle->model = modelNdx;
  vehicle->pilot.lane = 1;

  item color = randomWandererColor();
  vehicle->flags |= color << _vehicleColorShift;

  setVehicleActive_g(ndx);
  renderVehicle(ndx);
  setEntityVisible(vehicle->entity, true);
  setEntityVisible(vehicle->lightEntity, false);
  addVehicleCommand(ndx);

  return ndx;
}

Vehicle* getVehicle(item ndx) {
  return vehicles->get(ndx);
}

void removeVehicle(item ndx) {
  if (ndx <= 0 || ndx > vehicles->size()) return;

  Vehicle* vehicle = getVehicle(ndx);
  if (!(vehicle->flags & _vehicleExists)) return;
  //removeVehicleFromLane(ndx, vehicle->pilot.lane);
  //removeVehicleFromLane(ndx, vehicle->laneLoc.lane);

  for (int i = vehicle->travelGroups.size()-1; i >= 0; i--) {
    removeTravelGroup_g(vehicle->travelGroups[i]);
  }

  if (vehicle->trailer) {
    removeVehicle(vehicle->trailer);
  }

  if (vehicle->trailing == 0 && !(vehicle->flags & _vehicleWanderer)) {
    float time = getCurrentDateTime() - vehicle->creationTime;
    time *= 24*60; // to minutes;
    money fuelPrice = getStatistic(nationalEconNdx(), FuelPrice);
    money fuelUsage = c(CLitersPerMinuteCar);

    if (vehicle->flags & _vehicleIsTransit) {
      money fuelUsage = c(CLitersPerMinuteBus);
      money maint = vehicle->trailing == 0 ? c(CMaintPerMinuteBus) : 0;
      maint *= getInflation();
      float cost = (fuelUsage * fuelPrice + maint) * time;
      cost *= getTransitMoneyMultiplier();
      transaction(TransitExpenses, -cost);
      removeVehicleFromTransitLine_g(ndx);

    } else {
      money tax = fuelUsage * fuelPrice * time * getTaxRate(FuelTaxIncome) * getTransitMoneyMultiplier();
      transaction(FuelTaxIncome, tax);
    }
  }

  if (vehicle->entity != 0) {
    removeEntity(vehicle->entity);
    vehicle->entity = 0;
    //removeEntity(vehicle->pilotEntity);
  }

  if (vehicle->lightEntity != 0) {
    removeEntity(vehicle->lightEntity);
    vehicle->lightEntity = 0;
  }

  if (vehicle->numberEntity != 0) {
    removeEntityAndMesh(vehicle->numberEntity);
    removeEntityAndMesh(vehicle->numberShadowEntity);
    vehicle->numberEntity = 0;
    vehicle->numberShadowEntity = 0;
    vehicle->lastNumberRendered = -1;
  }

  vehicle->numPassengers = 0;
  clearRoute(&vehicle->route);
  deselect(SelectionVehicle, ndx);
  removeMessageByObject(VehicleMessage, ndx);
  removeVehicleCommand(ndx);

  if (vehicle->model > 0) {
    VehicleModel* model = getVehicleModel(vehicle->model);
    model->count --;
  }

  vehicle->flags = _vehicleSettling;
  vehicle->creationTime = getCurrentDateTime();
  vehicles->settle(ndx);
}

void freeVehicle(item ndx) {
  vehicles->free(ndx);
}

void updateVehicleHeadlights(item vNdx, float duration) {
  Vehicle* vehicle = getVehicle(vNdx);
  bool headlights = false;

  if (vehicle->flags & _vehicleWanderer) {
    headlights = true;

  } else if (vehicle->trailing != 0) {
    Vehicle* trailing = getVehicle(vehicle->trailing);
    headlights = trailing->flags & _vehicleHeadlights;

  } else {
    headlights = vehicle->flags & _vehicleHeadlights;
    float lightLevel = getLightLevel();
    float currentTime = getCurrentTime();
    DaylightMode dayMode = getDaylightMode();

    if (getGameMode() != ModeGame) {
      headlights = lightLevel < 0.4;
    } else if (dayMode == DaylightAlwaysDay) {
      headlights = false;
    } else if (dayMode == DaylightAlwaysNight) {
      headlights = true;
    } else if (lightLevel > 0.7) {
      headlights = false;
    } else if (lightLevel < 0.3) {
      headlights = true;

    } else if (randFloat()*oneHour < duration/gameDayInRealSeconds) {
      if (currentTime < 0.5) {
        headlights = false;
      } else {
        headlights = true;
      }
    }
  }

  if (headlights) {
    vehicle->flags |= _vehicleHeadlights;
  } else {
    vehicle->flags &= ~_vehicleHeadlights;
  }

  if (vehicle->lightEntity != 0) {
    bool viz = vehicle->laneLoc.lane != 0 && vehicle->trailing == 0 &&
      (headlights || getHeatMap() == TrafficHeatMap);
    setEntityVisible(vehicle->lightEntity, viz);
  }
}

float getVehicleRate() {
  return vehicleRate;
}

float getEffectiveTrafficRate() {
  float pop = getStatistic(nationalEconNdx(), Population);
  if (pop > 1000000) {
    return c(CTrafficRate1M);

  } else if (pop > 100000) {
    float popFactor = clamp((pop-100000.f)/900000.f, 0.f, 1.f);
    popFactor = 1-pow(1-popFactor, 4);
    return mix(c(CTrafficRate100K), c(CTrafficRate1M), popFactor);

  } else {
    float popFactor = clamp(pop/100000.f, 0.f, 1.f);
    popFactor = 1-pow(1-popFactor, 4);
    return mix(c(CTrafficRate0), c(CTrafficRate100K), popFactor);
  }
}

float getTransitMoneyMultiplier() {
  return 90 / getEffectiveTrafficRate();
}

item nearestVehicle(Line ml) {
  float leastDist = FLT_MAX;
  item best=0;
  for (item i=1; i <= vehicles->size(); i++) {
    Vehicle* vehicle = getVehicle(i);
    if (vehicle->flags & _vehicleExists) {
      float dist = pointLineDistance(vehicle->location, ml);
      if (dist < leastDist) {
        leastDist = dist;
        best = i;
      }
    }
  }
  return best;
}

item getRandomVehicle() {
  if (vehicles->count() == 0) {
    return 0;
  }

  int i = 0;
  while(i < 10) {
    item result = randItem(vehicles->size())+1;
    Vehicle* v = getVehicle(result);
    if (v->flags & _vehicleExists) {
      return result;
    }

    i++;
  }

  return 0;
}

void selectVehicleAndPause(item ndx) {
  #if LP_DEBUG
  if (debugMode()) {
    setSelection(SelectionVehicle, ndx);
    setGameSpeed(0);
    Vehicle* v = getVehicle(ndx);
    SPDLOG_INFO("{}: f:{:#b} vA:{} e:{} pl:{} ll:{}",
        ndx, v->flags, v->vehicleAhead[0], v->entity,
        v->pilot.lane, v->laneLoc.lane);

    /*
    item vAndx = v->vehicleAhead;
    if (vAndx != 0) {
      v = getVehicle(vAndx);
      SPDLOG_INFO("vA:({}): f:{:#b} vA:{} e:{} pl:{}, ll:{}",
          vAndx, v->flags, v->vehicleAhead, v->entity,
          v->pilot.lane, v->laneLoc.lane);
    }
    */
  }
  #endif
}

void resetVehicles() {
  resetVehiclesCommand();
  vehicleRate = 0;
  for (int i = 1; i <= vehicles->size(); i ++) {
    Vehicle* v = getVehicle(i);
    v->travelGroups.clear();
    clearRoute(&v->route);
  }
  vehicles->clear();
  vehicleActive_g.clear();
  resetVehicleRender();
}

void removeAllVehicles_g() {
  for (int i = vehicles->size(); i > 0; i--) {
    removeVehicle(i);
  }
  vehicles->defragment("vehicles");
}

void initVehiclesEntities() {
  for (int i = 1; i <= vehicles->size(); i ++) {
    Vehicle* vehicle = getVehicle(i);
    if (vehicle->flags & _vehicleExists && vehicle->entity == 0) {
      vehicle->entity = addEntity(VehicleShader);
      if (vehicle->trailing == 0) {
        vehicle->lightEntity = addEntity(LightingShader);
      }

      /*
      if (vehicle->flags & _vehicleIsTransit) {
        vehicle->numberEntity = addEntity(WSUITextShader);
        vehicle->numberShadowEntity = addEntity(WSUIShader);
        createMeshForEntity(vehicle->numberEntity);
        createMeshForEntity(vehicle->numberShadowEntity);
      }
      */

      //vehicle->pilotEntity = addEntity(DecoShader);
    }
  }

  initVehicleMeshes();
}

void renderVehicles() {
  renderVehicleStyles();
  for (int i = 1; i <= vehicles->size(); i ++) {
    if (!isVehicleActive_g(i)) continue;
    Vehicle* v = getVehicle(i);
    if (!(v->flags & _vehicleExists)) continue;
    if (!(v->flags & _vehiclePlaced)) continue;
    renderVehicle(i);
    updateVehicleHeadlights(i, getFrameDuration());
    placeVehicleEntity(v, i);
    if (!(v->flags & _vehicleIsTransit) &&
        v->route.steps.size() > 0 && v->transitLine > 0) {
      updateTransitRoute_g(v->transitLine, &v->route);
    }
  }
}

void placeVehicles() {
  for (int i = 1; i <= vehicles->size(); i ++) {
    placeVehicle(i);
  }
}

void placeVehicle(item ndx) {
  if (!isVehicleActive_g(ndx)) return;
  Vehicle* v = getVehicle(ndx);
  if (!(v->flags & _vehicleExists)) return;
  if (!(v->flags & _vehiclePlaced)) return;
  //SPDLOG_INFO("placeVehicle {} {:#b}", ndx, v->flags);
  if (v->entity == 0 || getEntity(v->entity)->mesh == 0) {
    renderVehicle(ndx);
  }
  updateVehicleHeadlights(ndx, getFrameDuration());
  renderVehicleNumber(ndx);
  placeVehicleEntity(v, ndx);
}

void validateVehicles(char* msg) {
  for (int i = 1; i <= vehicles->size(); i ++) {
    Vehicle* v = getVehicle(i);
    bool transit = v->flags & _vehicleIsTransit;
    if (vehicleColor(v) > numVehicleColors) {
      SPDLOG_ERROR("invalid vehicle color {}:{} - {}",
          i, vehicleColor(v), msg);
      handleError("invalid vehicle");
    }

    if (v->model <= 0 || v->model > vehicleModelsSize()) {
      SPDLOG_ERROR("invalid vehicle model {}:{} - {}",
          i, v->model, msg);
      handleError("invalid vehicle");
    }

    if (v->entity != 0) {
      if (v->entity < 0 || v->entity > entitiesSize()) {
        SPDLOG_ERROR("invalid vehicle entity {}:{} - {}",
            i, v->entity, msg);
        handleError("invalid vehicle");
      }

      Entity* e = getEntity(v->entity);
      if (e->texture < vehicleTexture ||
          e->texture > vehicleTextureBrakesNight) {
        SPDLOG_ERROR("invalid vehicle texture {}:{} - {}",
            i, e->texture, msg);
        handleError("invalid vehicle");
      }
    }

    if (v->numberEntity != 0 && !transit) {
      SPDLOG_WARN("numberEntity on non-transit vehicle");
    }

    if (v->route.steps.size() > 4) {
      bool anyTransit = false;
      bool anyBlk = false;
      for (int j = 0; j < v->route.steps.size(); j++) {
        Location loc = v->route.steps[j];
        item type = locationType(loc);
        if (type == LocLaneBlock) {
          anyBlk = true;
        } else if (type == LocTransitLeg || type == LocTransitStop) {
          anyTransit = true;
        } else if (type != LocDap) {
          SPDLOG_ERROR("invalid vehicle route {}:{} - {}",
              i, routeString(&v->route), msg);
          handleError("invalid vehicle");
        }
      }

      if (!anyBlk) {
        SPDLOG_ERROR("vehicle route is commuter {}:{} - {}",
            i, routeString(&v->route), msg);
        handleError("invalid vehicle");

      } else if (anyTransit != transit) {

        SPDLOG_ERROR("{} - invalid {} vehicle({}) route: {}",
            msg, transit ? "transit" : "private",
            i, routeString(&v->route));
        handleError("invalid vehicle");
      }
    }
  }
}

void settleVehicles() {
  setStat(ourCityEconNdx(), VehiclesSettling, vehicles->settling.size());
  setStat(ourCityEconNdx(), VehiclesFree, vehicles->gaps.size());

  float time = getCurrentDateTime();
  item largestNdx = 0 ;

  for (int i = vehicles->settling.size()-1; i >= 0; i--) {
    item vNdx = vehicles->settling[i];
    Vehicle* v = getVehicle(vNdx);
    if (!(v->flags & _vehicleSettling)) {
      SPDLOG_WARN("bad vehicle settle {}", vNdx);
      continue;
    }

    float diff = time - v->creationTime;
    if (diff > oneHour) {
      v->flags = 0;
      freeVehicle(vNdx);
      if (vNdx > largestNdx) largestNdx = vNdx;
    }
  }

  //if (largestNdx >= vehicles->size()*.01f) {
    //vehicles->defragment("vehicles");
  //}
}

void writeVehicle(FileBuffer* file, item ndx) {
  Vehicle* vehicle = getVehicle(ndx);

  fwrite_int  (file, vehicle->flags);
  fwrite_float(file, vehicle->yaw);
  fwrite_float(file, vehicle->pitch);
  fwrite_float(file, vehicle->distanceSinceMerge);
  fwrite_float(file, vehicle->aggressiveness);
  fwrite_float(file, vehicle->creationTime);
  fwrite_float(file, vehicle->laneBlockEnterTime);
  fwrite_vec3 (file, vehicle->location);
  fwrite_vec3 (file, vehicle->velocity);
  fwrite_vec2 (file, vehicle->acceleration);
  fwrite_item(file, vehicle->vehicleAhead[0]);
  fwrite_graph_location(file, vehicle->pilot);
  fwrite_graph_location(file, vehicle->laneLoc);
  fwrite_graph_location(file, vehicle->destination);
  fwrite_route(file, &vehicle->route);
  fwrite_item_vector(file, &vehicle->travelGroups);
  fwrite_item(file, vehicle->trailer);
  fwrite_item(file, vehicle->trailing);
  fwrite_item(file, vehicle->yieldTo);
  fwrite_item(file, vehicle->yieldFrom);
  fwrite_item(file, vehicle->transitLine);
  fwrite_item(file, vehicle->model);
}

void readVehicle(FileBuffer* file, int version, item ndx) {
  Vehicle* vehicle = getVehicle(ndx);

  vehicle->flags = fread_int(file);
  if (version < 41) {
    int mode = fread_int(file);
    if (mode == 0) {
      vehicle->flags |= _vehicleHasRoute;
    }
  }
  vehicle->yaw = fread_float(file);
  vehicle->pitch = fread_float(file);
  if (version < 42) {
    float length = fread_float(file);
  }
  vehicle->distanceSinceMerge = fread_float(file);

  if (version >= 2) {
    vehicle->aggressiveness = fread_float(file);
  } else {
    vehicle->aggressiveness = randFloat(0.8, 1.2);
  }

  if (version >= 6) {
    vehicle->creationTime = fread_float(file);
  } else {
    vehicle->creationTime = getCurrentDateTime();
  }

  if (version >= 31) {
    vehicle->laneBlockEnterTime = fread_float(file);
  } else {
    vehicle->laneBlockEnterTime = getCurrentDateTime();
  }

  vehicle->location = fread_vec3(file);
  vehicle->velocity = fread_vec3(file);
  if (version >= 22) {
    vehicle->acceleration = fread_vec2(file);
  } else {
    vehicle->acceleration = vec2(0,0);
  }
  vehicle->vehicleAhead[0] = fread_item(file, version);
  vehicle->vehicleAhead[1] = 0;

  if (version < 51) {
    item destinationBuilding = fread_item(file, version);
  }

  if (version < 42) {
    vehicle->flags |= fread_item(file, version) << _vehicleColorShift;
  }
  vehicle->pilot = fread_graph_location(file, version);
  vehicle->laneLoc = fread_graph_location(file, version);
  vehicle->destination = fread_graph_location(file, version);

  if (version < 51) {
    vector<Location> routeSteps;
    fread_location_vector(file, &routeSteps, version);
    reverse(routeSteps.begin(), routeSteps.end());
    vehicle->route.steps.fromVector(routeSteps);
    vehicle->route.steps.push_back(dapLocation(vehicle->destination.dap));
    vehicle->route.destination = vehicle->destination.lane;

  } else {
    fread_route(file, &vehicle->route);
  }

  // Pre-51, this was passengers (person indexes)
  fread_item_vector(file, &vehicle->travelGroups, version);

  if (version >= 41) {
    vehicle->trailer = fread_item(file, version);
    vehicle->trailing = fread_item(file, version);
  } else {
    vehicle->trailer = 0;
    vehicle->trailing = 0;
  }

  if (version >= 56) {
    vehicle->yieldTo = fread_item(file, version);
    vehicle->yieldFrom = fread_item(file, version);
  } else {
    vehicle->yieldTo = 0;
    vehicle->yieldFrom = 0;
  }

  if (version >= 51) {
    vehicle->transitLine = fread_item(file, version);
  } else {
    vehicle->transitLine = 0;
  }

  item model = 0;
  if (version >= 54) {
    model = fread_item(file, version);
  } else {
    model = vehicleColor(vehicle) / 15;
    vehicle->flags &= ~_vehicleColorMask;
    vehicle->flags |= randomVehicleColor() << _vehicleColorShift;
  }
  model = renumVehicleModel(model);
  if (vehicle->flags & _vehicleIsTransit) {
    if (vehicle->trailing != 0) {
      model = randomVehicleModel(VhTypeTrainMiddle);
    } else if (vehicle->trailer != 0) {
      model = randomVehicleModel(VhTypeTrainFront);
    }
  }
  if (model <= 0 || model > vehicleModelsSize()) {
    model = randomVehicleModel(VhTypePrivate);
  }

  vehicle->model = model;
  if ((vehicle->flags & _vehicleExists) && vehicle->model > 0) {
    VehicleModel* model = getVehicleModel(vehicle->model);
    model->count ++;
    if (model->type == VhTypeTruck ||
      model->type == VhTypeTruckFront ||
      model->type == VhTypeTruckMiddle) vehicle->flags |= _vehicleIsFrieght;
  }

  //if (vehicle->model == 0) TRIP;

  if (version < 31) {
    vehicle->flags |= _vehiclePlaced;
  }

  vehicle->entity = 0;
  vehicle->lightEntity = 0;
  vehicle->numberEntity = 0;
  vehicle->numberShadowEntity = 0;
  vehicle->lastNumberRendered = -1;
  //vehicle->pilotEntity = 0;

  if ((vehicle->flags & _vehicleExists) &&
      !(vehicle->flags & _vehicleHasRoute)) {
    routeVehicle(ndx);
  }

  setVehicleActive_g(ndx);
}

item countVehicles() {
  return vehicles->count();
}

item sizeVehicles() {
  return vehicles->size();
}

void writeVehicles(FileBuffer* file) {
  vehicles->defragment("vehicles");

  writeVehicleModels(file);
  fwrite_float(file, vehicleRate);
  vehicles->write(file);

  for (int i=1; i <= vehicles->size(); i++) {
    writeVehicle(file, i);
  }
}

void vehiclePassengersToTravelGroups51_g() {
  for (int i = 1; i <= vehicles->size(); i++) {
    Vehicle* vehicle = getVehicle(i);
    vector<item> passengers = vehicle->travelGroups;
    vehicle->travelGroups.clear();

    item groupNdx = addTravelGroup_g();
    TravelGroup* group = getTravelGroup_g(groupNdx);
    copyRoute(&vehicle->route, &group->route);
    group->route.currentStep = 0;
    group->location = vehicleLocation(i);
    vehicle->travelGroups.push_back(groupNdx);

    for (int j = 0; j < passengers.size(); j++) {
      group->members.push_back(passengers[j]);
      Person* p = getPerson(passengers[j]);
      p->location = travelGroupLocation(groupNdx);
      p->flags |= _personTraveling;
    }
  }
}

void readVehicles(FileBuffer* file, int version) {
  readVehicleModels(file);
  vehicleRate = fread_float(file);
  vehicles->read(file, version);
  vehicleActive_g.resize(vehicles->size()+1);

  for (int i=1; i <= vehicles->size(); i++) {
    readVehicle(file, version, i);
  }
  SPDLOG_INFO("{} vehicles", vehicles->count());

  vehicles->gaps.clear();
  for (int i=1; i <= vehicles->size(); i++) {
    Vehicle* v = getVehicle(i);
    if (!(v->flags & _vehicleExists)) {
      vehicles->gaps.push_back(i);
    }
  }

  vehicles->defragment("vehicles");
}

