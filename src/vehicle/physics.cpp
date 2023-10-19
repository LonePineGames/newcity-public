#include "physics.hpp"

#include "interpolator.hpp"
#include "laneLoc.hpp"
#include "model.hpp"
#include "renderVehicle.hpp"
#include "transitPhysics.hpp"
#include "travelGroup.hpp"
#include "vehicle.hpp"
#include "update.hpp"

#include "../game/constants.hpp"
#include "../game/game.hpp"
#include "../game/update.hpp"
#include "../graph/transit.hpp"
#include "../cup.hpp"
#include "../heatmap.hpp"
#include "../intersection.hpp"
#include "../selection.hpp"
#include "../time.hpp"
#include "../util.hpp"

#include "spdlog/spdlog.h"

Cup<uint32_t> vFlags;
Cup<vec3> vLoc;
Cup<vec3> vVel;
Cup<float> vSpeed;
Cup<double> laneEnterTime;
Cup<float> creationTime;
Cup<float> vTripLimit;
Cup<GraphLocation> vPilot;
Cup<item> vTrailing;
Cup<item> vTrailer;
Cup<Route> vRoute;
Cup<vector<item>> vPassengers;
Cup<item> vNumPassengers;
Cup<item> vUpdateNdx;
Cup<item> vModel;
Cup<item> vYieldTo;
Cup<item> vYieldFrom;

double simulationTime = 0;

void resetVehiclePhysics() {
  SPDLOG_INFO("resetVehiclePhysics");

  simulationTime = 0;

  for (int i = 0; i < vRoute.size(); i++) {
    clearRoute(vRoute.get(i));
  }
  vRoute.clear();

  for (int i = 0; i < vPassengers.size(); i++) {
    vPassengers.get(i)->clear();
  }
  vPassengers.clear();

  vNumPassengers.clear();
  vFlags.clear();
  vLoc.clear();
  vVel.clear();
  vSpeed.clear();
  laneEnterTime.clear();
  creationTime.clear();
  vTripLimit.clear();
  vPilot.clear();
  vTrailing.clear();
  vTrailer.clear();
  vUpdateNdx.clear();
  vModel.clear();
  vYieldTo.clear();
  vYieldFrom.clear();
  resetLaneLocs();
  resetVehicleInterpolator();
}

void resizePhysicalVehicles(item size) {
  resizeLaneLocs(size);
  if (vFlags.size() < size) {
    vFlags.resize(size);
    vLoc.resize(size);
    vVel.resize(size);
    vSpeed.resize(size);
    vPilot.resize(size);
    laneEnterTime.resize(size);
    creationTime.resize(size);
    vTripLimit.resize(size);
    vTrailing.resize(size);
    vTrailer.resize(size);
    vPassengers.resize(size);
    vNumPassengers.resize(size);
    vUpdateNdx.resize(size);
    vModel.resize(size);
    vYieldTo.resize(size);
    vYieldFrom.resize(size);

    int oldsize = vRoute.size();
    vRoute.resize(size);
    for (int i = oldsize; i < vRoute.size(); i++) {
      clearRoute(vRoute.get(i));
    }
  }
}

void validateRoute(item ndx) {
  Route* route = vRoute.get(ndx);
  for (int i = 0; i < route->steps.size(); i++) {
    Location loc = route->steps[i];
    item type = locationType(loc);
    if (type > LocTransitStop) {
      handleError("Bad Route %d %s", ndx, format(loc));
    }
  }
}

float vehicleLength(item ndx) {
  return modelLength_v(vModel[ndx]);
}

void recordLaneLeave(item ndx) {
  LaneLocInfo pilot = getLaneLoc(ndx, LLPilot);
  if (pilot.loc.lane != 0) {
    float time = simulationTime - laneEnterTime[ndx];
    recordTraversal_v(pilot.loc.lane, time);
  }
  laneEnterTime.set(ndx, simulationTime);
}

void removeAllGroupsFromVehicle_v(item vNdx) {
  resizePhysicalVehicles(vNdx+1);
  vector<item> passengers = vPassengers[vNdx];

  for (int i = passengers.size()-1; i >= 0; i--) {
    removeTravelGroup_v(passengers[i]);
  }

  vPassengers.get(vNdx)->clear();
  vNumPassengers.set(vNdx, 0);
}

void removeVehicleBack(item ndx) {
  removeAllGroupsFromVehicle_v(ndx);
  uint32_t flags = vFlags[ndx];
  flags &= ~_vehicleExists;
  vFlags.set(ndx, flags);
  leaveLane(ndx);
  removeVehicleCommandBack(ndx);
}

LaneLocInfo getVehicleAheadFull(item ndx, LaneLocType type) {
  LaneLocInfo result = getVehicleAhead(ndx, type);
  while (vTrailing[result.vNdx] == ndx) {
    result = getVehicleAhead(result.vNdx, result.type);
  }
  return result;
}

struct Impediment {
  float dist;
  float speed;
  bool active;
};

Impediment combine(Impediment a, Impediment b) {
  if (!b.active) {
    return a;
  } else if (!a.active) {
    return b;
  } else if (b.dist < a.dist) {
    return b;
  } else {
    return a;
  }
}

