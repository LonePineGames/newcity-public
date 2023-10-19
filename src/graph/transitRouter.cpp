#include "transitRouter.hpp"

#include "transit.hpp"
#include "stop.hpp"

#include "../cup.hpp"
#include "../util.hpp"

#include <boost/dynamic_bitset.hpp>
#include "spdlog/spdlog.h"

item numStops_r = 0;
item numLines_r = 0;
boost::dynamic_bitset<> stopExists_r(1);
Cup<vec3> stopLoc_r;
Cup<item> legSlot_r;
Cup<item> legSlot_v;
Cup<float> legCost_r;
Cup<float> legWaitCost_r;
Cup<vector<Location>> legsForStop_r;
Cup<item> stopForLeg_r;
Cup<item> stopForLeg_v;
Cup<item> lineLength_r;

void resetTransitRouterData_g() {
  numStops_r = 0;
  numLines_r = 0;
  stopExists_r.clear();
  stopLoc_r.clear();
  legSlot_r.clear();
  legSlot_v.clear();
  legCost_r.clear();
  legWaitCost_r.clear();
  stopForLeg_r.clear();
  stopForLeg_v.clear();
  lineLength_r.clear();

  for (int i = 0; i < legsForStop_r.size(); i++) {
    legsForStop_r.get(i)->clear();
  }
  legsForStop_r.clear();
}

void swapTransitStopData_v() {
  item numLines_v = sizeTransitLines();
  legSlot_v.ensureSize(numLines_v+1);
  stopForLeg_v.empty();

  item currentLegSlot = 0;
  for (int i = 1; i <= numLines_v; i++) {
    TransitLine* line = getTransitLine(i);
    bool exists = (line->flags & _transitExists);
    if (!exists) {
      legSlot_v.set(i, 0);

    } else {
      legSlot_v.set(i, currentLegSlot);
      item lineLength = line->legs.size();

      for (int j = 0; j < lineLength; j++) {
        TransitLeg leg = line->legs[j];
        stopForLeg_v.push_back(leg.stop);
        currentLegSlot ++;
      }
    }
  }
}

void swapTransitRouterData_g() {
  numStops_r = sizeStops();
  numLines_r = sizeTransitLines();

  stopExists_r.resize(numStops_r+1);
  stopLoc_r.ensureSize(numStops_r+1);
  legsForStop_r.ensureSize(numStops_r+1);
  legSlot_r.ensureSize(numLines_r+1);
  lineLength_r.ensureSize(numLines_r+1);
  legCost_r.empty();
  legWaitCost_r.empty();
  stopForLeg_r.empty();

  for (int i = 1; i <= numStops_r; i++) {
    Stop* stop = getStop(i);
    bool exists = (stop->flags & _stopExists) && (stop->flags & _stopComplete);
    stopExists_r[i] = exists;
    stopLoc_r.set(i, stop->location);
    legsForStop_r.get(i)->clear();
  }

  item currentLegSlot = 0;
  for (int i = 1; i <= numLines_r; i++) {
    TransitLine* line = getTransitLine(i);
    bool exists = (line->flags & _transitExists) &&
      (line->flags & _transitEnabled);

    if (!exists) {
      legSlot_r.set(i, 0);
      lineLength_r.set(i, 0);

    } else {
      legSlot_r.set(i, currentLegSlot);
      item lineLength = line->legs.size();
      lineLength_r.set(i, lineLength);
      for (int j = 0; j < lineLength; j++) {
        TransitLeg leg = line->legs[j];
        Location legLoc = transitLegStopLocation(i, j);
        legsForStop_r.get(leg.stop)->push_back(legLoc);
        legWaitCost_r.push_back(line->headway[0]*.5f);
        stopForLeg_r.push_back(leg.stop);
        //float cost = mix(leg.timeEstimate, leg.timeRecord,
            //c(CRoutingTrafficAwareness));
        float cost = leg.timeEstimate;
        cost = clamp(cost, 0.0000000001f, 1.f/24.f);
        legCost_r.push_back(cost);

        currentLegSlot ++;
      }
    }
  }
}

