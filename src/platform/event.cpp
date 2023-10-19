#include "event.hpp"

#include "../item.hpp"

#include "lua.hpp"

#include "spdlog/spdlog.h"
#include <vector>
using namespace std;

vector<vector<int>> eventHandlers;

const char* eventNames[numEvents] = {
  #define EVENT(N) #N,
  #include "eventsEnum.hpp"
  #undef EVENT
};

void fireEvent(Event event) {
  for (int i = 0; i < eventHandlers[event].size(); i++) {
    callLuaPlainFunction(eventHandlers[event][i], "event handler");
  }
}

int addEventHandler(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 2) {
    handleError("call to on(): must provide two parameters");
    return 0;
  }

  if (!lua_isnumber(L, 1)) {
    handleError("call to on(): first parameter must be an event");
    return 0;
  }

  item event = lua_tonumber(L, 1);
  if (event < 0 || event >= numEvents) {
    handleError("call to on(): first parameter must be an event");
    return 0;
  }

  if (!lua_isfunction(L, 2)) {
    handleError("call to on(%s, ...): second parameter must be a function",
        eventNames[event]);
    return 0;
  }

  lua_pushvalue(L, 2);
  int func = luaL_ref(L, LUA_REGISTRYINDEX);
  eventHandlers[event].push_back(func);
  SPDLOG_INFO("Added event handler for {}", eventNames[event]);

  return 0;
}

void initEvents() {
  eventHandlers.clear();
  eventHandlers.resize(numEvents);

  addLuaCallback("on", addEventHandler);

  for (int i = 0; i < numEvents; i++) {
    setLuaGlobal(eventNames[i], i);
  }
}