Impediment asImpediment(LaneLocInfo ll) {
  Impediment result;

  if (ll.vNdx == 0) {
    result.dist = 0;
    result.speed = 0;
    result.active = false;

  } else {
    float length = vehicleLength(ll.vNdx) * .5f;
    result.dist = ll.loc.dap - length;
    result.speed = vSpeed[ll.vNdx];;
    result.active = true;
  }

  return result;
}

Impediment getLaneImpediment(item lane) {
  if (lane == 0) {
    Impediment result;
    result.dist = 0;
    result.speed = 0;
    result.active = false;
    return result;

  } else if (!blockIsActive_r(lane)) {
    requestBlock_v(lane);
    Impediment result;
    result.dist = 0;
    result.speed = 0;
    result.active = true;
    return result;

  } else {
    return asImpediment(firstInLane(lane));
  }
}

Impediment getImpediment(item ndx, LaneLocType type) {
  LaneLocInfo ll = getLaneLoc(ndx, type);
  LaneLocInfo ahead = getVehicleAheadFull(ndx, type);
  Impediment result = asImpediment(ahead);
  result.dist -= ll.loc.dap;
  return result;
}

Impediment getImpediment(item ndx, item nextLane, float distToEnd) {
  Impediment physc = getImpediment(ndx, LLPhysical);
  Impediment pilot = getImpediment(ndx, LLPilot);

  Impediment result = combine(physc, pilot);
  //if (pilot.active) return !physc.active ? pilot :
      //((pilot.dist < physc.dist) ? pilot : physc);
  //else if (physc.active) return physc;

  if (!result.active) {
    if (nextLane != 0) {
      result = getLaneImpediment(nextLane);
      result.dist += distToEnd;
    } else {
      result.speed = 0;
      result.dist = distToEnd;
      result.active = true;
    }
  }

  return result;
}

/*
float getMergeScore(item ndx, item lane, item nextStep) {
  vector<item> drains = getLaneDrains(lane);
  bool hasDrain = false;
  for (int i = 0; i < drains.size(); i++) {
    item drain = drains[i];
    if (drain/10 == nextStep/10) {
      hasDrain = true;
    }
  }

  float result = hasDrain ? 100 : 0;
  result -= numVehiclesInLaneBack(lane);
  return result;
}
*/

bool canEnterLane_v(GraphLocation loc) {
  return getBlockCapacity_v(loc.lane) >= numVehiclesInBlock_v(loc.lane);
}

bool canEnterLane_g(item lane) {
  return getBlockCapacity_v(lane) >= numVehiclesInBlock(lane);
}

void putTravelGroupInVehicle_v(item groupNdx, item vNdx) {
  resizePhysicalVehicles(vNdx+1);
  vector<item> passengers = vPassengers[vNdx];
  item num = vNumPassengers[vNdx];
  int passengerS = passengers.size();

  for (int i = 0; i < passengerS; i++) {
    if (passengers[i] == groupNdx) {
      return;
    }
  }

  passengers.push_back(groupNdx);
  vPassengers.set(vNdx, passengers);
  num += getTravelGroupSize_v(groupNdx);
  vNumPassengers.set(vNdx, num);

  /*
  resizePhysicalVehicles(vNdx+1);
  vPassengers.get(vNdx)->push_back(groupNdx);
  vNumPassengers.set(vNdx, vNumPassengers[vNdx] +
      getTravelGroupSize_v(groupNdx));
      */
}

void removeTravelGroupFromVehicle_v(item groupNdx, item vNdx) {
  resizePhysicalVehicles(vNdx+1);
  vector<item> passengers = vPassengers[vNdx];
  item num = vNumPassengers[vNdx];

  for (int i = passengers.size()-1; i >= 0; i--) {
    if (passengers[i] == groupNdx) {
      passengers.erase(passengers.begin()+i);
      num -= getTravelGroupSize_v(groupNdx);
    }
  }

  if (num < 0) {
    SPDLOG_WARN("passengers miscounted v{} g{} {} numgroups{}",
        vNdx, groupNdx, num, vPassengers[vNdx].size());
  }
  vPassengers.set(vNdx, passengers);
  vNumPassengers.set(vNdx, num);
}

bool transitVehicleStop_v(item ndx, Location legStep, Location stopStep) {
  bool done = true;
  item trailer = vTrailer[ndx];
  vector<item> passengers = vPassengers[ndx];
  item numPassengers = passengers.size();
  if (trailer != 0) {
    done = transitVehicleStop_v(trailer, legStep, stopStep);
  }

  item updateNdx = vUpdateNdx[ndx];
  if (updateNdx == 0) {
    updateNdx = numPassengers;
    if (updateNdx == 0) updateNdx = -1;
    vUpdateNdx.set(ndx, updateNdx);

  } else if (updateNdx > 0) {
    updateNdx --;
    if (updateNdx >= numPassengers) updateNdx = numPassengers-1;
    while (updateNdx >= 0 &&
        !travelGroupInVehicle_v(passengers[updateNdx], ndx, legStep)) {
      updateNdx --;
    }
    if (updateNdx <= 0) {
      updateNdx = -1;
    }
    vUpdateNdx.set(ndx, updateNdx);

  } else if (vNumPassengers[ndx] >= modelPassengers_v(vModel[ndx]) ||
      vehicleAtStop_v(locationNdx(stopStep), legStep, ndx)) {
    return done;
  }

  return false;
}

