#pragma once

#include "design.hpp"

void resetBuildingRender();
void renderDesign(item designNdx);
void renderBuilding(item ndx);
void paintBuilding(item ndx);
void renderHandles(item entityNdx, item buildingNdx);
void renderDecoHandles(item entityNdx, item buildingNdx);
void unrenderHandles(item entityNdx, item buildingNdx);
void setDesignRender(bool val);
void designRender();
void resetDesignRender();
void setBuildingHighlight(item ndx, bool highlight);
void setBuildingRedHighlight(item ndx, bool highlight);
vec3 maxPoint(Design* d);

