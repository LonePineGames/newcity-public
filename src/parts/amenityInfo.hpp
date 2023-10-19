#pragma once

#include "part.hpp"

Part* amenityInfoPart(vec2 loc, vec2 size,
    item designNdx, item buildingNdx, bool unaffordableRed);
void amenityStats(Part* pnl, vec2 loc, vec2 size, item designNdx);
bool openCitipediaEffectPage(Part* part, InputEvent event);
