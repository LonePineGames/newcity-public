#include "travelGroup.hpp"

#include "../economy.hpp"
#include "../game/game.hpp"
#include "../person.hpp"
#include "../pool.hpp"
#include "../selection.hpp"
#include "../time.hpp"
#include "../util.hpp"

#include "model.hpp"
#include "renderPedestrian.hpp"
#include "update.hpp"

#include "spdlog/spdlog.h"

#include <set>

Pool<TravelGroup> travelGroups;

void resetTravelGroups_g() {
  for (int i = 1; i <= travelGroups.size(); i++) {
    TravelGroup* group = getTravelGroup_g(i);
    group->members.clear();
    group->route.steps.clear();
  }

  travelGroups.clear();
}

void setTravelGroupStats() {
  setStat(ourCityEconNdx(), NumTravelGroups, travelGroups.count());
  setStat(ourCityEconNdx(), TravelGroupsSettling, travelGroups.settling.size());
  setStat(ourCityEconNdx(), TravelGroupsFree, travelGroups.gaps.size());
}

item addTravelGroup_g() {
  item ndx = travelGroups.create();
  TravelGroup* group = getTravelGroup_g(ndx);
  group->flags = _groupExists;
  group->creationTime = getCurrentDateTime();
  group->members.clear();
  group->location = 0;
  Route nullRoute;
  group->route = nullRoute;
  setTravelGroupStats();
  return ndx;
}

TravelGroup* getTravelGroup_g(item ndx) {
  return travelGroups.get(ndx);
}

void removeAllTravelGroups_g() {
  for (int i = 1; i <= travelGroups.size(); i++) {
    removeTravelGroup_g(i);
  }
  travelGroups.defragment("travelGroups");
}

void removeTravelGroup_g(item ndx) {
  if (ndx <= 0 || ndx > travelGroups.size()) return;
  TravelGroup* group = getTravelGroup_g(ndx);
  if (!(group->flags & _groupExists)) return;
  group->flags = _groupSettling;

  for (int i = group->members.size()-1; i >= 0; i--) {
    item personNdx = group->members[i];
    putPersonInBuilding(personNdx, getPerson(personNdx)->activityBuilding);
  }

  group->members.clear();
  group->location = 0;
  group->creationTime = getCurrentDateTime();
  Route nullRoute;
  group->route = nullRoute;
  travelGroups.settle(ndx);
  updatePedestrianRender_g(ndx);
  setTravelGroupStats();
}

void freeTravelGroups() {
  float time = getCurrentDateTime();
  item largestNdx = 0;
  for (int i = travelGroups.settling.size()-1; i >= 0; i--) {
    item ndx = travelGroups.settling[i];
    TravelGroup* tg = getTravelGroup_g(ndx);

    if (!(tg->flags & _groupSettling)) {
      SPDLOG_WARN("Settling group which isn't marked as settling.");
    }

    if (time - tg->creationTime > oneHour) {
      tg->flags = 0;
      travelGroups.free(ndx);
      if (ndx > largestNdx) largestNdx = ndx;
    }
  }

  //if (largestNdx >= travelGroups.size()*.01f) {// || randFloat() < 0.001) {
    //travelGroups.defragment("travelGroups");
  //}
}

void removePersonFromTravelGroup_g(item personNdx, item travelGroupNdx) {
  if (travelGroupNdx > travelGroups.size()) return;
  TravelGroup* group = getTravelGroup_g(travelGroupNdx);
  if (!(group->flags & _groupExists)) return;
  group->members.removeByValue(personNdx);

  /*
  for (int i = group->members.size()-1; i >= 0; i--) {
    if (group->members[i] == personNdx) {
      group->members.erase(group->members.begin()+i);
    }
  }
  */

  if (group->members.size() == 0) {
    removeTravelGroup_g(travelGroupNdx);
  }
}

void putTravelGroupInVehicle_g(item groupNdx, item vNdx) {
  TravelGroup* group = getTravelGroup_g(groupNdx);
  Vehicle* v = getVehicle(vNdx);
  group->location = vehicleLocation(vNdx);
  v->travelGroups.push_back(groupNdx);
}

