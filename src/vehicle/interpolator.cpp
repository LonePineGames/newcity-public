#include "interpolator.hpp"

#include "model.hpp"
#include "physics.hpp"
#include "vehicle.hpp"
#include "wanderer.hpp"

#include "../draw/entity.hpp"
#include "../graph.hpp"
#include "../heatmap.hpp"
#include "../selection.hpp"
#include "../time.hpp"
#include "../util.hpp"

#include "spdlog/spdlog.h"
#include <atomic>

static item keyOffset = numKeyframes-1;
Cup<GraphLocation> keyframes;
Cup<GraphLocation> vInterpolateLoc;
//double lastSwapTime = 0;
double lastInterpolateTime = 0;
double lastAlpha = 0;
Cup<double> swapTime;
static double heatmapsAndWearToGo = 0;
Cup<vec3> vLastSample;
atomic<item> currentInterpolatorKeyframe(0);
atomic<item> currentPhysicalKeyframe(0);
atomic<float> lastSimulateTime(0);
item interpolateSkipNdx = 0;
const item interpolateSkips = 20;
//double nextSwapTime = 0;

void resetVehicleInterpolator() {
  keyOffset = numKeyframes-1;
  lastInterpolateTime = 0;
  heatmapsAndWearToGo = 0;

  keyframes.clear();
  swapTime.clear();
  vInterpolateLoc.clear();

  currentInterpolatorKeyframe = 0;
  currentPhysicalKeyframe = 0;
}

item getPhysicalKeyframe() {
  return currentPhysicalKeyframe;
}

item keyframeIndex(item key, item ndx) {
  //key = key%numKeyframes;
  return ndx*numKeyframes + key;
}

GraphLocation getKeyframe(item key, item ndx) {
  item n = keyframeIndex(key, ndx);
  //SPDLOG_INFO("getKeyframe {} {} {} {} {}", key, ndx, keyOffset, n,
      //keyframes.size());
  //if (keyframes.size() < n) {
    return keyframes[n];
  //} else {
    //return vec3(0,0,0);
  //}
}

void setKeyframe(item key, item ndx, GraphLocation val) {
  item n = keyframeIndex(key, ndx);
  //SPDLOG_INFO("setKeyframe {} {} {} {} {}", key, ndx, keyOffset, n,
      //keyframes.size());
  keyframes.ensureSize(n+1);
  val.dap = clamp(val.dap, 0.f, getLaneLength(val.lane));
  keyframes.set(n, val);
}

void setAllKeyframes(item ndx, GraphLocation val) {
  for (int i = 0; i < numKeyframes; i++) {
    setKeyframe(i, ndx, val);
  }
  vLastSample.ensureSize(ndx+1);
  vLastSample.set(ndx, getLocation(val));
  vInterpolateLoc.ensureSize(ndx+1);
  vInterpolateLoc.set(ndx, val);
}