void handleVehicleAtDest(item ndx) {
  uint32_t flags = vFlags[ndx];
  if (!(flags & _vehicleExists)) return;
  if (!(flags & _vehicleHasRoute)) return;

  resizePhysicalVehicles(ndx+1);
  vector<item> passengers = vPassengers[ndx];
  item updateNdx = vUpdateNdx[ndx];
  item numPassengers = passengers.size();
  bool isTransit = flags & _vehicleIsTransit;
  validateRoute(ndx);
  Route* route = vRoute.get(ndx);
  item numSteps = route->steps.size();

  //if (ndx == getSelection()) {
    //SPDLOG_INFO("handleVehicleAtDest {}", routeString(*route));
  //}

  if (numSteps <= 0) {
    return;
  } else if (route->currentStep >= numSteps) {
    removeVehicleBack(ndx);
    return;
  }

  Location stopStep = route->steps[route->currentStep];
  item stopStepType = locationType(stopStep);

  if (stopStepType == LocDap) {
    route->currentStep ++;
    //if (ndx == getSelection()) {
      //SPDLOG_INFO("advance at dap");
    //}
    return;

  } else if (stopStep < 10) {
    route->currentStep ++;
    //if (ndx == getSelection()) {
      //SPDLOG_INFO("advance blk0");
    //}
    return;

  } else if (isTransit) {
    if (numSteps > route->currentStep+1) {
      Location legStep = route->steps[route->currentStep+1];

      if (stopStepType == LocTransitStop &&
          locationType(legStep) == LocTransitLeg) {

        if (transitVehicleStop_v(ndx, legStep, stopStep)) {
          route->currentStep += 2;
          //if (ndx == getSelection()) {
            //SPDLOG_INFO("advance after stop");
          //}

          item trailer = vTrailer[ndx];
          while (trailer != 0) {
            vUpdateNdx.set(trailer, 0);
            trailer = vTrailer[trailer];
          }

          updateNdx = 0;
          vUpdateNdx.set(ndx, updateNdx);
        }

      } else {
        if (stopStepType != LocLaneBlock) {
          route->currentStep ++;
          //if (ndx == getSelection()) {
            //SPDLOG_INFO("advance ?1");
          //}
        }
        updateNdx = 0;
        vUpdateNdx.set(ndx, updateNdx);
      }

    } else {
      SPDLOG_WARN("Unknown step for vehicle({}): {}",
          ndx, format(stopStep));
      route->currentStep ++;
    }

  } else {
    SPDLOG_WARN("Unknown step for vehicle({}): {}",
        ndx, format(stopStep));
    route->currentStep ++;
  }
}

