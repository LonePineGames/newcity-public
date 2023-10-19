#include "transitPhysics.hpp"

#include "../game/game.hpp"
#include "../game/update.hpp"
#include "../graph/stop.hpp"
#include "../graph/transit.hpp"
#include "../selection.hpp"
#include "../vehicle/renderPedestrian.hpp"
#include "../vehicle/travelGroup.hpp"
#include "../vehicle/physics.hpp"
#include "../vehicle/update.hpp"

#include "spdlog/spdlog.h"

Cup<vector<item>> groupsAtStop_v;
//Cup<item> waitingAtStop_v;
Cup<item> stopUpdateNdx_v;
Cup<Route> groupRoute_v;
Cup<item> groupLocation_v;
Cup<item> groupSize_v;
//Cup<Cup<item>> transitLineStops_v;
Cup<item> transitLinePassengersNow_v;
Cup<item> transitLinePassengersEver_v;

void resetTransitPhysics() {
  for (int i = 0; i < groupsAtStop_v.size(); i++) {
    groupsAtStop_v.get(i)->clear();
  }

  for (int i = 0; i < groupRoute_v.size(); i++) {
    clearRoute(groupRoute_v.get(i));
  }

  /*
  for (int i = 0; i < transitLineStops_v.size(); i++) {
    transitLineStops_v.get(i)->clear();
  }
  */

  groupsAtStop_v.clear();
  stopUpdateNdx_v.clear();
  groupRoute_v.clear();
  groupLocation_v.clear();
  groupSize_v.clear();
  //transitLineStops_v.clear();
  transitLinePassengersNow_v.clear();
  transitLinePassengersEver_v.clear();
}

void moveGroupTo_v(item groupNdx, Location newLoc) {
  groupLocation_v.ensureSize(groupNdx+1);
  Location oldLoc = groupLocation_v[groupNdx];
  if (oldLoc == newLoc) return;

  item oldType = locationType(oldLoc);
  item oldNdx = locationNdx(oldLoc);
  item newType = locationType(newLoc);
  item newNdx = locationNdx(newLoc);

  if (oldType == LocVehicle) {
    removeTravelGroupFromVehicle_v(groupNdx, oldNdx);

  } else if (oldType == LocTransitStop) {
    groupsAtStop_v.ensureSize(oldNdx+1);
    //waitingAtStop_v.ensureSize(oldNdx+1);

    vector<item>* groups = groupsAtStop_v.get(oldNdx);
    //item num = waitingAtStop_v[oldNdx];
    for (int i = groups->size()-1; i >= 0; i--) {
      item ndx = groups->at(i);
      if (groupNdx == ndx) {
        //num -= groupSize_v[groupNdx];
        groups->erase(groups->begin()+i);
      }
    }
    //waitingAtStop_v.set(oldNdx, num);

  } else if (oldLoc != 0) {
    SPDLOG_WARN("travelGroup({}) removed from unknown loc: {}",
        groupNdx, format(oldLoc));
      //handleError("bad move");
  }

  if (newType == LocVehicle) {
    putTravelGroupInVehicle_v(groupNdx, newNdx);
    groupLocation_v.set(groupNdx, newLoc);

  } else if (newType == LocTransitStop) {
    groupsAtStop_v.ensureSize(newNdx+1);
    //waitingAtStop_v.ensureSize(newNdx+1);

    groupsAtStop_v.get(newNdx)->push_back(groupNdx);
    //waitingAtStop_v.set(newNdx, waitingAtStop_v[newNdx] +
        //groupSize_v[groupNdx]);
    groupLocation_v.set(groupNdx, newLoc);

  } else if (newLoc == 0) {
    groupLocation_v.set(groupNdx, 0);

  } else {
    SPDLOG_WARN("travelGroup({}) added to unknown loc: {}",
        groupNdx, format(newLoc));
    //handleError("bad move");
  }
}

