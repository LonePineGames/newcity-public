#include "config.hpp"

#include "../pool.hpp"
#include "../platform/lua.hpp"

Pool<GraphFeature> features;
vector<vector<item>> featuresByType;
unordered_map<string, item> featuresByCode;

void resetGraphConfig() {
  features.clear();
  featuresByType.clear();
  featuresByCode.clear();
}

GraphFeature* getGraphFeature(item ndx) {
  return features.get(ndx);
}

int addGraphFeature(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;
  item ndx = features.create();
  GraphFeature* feature = features.get(ndx);
  feature->code = luaFieldString(L, "code");
  feature->name = luaFieldString(L, "name");
  feature->text = luaFieldString(L, "text");
  feature->icon = luaFieldVec3(L, "icon");
  feature->maxCars = luaFieldNumber(L, "maxCars");

  char* type = luaFieldString(L, "type");
  if (type == 0) {
    feature->type = 0;
  } else if (streql(type, "traction")) {
    feature->type = TractionFeature;
  } else if (streql(type, "power")) {
    feature->type = PowerFeature;
  } else if (streql(type, "automation")) {
    feature->type = AutomationFeature;
  } else {
    feature->type = 0;
  }

  featuresByType[feature->type].push_back(ndx);
  featuresByCode[feature->code] = ndx;

  return ndx;
}

int addGraphClass(lua_State* L) {
  return 0;
}

int addGraphConfiguration(lua_State* L) {
  return 0;
}

int addGraphVehicle(lua_State* L) {
  return 0;
}

vector<item> getGraphFeatures(item type) {
  return featuresByType[type];
}

item getGraphFeature(const char* code) {
  return featuresByCode[code];
}

void initGraphConfigCallbacks() {
  featuresByType.resize(numFeatureTypes);
  addLuaCallback("addGraphFeature", addGraphFeature);
  addLuaCallback("addGraphClass", addGraphClass);
  addLuaCallback("addGraphConfiguration", addGraphConfiguration);
}

