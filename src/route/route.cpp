#include "route.hpp"

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

GraphLocation getNextGraphLoc(Route* route) {
  GraphLocation result;
  result.lane = 0;
  result.dap = 0;

  for (int i = route->currentStep; i < route->steps.size(); i++) {
    Location loc = route->steps[i];
    item type = locationType(loc);
    if (type == LocLaneBlock) {
      result.lane = locationNdx(loc);
    } else if (type == LocDap) {
      result.dap = locationNdx(loc);
      if (result.lane != 0) break;
    }
  }

  return result;
}

RouteInfo computeRouteInfo_g(Cup<Location>* steps, bool walking, bool bus) {
  RouteInfo result;
  Location lastTransitLeg = 0;
  const float walkingSpeedFactor = oneHour / c(CWalkingSpeed) / 1000;
  bool wasWalking = false;
  vec3 lastWorldspace = vec3(0,0,0);
  item lastLaneBlock = 0;
  item linesTaken = 0;
  bool transit = false;

  for (int i = 0; i < steps->size(); i++) {
    Location loc = steps->at(i);
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
          SPDLOG_WARN("bad transit route {}",
              routeString(steps->toVector(), -1));
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
  money fuelPrice = getStatistic(nationalEconNdx(), FuelPrice);
  money fuelUsage = bus ? c(CLitersPerMinuteBus) : c(CLitersPerMinuteCar);
  money fuelTaxRate = getTaxRate(FuelTaxIncome);
  money baseCost = fuelUsage * fuelPrice;
  //SPDLOG_WARN("fuelTaxRate {} baseCost {} fuelPrice {} fuelUsage {}",
      //fuelTaxRate, baseCost, fuelPrice, fuelUsage);
  money maint = bus ? c(CMaintPerMinuteBus) : c(CMaintPerMinuteCar);
  maint *= getInflation();

  result.fuelMaintCost = (baseCost * (1 + fuelTaxRate) + maint) * time;
  result.time = result.walkingTime + result.waitTime + result.travelTime;
  result.fuelTax = baseCost * fuelTaxRate * time;
  result.multiplier = (transit && result.time < c(CMaxCommute)*oneHour*.5f) ?
    1.f/c(CTransitBias) : 1;
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
        estimate += getLegWaitCost_r(loc);
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

        for (int j = 0; j < diff; j++) {
          estimate += cost[j];
        }
        lastTransitLeg = 0;
      }
    }
  }
  return estimate;
}

float estimateRouteCost_g(Route route, bool walking) {
  return estimateRouteCost_g(route.steps.toVector(), walking);
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

bool isRouteValid_g(vector<Location> route, Location start, Location end) {
  //TODO: handle transit
  int routeS = route.size();
  if (routeS == 0) return false;
  //if (route[0]/10 != start/10) return false;
  //if (route[routeS-1]/10 != end/10) return false;
  for (int i = 0; i < routeS-1; i++) {
    Location loc = route[i];
    if ((locationType(loc)) != LocLaneBlock) return true;
    LaneBlock* blk = getLaneBlock(route[i]);
    item next = route[i+1];
    if ((locationType(next)) != LocLaneBlock) return true;
    bool anyMatched = false;
    for (int j = 0; j < blk->drains.size(); j++) {
      if (blk->drains[j] == next) {
        anyMatched = true;
        break;
      }
    }
    if (!anyMatched) {
      return false;
    }
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

char* routeString(Route* route) {
  return routeString(route->steps.toVector(), route->currentStep);
}

void clearRoute(Route* route) {
  route->flags = 0;
  route->version = 0;
  route->source = 0;
  route->destination = 0;
  route->currentStep = 0;
  route->steps.clear();
}

void copyRoute(Route* source, Route* dest) {
  dest->flags = source->flags;
  dest->version = source->version;
  dest->source = source->source;
  dest->destination = source->destination;
  dest->currentStep = source->currentStep;
  //dest->steps.clear();
  source->steps.copy(&dest->steps);
}

void fwrite_route(FileBuffer* in, Route* route) {
  fwrite_uint32_t(in, route->flags);
  fwrite_uint32_t(in, route->version);
  fwrite_uint32_t(in, route->source);
  fwrite_uint32_t(in, route->destination);
  fwrite_uint32_t(in, route->currentStep);
  route->steps.write(in);
  //fwrite_location_vector(in, &route.steps);
}

void fread_route(FileBuffer* in, Route* result) {
  result->flags = fread_uint32_t(in);
  result->version = fread_uint32_t(in);
  result->source = fread_uint32_t(in);
  result->destination = fread_uint32_t(in);
  result->currentStep = fread_uint32_t(in);
  result->steps.read(in, in->version);
}

