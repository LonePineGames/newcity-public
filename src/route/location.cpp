#include "location.hpp"

#include "../building/building.hpp"
#include "../economy.hpp"
#include "../graph/stop.hpp"
#include "../graph/transit.hpp"
#include "../graph/transitRouter.hpp"
#include "../lane.hpp"
#include "../money.hpp"
#include "../string_proxy.hpp"
#include "../time.hpp"
#include "../vehicle/travelGroup.hpp"
#include "../vehicle/vehicle.hpp"

#include <cstdlib>
#include "spdlog/spdlog.h"

Location transitLegStopLocation(item lineNdx, item stopNdx) {
  Location result = LocTransitLeg << _locationTypeShift;
  result |= lineNdx << _locationLegLineShift;
  result |= stopNdx;
  return result;
}

Location buildingLocation(item buildingNdx) {
  Location result = LocBuilding << _locationTypeShift;
  result |= buildingNdx;
  return result;
}

Location vehicleLocation(item vNdx) {
  Location result = LocVehicle << _locationTypeShift;
  result |= vNdx;
  return result;
}

Location travelGroupLocation(item tgNdx) {
  Location result = LocTravelGroup << _locationTypeShift;
  result |= tgNdx;
  return result;
}

Location dapLocation(item dap) {
  Location result = LocDap << _locationTypeShift;
  result |= dap;
  return result;
}

Location transitStopLocation(item ndx) {
  Location result = LocTransitStop << _locationTypeShift;
  result |= ndx;
  return result;
}

item locationType(Location location) {
  return location >> _locationTypeShift;
}

item locationNdx(Location location) {
  return location & ~_locationTypeMask;
}

item locationLineNdx(Location location) {
  return (location & ~_locationTypeMask) >> _locationLegLineShift;
}

item locationLegNdx(Location location) {
  return location & 4095;
}

item getStopForLeg_g(Location location) {
  return getStopForLeg_r(location);
  //TransitLine* line = getTransitLine(locationLineNdx(location));
  //item ndx = locationLegNdx(location);
  //if (ndx < line->legs.size()) {
    //return line->legs[ndx].stop;
  //} else {
    //return 0;
  //}
}

vec3 locationToWorldspace_g(Location loc) {
  if (loc <= 10) return vec3(0,0,0);
  item type = locationType(loc);
  if (type == LocLaneBlock) {
    GraphLocation gl;
    gl.lane = loc;
    gl.dap = 0;
    return getLocation(gl);

  } else if (type == LocTransitStop) {
    return getStopLocation_r(locationNdx(loc));

  } else if (type == LocTransitLeg) {
    return locationToWorldspace_g(getStopForLeg_g(loc));

  } else if (type == LocVehicle) {
    return getVehicle(locationNdx(loc))->location;

  } else if (type == LocTravelGroup) {
    return locationToWorldspace_g(
        getTravelGroup_g(locationNdx(loc))->location);

  } else if (type == LocBuilding) {
    return getBuilding(locationNdx(loc))->location;

  } else {
    return vec3(0,0,0);
  }
}

