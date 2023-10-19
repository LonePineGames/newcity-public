#pragma once

#include "../route/location.hpp"

void resetTransitPhysics();
void swapOneTravelGroup(item ndx, bool fromSave);
void swapOneTransitStop(item ndx, bool fromSave);
void swapOneTransitLine(item ndx, bool fromSave);
void swapTransitPhysics(bool fromSave);
item getTravelGroupSize_v(item groupNdx);
void removeTravelGroup_v(item ndx);
bool vehicleAtStop_v(item stopNdx, Location legNdx, item vNdx);
bool travelGroupInVehicle_v(item groupNdx, item vNdx, Location loc);