bool updateOneVehicle(item ndx, float duration) {
  uint32_t flags = vFlags[ndx];
  if (!(flags & _vehicleExists)) return false;
  if (flags & _vehicleWanderer) return false;

  LaneLocInfo physc = getLaneLoc(ndx, LLPhysical);
  LaneLocInfo pilot = getLaneLoc(ndx, LLPilot);

  if (!validate(pilot.loc.dap)) {
    pilot.loc.dap = getLaneLength(pilot.loc.lane);
    pilot = moveTo(ndx, LLPilot, pilot.loc);
  }

  if (!validate(physc.loc.dap)) {
    physc.loc.dap = getLaneLength(physc.loc.lane);
    physc = moveTo(ndx, LLPhysical, physc.loc);
  }

  float created = creationTime[ndx];
  float age = getCurrentDateTime() - created;
  if (age > vTripLimit[ndx]) {
    removeVehicleBack(ndx);
    return false;
  }

  item trailingNdx = vTrailing[ndx];
  //if (ndx == getSelection()) {
    //SPDLOG_INFO("{} trailingNdx{} pilot{}@{} physc{}@{}",
        //ndx, trailingNdx,
        //pilot.loc.lane, pilot.loc.dap,
        //physc.loc.lane, physc.loc.dap);
  //}

  if (trailingNdx > 0) {
    LaneLocInfo trailingLoc = getLaneLoc(trailingNdx, LLPhysical);
    GraphLocation tloc = trailingLoc.loc;
    float tLength = getLaneLength(tloc.lane);
    if (tloc.dap > tLength) tloc.dap = tLength;
    //pilot = moveTo(ndx, LLPilot, trailingLoc.loc);
    float trailerDist = vehicleLength(ndx)*.5f +
      vehicleLength(trailingNdx)*.5f + 0.5f;

    if (physc.loc.lane/10 == tloc.lane/10) {
      float target = tloc.dap - trailerDist;
      float advance = target - physc.loc.dap;
      if (advance > 0) {
        physc = moveInLane(ndx, LLPhysical, advance);
      }

    } else if (tloc.dap > trailerDist) {
      GraphLocation target = tloc;
      target.dap -= trailerDist;
      if (target.dap > 0) {
        physc = moveToLane(ndx, LLPhysical, target);
      }

    } else {
      float physLength = getLaneLength(physc.loc.lane);
      float target = tloc.dap - trailerDist + physLength;
      target = clamp(target, 0.f, physLength);
      float advance = target - physc.loc.dap;
      if (advance > 0) {
        physc = moveInLane(ndx, LLPhysical, advance);
      }
    }

    pilot = moveTo(ndx, LLPilot, physc.loc);
    vSpeed.set(ndx, vSpeed[trailingNdx]);
    flags |= _vehiclePlaced;
    vFlags.set(ndx, flags);
    return false;
  }

  // Get next block
  Route* route = vRoute.get(ndx);
  validateRoute(ndx);
  Location nextStep = 0;
  item routeSize = route->steps.size();

  //SPDLOG_INFO("vehicleUpdate:{} stepType:{} stepNdx:{} pilot:{}@{}/{}",
      //ndx, nextStepType, nextStepNdx, pilot.loc.lane,
      //pilot.loc.dap, laneLength);

  if (routeSize > route->currentStep) {
    nextStep = route->steps[route->currentStep];
    if (nextStep/10 == pilot.loc.lane/10 && routeSize > route->currentStep+1) {
      //SPDLOG_INFO("advance route step:{} pilot:{} {}/{}",
          //nextStep, pilot.loc.lane, route->currentStep, routeSize);
      //if (ndx == getSelection()) {
        //SPDLOG_INFO("advance by pilot");
      //}
      route->currentStep ++;
      validateRoute(ndx);
      nextStep = route->steps[route->currentStep];
    }

    //if (flags & _vehicleIsTransit) {
      //GraphLocation pilotLoc = vPilot[ndx];
      //SPDLOG_INFO("pilotLoc:{}@{}", pilotLoc.lane, pilotLoc.dap);
      //char* routeTxt = routeString(route);
      //SPDLOG_INFO("route {}", routeTxt);
      //free(routeTxt);
    //}
  } else {
    handleVehicleAtDest(ndx);
  }

  item nextStepType = locationType(nextStep);
  item nextStepNdx = locationNdx(nextStep);
  bool atDap = nextStepType == LocDap;
  if (ndx == getSelection()) {
    SPDLOG_INFO("vehicleUpdate:{} nextStep:{} pilot:{}@{}/{} atDap:{}",
        ndx, format(nextStep), pilot.loc.lane,
        pilot.loc.dap, getLaneLength(pilot.loc.lane), atDap);
    char* routeTxt = routeString(route);
    SPDLOG_INFO("route {}", routeTxt);
    free(routeTxt);
  }

  if (nextStepType != LocLaneBlock && nextStepType != LocDap) {
    handleVehicleAtDest(ndx);
    return false;

  } else if (route->currentStep >= routeSize-1 &&
      nextStepType == LocLaneBlock &&
      nextStepNdx/10 == pilot.loc.lane/10) {
    atDap = true;
    nextStepNdx = getLaneLength(pilot.loc.lane);
  }

  if (pilot.loc.lane == 0) {
    GraphLocation pilotLoc = vPilot[ndx];
    if (pilotLoc.lane == 0 && nextStepType == LocLaneBlock) {
      pilotLoc.lane = nextStep;
      if (route->currentStep+1 < routeSize) {
        item dapStep = route->steps[route->currentStep+1];
        if (locationType(dapStep) == LocDap) {
          pilotLoc.dap = locationNdx(dapStep);
        }
      }
      vPilot.set(ndx, pilotLoc);
    }

    if ((flags & _vehicleHasRoute) && canEnterLane_v(pilotLoc)) {
      laneEnterTime.set(ndx, simulationTime);
      pilot = moveToLane(ndx, LLPilot, pilotLoc);
      physc = moveToLane(ndx, LLPhysical, pilotLoc);
      //SPDLOG_INFO("moveToLane init {} pilot{}@{}",
          //ndx, pilot.loc.lane, pilot.loc.dap);
    } else {
      return false;
    }

    //SPDLOG_INFO("unplaced vehicle {} {} {} {}",
        //ndx, flags & _vehiclePlaced, flags & _vehicleExists,
        //pilot.loc.lane);
    //removeVehicleBack(ndx);
    //return false;

    /*
    if (route->size() > 0) {
      GraphLocation loc = vPilot[ndx];
      recordLaneLeave(ndx);
      pilot = moveToLane(ndx, LLPilot, loc);
      physc = moveToLane(ndx, LLPhysical, loc);
    } else {
      return false;
    }
    */

    //SPDLOG_INFO("vehicle placed {} {} {} {}",
     //   ndx, flags & _vehiclePlaced, flags & _vehicleExists,
      //  pilot.loc.lane);
  }

  flags |= _vehiclePlaced;
  vFlags.set(ndx, flags);

  // Get mergeTarget, refine nextLane
  item nextBlk2 = routeSize > route->currentStep+1 ?
    route->steps[route->currentStep+1] : 0;
  if (locationType(nextBlk2) != LocLaneBlock) nextBlk2 = 0;
  item nextLane = nextStepType == LocLaneBlock ? nextStep : 0;
  item mergeTarget = pilot.loc.lane;
  bool validRoute = atDap;
  item pilotBlk = (pilot.loc.lane/10)*10;
  item numLanes = getBlockLanes(pilotBlk);
  float laneLength = getLaneLength(pilot.loc.lane);
  float distToEnd = laneLength - pilot.loc.dap;
  float dapRatio = pilot.loc.dap/laneLength;
  float bestMergeScore = -100;
  float speed = vSpeed[ndx];

  //if (getSelection() == ndx) {
    //SPDLOG_INFO("vehicleUpdate:{} stepType:{} stepNdx:{} pilot:{}@{}/{}",
        //ndx, nextStepType, nextStepNdx, pilot.loc.lane,
        //pilot.loc.dap, laneLength);
    //char* routeTxt = routeString(route);
    //SPDLOG_INFO("route {}", routeTxt);
    //free(routeTxt);
  //}

  for (int i = 0; i < numLanes; i++) {
    item lane = pilotBlk+i;
    bool hasDrain = false;

    if (atDap) {
      if (i == numLanes-1) hasDrain = true;

    } else if (nextLane != 0) {
      item* drains = getLaneDrains(lane);
      for (int j = 0; j < maxLaneDrains; j++) {
        item drain = drains[j];
        //SPDLOG_INFO("check {} drain:{} next:{}", lane, drain, nextLane);
        if (drain == 0) {
          break;
        } else if (drain/10 == nextLane/10) {
          hasDrain = true;
          validRoute = true;
          if (lane == pilot.loc.lane) {
            nextLane = drain;
          }
        }
      }
    }

    //float score = getMergeScore(ndx, lane, nextStep);
    float score = 20;
    score -= numVehiclesInLaneBack(lane)*(1-dapRatio);
    if (lane == pilot.loc.lane) {
      score += 10+ndx%10;
    }

    if (hasDrain) {
      score += 100*dapRatio;
    }

    if (score > bestMergeScore) {
      bestMergeScore = score;
      mergeTarget = lane;
    }
  }

  /*
  if (getSelection() == ndx) {
    SPDLOG_INFO("vehicleUpdate validRoute:{}", validRoute);
  }
  */

  if (!validRoute && nextStep/10 != pilot.loc.lane/10 &&
      nextStepType == LocLaneBlock && nextStep >= 10) {

    removeVehicleBack(ndx);
    return false;
    /*
    if (distToEnd < 10) {
    //SPDLOG_INFO("invalid route {}", routeString(route));
      //invalidateRouteCommandBack(ndx);
      removeVehicleBack(ndx);
      return;
    } else if (flags & _vehicleHasRoute) {
      flags &= ~_vehicleHasRoute;
      vFlags.set(ndx, flags);
      rerouteCommandBack(ndx);
    }
    */
  }

  Impediment mergeImpediment;
  mergeImpediment.dist = 0;
  mergeImpediment.speed = 0;
  mergeImpediment.active = false;
  if (mergeTarget != pilot.loc.lane) {
    if (mergeTarget < pilot.loc.lane) {
      mergeTarget = pilot.loc.lane-1;
    } else if (mergeTarget > pilot.loc.lane) {
      mergeTarget = pilot.loc.lane+1;
    }

    // Get mergeConflict
    GraphLocation mergeLoc = graphLocation(mergeTarget, pilot.loc.dap);
    LaneLocInfo mergeConflict = getNearestLaneLoc(mergeLoc);
    while (mergeConflict.loc.dap < pilot.loc.dap && mergeConflict.ahead != 0) {
      mergeConflict = getLaneLoc(mergeConflict.ahead, mergeConflict.aheadType);
    }

    // Make the merge, if we can
    //mergeImpediment = asImpediment(mergeConflict);
    float dist = abs(mergeConflict.loc.dap - pilot.loc.dap);
    float speedDiff = mergeImpediment.speed - speed;
    if (dist > c(CMergeSpace) || abs(speedDiff) < c(CMergeMaxSpeedDiff)) {
      pilot = moveToLane(ndx, LLPilot, mergeLoc);
      physc = moveToLane(ndx, LLPhysical, mergeLoc);
      //SPDLOG_INFO("moveToLane merge {} pilot{}@{}",
          //ndx, pilot.loc.lane, pilot.loc.dap);

    // Otherwise, get the merge impediment
    } else {
      if (mergeConflict.loc.dap < pilot.loc.dap) {
        mergeImpediment.active = false;
      } else {
        mergeImpediment.dist -= pilot.loc.dap;
      }
    }
  }

  // Get impediment and advance
  Impediment impediment = combine(mergeImpediment,
    getImpediment(ndx, nextLane, distToEnd));

  if (atDap) {
    Impediment endImpediment;
    endImpediment.active = true;
    endImpediment.dist = nextStepNdx - pilot.loc.dap;
    endImpediment.speed = 0;
    impediment = combine(impediment, endImpediment);
    nextStepNdx = clamp(float(nextStepNdx), 0.f, laneLength-10);
  }

  float targetSpeed = getBlockSpeed(physc.loc.lane)*c(CVehicleSpeed);
  float advance = targetSpeed * duration;
  float length = vehicleLength(ndx);
  if (impediment.active) {
    float maxAdvance = impediment.dist;
    if (atDap || nextLane != 0) {
      maxAdvance -= length - c(CBumperDistance);
    }
    advance = std::min(maxAdvance, advance);
  }

  // Apply backpressure to prevent laneblocks from overfilling
  if (nextStep > 0 && nextStepType == LocLaneBlock) {
    int vehiclesInBlks = 1;
    float blocksCapacity = 1 + (ndx%20);
    int nextCap = getBlockCapacity_v(nextStep);
    //int myCap = getBlockCapacity_v(pilot.loc.lane);
    //blocksCapacity += myCap;
    //vehiclesInBlks += numVehiclesInBlock_v(pilot.loc.lane);
    blocksCapacity += nextCap;
    vehiclesInBlks += numVehiclesInBlock_v(nextStep);
    if (nextBlk2 > 0) {
      vehiclesInBlks += numVehiclesInBlock_v(nextBlk2);
      blocksCapacity += getBlockCapacity_v(nextBlk2);
    }

    float backpressureFactor = blocksCapacity/vehiclesInBlks *
      c(CBackpressureFactor) - c(CBackpressureBias);
    backpressureFactor = mix(1.f, backpressureFactor, dapRatio);
    backpressureFactor = clamp(backpressureFactor, 0.f, 1.f);
    //SPDLOG_INFO("backpressure {} {} {} {} {}", blocksCapacity,
      //vehiclesInBlks, backpressureFactor, nextStep, nextBlk2);
    advance *= backpressureFactor;
  }

  // Apply acceleration
  float maxSpeed = speed +
    c(CVehicleAcceleration) * duration;
  advance = clamp(advance, 0.f, maxSpeed * duration);

  pilot = moveInLane(ndx, LLPilot, advance);
  physc = moveInLane(ndx, LLPhysical, advance);
  if (duration > 0) {
    vSpeed.set(ndx, advance/duration);
  }

  //if (ndx == getSelection()) {
    //SPDLOG_INFO("vehicle {} atDap:{} nextLane:{} pilot:{}@{}/{}",
        //ndx, atDap, format(nextLane), pilot.loc.lane, pilot.loc.dap,
        //laneLength);
    //SPDLOG_INFO("{}", routeString(route));
  //}

  // Move to next lane, if at end
  if (!atDap && pilot.loc.dap >= laneLength-0.1) {
    if (nextLane == pilot.loc.lane) {
      //SPDLOG_WARN("nextLane == pilot.loc.lane {}",
          //routeString(route));
      //if (ndx == getSelection()) {
        //SPDLOG_INFO("advance lane end");
      //}
      route->currentStep ++;
      validateRoute(ndx);

    } else if (nextLane == 0 || locationType(nextLane) != LocLaneBlock) {
      handleVehicleAtDest(ndx);

    } else {
      GraphLocation target = pilot.loc;
      target.lane = nextLane;
      target.dap -= laneLength;
      //target.dap = clamp(target.dap, 0.f, getLaneLength(nextLane));
      //float laneLength = getLaneLength(nextLane);
      //if (target.dap > laneLength && laneLength > 10) {
      //  target.dap = laneLength;
      //}
      recordLaneLeave(ndx);
      pilot = moveToLane(ndx, LLPilot, target);
      physc = moveToLane(ndx, LLPhysical, target);
      return target.dap > laneLength;
      //SPDLOG_INFO("moveToLane at end {} pilot{}@{}",
          //ndx, pilot.loc.lane, pilot.loc.dap);
    }

  } else if (atDap && pilot.loc.dap+length >= nextStepNdx) {
    handleVehicleAtDest(ndx);
  }

  return false;
}