void swapOneTravelGroup(item ndx, bool fromSave) {
  TravelGroup* group = getTravelGroup_g(ndx);
  groupSize_v.set(ndx, group->members.size());

  groupRoute_v.ensureSize(ndx+1);
  groupLocation_v.ensureSize(ndx+1);
  groupSize_v.ensureSize(ndx+1);
  item lastLoc = group->location;

  if (!(group->flags & _groupExists)) {
    moveGroupTo_v(ndx, 0);
    groupSize_v.set(ndx, 0);
    clearRoute(groupRoute_v.get(ndx));

  } else if (fromSave) {
    moveGroupTo_v(ndx, group->location);
    copyRoute(&group->route, groupRoute_v.get(ndx));
    //SPDLOG_INFO("swapOneTravelGroup {}:{}=>{} from save",
        //ndx, groupLocation_v[ndx], group->location);

  } else if (groupLocation_v[ndx] != 0) {
    group->location = groupLocation_v[ndx];
    copyRoute(groupRoute_v.get(ndx), &group->route);
    //SPDLOG_INFO("swapOneTravelGroup {}:{}=>{} from physics",
     //   ndx, group->location, groupLocation_v[ndx]);
  }

  if (group->location != lastLoc) {
    updatePedestrianRender_g(ndx);
  }
}

void swapOneTransitStop(item ndx, bool fromSave) {
  Stop* stop = getStop(ndx);

  groupsAtStop_v.ensureSize(ndx+1);
  //waitingAtStop_v.ensureSize(ndx+1);
  stopUpdateNdx_v.ensureSize(ndx+1);

  if (!(stop->flags & _stopExists)) {
    vector<item> groups = groupsAtStop_v[ndx];

    for (int j = 0; j < groups.size(); j++) {
      TravelGroup* group = getTravelGroup_g(groups[j]);
      if (group->location == transitStopLocation(ndx)) {
        removeTravelGroup_v(groups[j]);
      }
    }

    groupsAtStop_v.get(ndx)->clear();
    //waitingAtStop_v.set(ndx, 0);
    stopUpdateNdx_v.set(ndx, 0);

  } else {
    if (fromSave) {
      stopUpdateNdx_v.set(ndx, stop->updateNdx);

    } else {
      stop->updateNdx = stopUpdateNdx_v[ndx];
      stop->travelGroups = groupsAtStop_v[ndx];
      //stop->numWaiting = waitingAtStop_v[ndx];
    }
  }
}

void swapOneTransitLine(item ndx, bool fromSave) {
  TransitLine* line = getTransitLine(ndx);

  //transitLineStops_v.ensureSize(ndx+1);
  transitLinePassengersNow_v.ensureSize(ndx+1);
  transitLinePassengersEver_v.ensureSize(ndx+1);

  if (!(line->flags & _transitExists)) {
    //transitLineStops_v.get(ndx)->clear();
    transitLinePassengersNow_v.set(ndx, 0);
    transitLinePassengersEver_v.set(ndx, 0);

  } else {
    //line->stops.copy(transitLineStops_v.get(ndx));

    if (fromSave) {
      transitLinePassengersNow_v.set(ndx, line->passengersNow);
      transitLinePassengersEver_v.set(ndx, line->passengersEver);

    } else {
      line->passengersNow = transitLinePassengersNow_v[ndx];
      line->passengersEver = transitLinePassengersEver_v[ndx];
    }
  }
}

void swapTransitPhysics(bool fromSave) {
  //logStage("resize transit physics");
  int stopS = sizeStops();
  int groupS = sizeTravelGroups();
  int lineS = sizeTransitLines();

  groupsAtStop_v.ensureSize(stopS+1);
  //waitingAtStop_v.ensureSize(stopS+1);
  stopUpdateNdx_v.ensureSize(stopS+1);
  groupRoute_v.ensureSize(groupS+1);
  groupLocation_v.ensureSize(groupS+1);
  groupSize_v.ensureSize(groupS+1);
  //transitLineStops_v.ensureSize(lineS+1);
  transitLinePassengersNow_v.ensureSize(lineS+1);
  transitLinePassengersEver_v.ensureSize(lineS+1);

  //logStage("swapOneTransitStop");
  for (int i = 1; i < stopS; i++) {
    swapOneTransitStop(i, fromSave);
  }

  //logStage("swapOneTransitLine");
  for (int i = 1; i < lineS; i++) {
    swapOneTransitLine(i, fromSave);
  }

  //logStage("swapOneTravelGroup");
  for (int i = 1; i <= groupS; i++) {
    swapOneTravelGroup(i, fromSave);
  }
}

