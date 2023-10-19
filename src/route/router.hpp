#pragma once

#include "../cup.hpp"
#include "../item.hpp"
#include "../serialize.hpp"

#include "location.hpp"
#include "route.hpp"

#include <vector>
using namespace std;

void enqueueRoute_g(item ndx);
void routeVehicle_g(item vehicleNdx, GraphLocation source, GraphLocation dest);
void routePerson_g(item personNdx, GraphLocation source, GraphLocation dest);
void routeTransitVehicle_g(item vehicleNdx, item lineNdx);
vector<Location> routeInstant(Location source, Location dest);
vector<Location> routeInstant(Location source, Location dest, bool transit);
void invalidateRouteCache(Location source, Location dest);
void invalidateRouteCache(Route* route);
int routeCacheSize();
void routerQueueLoop();
void sendRouteBatches(double duration);
void finishRouting(double duration);
void finish1000Routes(double duration);
void doSingleThreadedRouting();
void initRoutingThreads();
void resetRouting();
void killRouting();
bool isRoutingDone();
void readRouter(FileBuffer* file, int version);
void writeRouter(FileBuffer* file);