void swapOneVehicleKeyframe(item ndx, int keyframe) {
  //uint32_t flags = vFlags[ndx];
  //if (!(flags & _vehicleExists)) return;

  LaneLocInfo physc = getLaneLoc(ndx, LLPhysical);
  setKeyframe(keyframe, ndx, physc.loc);
}

void oneVehicleUpdate() {
  simulationTime += c(CVehicleUpdateTime);
  updateIntersections_v(c(CVehicleUpdateTime));
  swapInterpolator(simulationTime);
  int keyframe = getPhysicalKeyframe();
  for (int i = 1; i < vFlags.size(); i++) {
    for (int j = 0; j < 10; j++) {
      float dur = j == 0 ? c(CVehicleUpdateTime) : 0;
      if (!updateOneVehicle(i, dur)) break;
    }
    swapOneVehicleKeyframe(i, keyframe);
  }
}

void swapOneVehicle(item ndx, item keyframe) {
  if (ndx <= 0 || ndx >= sizeVehicles()) return;
  Vehicle* v = getVehicle(ndx);
  uint32_t flags = vFlags[ndx];
  if (!(flags & _vehicleExists)) return;
  if (flags & _vehicleWanderer) return;

  /*
  if (v->flags & _vehicleExists) {
    flags |= _vehicleExists;
  } else {
    flags &= ~_vehicleExists;
  }

  if (v->flags & _vehicleHeadlights) {
    flags |= _vehicleHeadlights;
  } else {
    flags &= ~_vehicleHeadlights;
  }
  */

  v->flags &= ~_vehicleSwapMask;
  v->flags |= flags & _vehicleSwapMask;

  LaneLocInfo physc = getLaneLoc(ndx, LLPhysical);
  LaneLocInfo pilot = getLaneLoc(ndx, LLPilot);
  vec3 oldPos = vLoc[ndx];
  vec3 pos = getLocationV(physc.loc);
  if (validate(pos)) {
    vLoc.set(ndx, pos);
    //vec3 vel = (pos-oldPos)/c(CVehicleUpdateTime);
    //vVel.set(ndx, vel);
  } else {
    pos = oldPos;
  }

  //v->location = pos;
  setKeyframe(keyframe, ndx, physc.loc);

  copyRoute(vRoute.get(ndx), &v->route);
  validateRoute(ndx);
  v->laneLoc = physc.loc;
  v->pilot = pilot.loc;
  v->travelGroups = vPassengers[ndx];
  v->numPassengers = vNumPassengers[ndx];
  //if (v->pilot.lane == 0) {
    //v->pilot = vPilot[ndx];
  //}
  v->vehicleAhead[0] = getVehicleAheadFull(ndx, LLPhysical).vNdx;
  v->vehicleAhead[1] = getVehicleAheadFull(ndx, LLPilot).vNdx;
  v->yieldTo = vYieldTo[ndx];
  v->yieldFrom = vYieldFrom[ndx];

  setVehicleActive_g(ndx);

  /*
  if (ndx == getSelection()) {
    item pilotLane = vPilot[ndx].lane;
    SPDLOG_INFO("vehicle {}: pilot:{}@{} physc:{}@{}, vPilot:{} canEnter:{}"
        " block cap:{} numVehiclesInBlock_v:{}",
        ndx, pilot.loc.lane, pilot.loc.dap, physc.loc.lane, physc.loc.dap,
        vPilot[ndx].lane, canEnterLane_v(vPilot[ndx]),
        getBlockCapacity_v(pilotLane), numVehiclesInBlock_v(pilotLane));
  }
  */
}

