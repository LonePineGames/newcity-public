#include "vehicleCallbacks.hpp"

#include "vehicle.hpp"

#include "../platform/lua.hpp"

int setVehicle(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  item vNdx = luaFieldNumber(L, "ndx");
  if (vNdx <= 0 || vNdx > sizeVehicles()) return 0;

  Vehicle* v = getVehicle(vNdx);
  if (!(v->flags & _vehicleExists)) return 0;

  luaSetFromField(L, "x", &(v->location.x));
  luaSetFromField(L, "y", &(v->location.y));
  luaSetFromField(L, "z", &(v->location.z));
  luaSetFromField(L, "yaw", &(v->yaw));
  luaSetFromField(L, "pitch", &(v->pitch));
  luaSetFromField(L, "vx", &(v->velocity.x));
  luaSetFromField(L, "vy", &(v->velocity.y));
  luaSetFromField(L, "vz", &(v->velocity.z));
  luaSetFromField(L, "ax", &(v->acceleration.x));
  luaSetFromField(L, "ay", &(v->acceleration.y));
  luaSetFromField(L, "createdAt", &(v->creationTime));
  luaSetFromField(L, "model", &(v->model));

  return 0;
}

int getVehicle(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  item vNdx = luaL_checknumber(L, 1);
  if (vNdx <= 0 || vNdx > sizeVehicles()) return 0;

  Vehicle* v = getVehicle(vNdx);
  if (!(v->flags & _vehicleExists)) return 0;

  lua_newtable(L);
  luaSetTableNumber(L, "ndx", vNdx);
  luaSetTableNumber(L, "x", v->location.x);
  luaSetTableNumber(L, "y", v->location.y);
  luaSetTableNumber(L, "z", v->location.z);
  luaSetTableNumber(L, "yaw", v->yaw);
  luaSetTableNumber(L, "pitch", v->pitch);
  luaSetTableNumber(L, "vx", v->velocity.x);
  luaSetTableNumber(L, "vy", v->velocity.y);
  luaSetTableNumber(L, "vz", v->velocity.z);
  luaSetTableNumber(L, "ax", v->acceleration.x);
  luaSetTableNumber(L, "ay", v->acceleration.y);
  luaSetTableNumber(L, "trailer", v->trailer);
  luaSetTableNumber(L, "trailing", v->trailing);
  luaSetTableNumber(L, "yieldTo", v->yieldTo);
  luaSetTableNumber(L, "yieldFrom", v->yieldFrom);
  luaSetTableNumber(L, "createdAt", v->creationTime);
  luaSetTableNumber(L, "numPassengers", v->numPassengers);
  luaSetTableNumber(L, "transitLine", v->transitLine);
  luaSetTableNumber(L, "model", v->model);

  return 1;
}

int removeVehicle(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  item vNdx = luaL_checknumber(L, 1);
  if (vNdx <= 0 || vNdx > sizeVehicles()) return 0;

  removeVehicle(vNdx);

  return 0;
}

int sizeVehicles(lua_State* L) {
  lua_pushnumber(L, sizeVehicles());
  return 1;
}

void initLuaVehicleCallbacks() {
  addLuaCallback("getVehicle", getVehicle);
  addLuaCallback("setVehicle", setVehicle);
  addLuaCallback("removeVehicle", removeVehicle);
  addLuaCallback("maxVehicleNdx", sizeVehicles);
}