void updateOneVehicleHeatmapsAndWear(item ndx, float duration,
    float trafficRate) {
  if (!isVehicleActive_g(ndx)) return;
  Vehicle* v = getVehicle(ndx);
  if (!(v->flags & _vehicleExists)) return;
  if (!(v->flags & _vehiclePlaced)) return;

  vec3 loc = v->location;
  item lane = v->laneLoc.lane;
  vLastSample.ensureSize(ndx+1);
  vec3 lastLoc = vLastSample[ndx];
  vLastSample.set(ndx, loc);
  double distance = clamp(vecDistance(loc, lastLoc), 0.f, 100.f) / trafficRate;
  double multiplier = duration / trafficRate;
  int type = getVehicleModel(v->model)->type;
  bool truck = type == VhTypeTruck || type == VhTypeTruckFront ||
    type == VhTypeTruckMiddle;
  //bool transit = type == VhTypeBus || type == VhTypeTrainFront ||
    //type == VhTypeTrainMiddle;
  bool transit = v->flags & _vehicleIsTransit;
  float density = distance * (transit ? c(CTransitDensity) :
      truck ? c(CTruckDensity) : c(CVehicleDensity));
  float wear =    distance * (truck ? c(CTruckWear)    : c(CVehicleWear));
  float value = multiplier * (truck ? c(CTruckValue)   : c(CVehicleValue));
  float pollution = multiplier *
    (truck ? c(CTruckPollution) : c(CVehiclePollution));
  float prosperity = distance *
    (truck ? c(CTruckProsperity) : c(CVehicleProsperity));
  float community = v->numPassengers > 1 ?
    0.0f :
    distance * c(CCommunityEffect) * -1.0f;
  // Vehicle interpolation seems too inconsistent over time to rely on
  // for distributing negative health effects, for now (2020-04-22)
  float health = (truck ? c(CHealthTruckEffect) : c(CHealthVehicleEffect));
  health = health / gameDayInRealSeconds;

  prosperity *= v->numPassengers;
  density *= v->numPassengers;

  heatMapAdd(Pollution, loc, pollution);
  heatMapAdd(Value, loc, value);
  heatMapAdd(CommunityHM, loc, community);
  heatMapAdd(HealthHM, loc, health);

  if (lane >= 10) {
    LaneBlock* b = getLaneBlock(lane);
    item elem = b->graphElements[1];
    if (elem != 0) {
      Configuration config = getElementConfiguration(elem);
      if (elem > 0) wearEdge(elem, wear);
      if (config.type != ConfigTypeExpressway) {
        heatMapAdd(Density, loc, density);
        heatMapAdd(Prosperity, loc, prosperity);
      }
    }
  }
}

void updateVehicleHeatmapsAndWear(float duration) {
  heatmapsAndWearToGo += duration;
  float utime = c(CVehicleHeatmapTime);
  if (heatmapsAndWearToGo > utime) {
    float dur = floor(heatmapsAndWearToGo/utime) * utime;
    float trafficRate = getEffectiveTrafficRate();
    heatmapsAndWearToGo -= dur;

    for (int i = 1; i < sizeVehicles(); i++) {
      updateOneVehicleHeatmapsAndWear(i, dur, trafficRate);
    }
  }
}