/*
RouteInfo computeRouteInfo_g(vector<Location> route, bool walking, bool bus) {
  RouteInfo result;
  Location lastTransitLeg = 0;
  const float walkingSpeedFactor = oneHour / c(CWalkingSpeed) / 1000;
  bool wasWalking = false;
  vec3 lastWorldspace = vec3(0,0,0);
  item lastLaneBlock = 0;
  item linesTaken = 0;
  bool transit = false;

  for (int i = 0; i < route.size(); i++) {
    Location loc = route[i];
    item type = locationType(loc);

    if (type == LocLaneBlock) {
      if (lastLaneBlock == loc) continue;
      lastLaneBlock = loc;

      vec3 newLoc = getBlockLoc_r(loc);
      if (wasWalking) {
        float dist = length(lastWorldspace - newLoc);
        result.walkingTime += dist * walkingSpeedFactor;
      } else if (!walking) {
        result.travelTime += laneBlockCost_r(loc);
      }

      lastWorldspace = newLoc;
      wasWalking = walking;

    } else if (type == LocTransitStop) {
      vec3 newLoc = getStopLocation_r(locationNdx(loc));
      if (wasWalking) {
        float dist = length(lastWorldspace - newLoc);
        result.walkingTime += dist * walkingSpeedFactor;
      }
      lastWorldspace = newLoc;
      wasWalking = walking;

    } else if (type == LocTransitLeg) {
      wasWalking = false;
      transit = true;
      if (lastTransitLeg == 0) {
        item line = locationLineNdx(loc);
        TransitLine* l = getTransitLine(line);
        TransitSystem* s = getTransitSystem(l->system);
        result.ticketCost += linesTaken == 0 ? s->ticketPrice :
          s->transferPrice;
        result.waitTime += getLegWaitCost_r(loc);
        linesTaken ++;
        lastTransitLeg = loc;

      } else {
        float* cost = getLineLegCostTable_r(lastTransitLeg);
        item entryNdx = locationLegNdx(lastTransitLeg);
        item exitNdx = locationLegNdx(loc);
        item diff = exitNdx - entryNdx;
        if (diff < 0 ||
            locationLineNdx(lastTransitLeg) != locationLineNdx(loc)) {
          SPDLOG_WARN("bad transit route {}", routeString(route, -1));
          //handleError("Bad Transit Route");
        }

        if (cost != NULL) {
          for (int j = 0; j < diff; j++) {
            result.travelTime += cost[j];
          }
        }
        lastTransitLeg = 0;
      }
    }
  }

  float time = result.travelTime * 24*60; // to minutes;
  money fuelPrice = getStatistic(ourCityEconNdx(), FuelPrice);
  money fuelUsage = bus ? c(CLitersPerMinuteBus) : c(CLitersPerMinuteCar);
  money maint = bus ? c(CMaintPerMinuteBus) : c(CMaintPerMinuteCar);
  maint *= getInflation();
  result.fuelMaintCost = (fuelUsage * fuelPrice + maint) * time;

  result.time = result.walkingTime + result.waitTime + result.travelTime;
  result.multiplier = transit ? c(CTransitBias) : 1;
  result.cost = (transit && !bus) ? result.ticketCost : result.fuelMaintCost;
  result.costAdjustedTime = result.time * result.multiplier +
    moneyToTime(result.cost);

  return result;
}

/*
float estimateRouteCost_g(vector<Location> route, bool walking) {
  float estimate = 0;
  Location lastTransitLeg = 0;
  const float walkingSpeedFactor = oneHour / c(CWalkingSpeed) / 1000;
  bool wasWalking = false;
  vec3 lastWorldspace = vec3(0,0,0);
  item lastLaneBlock = 0;

  for (int i = 0; i < route.size(); i++) {
    Location loc = route[i];
    item type = locationType(loc);

    if (type == LocLaneBlock) {
      if (lastLaneBlock == loc) continue;
      lastLaneBlock = loc;

      vec3 newLoc = getBlockLoc_r(loc);
      if (wasWalking) {
        float dist = length(lastWorldspace - newLoc);
        estimate += dist * walkingSpeedFactor;
      } else if (!walking) {
        estimate += laneBlockCost_r(loc);
      }

      lastWorldspace = newLoc;
      wasWalking = walking;

    } else if (type == LocTransitStop) {
      vec3 newLoc = getStopLocation_r(locationNdx(loc));
      if (wasWalking) {
        float dist = length(lastWorldspace - newLoc);
        estimate += dist * walkingSpeedFactor;
      }
      lastWorldspace = newLoc;
      wasWalking = walking;

    } else if (type == LocTransitLeg) {
      wasWalking = false;
      if (lastTransitLeg == 0) {
        //estimate += getLegWaitCost_r(loc);
        lastTransitLeg = loc;

      } else {
        float* cost = getLineLegCostTable_r(lastTransitLeg);
        item entryNdx = locationLegNdx(lastTransitLeg);
        item exitNdx = locationLegNdx(loc);
        item diff = exitNdx - entryNdx;
        if (diff < 0 ||
            locationLineNdx(lastTransitLeg) != locationLineNdx(loc)) {
          SPDLOG_WARN("bad transit route {}", routeString(route, -1));
          //handleError("Bad Transit Route");
        }

        if (cost != NULL) {
          for (int j = 0; j < diff; j++) {
            estimate += cost[j];
          }
        }
        lastTransitLeg = 0;
      }
    }
  }
  return estimate;
}

float estimateRouteCost_g(Route route, bool walking) {
  return estimateRouteCost_g(route.steps, walking);
}

double getRouteTransitPrice_g(vector<Location> route) {
  Location lastTransitLeg = 0;
  item linesTaken = 0;
  float result = 0;

  for (int i = 0; i < route.size(); i++) {
    Location loc = route[i];
    item type = locationType(loc);
    if (type == LocTransitLeg) {
      if (lastTransitLeg == 0) {

        item line = locationLineNdx(loc);
        TransitLine* l = getTransitLine(line);
        TransitSystem* s = getTransitSystem(l->system);
        result += linesTaken == 0 ? s->ticketPrice : s->transferPrice;
        linesTaken ++;
        lastTransitLeg = loc;

      } else {
        lastTransitLeg = 0;
      }
    }
  }

  return result;
}

double getRouteFuelMaintPrice_r(vector<Location> route, bool bus) {
  money time = 0;

  for (int i = 0; i < route.size(); i++) {
    Location loc = route[i];
    item type = locationType(loc);
    if (type == LocLaneBlock) {
      time += laneBlockCost_r(loc);
    }
  }

  time *= 24*60; // to minutes;
  money fuelPrice = getStatistic(ourCityEconNdx(), FuelPrice);
  money fuelUsage = bus ? c(CLitersPerMinuteBus) : c(CLitersPerMinuteCar);
  money maint = bus ? c(CMaintPerMinuteBus) : c(CMaintPerMinuteCar);
  maint *= getInflation();
  return (fuelUsage * fuelPrice + maint) * time;
}
*/

