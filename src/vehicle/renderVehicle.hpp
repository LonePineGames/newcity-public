#ifndef RENDER_VEHICLE_H
#define RENDER_VEHICLE_H

#include "../item.hpp"

void initVehicleMeshes();
void renderVehicle(item ndx);
void renderVehicleNumber(item ndx);
void renderVehicleStyles();
void setVehicleHighlight(item ndx, bool highlight);
void resetVehicleRender();

#endif
