#pragma once

#include "tool.hpp"

void stopVehicleFollowMode();
bool issueLegendMode();
void toggleQueryInfoPanel();
bool isShowHeatmapInfoPanel();
bool isQueryHeatmapSet();
bool setHeatMap(Part* part, InputEvent event);
void setQuerySubTool(item tool);
Tool* getToolQuery();

