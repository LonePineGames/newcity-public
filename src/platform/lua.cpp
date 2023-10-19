#include "lua.hpp"

#include "event.hpp"

#include "../building/buildingCallbacks.hpp"
#include "../building/deco.hpp"
#include "../draw/drawCallbacks.hpp"
#include "../economy.hpp"
#include "../error.hpp"
#include "../game/achievement.hpp"
#include "../graph/config.hpp"
#include "../input.hpp"
#include "../newspaper/newspaperCallbacks.hpp"
#include "../newspaper/article.hpp"
#include "../string_proxy.hpp"
#include "../platform/file.hpp"
#include "../platform/lookup.hpp"
#include "../terrain/tree.hpp"
#include "../util.hpp"
#include "../vehicle/model.hpp"
#include "../vehicle/vehicleCallbacks.hpp"

#include "spdlog/spdlog.h"
#include <stdio.h>
#include <string.h>
#include <vector>

static lua_State* luaState;

const char* const constantsFile = "data/constants.lua";
const char* const difficultyEasyFile = "data/easy.lua";
const char* const difficultyMediumFile = "data/medium.lua";
const char* const difficultyHardFile = "data/hard.lua";
const char* const difficultyVeryHardFile = "data/veryhard.lua";
const char* const achievementsFile = "data/achievements.lua";
const char* const graphConfigFile = "data/graph.lua";
const char* const vehicleModelFile = "data/vehicles.lua";
const char* const decoFile = "data/decos.lua";
const char* const newspaperFile = "newspaper/articles.lua";
const char* const terrainFile = "data/terrain.lua";
const char* const scriptsFile = "data/scripts.lua";
const char* const statuesFile = "data/statues.lua";

char* getDifficultyInMod(std::string &srcFile);

#include "luaCallbacks.cpp"

lua_State* getLuaState() {
  return luaState;
}

void handleLuaError(lua_State* L, const char* msg, const char* filename) {
  handleError("%s %s: %s", msg, filename, lua_tostring(luaState, -1));
  lua_pop(L, 1);
  luaL_traceback (L, L, msg, 2);
  char* result = 0;
  if (lua_isstring(L, -1)) result = strdup_s(lua_tostring(L, -1));
  lua_pop(L, 1);
  SPDLOG_WARN("traceback: {}", result);
}

void readLua(const char *filename) {
  SPDLOG_INFO("readLua {}", filename);
  if (luaL_loadfile(luaState, filename) || lua_pcall(luaState, 0, 0, 0)) {
    handleLuaError(luaState, "Reading file", filename);
  }
}

void readLuaInMod(const char *filename, bool disableDefault) {
  vector<string> versions = lookupFileVersions(filename,
      disableDefault ? _lookupExcludeBase : 0);

  for (int i = versions.size()-1; i >= 0; i--) {
    readLua(versions[i].c_str());
  }

  /*
  if (!disableDefault) readLua(filename);
  char* modFilename = getFilenameInMod(filename);
  if (!streql(modFilename, filename)) {
    readLua(modFilename);
  }
  free(modFilename);
  */
}

void readLuaInMod(const char *filename) {
  readLuaInMod(filename, false);
}

void initLua() {
  luaState = luaL_newstate();
  luaL_openlibs(luaState);
  initLuaGeneralCallbacks();
  initLuaDrawCallbacks();
  initLuaInputCallbacks();
  initGraphConfigCallbacks();
  initVehicleModelCallbacks();
  initLuaVehicleCallbacks();
  initDecoCallbacks();
  initNewspaperLua();
  initAchievementsLua();
  initTreeCallbacks();
  initLuaBuildingCallbacks();
  initLuaNewspaperCallbacks();
  initEvents();
  readLuaInMod(constantsFile);

  GameDifficulty difficulty = getGameDifficulty();
  if (difficulty == DifficultyHard) {
    readLuaInMod(difficultyHardFile);
  } else if (difficulty == DifficultyVeryHard) {
    readLuaInMod(difficultyVeryHardFile);
  } else if (difficulty == DifficultyEasy) {
    readLuaInMod(difficultyEasyFile);
  } else {
    readLuaInMod(difficultyMediumFile);
  }

  readLuaInMod(graphConfigFile);
  readLuaInMod(achievementsFile, c(CDisableDefaultAchievements));
  readLuaInMod(vehicleModelFile, c(CDisableDefaultVehicleModels));
  readLuaInMod(decoFile);
  readLuaInMod(newspaperFile, c(CDisableDefaultNewspaperArticles));
  readLuaInMod(terrainFile, c(CDisableDefaultTerrainConfig));
  readLuaInMod(scriptsFile, c(CDisableDefaultEvents));
  readLuaInMod(statuesFile);
}

