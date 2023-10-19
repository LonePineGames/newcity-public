#pragma once

#include "part.hpp"

bool isFixBuildingMode();
void setFixBuildingMode(bool val);
bool setFixBuildingMode(Part* part, InputEvent event);
bool setOrganizerZone(Part* part, InputEvent event);