item startTripInner_g(Route* route, item personNdx, bool freight) {
  item groupNdx = addTravelGroup_g();
  TravelGroup* group = getTravelGroup_g(groupNdx);
  putPersonInTravelGroup_g(personNdx, groupNdx);
  group->members.push_back(personNdx);

  bool isTransit = false;
  for (int i = 0; i < route->steps.size(); i++) {
    if (locationType(route->steps[i]) == LocTransitStop) {
      isTransit = true;
      break;
    }
  }

  RouteInfo info = computeRouteInfo_g(&route->steps, false, false);
  if (isTransit) {
    for (int i = route->steps.size()-1; i >= 0; i--) {
      item type = locationType(route->steps[i]);
      if (type != LocTransitStop && type != LocTransitLeg) {
        route->steps.remove(i);
      }
    }

    transaction(TransitIncome, info.ticketCost * getTransitMoneyMultiplier());

  } else {
    //SPDLOG_INFO("making fuel tax transaction {}", info.fuelTax);
    //transaction(FuelTaxIncome, info.fuelTax * getTransitMoneyMultiplier());
  }

  copyRoute(route, &group->route);
  if (route->steps.size() > 0) {
    group->flags |= _groupHasRoute;
    Location start = route->steps[0];
    item type = locationType(start);

    if (isTransit) {
      adjustStat(ourCityEconNdx(), TransitTrips, 1);
      group->location = start;
    } else if (type == LocLaneBlock) {
      adjustStat(ourCityEconNdx(), VehicleTrips, 1);
      item vNdx = 0;
      if (freight) {
        vNdx = addFreightVehicle_g(groupNdx, route);
      } else {
        Person* p = getPerson(personNdx);
        VehicleDescription desc = createVehicleDescription();
        vNdx = addVehicle_g(desc.style, groupNdx, route);
        Vehicle* v = getVehicle(vNdx);
        v->model = randomVehicleModel(VhTypePrivate);
      }
      group->location = vehicleLocation(vNdx);

    } else {
      SPDLOG_WARN("don't know where to send group {} {}",
          groupNdx, routeString(route));
    }
  }

  //SPDLOG_WARN("startTripInner_g {}@{} {}", groupNdx, format(group->location),
      //routeString(route));
  addTravelGroupCommand(groupNdx);
  return groupNdx;
}

item startTrip_g(Route* route, item personNdx) {
  return startTripInner_g(route, personNdx, false);
}

item startFreightTrip_g(Route* route, item personNdx) {
  return startTripInner_g(route, personNdx, true);
}

item sizeTravelGroups() {
  return travelGroups.size();
}

void writeTravelGroup_g(FileBuffer* file, item ndx) {
  TravelGroup* group = getTravelGroup_g(ndx);
  fwrite_uint32_t(file, group->flags);
  group->members.write(file);
  //fwrite_item_vector(file, &group->members);
  fwrite_route(file, &group->route);
  fwrite_location(file, group->location);
  fwrite_float(file, group->creationTime);
}

void writeTravelGroups_g(FileBuffer* file) {
  travelGroups.defragment("travelGroups");
  travelGroups.write(file);
  for (int i = 1; i <= travelGroups.size(); i++) {
    writeTravelGroup_g(file, i);
  }
}

void readTravelGroup_g(FileBuffer* file, item ndx) {
  TravelGroup* group = getTravelGroup_g(ndx);
  group->flags = fread_uint32_t(file);
  group->members.read(file, file->version);
  //fread_item_vector(file, &group->members, file->version);
  fread_route(file, &group->route);
  group->location = fread_location(file);
  group->creationTime = fread_float(file);
}

void readTravelGroups_g(FileBuffer* file) {
  if (file->version >= 51) {
    travelGroups.read(file, file->version);
    for (int i = 1; i <= travelGroups.size(); i++) {
      readTravelGroup_g(file, i);
    }
    SPDLOG_INFO("travelGroups size {}", travelGroups.size());

    travelGroups.gaps.clear();
    for (int i = 1; i <= travelGroups.size(); i++) {
      TravelGroup* tg = getTravelGroup_g(i);
      if ((tg->flags & _groupSettling) || !(tg->flags & _groupExists)) {
        tg->flags = 0;
        travelGroups.free(i);
      }
    }

    travelGroups.defragment("travelGroups");

    SPDLOG_INFO("travelGroups count {}", travelGroups.count());
  }

}