void setLuaGlobal(const char* name, float val) {
  lua_pushnumber(luaState, val);
  lua_setglobal(luaState, name);
}

void postInitLua() {
  setLuaGlobal("CityEcon", ourCityEconNdx());
  setLuaGlobal("NationalEcon", nationalEconNdx());
}

void resetLua() {
  if(luaState != 0) {
    lua_close(luaState);
    luaState = 0;
  }

  resetGraphConfig();
  resetVehicleModels();
}

void refreshLua() {
  // Uncomment to notify when entering refreshLua function
  // consolePrintDebug("refreshLua");
  resetLua();
  initLua();
  resetConstants();
  loadConstantsFromLua();
}

static void stackDump (lua_State *L) {
  int i;
  int top = lua_gettop(L);
  for (i = 1; i <= top; i++) {  /* repeat for each level */
    int t = lua_type(L, i);
    switch (t) {

      case LUA_TSTRING:  /* strings */
        printf("'%s'", lua_tostring(L, i));
        break;

      case LUA_TBOOLEAN:  /* booleans */
        printf(lua_toboolean(L, i) ? "true" : "false");
        break;

      case LUA_TNUMBER:  /* numbers */
        printf("%g", lua_tonumber(L, i));
        break;

      default:  /* other values */
        printf("%s", lua_typename(L, t));
        break;

    }
    printf("  ");  /* put a separator */
  }
  printf("\n");  /* end the listing */
}

int luaInterpreter () {
  char buff[256];
  int error;
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  while (fgets(buff, sizeof(buff), stdin) != NULL) {
    error = luaL_loadbuffer(L, buff, strlen(buff), "line") ||
            lua_pcall(L, 0, 0, 0);
    if (error) {
      fprintf(stderr, "%s", lua_tostring(L, -1));
      lua_pop(L, 1);  /* pop error message from the stack */
    }
  }

  lua_close(L);
  return 0;
}

char* interpretLua(lua_State* L, const char* in, const char* filename) {
  char* code = sprintf_o("return %s", in);
  int error = luaL_loadstring(L, code) || lua_pcall(L, 0, 1, 0);
  free(code);

  if (error) {
    const char* errorMsg = lua_tostring(L, -1);
    SPDLOG_WARN("Error in {}: {}", filename, errorMsg);
    char* result = sprintf_o("<ERROR: %s in file %s>", errorMsg, filename);
    lua_pop(L, 1);  /* pop error message from the stack */
    return result;

  } else {
    char* result = 0;
    int i = -1;
    int t = lua_type(L, i);

    switch (t) {
      case LUA_TSTRING:  /* strings */
        result = strdup_s(lua_tostring(L, i));
        break;

      case LUA_TBOOLEAN:  /* booleans */
        result = strdup_s(lua_toboolean(L, i) ? "true" : "false");
        break;

      case LUA_TNUMBER:  /* numbers */
        result = formatFloat(lua_tonumber(L, i));
        break;

      default:  /* other values */
        result = strdup_s(lua_typename(L, t));
        break;
    }

    lua_pop(L, 1);
    return result;
  }
}

char* interpretLua(const char* in, const char* filename) {
  return interpretLua(luaState, in, filename);
}

