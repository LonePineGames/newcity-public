#ifndef SELECTION_PANEL_H
#define SELECTION_PANEL_H

#include "../graph.hpp"

#include "part.hpp"

Part* selectionPanel();
Part* graphElementButton(vec2 loc, GraphLocation graphLoc);
bool loadInBuildingDesigner(Part* part, InputEvent event);

void clearSelectionPanel();
bool toggleEnableGovBuilding(Part* part, InputEvent event);
bool selectPerson(Part* part, InputEvent event);
bool selectGraphElement(Part* part, InputEvent event);
bool selectVehicle(Part* part, InputEvent event);
bool selectBuilding(Part* part, InputEvent event);
bool selectFamily(Part* part, InputEvent event);
bool selectBusiness(Part* part, InputEvent event);

#endif