void setTripLimit(item ndx) {
  uint32_t flags = vFlags[ndx];
  bool isTransit = flags & _vehicleIsTransit;
  bool isFreight = flags & _vehicleIsFrieght;
  item model = vModel[ndx];
  float tripLimit = c(CMaxVehicleAge);

  if (!isTransit) {
    Route* route = vRoute.get(ndx);
    float timeEst = computeRouteInfo_g(&route->steps, isTransit, false).time;
    timeEst *= c(CTripLimitMultiplier);
    if (timeEst < tripLimit) tripLimit = timeEst;
    if (!isFreight) {
      float maxCommute = c(CMaxCommute)*oneHour;
      if (maxCommute < tripLimit) tripLimit = maxCommute;
    }
  }

  tripLimit = clamp(tripLimit, float(oneHour), c(CMaxVehicleAge));
  vTripLimit.set(ndx, tripLimit);
}

void swapOneVehicleBack(item ndx) {
  Vehicle* v = getVehicle(ndx);
  uint32_t flags = v->flags;

  /*
  if (vFlags[ndx] & _vehicleExists) {
    flags |= _vehicleExists;
  } else {
    flags &= ~_vehicleExists;
  }
  */

  vFlags.set(ndx, flags);
  if (flags & _vehicleExists) {
    vLoc.set(ndx, v->location);
    setAllKeyframes(ndx, v->laneLoc);
    vVel.set(ndx, v->velocity);
    vSpeed.set(ndx, length(v->velocity));
    copyRoute(&v->route, vRoute.get(ndx));
    validateRoute(ndx);
    vPilot.set(ndx, v->pilot);
    vTrailing.set(ndx, v->trailing);
    vTrailer.set(ndx, v->trailer);
    vPassengers.set(ndx, v->travelGroups);
    vModel.set(ndx, v->model);
    vYieldTo.set(ndx, v->yieldTo);
    vYieldFrom.set(ndx, v->yieldFrom);
    creationTime.set(ndx, v->creationTime);

    if (vTripLimit[ndx] < oneHour*.5f) {
      setTripLimit(ndx);
    }

    if ((flags & _vehiclePlaced) &&
        (v->route.steps.size() > 0 || v->destination.lane == v->pilot.lane) &&
        canEnterLane_v(v->pilot)) {
      laneEnterTime.set(ndx, simulationTime);
      moveToLane(ndx, LLPilot, v->pilot);
      moveToLane(ndx, LLPhysical, v->laneLoc);
    } else {
      leaveLane(ndx);
    }

    int num = 0;
    for (int i = 0; i < v->travelGroups.size(); i++) {
      item groupNdx = v->travelGroups[i];
      num += getTravelGroupSize_v(groupNdx);
    }
    vNumPassengers.set(ndx, num);
    v->numPassengers = num;

  } else {
    vLoc.set(ndx, vec3(-1,-1,0));
    clearRoute(vRoute.get(ndx));
    validateRoute(ndx);
    creationTime.set(ndx, 0);
    vTripLimit.set(ndx, 0);
    leaveLane(ndx);
  }

  setAllKeyframes(ndx, v->laneLoc);
  //SPDLOG_INFO("swap vehicle {} {} {} {} {}",
      //ndx, flags & _vehiclePlaced, flags & _vehicleExists,
      //getLaneLoc(ndx, LLPilot).loc.lane, v->pilot.lane);
}

