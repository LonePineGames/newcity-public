#include "platform/lua.hpp"

vector<item> inputHandlers;

int isKeyDown(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;
  int key = luaL_checknumber(L, 1);

  if (key < 0 || key > GLFW_KEY_LAST+1) {
    lua_pushboolean(L, false);
  } else {
    lua_pushboolean(L, lastEvent.isKeyDown[key]);
  }

  return 1;
}

int getMouseLocation_screenspace(lua_State* L) {
  vec2 mouseLoc = lastEvent.cameraSpaceMouseLoc;

  lua_newtable(L);
  luaSetTableNumber(L, "x", mouseLoc.x);
  luaSetTableNumber(L, "y", mouseLoc.y);

  return 1;
}

int getMouseLocation_worldspace(lua_State* L) {
  vec3 mouseLoc = landIntersect(lastEvent.mouseLine);

  lua_newtable(L);
  luaSetTableNumber(L, "x", mouseLoc.x);
  luaSetTableNumber(L, "y", mouseLoc.y);
  luaSetTableNumber(L, "z", mouseLoc.z);

  return 1;
}

int addInputHandler(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;
  if (!lua_isfunction(L, 1)) handleError("addInputHandler takes a function");

  lua_pushvalue(L, 1);
  int func = luaL_ref(L, LUA_REGISTRYINDEX);
  inputHandlers.push_back(func);
  return 0;
}

void initLuaInputCallbacks() {
  inputHandlers.clear();
  addLuaCallback("isKeyDown", isKeyDown);
  addLuaCallback("getMouseLocation_screenspace", getMouseLocation_screenspace);
  addLuaCallback("getMouseLocation_worldspace", getMouseLocation_worldspace);
  addLuaCallback("addInputHandler", addInputHandler);
}

bool callLuaEventFunction(int ref, InputEvent event) {
  lua_State* L = getLuaState();

  lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

  lua_newtable(L);
  luaSetTableNumber(L, "button", event.button);
  luaSetTableNumber(L, "key", event.key);
  luaSetTableNumber(L, "scancode", event.scancode);
  luaSetTableNumber(L, "action", event.action);
  luaSetTableNumber(L, "mods", event.mods);
  luaSetTableNumber(L, "unicode", event.unicode);

  int error = lua_pcall(L, 1, 1, 0);

  if (error) {
    char* errorMsg = strdup_s(lua_tostring(L, -1));
    lua_pop(L, 1);  /* pop error message from the stack */
    SPDLOG_WARN("Error in event handler: {}", errorMsg);
    free(errorMsg);
    return false;

  } else if (!lua_isboolean(L, -1)) {
    SPDLOG_WARN("Error in event handler:"
        " Function returned non-boolean.");
    return false;

  } else {
    bool result = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return result;
  }
}

bool runLuaInputHandlers(InputEvent event) {
  for (int i = 0; i < inputHandlers.size(); i++) {
    bool result = callLuaEventFunction(inputHandlers[i], event);
    if (result) return true;
  }

  return false;
}

