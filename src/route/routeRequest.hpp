#pragma once

#include "../main.hpp"

#include "location.hpp"

const uint32_t _routeCompleted = 1 << 0;
const uint32_t _routeMeta = 1 << 1;
const uint32_t _routeChoice = 1 << 2;
const uint32_t _routeTransit = 1 << 3;

enum RouteRequestType {
  RouteForNull, RouteInstant, RouteForMeta, RouteForPerson, RouteForVehicle,
  RouteForTransitLine,
  numRouteRequestTypes
};

struct RouteRequest {
  uint32_t flags;
  uint32_t ndx;
  item type;
  item element;
  item source;
  item dest;
  vec3 sourceLoc;
  vec3 destLoc;
  vector<item> subRequests;
  vector<Location> steps;
};

void clearRequest(RouteRequest* req);
RouteRequest getRouteRequest_g(item ndx);
void finishOneRoute_g(RouteRequest result);
void routeVehicle_g(item vehicleNdx, GraphLocation source, GraphLocation dest);
void routePerson_g(item personNdx, GraphLocation source, GraphLocation dest);
void routeTransitVehicle_g(item vehicleNdx, item lineNdx);
void routeTransitLineInstant_g(item lineNdx);
void resetRouteRequests_g();
void defragmentRouteRequests();

