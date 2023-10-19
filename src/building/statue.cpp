#include "statue.hpp"

#include "../import/mesh-import.hpp"
#include "../platform/lua.hpp"
#include "../pool.hpp"
#include "../renum.hpp"

#include "spdlog/spdlog.h"

Pool<Statue> statues;

item sizeStatues() {
  return statues.size();
}

Statue* getStatue(item ndx) {
  return statues.get(ndx);
}

item getStatueForBuilding(item buildingNdx, item decoNdx) {
  return (buildingNdx + decoNdx - 1) % sizeStatues() + 1;
}

const char* getStatueName(item statue) {
  return getStatue(statue)->name;
}

int addStatue(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  item ndx = statues.create();
  Statue* statue = statues.get(ndx);
  //statue->flags = _decoExists;
  statue->name = luaFieldString(L, "name");
  statue->code = luaFieldString(L, "code");

  statue->meshImport = readMeshImportData(L, false);
  MeshImport* import = getMeshImport(statue->meshImport);
  if (import->objFile == 0) {
    handleError("Must provide mesh obj file for statue %s", statue->code);
  }

  return 0;
}

void initStatueCallbacks() {
  statues.clear();
  addLuaCallback("addStatue", addStatue);
}