void addPhysicalVehicle(item ndx) {
  resizePhysicalVehicles(ndx+1);
  vFlags.set(ndx, _vehicleExists);
  swapOneVehicleBack(ndx);
}

void removePhysicalVehicle(item ndx) {
  removeAllGroupsFromVehicle_v(ndx);
  leaveLane(ndx);
  vFlags.set(ndx, 0);
  if (ndx < vRoute.size()) {
    clearRoute(vRoute.get(ndx));
    validateRoute(ndx);
  }
}

void recieveVehicleRoute(item ndx) { //, Route* route) {
  resizePhysicalVehicles(ndx+1);
  int flags = vFlags[ndx];
  flags |= _vehicleHasRoute;
  vFlags.set(ndx, flags);
  Vehicle* v = getVehicle(ndx);
  Route* route = &v->route;
  copyRoute(route, vRoute.get(ndx));
  validateRoute(ndx);

  if (!(flags & _vehiclePlaced)) {
    vPilot.set(ndx, getNextGraphLoc(route));

  } else if (flags & _vehicleIsTransit) {
    LaneLocInfo pilot = getLaneLoc(ndx, LLPilot);
    item lane = pilot.loc.lane < 10 ? vPilot[ndx].lane : pilot.loc.lane;
    int step = route->steps.size() - 1;
    for (; step >= 0; step--) {
      if (route->steps[step]/10 == lane/10) {
        break;
      }
    }

    if (step >= 0) {
      route->currentStep = step;
    } else {
      removeVehicleBack(ndx);
    }
  }
}

