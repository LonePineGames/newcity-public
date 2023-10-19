#pragma once

#include <lua.hpp>

#include "../console/conDisplay.hpp"
#include "../game/game.hpp"
#include "../money.hpp"

int luaInterpreter();
void initLua();
void resetLua();
void refreshLua();
bool getBoolFromLua(const char* name);
float getValueFromLua(const char* name);
bool luaDoesBoolExist(lua_State* state, const char* stateName, const char* name);
bool luaDoesValueExist(lua_State* state, const char* stateName, const char* name);
bool luaDoesBoolExistAll(const char* name);
bool luaDoesValueExistAll(const char* name);
bool luaSetBoolAll(const char* name, bool value);
bool luaSetValueAll(const char* name, float value);

void luaSetTableNumber(lua_State* L, const char* key, float value);
void luaSetTableString(lua_State* L, const char* key, const char* value);
void luaSetTableBool(lua_State* L, const char* key, bool value);
void lua_pushstring_free(lua_State* L, char* str);

void addLuaCallback(const char* name, lua_CFunction func);
int luaFieldType(lua_State* L, const char* key);
char* luaFieldString(lua_State* L, const char* key);
float luaFieldNumber(lua_State* L, const char* key);
bool luaFieldBool(lua_State* L, const char* key);
vec3 luaFieldVec3(lua_State* L, const char* key);
int luaFieldFunction(lua_State* L, const char* key);
bool luaSetCharNumFromField(lua_State* L, const char* key, char* set);
bool luaSetFromField(lua_State* L, const char* key, char** set);
bool luaSetFromField(lua_State* L, const char* key, float* set);
bool luaSetFromField(lua_State* L, const char* key, item* set);
bool luaSetFromField(lua_State* L, const char* key, money* set);
void callLuaUpdateFunction(int ref, const char* filename,
    item toUpdate, float duration);
float callLuaScoringFunction(int ref, const char* filename);
bool callLuaBooleanFunction(int ref, const char* filename);
void callLuaPlainFunction(int ref, const char* filename);
void setLuaGlobal(const char* name, float val);

char* interpretLua(const char* in, const char* filename);
void postInitLua();
lua_State* getLuaState();

