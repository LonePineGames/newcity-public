#pragma once

#include "part.hpp"

Part* transitPanel();
bool setCurrentTransitSystem(Part* part, InputEvent event);
bool toggleEditingTransitBrand(Part* part, InputEvent event);

