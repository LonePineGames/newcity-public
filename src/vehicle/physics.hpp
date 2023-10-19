#pragma once

#include "../item.hpp"
#include "../route/route.hpp"

void oneVehicleUpdate();
void addPhysicalVehicle(item ndx);
void removePhysicalVehicle(item ndx);
void removeVehicleBack(item ndx);
void swapVehiclePhysics();
void swapVehiclePhysicsBack();
void swapVehicleKeyframe();
void resetVehiclePhysics();
float vehicleLength(item ndx);
void validatePhysicalVehicles(char* msg);
void validateSwapPhysicalVehicles(char* msg);
void recieveVehicleRoute(item ndx); //, Route* route);
void putTravelGroupInVehicle_v(item groupNdx, item vNdx);
void removeTravelGroupFromVehicle_v(item groupNdx, item vNdx);
bool canEnterLane_g(item lane);

