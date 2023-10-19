#pragma once

#include "../item.hpp"
#include "../main.hpp"

const int numKeyframes = 40;

void resetVehicleInterpolator();
item getPhysicalKeyframe();
void interpolateVehicles(double duration);
void swapInterpolator(double simulationTime);
void setKeyframe(item key, item ndx, GraphLocation val);
void setAllKeyframes(item ndx, GraphLocation val);
void updateVehicleHeatmapsAndWear(float duration);
void interpolateVehicles(double duration);