void removeTravelGroup_v(item ndx) {
  moveGroupTo_v(ndx, 0);
  clearRoute(groupRoute_v.get(ndx));
  removeTravelGroupCommand_v(ndx);
  //SPDLOG_WARN("removeTravelGroup_v {}", ndx);
}

item getTravelGroupSize_v(item groupNdx) {
  groupSize_v.ensureSize(groupNdx+1);
  return groupSize_v[groupNdx];
}

bool vehicleAtStop_v(item stopNdx, Location legNdx, item vNdx) {
  vector<item> groups = groupsAtStop_v[stopNdx];
  item updateNdx = stopUpdateNdx_v[stopNdx];
  item numToMove = c(CAlightingRate);

  while (updateNdx < groups.size()) {

    item groupNdx = groups[updateNdx];
    Route* route = groupRoute_v.get(groupNdx);
    item numSteps = route->steps.size();
    if (vNdx == getSelection()) {
      SPDLOG_INFO("vehicleAtStop_v grp{} {}/{}",
          groupNdx, updateNdx, groups.size());
      SPDLOG_INFO("{}", routeString(route));
    }

    while (numSteps > route->currentStep) {
      Location nextStep = route->steps[route->currentStep];
      item stepType = locationType(nextStep);
      item stepNdx = locationNdx(nextStep);

      if (vNdx == getSelection()) {
      // log info for selection
        SPDLOG_INFO("vehicleAtStop_v {}:{}=>{}/{}",
            groupNdx, format(groupLocation_v[groupNdx]),
            format(nextStep), format(legNdx));
      }

      if (stepType == LocTransitStop) {
        if (stepNdx == stopNdx) {
        // nextStep is this stop, advance route
          route->currentStep ++;

        } else {
        // Go to a different stop
          moveGroupTo_v(groupNdx, nextStep);
          route->currentStep ++;
          break;
        }

      } else if (stepType == LocTransitLeg) {
        if (locationLineNdx(nextStep) ==
            locationLineNdx(legNdx)) {
        // Get in the vehicle
          moveGroupTo_v(groupNdx, vehicleLocation(vNdx));
          route->currentStep ++;
          numToMove --;
          if (numToMove <= 0) {
            return false;
          }
          break;

        } else {
        // Waiting for different vehicle
          updateNdx ++;
          stopUpdateNdx_v.set(stopNdx, updateNdx);
          break;
        }

      } else {
      // WTF?
        SPDLOG_WARN("Unknown next step for travelGroup({}): {}",
            groupNdx, format(nextStep));
        updateNdx ++;
        stopUpdateNdx_v.set(stopNdx, updateNdx);
        break;
      }
    }

    if (route->currentStep >= numSteps) {
    // Group is complete
      removeTravelGroup_v(groupNdx);
    }

    groups = groupsAtStop_v[stopNdx];
  }

  updateNdx = 0;
  stopUpdateNdx_v.set(stopNdx, updateNdx);
  return true;
}

bool travelGroupInVehicle_v(item groupNdx, item vNdx, Location loc) {
  Route* route = groupRoute_v.get(groupNdx);
  item numSteps = route->steps.size();

  //SPDLOG_INFO("groupLocation_v {}:{}=>{} travelGroupInVehicle_v",
      //groupNdx, format(groupLocation_v[groupNdx]), format(loc));
  //SPDLOG_INFO("{}", routeString(*route));

  while (numSteps > route->currentStep) {
    item nextStep = route->steps[route->currentStep];
    item stepType = locationType(nextStep);
    if (stepType == LocTransitLeg) {
      if (nextStep == loc) {
        moveGroupTo_v(groupNdx, 0);
        route->currentStep ++;
      } else {
        // stay in vehicle
        return false;
      }

    } else if (stepType == LocTransitStop) {
      moveGroupTo_v(groupNdx, nextStep);
      return true;

    } else {
      SPDLOG_WARN("Unknown next step for travelGroup({}): {}",
          groupNdx, format(nextStep));
      SPDLOG_WARN("{}", routeString(route));
      //handleError("bad move");
      return false;
    }
  }

  removeTravelGroup_v(groupNdx);
  return true;
}