bool luaBool(lua_State* L, const char* name, bool &succeeded) {
  if(L == 0) {
    succeeded = false;
    return false;
  }

  lua_getglobal(L, name);
  succeeded = lua_isboolean(L, -1);
  bool result = lua_toboolean(L, -1);
  lua_pop(L, 1);
  return result;
}

float luaFloat(lua_State* L, const char* name, bool &succeeded) {
  if(L == 0) {
    succeeded = false;
    return false;
  }

  lua_getglobal(L, name);
  succeeded = lua_isnumber(L, -1);
  float result = lua_tonumber(L, -1);
  lua_pop(L, 1);
  return result;
}

int luaFieldType(lua_State* L, const char* key) {
  lua_getfield(L, -1, key);
  int result = lua_type(L, -1);
  lua_pop(L, 1);
  return result;
}

char* luaFieldString(lua_State* L, const char* key) {
  //SPDLOG_INFO("luaFieldString {}", key);
  //stackDump(L);
  lua_getfield(L, -1, key);
  //stackDump(L);
  char* result = 0;
  if (lua_isstring(L, -1)) result = strdup_s(lua_tostring(L, -1));
  lua_pop(L, 1);
  return result;
}

float luaFieldNumber(lua_State* L, const char* key) {
  //SPDLOG_INFO("luaFieldNumber {}", key);
  //stackDump(L);
  lua_getfield(L, -1, key);
  //stackDump(L);
  float result = 0;
  if (lua_isnumber(L, -1)) result = lua_tonumber(L, -1);
  lua_pop(L, 1);
  return result;
}

float luaFieldNumber(lua_State* L, int key) {
  //SPDLOG_INFO("luaFieldNumber {}", key);
  //stackDump(L);
  lua_pushnumber(L, key);
  //stackDump(L);
  lua_gettable(L, -2);
  //stackDump(L);
  float result = 0;
  if (lua_isnumber(L, -1)) result = lua_tonumber(L, -1);
  lua_pop(L, 1);
  return result;
}

bool luaFieldBool(lua_State* L, const char* key) {
  lua_getfield(L, -1, key);
  bool result = false;
  if (lua_isboolean(L, -1)) result = lua_toboolean(L, -1);
  lua_pop(L, 1);
  return result;
}

vec3 luaFieldVec3(lua_State* L, const char* key) {
  vec3 result = vec3(0,0,0);
  //SPDLOG_INFO("luaFieldVec3 {}", key);
  //stackDump(L);
  lua_getfield(L, -1, key);
  //stackDump(L);

  if (lua_istable(L, -1)) {
    result.x = luaFieldNumber(L, 1);
    result.y = luaFieldNumber(L, 2);
    result.z = luaFieldNumber(L, 3);
  }

  lua_pop(L, 1);
  return result;
}

int luaFieldFunction(lua_State* L, const char* key) {
  //stackDump(L);
  lua_getfield(L, -1, key);
  //stackDump(L);
  if (lua_isfunction(L, -1)) return luaL_ref(L, LUA_REGISTRYINDEX);

  lua_pop(L, 1);
  return -1;
}

bool luaSetFromField(lua_State* L, const char* key, float* set) {
  bool done = false;
  //stackDump(L);
  lua_getfield(L, -1, key);
  //stackDump(L);

  if (lua_isnumber(L, -1)) {
    float result = lua_tonumber(L, -1);
    *set = result;
    //SPDLOG_INFO("luaSetFromField {} set to {}", key, result);
    done = true;
  } else {
    //SPDLOG_INFO("luaSetFromField not found {}", key);
    done = false;
  }

  lua_pop(L, 1);
  return done;
}

bool luaSetFromField(lua_State* L, const char* key, item* set) {
  float hold = 0;
  bool done = luaSetFromField(L, key, &hold);
  if (done) *set = hold;
  return done;
}

bool luaSetFromField(lua_State* L, const char* key, money* set) {
  float hold = 0;
  bool done = luaSetFromField(L, key, &hold);
  if (done) *set = hold;
  return done;
}

bool luaSetCharNumFromField(lua_State* L, const char* key, char* set) {
  float hold = 0;
  bool done = luaSetFromField(L, key, &hold);
  if (done) *set = hold;
  return done;
}