/*
bool isRouteValid_g(vector<Location> route, Location start, Location end) {
  //TODO: handle transit
  int routeS = route.size();
  if (routeS == 0) return false;
  Location lastLaneBlock = 0;

  //if (route[0]/10 != start/10) return false;
  //if (route[routeS-1]/10 != end/10) return false;

  for (int i = 0; i < routeS; i++) {
    Location loc = route[i];
    if ((locationType(loc)) != LocLaneBlock) continue;
    if (lastLaneBlock == 0) {
      if (loc/10 != start/10) return false;

    } else {
      LaneBlock* blk = getLaneBlock(lastLaneBlock);
      bool anyMatched = false;
      for (int j = 0; j < blk->drains.size(); j++) {
        if (blk->drains[j]/10 == loc/10) {
          anyMatched = true;
          break;
        }
      }
      if (!anyMatched) return false;
    }

    lastLaneBlock = loc;
  }

  return true;
}

char* routeString(vector<Location> steps, item currentStep) {
  int stepS = steps.size();
  if (stepS == 0) return strdup_s("[empty route]");

  int start = currentStep-5;
  if (start < 0) start = 0;
  int end = start+21;
  if (end > stepS) end = stepS;

  char* result = strdup_s(start == 0 ? "" : "...");

  for (int i = start; i < end; i++) {
    char* stepTxt = 0;
    item step = steps[i];
    item type = locationType(step);
    item ndx = locationNdx(step);

    if (type == LocLaneBlock) {
      stepTxt = sprintf_o("blk%d", ndx);
    } else if (type == LocPathBlock) {
      stepTxt = sprintf_o("pth%d", ndx);
    } else if (type == LocDap) {
      stepTxt = sprintf_o("dap%d", ndx);
    } else if (type == LocTransitLeg) {
      item lineNdx = locationLineNdx(step);
      item stopNdx = locationLegNdx(step);
      stepTxt = sprintf_o("line%d-leg%d", lineNdx, stopNdx);
    } else if (type == LocTransitStop) {
      stepTxt = sprintf_o("stp%d", ndx);
    } else if (type == LocVehicle) {
      stepTxt = sprintf_o("vhc%d", ndx);
    } else if (type == LocPedestrian) {
      stepTxt = sprintf_o("ped%d", ndx);
    } else if (type == LocTravelGroup) {
      stepTxt = sprintf_o("grp%d", ndx);
    } else if (type == LocBuilding) {
      stepTxt = sprintf_o("bld%d", ndx);
    } else if (type == LocFail) {
      stepTxt = strdup_s("fail");
    }

    if (stepTxt == 0) continue;

    if (i == currentStep) {
      char* tmp = stepTxt;
      stepTxt = sprintf_o("[%s]", stepTxt);
      free(tmp);
    }

    if (result[0] == '\0') {
      free(result);
      result = stepTxt;

    } else {
      char* tmp = result;
      result = sprintf_o("%s=>%s", result, stepTxt);
      free(stepTxt);
      free(tmp);
    }
  }

  if (end < stepS) {
    char* tmp = result;
    result = sprintf_o("%s...", result);
    free(tmp);
  }

  return result;
}

char* routeString(Route route) {
  return routeString(route.steps, route.currentStep);
}
*/

char* format(const Location& loc) {
  item type = locationType(loc);
  item ndx = locationNdx(loc);

  if (type == LocLaneBlock) {
    return sprintf_o("blk%d", ndx);
  } else if (type == LocPathBlock) {
    return sprintf_o("pth%d", ndx);
  } else if (type == LocDap) {
    return sprintf_o("dap%d", ndx);
  } else if (type == LocTransitLeg) {
    item lineNdx = locationLineNdx(loc);
    item stopNdx = locationLegNdx(loc);
    return sprintf_o("line%d-leg%d", lineNdx, stopNdx);
  } else if (type == LocTransitStop) {
    return sprintf_o("stp%d", ndx);
  } else if (type == LocVehicle) {
    return sprintf_o("vhc%d", ndx);
  } else if (type == LocPedestrian) {
    return sprintf_o("ped%d", ndx);
  } else if (type == LocTravelGroup) {
    return sprintf_o("grp%d", ndx);
  } else if (type == LocBuilding) {
    return sprintf_o("bld%d", ndx);
  } else { //if (type == LocFail) {
    return sprintf_o("fail%d-%d", type, ndx);
  }
};

