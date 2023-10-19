#include "newspaperCallbacks.hpp"

#include "newspaper.hpp"

#include "../platform/lua.hpp"
#include "../time.hpp"

int newspaperNameShort(lua_State* L) {
  lua_pushstring(L, getNewspaperNameShort().c_str());
  return 1;
}

int newspaperNameLong(lua_State* L) {
  lua_pushstring(L, getNewspaperName().c_str());
  return 1;
}

int maxNewspaperNdx(lua_State* L) {
  lua_pushnumber(L, getNumNewspaperIssues());
  return 1;
}


float timeArticleAppeared(const char* code) {
  for(int i = getNumNewspaperIssues()-1; i >= 0; i--) {
    NewspaperIssue* issue = getNewspaperIssue(i);
    for (int articleNum=0; articleNum < issue->articles.size(); articleNum++) {
      if (startsWith(issue->articles[articleNum].code, code)) {
        return issue->date;
      }
    }
  }

  return -oneYear*4950;
}

int timeSinceArticleAppeared(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) {
    SPDLOG_INFO("timeSinceArticleAppeared: missing argument");
    return 0;
  }

  const char* code = luaL_checkstring(L, 1);
  lua_pushnumber(L, getCurrentDateTime() - timeArticleAppeared(code));
  return 1;
}

int hasArticleAppeared(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) {
    SPDLOG_INFO("hasArticleAppeared: missing argument");
    return 0;
  }

  const char* code = luaL_checkstring(L, 1);
  lua_pushboolean(L, timeArticleAppeared(code) > -1000*oneYear);
  return 1;
}

int newspaperTime(lua_State* L) {
  lua_pushnumber(L, getNewspaperTime());
  return 1;
}

void initLuaNewspaperCallbacks() {
  addLuaCallback("newspaperNameShort", newspaperNameShort);
  addLuaCallback("newspaperNameLong", newspaperNameLong);
  addLuaCallback("maxNewspaperNdx", maxNewspaperNdx);
  addLuaCallback("hasArticleAppeared", hasArticleAppeared);
  addLuaCallback("timeSinceArticleAppeared", timeSinceArticleAppeared);
  addLuaCallback("newspaperTime", newspaperTime);
}

