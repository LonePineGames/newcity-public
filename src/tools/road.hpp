#pragma once

#include "tool.hpp"

bool isTransitTool();
bool isCutTool();
void resetRoadTool();
Tool* getToolRoad();
void setRoadInfo();
bool togglePlanPanel(Part* part, InputEvent event);
item getLastLegAdded();
void removeLastAddedStop(item stopNdx);