bool luaSetFromField(lua_State* L, const char* key, char** set) {
  bool done = false;
  //stackDump(L);
  lua_getfield(L, -1, key);
  //stackDump(L);

  if (lua_isstring(L, -1)) {
    char* result = strdup_s(lua_tostring(L, -1));
    if (*set != 0) free(*set);
    *set = result;
    //SPDLOG_INFO("luaSetFromField {} set to {}", key, result);
    done = true;
  } else {
    //SPDLOG_INFO("luaSetFromField not found {}", key);
    done = false;
  }

  lua_pop(L, 1);
  return done;
}

bool luaWriteBool(lua_State* L, const char* name, bool value) {
  if(L == 0) {
    return false;
  }

  lua_getglobal(L, name);
  bool success = lua_isboolean(L, -1);
  if(success) {
    lua_pushboolean(L, value);
    lua_setglobal(L, name);
  }
  lua_pop(L, 1);
  return success;
}

bool luaWriteValue(lua_State* L, const char* name, float value) {
  if(L == 0) {
    return false;
  }

  lua_getglobal(L, name);
  bool success = lua_isnumber(L, -1);
  if(success) {
    lua_pushnumber(L, value);
    lua_setglobal(L, name);
  }
  lua_pop(L, 1);
  return success;
}

bool getBoolFromLua(const char* name) {
  bool succeeded = false;
  bool result = false;
  result = luaBool(luaState, name, succeeded);
  return result;
}

float getValueFromLua(const char* name) {
  bool succeeded = false;
  float result = 0;
  result = luaFloat(luaState, name, succeeded);
  return result;
}

bool luaDoesBoolExist(lua_State* state, const char* stateName, const char* name) {
  bool succeeded = false;
  bool value = luaBool(state, name, succeeded);
  if(succeeded) {
    consolePrintLine("Value " + std::string(name) + " exists in state (pos from chain end) " + std::string(stateName) + ": " + std::to_string(value));
  }
  return succeeded;
}

bool luaDoesValueExist(lua_State* state, const char* stateName, const char* name) {
  bool succeeded = false;
  float value = luaFloat(state, name, succeeded);
  if(succeeded) {
    consolePrintLine("Value " + std::string(name) + " exists in state (pos from chain end) " + std::string(stateName) + ": " + std::to_string(value));
  }
  return succeeded;
}

bool luaDoesBoolExistAll(const char* name) {
  return luaDoesBoolExist(luaState, "main", name);
}

bool luaDoesValueExistAll(const char* name) {
  return luaDoesValueExist(luaState, "main", name);
}

bool luaSetBool(lua_State* state, const char* stateName, const char* name, bool value) {
  if(!luaDoesBoolExist(state, stateName, name)) {
    return false;
  }

  consolePrintDebug(sprintf_o("Writing bool %s to Lua state %s", value ? "true" : "false", stateName));
  luaWriteBool(state, name, value);
  return true;
}

bool luaSetBoolAll(const char* name, bool value) {
  return luaSetBool(luaState, "main", name, value);
}

bool luaSetValue(lua_State* state, const char* stateName, const char* name, float value) {
  if(!luaDoesValueExist(state, stateName, name)) {
    return false;
  }

  consolePrintDebug(sprintf_o("Writing value %f to Lua state %s", value, stateName));
  luaWriteValue(state, name, value);
  return true;
}

// Tries to set value in all states, returns false
// if one or more state could not be set
bool luaSetValueAll(const char* name, float value) {
  return luaSetValue(luaState, "main", name, value);
}

void luaSetTableNumber(lua_State* L, const char* key, float value) {
  lua_pushstring(L, key);
  lua_pushnumber(L, value);
  lua_settable(L, -3);
}

void luaSetTableString(lua_State* L, const char* key, const char* value) {
  lua_pushstring(L, key);
  lua_pushstring(L, value);
  lua_settable(L, -3);
}