void interpolateOneVehicle(item ndx, int key, int key1,
    float alpha, float duration) {
  if (!isVehicleActive_g(ndx)) return;
  Vehicle* v = getVehicle(ndx);
  if (!(v->flags & _vehicleExists)) return;
  if (!(v->flags & _vehiclePlaced)) return;

  bool wasCulled = wasEntityCulled_g(v->entity);
  if (wasCulled && (ndx % interpolateSkips != interpolateSkipNdx) &&
      (getSelectionType() != SelectionVehicle || ndx != getSelection())) {
    return;
  }

  //if (!wasCulled) SPDLOG_INFO("vehicle not culled");

  if (v->flags & _vehicleWanderer) {
    updateOneWanderer_g(ndx, duration * (wasCulled?interpolateSkips:1));
    return;
  }

  vec3 target;
  GraphLocation g1 = getKeyframe(key1, ndx);
  GraphLocation result = g1;
  if (v->trailing != 0) {
    vInterpolateLoc.ensureSize(v->trailing+1);
    GraphLocation t0 = vInterpolateLoc[v->trailing];
    t0.dap -= vehicleLength(v->trailing)*.5f + vehicleLength(ndx) * .5f;

    if (t0.dap < 0) {
      // Get front vehicle
      Vehicle* t = getVehicle(v->trailing);
      while (t->trailing) t = getVehicle(t->trailing);

      // Get current step in route
      Route route = t->route;
      route.currentStep = -1;
      for (int i = route.steps.size()-1; i >= 0; i--) {
        if (route.steps[i]/10 == t0.lane/10) {
          route.currentStep = i;
          break;
        }
      }

      while (t0.lane >= 10 && t0.dap < 0 && route.currentStep > 0) {
        route.currentStep --;
        Location step = route.steps[route.currentStep];
        Lane* lane = getLane(t0.lane);
        item nextLane = 0;
        for (int i = 0; i < lane->sources.size(); i++) {
          if (lane->sources[i]/10 == step/10) {
            nextLane = lane->sources[i];
            break;
          }
        }

        if (nextLane == 0) break;
        t0.dap += getLaneLength(nextLane);
        t0.lane = nextLane;

        /*
        if (locationType(step) == LocLaneBlock && step >= 10) {
          item* drains = getLaneDrains(step);
          for (int j = 0; j < maxLaneDrains; j++) {
            item drain = drains[j];
            if (drain == 0) {
              break;
            } else if (drain == t0.lane) {
              t0.lane = 
            }
          t0.lane = step;
        }
        */
      }
    }

    result = t0;
    target = getLocation(result);

  } else if (c(CInterpolateVehicles)) {
    float invAlpha = 1 - alpha;
    bool locFound = false;
    GraphLocation g0 = getKeyframe(key, ndx);

    if (v->entity != 0 && !wasCulled) {
      item routeStartStep = -1;
      item routeEndStep = -1;
      Route route = v->route;
      for (int i = 0; i < route.steps.size(); i++) {
        if (route.steps[i]/10 == g0.lane/10) {
          routeStartStep = i;
        }
        if (route.steps[i]/10 == g1.lane/10) {
          routeEndStep = i;
        }
      }

      item numSteps = routeEndStep-routeStartStep+1;
      if (routeStartStep >= 0 && routeEndStep > routeStartStep &&
          numSteps > 2) {
        locFound = true;
        float* blockLengths = (float*)alloca(sizeof(float)*numSteps);
        float totalDistance = 0;

        for (int i = 0; i < numSteps; i++) {
          Location blkNdx = route.steps[routeStartStep+i];
          if (locationType(blkNdx) != LocLaneBlock) {
            blockLengths[i] = 0;
            continue;
          }
          float blkLength = getLaneLength(blkNdx);
          if (i == 0) blkLength -= g0.dap;
          if (i == numSteps-1) blkLength = g1.dap;
          blockLengths[i] = blkLength;
          totalDistance += blkLength;
        }

        float advance = totalDistance * alpha;
        for (int i = 0; i < numSteps; i++) {
          if (advance < blockLengths[i]) {
            GraphLocation r0;
            r0.lane = route.steps[routeStartStep+i];
            r0.lane -= r0.lane%10;
            item laneInBlk = round(.5f*(g0.lane%10 + g1.lane%10));
            item numLanesInBlk = getBlockLanes(r0.lane);
            if (numLanesInBlk <= laneInBlk) laneInBlk = numLanesInBlk-1;
            r0.lane += laneInBlk;
            r0.dap = advance;
            target = getLocation(r0);
            break;
          } else {
            advance -= blockLengths[i];
          }
        }
      }
    }

    if (!locFound) {
      if (g0.lane/10 == g1.lane/10) {
        float advance = g1.dap - g0.dap;
        float theta = alpha * advance;
        GraphLocation r0 = g0, r1 = g1;
        r0.dap = r1.dap = r0.dap + theta;
        result = r1;
        vec3 l0 = getLocation(r0);
        vec3 l1 = getLocation(r1);
        target = l0*invAlpha + l1*alpha;

      } else {
        float g0remain = getLaneLength(g0.lane) - g0.dap;
        if (g0remain < 0) g0remain = 0;
        float advance = g0remain + g1.dap;
        float theta = alpha * advance;

        if (theta > g0remain) {
          result.lane = g1.lane;
          result.dap = theta - g0remain;
        } else {
          result.lane = g0.lane;
          result.dap  = g0.dap + theta;
        }

        target = getLocation(result);
      }
    }

  } else {
    target = getLocation(g1);
  }

  //vec3 l0 = getLocation(g0);
  //vec3 l1 = getLocation(g1);
  //vec3 target = l0*invAlpha + l1*alpha;

  vec3 oldLoc = v->location;
  //vec3 loc = duration > c(CVehicleUpdateTime) ?
    //target : mix(oldLoc, target, duration);
  //SPDLOG_INFO("loc:{},{} {} {} {}", l0.x, l0.y, key, ndx, alpha);

  if (duration > 0) {
    vec3 vel = target - oldLoc;
    vel /= duration;
    if (validate(v->velocity)) {
      v->velocity = mix(v->velocity, vel, 0.1);
    } else {
      v->velocity = vel;
    }
  }

  //if (abs(vel.x) + abs(vel.y) > 0.01) {
    //v->yaw = atan2(vel.x, vel.y);
  //}

  v->location = target;
  vInterpolateLoc.ensureSize(ndx+1);
  vInterpolateLoc.set(ndx, result);

  vec3 dl;
  GraphLocation dr = result;
  if (result.dap > 0.2) {
    dr.dap -= 0.2;
    dl = getLocation(result) - getLocation(dr);
  } else {
    dr.dap += 0.2;
    dl = getLocation(dr) - getLocation(result);
  }
  v->yaw = atan2(dl.x, dl.y);
  v->pitch = atan2(dl.z, length(vec2(dl)));

  placeVehicle(ndx);
}

