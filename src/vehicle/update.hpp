#pragma once

#include "../item.hpp"
#include "../route/route.hpp"

#include <vector>
using namespace std;

double getVehicleTime();
int getVehicleUpdatesPending();
void updateVehicles(double duration);
void vehicleLoop();
void removeLaneVehicles(item ndx);
void removeVehicleCommand(item ndx);
void removeVehicleCommandBack(item ndx);
void finishVehicleRoute(item ndx, Route* route);
void invalidateRouteCommandBack(item ndx);
void rerouteCommandBack(item ndx);
void selectAndPauseBack(item ndx);
void addVehicleCommand(item ndx);
void addTravelGroupCommand(item ndx);
void removeTravelGroupCommand_v(item ndx);
void swapVehiclesCommand(bool doFull);
void validateSwapVehicles(char* msg);
void swapVehiclesBack();
void setPauseVehicleThread(bool val);
void resetVehiclesCommand();
void killVehicles();

