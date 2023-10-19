#pragma once

#include "../item.hpp"

void setBuildingTextureLightLevel(float l);
float getBuildingTextureLightLevel();
void loadBuildingTextures();
item getBuildingTexture(item ndx);
item getDrawTextureForBuildingTexture(item ndx);
bool isBuildingTextureIllumination(item ndx);
item numMatchingTextures(item ndx);
void swapBuildingTextures();
void resetBuildingTextures();
void softResetBuildingTextures();
void deleteBuildingTexture(item buildingNdx, bool illum);
item numBuildingTextures();