void luaSetTableBool(lua_State* L, const char* key, bool value) {
  lua_pushstring(L, key);
  lua_pushboolean(L, value);
  lua_settable(L, -3);
}

void addLuaCallback(const char* name, lua_CFunction func) {
  lua_pushcfunction(luaState, func);
  lua_setglobal(luaState, name);
}

void callLuaUpdateFunction(int ref, const char* filename,
    item toUpdate, float duration) {
  lua_rawgeti(luaState, LUA_REGISTRYINDEX, ref);
  lua_pushnumber(luaState, toUpdate);
  lua_pushnumber(luaState, duration);
  int error = lua_pcall(luaState, 2, 0, 0);

  if (error) {
    char* errorMsg = strdup_s(lua_tostring(luaState, -1));
    lua_pop(luaState, 1);  /* pop error message from the stack */
    SPDLOG_WARN("Error in {} update function: {}", filename, errorMsg);
    free(errorMsg);
  }
}

void lua_pushstring_free(lua_State* L, char* str) {
  lua_pushstring(L, str);
  free(str);
}

float callLuaScoringFunction(int ref, const char* filename) {
  lua_rawgeti(luaState, LUA_REGISTRYINDEX, ref);
  int error = lua_pcall(luaState, 0, 1, 0);

  if (error) {
    char* errorMsg = strdup_s(lua_tostring(luaState, -1));
    lua_pop(luaState, 1);  /* pop error message from the stack */
    SPDLOG_WARN("Error in {} score function: {}", filename, errorMsg);
    free(errorMsg);
    return -1;

  } else if (!lua_isnumber(luaState, -1)) {
    SPDLOG_WARN("Error in {} score function:"
        " Function returned non-number.", filename);
    return -1;

  } else {
    float result = lua_tonumber(luaState, -1);
    lua_pop(luaState, 1);
    return result;
  }
}

bool callLuaBooleanFunction(int ref, const char* filename) {
  lua_rawgeti(luaState, LUA_REGISTRYINDEX, ref);
  int error = lua_pcall(luaState, 0, 1, 0);

  if (error) {
    char* errorMsg = strdup_s(lua_tostring(luaState, -1));
    lua_pop(luaState, 1);  /* pop error message from the stack */
    SPDLOG_WARN("Error in {} condition function: {}", filename, errorMsg);
    free(errorMsg);
    return false;

  } else if (!lua_isboolean(luaState, -1)) {
    SPDLOG_WARN("Error in {} condition function:"
        " Function returned non-boolean.", filename);
    return false;

  } else {
    bool result = lua_toboolean(luaState, -1);
    lua_pop(luaState, 1);
    return result;
  }
}

void callLuaPlainFunction(int ref, const char* filename) {
  lua_rawgeti(luaState, LUA_REGISTRYINDEX, ref);
  int error = lua_pcall(luaState, 0, 0, 0);

  if (error) {
    char* errorMsg = strdup_s(lua_tostring(luaState, -1));
    lua_pop(luaState, 1);  /* pop error message from the stack */
    SPDLOG_WARN("Error in {} function: {}", filename, errorMsg);
    free(errorMsg);
  }
}

/*
lua_State* read(const char *filename) {
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);

  if (luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, 0)) {
    handleError("LUA: %s", lua_tostring(L, -1));
    return 0;
  }

  return L;
}

static int foo (lua_State *L) {
 int n = lua_gettop(L);    // number of arguments
 lua_Number sum = 0.0;
 int i;
 for (i = 1; i <= n; i++) {
   if (!lua_isnumber(L, i)) {
     lua_pushliteral(L, "incorrect argument");
     lua_error(L);
   }
   sum += lua_tonumber(L, i);
 }
 return 0;                   // number of results
}

// assume that table is on the stack top
int getfield (const char *key) {
  int result;
  lua_pushstring(L, key);
  lua_gettable(L, -2);  // get background[key]
  if (!lua_isnumber(L, -1))
    error(L, "invalid component in background color");
  result = (int)lua_tonumber(L, -1) * MAX_COLOR;
  lua_pop(L, 1);  // remove number
  return result;
}
*/

