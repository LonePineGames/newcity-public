#pragma once

#include "item.hpp"
#include "serialize.hpp"

enum HeatMapIndex {
  Pollution, Value, Density, Crime, Education, Prosperity,
  CommunityHM, HealthHM,
  numHeatMaps
};

const HeatMapIndex TrafficHeatMap = (HeatMapIndex)-1;
const HeatMapIndex TransitHeatMap = (HeatMapIndex)-2;
const HeatMapIndex ZoneHeatMap = (HeatMapIndex)-3;
const HeatMapIndex RoadHeatMap = (HeatMapIndex)-4;

typedef float HeatMapTile;

void heatMapAdd(HeatMapIndex ndx, vec3 loc, float amount);
float heatMapGet(HeatMapIndex ndx, vec3 loc);
float heatMapTotal(HeatMapIndex ndx);
void setHeatMap(HeatMapIndex ndx, bool intense);
HeatMapIndex getHeatMap();
HeatMapIndex getHeatMap_d();
bool isHeatMapIntense();
void resetHeatMaps();
void initHeatMaps();
void initLandValue();
void swapHeatMaps();
void drawHeatMaps();
void updateHeatMaps(double duration);
void heatMapLoop();
void killHeatMaps();
void readHeatMaps(FileBuffer* file, int version);
void writeHeatMaps(FileBuffer* file);
void setupHeatmap(GLuint programID, bool showPollution);
void setupNoHeatmap(GLuint programID);

