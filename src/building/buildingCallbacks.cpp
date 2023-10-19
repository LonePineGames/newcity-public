#include "buildingCallbacks.hpp"

#include "building.hpp"
#include "design.hpp"
#include "renderBuilding.hpp"
#include "deco.hpp"

#include "../business.hpp"
#include "../platform/lua.hpp"
#include "../zone.hpp"

int setDesign(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  item dNdx = luaFieldNumber(L, "ndx");
  if (dNdx <= 0 || dNdx > sizeDesigns()) return 0;

  Design* d = getDesign(dNdx);

  luaSetFromField(L, "sx", &(d->size.x));
  luaSetFromField(L, "sy", &(d->size.y));
  luaSetFromField(L, "sz", &(d->size.z));
  luaSetFromField(L, "minYear", &(d->minYear));
  luaSetFromField(L, "maxYear", &(d->maxYear));
  luaSetFromField(L, "minLandValue", &(d->minLandValue));
  luaSetFromField(L, "minDensity", &(d->minDensity));
  luaSetFromField(L, "lowTide", &(d->lowTide));
  luaSetFromField(L, "highTide", &(d->highTide));
  luaSetCharNumFromField(L, "zone", &(d->zone));
  luaSetFromField(L, "numFamilies", &(d->numFamilies));
  luaSetFromField(L, "numRetail", &(d->numBusinesses[Retail]));
  luaSetFromField(L, "numOffices", &(d->numBusinesses[Office]));
  luaSetFromField(L, "numFarms", &(d->numBusinesses[Farm]));
  luaSetFromField(L, "numFactories", &(d->numBusinesses[Factory]));
  luaSetFromField(L, "numInstitutions", &(d->numBusinesses[Institution]));
  luaSetFromField(L, "name", &(d->name));
  luaSetFromField(L, "displayName", &(d->displayName));

  return 0;
}

int getDesign(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  item dNdx = luaL_checknumber(L, 1);
  if (dNdx <= 0 || dNdx > sizeDesigns()) return 0;

  Design* d = getDesign(dNdx);

  lua_newtable(L);
  luaSetTableNumber(L, "ndx", dNdx);
  luaSetTableNumber(L, "sx", d->size.x);
  luaSetTableNumber(L, "sy", d->size.y);
  luaSetTableNumber(L, "sz", d->size.z);
  luaSetTableNumber(L, "numBuildings", d->numBuildings);
  luaSetTableNumber(L, "minYear", d->minYear);
  luaSetTableNumber(L, "maxYear", d->maxYear);
  luaSetTableNumber(L, "minLandValue", d->minLandValue);
  luaSetTableNumber(L, "minDensity", d->minDensity);
  luaSetTableNumber(L, "lowTide", d->lowTide);
  luaSetTableNumber(L, "highTide", d->highTide);
  luaSetTableNumber(L, "zone", d->zone);
  luaSetTableNumber(L, "numFamilies", d->numFamilies);
  luaSetTableNumber(L, "numRetail", d->numBusinesses[Retail]);
  luaSetTableNumber(L, "numOffices", d->numBusinesses[Office]);
  luaSetTableNumber(L, "numFarms", d->numBusinesses[Farm]);
  luaSetTableNumber(L, "numFactories", d->numBusinesses[Factory]);
  luaSetTableNumber(L, "numInstitutions", d->numBusinesses[Institution]);
  luaSetTableString(L, "name", d->name);
  luaSetTableString(L, "displayName", d->displayName);

  return 1;
}

int renderDesign(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  item dNdx = luaL_checknumber(L, 1);
  if (dNdx <= 0 || dNdx > sizeDesigns()) return 0;

  renderDesign(dNdx);

  return 0;
}

int sizeDesigns(lua_State* L) {
  lua_pushnumber(L, sizeDesigns());
  return 1;
}

int setBuilding(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  item bNdx = luaFieldNumber(L, "ndx");
  if (bNdx <= 0 || bNdx > sizeBuildings()) return 0;

  Building* b = getBuilding(bNdx);
  if (!(b->flags & _buildingExists)) return 0;

  luaSetFromField(L, "x", &(b->location.x));
  luaSetFromField(L, "y", &(b->location.y));
  luaSetFromField(L, "z", &(b->location.z));
  luaSetFromField(L, "nx", &(b->normal.x));
  luaSetFromField(L, "ny", &(b->normal.y));
  luaSetFromField(L, "nz", &(b->normal.z));
  luaSetFromField(L, "design", &(b->design));
  luaSetFromField(L, "color", &(b->color));
  luaSetFromField(L, "econ", &(b->econ));
  luaSetFromField(L, "value", &(b->value));
  luaSetFromField(L, "lastUpdatedAt", &(b->lastUpdateTime));
  luaSetFromField(L, "builtAt", &(b->builtTime));
  luaSetFromField(L, "zone", &(b->zone));
  luaSetFromField(L, "plan", &(b->plan));
  luaSetFromField(L, "name", &(b->name));

  return 0;
}

