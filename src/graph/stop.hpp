#pragma once

#include "../money.hpp"

#include <set>

const int _stopExists = 1 << 0;
const int _stopComplete = 1 << 1;

struct Stop {
  uint32_t flags;
  uint8_t type;
  char* name;
  item plan;
  item currentVehicle;
  vector<item> travelGroups;
  item numWaiting;
  item updateNdx;
  GraphLocation graphLoc;
  vec3 location;
  vector<item> lines;
};

item addStop(GraphLocation loc);
Stop* getStop(item ndx);
void removeStop(item ndx);
money stopCost(item ndx);
const char* stopLegalMessage(item ndx);
void completeStop(item ndx);
void updateStop(item ndx);
void repositionTransitStops();
item nearestStop(Line ml);
item sizeStops();
void resetStops();
void renderStops();
void initStopEntities();
void setStopHighlight(item ndx, bool highlight);
void setStopRedHighlight(item ndx, bool highlight);
void readStops_g(FileBuffer* file);
void writeStops_g(FileBuffer* file);