void swapVehiclePhysics() {
  //logStage("swapLanesForVehicles");
  swapLanesForVehicles(false);
  //logStage("swapIntersections_v");
  swapIntersections_v(false);

  //logStage("swapOneVehicle");
  item currentKeyframe = getPhysicalKeyframe();
  for (int i = 1; i < vFlags.size(); i++) {
    swapOneVehicle(i, currentKeyframe);
  }

  //SPDLOG_INFO("swapVehiclePhysics done");
}

void swapVehicleKeyframe() {
  //logStage("swapInterpolator");
  swapInterpolator(simulationTime);

  //logStage("swapOneVehicleKeyframe");
  item currentKeyframe = getPhysicalKeyframe();
  for (int i = 1; i < vFlags.size(); i++) {
    swapOneVehicleKeyframe(i, currentKeyframe);
  }

  //SPDLOG_INFO("swapVehiclePhysics done");
}

void swapVehiclePhysicsBack() {
  swapVehicleModels();
  swapLanesForVehicles(true);
  swapIntersections_v(true);
  resizePhysicalVehicles(sizeVehicles()+1);
  for (int i = 1; i < vFlags.size(); i++) {
    swapOneVehicleBack(i);
  }
}

void validatePhysicalVehicles(char* msg) {
  for (int i = 1; i < vFlags.size(); i++) {
    uint32_t flags = vFlags[i];
    bool transit = flags & _vehicleIsTransit;
    Route* route = vRoute.get(i);
    validateRoute(i);
    if (route->steps.size() > 4) {
      bool anyTransit = false;
      bool anyBlk = false;
      for (int j = 0; j < route->steps.size(); j++) {
        Location loc = route->steps[j];
        item type = locationType(loc);
        if (type == LocLaneBlock) {
          anyBlk = true;
        } else if (type == LocTransitLeg || type == LocTransitStop) {
          anyTransit = true;
        } else if (type != LocDap) {
          SPDLOG_ERROR("{} - invalid vehicle({}) route: {}",
              msg, j, routeString(route));
          handleError("invalid vehicle");
        }
      }

      if (!anyBlk) {
        SPDLOG_ERROR("vehicle route is commuter {}:{} - {}",
            i, routeString(route), msg);
        handleError("invalid vehicle");
      } else if (anyTransit != transit) {
        SPDLOG_ERROR("{} vehicle route {}:{} - {}",
            transit ? "transit" : "private",
            i, routeString(route), msg);
        handleError("invalid vehicle");
      }
    }
  }
}

void validateSwapPhysicalVehicles(char* msg) {
  for (int i = 1; i < vFlags.size(); i++) {
    Vehicle* v = getVehicle(i);
    uint32_t flags = vFlags[i];
    if (!(v->flags & _vehicleExists)) continue;
    if (!(flags & _vehicleExists)) continue;

    bool ptransit = flags & _vehicleIsTransit;
    bool vtransit = v->flags & _vehicleIsTransit;

    if (ptransit != vtransit) {
      SPDLOG_ERROR("{} - physical vehicle({}) is {} but game vehicle is {}",
          msg, i,
          ptransit ? "Transit" : "Private",
          vtransit ? "Transit" : "Private");
      handleError("invalid vehicle");
    }
  }
}

