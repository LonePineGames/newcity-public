#pragma once

#include "location.hpp"

#include "../cup.hpp"

struct Route {
  uint32_t flags = 0;
  uint32_t version = 0;
  Location source = 0;
  Location destination = 0;
  item currentStep = 0;
  Cup<Location> steps;
};

struct RouteInfo {
  uint32_t flags = 0;
  float time = 0;
  float walkingTime = 0;
  float waitTime = 0;
  float travelTime = 0;
  float ticketCost = 0;
  float fuelMaintCost = 0;
  float fuelTax = 0;
  float multiplier = 0;
  float costAdjustedTime = 0;
  float cost = 0;
};

GraphLocation getNextGraphLoc(Route* route);
char* routeString(Route* route);
char* routeString(vector<Location> route, item currentStep);
bool isRouteValid_g(vector<Location> route, Location start, Location end);
RouteInfo computeRouteInfo_g(Cup<Location>* route, bool walking, bool bus);
void clearRoute(Route* route);
void copyRoute(Route* source, Route* dest);
void fwrite_route (FileBuffer* out, Route* route);
void fread_route (FileBuffer* in, Route* route);