item getNumStops_r() {
  return numStops_r;
}

float getStopsDistance_r(item stop1, item stop2) {
  vec3 loc1 = stopLoc_r[stop1];
  vec3 loc2 = stopLoc_r[stop1];
  return vecDistance(loc1,loc2);
}

vec3 getStopLocation_r(item stopNdx) {
  return stopLoc_r[stopNdx];
}

float transitRoutingEstimate_r(item stop1, item stop2) {
  return 0;
}

vector<item> getNearbyStops_r(vec3 loc1) {
  vector<item> result;

  for (int i = 1; i <= numStops_r; i++) {
    if (!stopExists_r[i]) continue;

    vec3 loc2 = stopLoc_r[i];
    float dist = vecDistance(loc1,loc2);
    //SPDLOG_INFO("getNearbyStops_r {} {} {} {}",
        //i, loc1.x, loc2.x, dist);
    if (dist < c(CStopRadius)) {
      result.push_back(i);
    }
  }

  return result;
}

vector<item> getNearbyStops_r(item stopNdx) {
  return getNearbyStops_r(stopLoc_r[stopNdx]);
}

item walkingCostStops_r(item stop1, item stop2) {
  return getStopsDistance_r(stop1, stop2) / c(CWalkingSpeed);
}

vector<Location> getLegsForStop_r(item stopNdx) {
  return legsForStop_r[stopNdx];
}

item getLineLength_r(Location legNdx) {
  item lineNdx = locationLineNdx(legNdx);
  item stopNdx = locationLegNdx(legNdx);
  if (lineNdx < 0 || lineNdx >= lineLength_r.size()) {
    return 0;
  }
  return lineLength_r[lineNdx] - stopNdx;
}

item getStopForLeg_r(Location legNdx) {
  item lineNdx = locationLineNdx(legNdx);
  item stopNdx = locationLegNdx(legNdx);
  if (lineNdx < 0 || lineNdx >= legSlot_r.size()) {
    return 0;
  }
  item legSlot = legSlot_r[lineNdx];
  item slot = legSlot + stopNdx;
  if (slot < 0 || slot >= stopForLeg_r.size()) {
    return 0;
  }
  return stopForLeg_r[slot];
}

item getStopForLeg_v(Location legNdx) {
  item lineNdx = locationLineNdx(legNdx);
  item stopNdx = locationLegNdx(legNdx);
  if (lineNdx < 0 || lineNdx >= legSlot_v.size()) {
    return 0;
  }
  item legSlot = legSlot_v[lineNdx];
  item slot = legSlot + stopNdx;
  if (slot < 0 || slot >= stopForLeg_v.size()) {
    return 0;
  }
  return stopForLeg_v[slot];
}

float getLegWaitCost_r(Location legNdx) {
  item lineNdx = locationLineNdx(legNdx);
  item stopNdx = locationLegNdx(legNdx);
  if (lineNdx < 0 || lineNdx >= legSlot_r.size()) {
    return FLT_MAX;
  }
  item legSlot = legSlot_r[lineNdx];
  item slot = legSlot + stopNdx;
  if (slot < 0 || slot >= legWaitCost_r.size()) {
    return FLT_MAX;
  }
  return legWaitCost_r[slot];
}

float* getLineLegCostTable_r(Location legNdx) {
  item lineNdx = locationLineNdx(legNdx);
  item stopNdx = locationLegNdx(legNdx);
  if (lineNdx < 0 || lineNdx >= legSlot_r.size()) {
    return NULL;
  }
  item legSlot = legSlot_r[lineNdx];
  item slot = legSlot + stopNdx;
  if (slot < 0 || slot >= legCost_r.size()) {
    return NULL;
  }
  return legCost_r.get(legSlot+stopNdx);
}

