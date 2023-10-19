#pragma once

#include "../route/location.hpp"

enum FeatureType {
  UntypedFeature, TractionFeature, PowerFeature, AutomationFeature,
  numFeatureTypes
};

struct GraphFeature {
  uint8_t type;
  char* code;
  char* name;
  char* text;
  vec3 icon;
  uint8_t maxCars;
};

item getGraphFeature(const char* code);
GraphFeature* getGraphFeature(item ndx);
void initGraphConfigCallbacks();
void resetGraphConfig();
vector<item> getGraphFeatures(item type);

