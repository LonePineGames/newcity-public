#pragma once

#include "../cup.hpp"
#include "../serialize.hpp"
#include "../route/route.hpp"

const uint32_t _groupExists = 1 << 0;
const uint32_t _groupHasRoute = 1 << 1;
const uint32_t _groupSettling = 1 << 2;

struct TravelGroup {
  uint32_t flags;
  float creationTime;
  Cup<item> members;
  Route route;
  Location location;
};

item addTravelGroup_g();
void removeTravelGroup_g(item ndx);
void removeAllTravelGroups_g();
TravelGroup* getTravelGroup_g(item ndx);
item startTrip_g(Route* route, item personNdx);
item startFreightTrip_g(Route* route, item personNdx);
void putTravelGroupInVehicle_g(item groupNdx, item vNdx);
void removePersonFromTravelGroup_g(item personNdx, item travelGroupNdx);
void freeTravelGroups();
item sizeTravelGroups();
void resetTravelGroups_g();
void setTravelGroupStats();
void readTravelGroups_g(FileBuffer* file);
void writeTravelGroups_g(FileBuffer* file);