int getBuilding(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  item bNdx = luaL_checknumber(L, 1);
  if (bNdx <= 0 || bNdx > sizeBuildings()) return 0;

  Building* b = getBuilding(bNdx);
  if (!(b->flags & _buildingExists)) return 0;

  lua_newtable(L);
  luaSetTableNumber(L, "ndx", bNdx);
  luaSetTableNumber(L, "x", b->location.x);
  luaSetTableNumber(L, "y", b->location.y);
  luaSetTableNumber(L, "z", b->location.z);
  luaSetTableNumber(L, "nx", b->normal.x);
  luaSetTableNumber(L, "ny", b->normal.y);
  luaSetTableNumber(L, "nz", b->normal.z);
  luaSetTableNumber(L, "design", b->design);
  luaSetTableNumber(L, "color", b->color);
  luaSetTableNumber(L, "econ", b->econ);
  luaSetTableNumber(L, "value", b->value);
  luaSetTableNumber(L, "lastUpdatedAt", b->lastUpdateTime);
  luaSetTableNumber(L, "builtAt", b->builtTime);
  luaSetTableNumber(L, "zone", b->zone);
  luaSetTableNumber(L, "plan", b->plan);
  luaSetTableString(L, "name", b->name);

  return 1;
}

int removeBuilding(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  item bNdx = luaL_checknumber(L, 1);
  if (bNdx <= 0 || bNdx > sizeBuildings()) return 0;

  removeBuilding(bNdx);

  return 0;
}

int renderBuilding(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  item bNdx = luaL_checknumber(L, 1);
  if (bNdx <= 0 || bNdx > sizeBuildings()) return 0;

  renderBuilding(bNdx);

  return 0;
}

int sizeBuildings(lua_State* L) {
  lua_pushnumber(L, sizeBuildings());
  return 1;
}

int zoneCode_cb(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) {
    lua_pushstring(L, "InvalidZone");
    return 1;
  }

  item zNdx = luaL_checknumber(L, 1);
  if (zNdx < 0 || zNdx >= numZoneTypes) {
    lua_pushstring(L, "InvalidZone");
  } else {
    lua_pushstring(L, zoneCode[zNdx]);
  }

  return 1;
}

int zoneName_cb(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) {
    lua_pushstring(L, "Invalid Zone");
    return 1;
  }

  item zNdx = luaL_checknumber(L, 1);
  if (zNdx < 0 || zNdx >= numZoneTypes) {
    lua_pushstring(L, "Invalid Zone");
  } else {
    lua_pushstring(L, zoneName[zNdx]);
  }

  return 1;
}

int numZones(lua_State* L) {
  lua_pushnumber(L, numZoneTypes);
  return 1;
}

int numBuildingsForDesignKeyword(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) {
    SPDLOG_WARN("numBuildingsForDesignKeyword: first argument must be string");
    lua_pushnumber(L, 0);
    return 1;
  }

  std::string keyword = luaL_checkstring(L, 1);
  lua_pushnumber(L, numBuildingsForDesignKeyword(keyword));

  return 1;
}

void initLuaBuildingCallbacks() {
  addLuaCallback("getDesign", getDesign);
  addLuaCallback("setDesign", setDesign);
  addLuaCallback("renderDesign", renderDesign);
  addLuaCallback("maxDesignNdx", sizeDesigns);
  addLuaCallback("getBuilding", getBuilding);
  addLuaCallback("setBuilding", setBuilding);
  addLuaCallback("removeBuilding", removeBuilding);
  addLuaCallback("renderBuilding", renderBuilding);
  addLuaCallback("maxBuildingNdx", sizeBuildings);
  addLuaCallback("numBuildings", numBuildingsForDesignKeyword);

  for (int i = 0; i < numZoneTypes; i ++) {
    setLuaGlobal(zoneCode[i], i);
  }
  addLuaCallback("numZones", numZones);
  addLuaCallback("zoneName", zoneName_cb);
  addLuaCallback("zoneCode", zoneCode_cb);
}

