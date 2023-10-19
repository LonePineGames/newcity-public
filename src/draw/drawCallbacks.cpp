#include "drawCallbacks.hpp"

#include "camera.hpp"

#include "../platform/lua.hpp"

int setCameraTarget(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  vec3 target;
  target.x = luaFieldNumber(L, "x");
  target.y = luaFieldNumber(L, "y");
  target.z = luaFieldNumber(L, "z");
  setCameraTarget(target);

  return 0;
}

int getCameraTarget(lua_State* L) {
  vec3 target = getCameraTarget();

  lua_newtable(L);
  luaSetTableNumber(L, "x", target.x);
  luaSetTableNumber(L, "y", target.x);
  luaSetTableNumber(L, "z", target.x);

  return 1;
}

int setCameraYaw(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  float yaw = luaL_checknumber(L, 1);
  setCameraYaw(yaw);

  return 0;
}

int setCameraPitch(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  float pitch = luaL_checknumber(L, 1);
  setCameraPitch(pitch);

  return 0;
}

int setCameraRoll(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  float roll = luaL_checknumber(L, 1);
  setCameraRoll(roll);

  return 0;
}

int setCameraZoom(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  float zoom = luaL_checknumber(L, 1);
  setCameraDistance(zoom);

  return 0;
}

int getFrameDuration(lua_State* L) {
  lua_pushnumber(L, getFrameDuration());
  return 1;
}

void initLuaDrawCallbacks() {
  addLuaCallback("setCameraTarget", setCameraTarget);
  addLuaCallback("getCameraTarget", getCameraTarget);
  addLuaCallback("setCameraYaw", setCameraYaw);
  addLuaCallback("setCameraPitch", setCameraPitch);
  addLuaCallback("setCameraRoll", setCameraRoll);
  addLuaCallback("setCameraZoom", setCameraZoom);
  addLuaCallback("getFrameDuration", getFrameDuration);
}