void interpolateVehicles(double duration) {
  swapTime.ensureSize(numKeyframes);

  double iTime = lastInterpolateTime + duration;
  iTime = clamp(iTime, 0., double(lastSimulateTime));
  //iTime = mix(iTime, swapTime[numKeyframes/2], 0.001f);
  //duration = iTime - lastInterpolateTime;
  lastInterpolateTime = iTime;
  //if (iTime < 10) return;

  if (keyframes.size() == 0) {
    return;
  }

  int key = currentInterpolatorKeyframe;
  int key1 = (key+1)%numKeyframes;
  if (key != currentPhysicalKeyframe && swapTime[key1] < iTime) {
    currentInterpolatorKeyframe = (currentInterpolatorKeyframe+1)%numKeyframes;
    key = (key+1)%numKeyframes;
    key1 = (key+1)%numKeyframes;
    lastAlpha = 0;
  }

  //for (int i = 0; i < numKeyframes-2; i ++) {
    //if (iTime > swapTime[i]) {
      //key = i;
    //}
  //}

  double alpha = (iTime - swapTime[key])/(swapTime[key1] - swapTime[key]);
  //alpha = clamp(alpha, lastAlpha, 1.);
  lastAlpha = alpha;

  //float time = getVehicleTime();
  //float alpha = (time - lastSwapTime)/(nextSwapTime - lastSwapTime);
  //SPDLOG_INFO("interpolateVehicles key:{} alpha:{} iTime:{}",
      //key, alpha, iTime);
  //lastInterpolateTime = time;

  interpolateSkipNdx = (interpolateSkipNdx+1) % interpolateSkips;
  for (int i = 1; i < sizeVehicles(); i++) {
    interpolateOneVehicle(i, key, key1, alpha, duration);
  }

  updateVehicleHeatmapsAndWear(duration);
}

void swapInterpolator(double simulationTime) {
  swapTime.ensureSize(numKeyframes);

  //if (0.1 > simulationTime - swapTime[numKeyframes-1]) return;

  lastSimulateTime = simulationTime;
  int nextPhysicalKeyframe = (currentPhysicalKeyframe+1)%numKeyframes;
  if (currentInterpolatorKeyframe != nextPhysicalKeyframe) {
    currentPhysicalKeyframe = nextPhysicalKeyframe;
  }
  //SPDLOG_INFO("swapInterpolator physical:{}x{} interp:{}x{}",
      //currentPhysicalKeyframe, simulationTime,
      //currentInterpolatorKeyframe, lastInterpolateTime);
  swapTime.set(currentPhysicalKeyframe, simulationTime);
}

