#include "deco.hpp"

#include "statue.hpp"

#include "../import/mesh-import.hpp"
#include "../game/game.hpp"
#include "../platform/lua.hpp"
#include "../pool.hpp"
#include "../renum.hpp"
#include "../vehicle/model.hpp"

#include "spdlog/spdlog.h"
#include <unordered_map>

RenumTable decosRenum;
Pool<DecoType> decoTypes;
Pool<DecoGroup> decoGroups;
unordered_map<string, item> decoGroupsByCode;
bool legacyGroupVisible = true;
item statueDecoTypeNdx = 0;
item specialGroupNdx = 0;

const char* legacyDecoNames[] = {
  "Tree", "Fence V", "Fence H", "Shrubs V", "Shrubs H",
  "Path V", "Path H", "Road V", "Road H",
  "Swing Set", "Small Pool", "Big Pool",
  "Short Sign", "Column Sign", "Tall Sign", "Hanging Sign", "Awning",
  "Smokestack", "Containers", "Small Tanks", "Big Tank",
  "Green Crops V", "Green Crops H", "Yellow Crops V", "Yellow Crops H",
  "Bleachers N", "Bleachers E", "Bleachers S", "Bleachers W",
  "Baseball Diamond", "Facade", "Pavilion", "Parking", "Parking Super",
  "Fench V Long", "Fence H Long"
};

int addBuildingDecorationGroup(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  item ndx = decoGroups.create();
  DecoGroup* group = decoGroups.get(ndx);
  group->name = luaFieldString(L, "name");
  group->code = luaFieldString(L, "code");
  group->flags = _decoGroupExists | _decoGroupVisible;

  decoGroupsByCode[group->code] = ndx;
  return 0;
}

void toggleDecoGroupVisible(item groupNdx) {
  if (groupNdx == 0) {
    legacyGroupVisible = !legacyGroupVisible;
    return;
  }
  DecoGroup* group = getDecoGroup(groupNdx);
  if (group->flags & _decoGroupVisible) {
    group->flags &= ~_decoGroupVisible;
  } else {
    group->flags |= _decoGroupVisible;
  }
}

bool isDecoGroupVisible(item groupNdx) {
  if (groupNdx <= 0) return legacyGroupVisible;
  DecoGroup* group = getDecoGroup(groupNdx);
  return group->flags & _decoGroupVisible;
}

bool isDecoVisible(item decoType) {
  if (getGameMode() != ModeBuildingDesigner) return true;
  if (decoType < numLegacyDecoTypes) return legacyGroupVisible;
  DecoType* deco = getDecoType(decoType);
  if (deco->group <= 0 || deco->group > decoGroups.size()) return true;
  DecoGroup* group = getDecoGroup(deco->group);
  /*
  const char* code = group->code;
  if (code == 0 || deco->group <= 0 || deco->group > decoGroups.size()) {
    if (code == 0) {
      code = "[]";
    }
    SPDLOG_INFO("isDecoVisible {} of group {}",
        decoType, deco->group);
    SPDLOG_INFO("isDecoVisible {}:{} of group {}:{}",
        decoType, deco->code, deco->group, group->code);
  }
  */
  return group->flags & _decoGroupVisible;
}

int addBuildingDecoration(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  item ndx = decoTypes.create();
  DecoType* deco = decoTypes.get(ndx);
  deco->flags = _decoExists;
  deco->name = luaFieldString(L, "name");
  deco->code = luaFieldString(L, "code");
  deco->wind = luaFieldNumber(L, "wind");

  char* textureCode = luaFieldString(L, "texture");
  if (textureCode != 0 && streql(textureCode, "building")) {
    deco->flags |= _decoUseBuildingTexture;
  }

  char* spawnsCode = luaFieldString(L, "spawns");
  if (spawnsCode != 0) {
    deco->spawns = vehicleModelByCode(spawnsCode);
  } else {
    deco->spawns = 0;
  }

  char* groupCode = luaFieldString(L, "group");
  if (groupCode == 0) handleError("Must provide group for deco %s", deco->code);
  deco->group = decoGroupsByCode[groupCode];
  free(groupCode);

  char* scaleCode = luaFieldString(L, "scale");
  if (scaleCode != 0) {
    for (int i = 0; scaleCode[i] != '\0'; i ++) {
      char c = scaleCode[i];
      if (c == 'x') deco->flags |= _decoScaleX;
      if (c == 'y') deco->flags |= _decoScaleY;
      if (c == 'z') deco->flags |= _decoScaleZ;
    }
    free(scaleCode);
  } else {
    deco->flags |= _decoScaleX | _decoScaleY | _decoScaleZ;
  }

  deco->meshImport = readMeshImportData(L, false);
  MeshImport* import = getMeshImport(deco->meshImport);
  if (import->objFile == 0) {
    handleError("Must provide mesh obj file for deco %s", deco->code);
  }

  item adjNdx = ndx + numLegacyDecoTypes - 1;
  pushRenum(&decosRenum, adjNdx, deco->code);
  return 0;
}

const char* getDecoTypeName(item type) {
  if (type < numLegacyDecoTypes) {
    return legacyDecoNames[type];
  } else {
    return getDecoType(type)->name;
  }
}

DecoGroup* getDecoGroup(item ndx) {
  return decoGroups.get(ndx);
}

DecoType* getDecoType(item ndx) {
  return decoTypes.get(ndx - numLegacyDecoTypes + 1);
}

void initDecoCallbacks() {

  for (int i = 1; i <= decoTypes.size(); i++) {
    DecoType* type = decoTypes.get(i);
    if (type->code != 0) free(type->code);
    if (type->name != 0) free(type->name);
  }
  decoTypes.clear();

  for (int i = 1; i <= decoTypes.size(); i++) {
    DecoGroup* group = decoGroups.get(i);
    if (group->code != 0) free(group->code);
    if (group->name != 0) free(group->name);
  }
  decoGroups.clear();

  decoGroupsByCode.clear();
  renumClear(&decosRenum);
  legacyGroupVisible = true;

  #define RENUM(N) pushRenum(&decosRenum, #N);
  #include "decoData.v54.renum"
  #undef RENUM

  specialGroupNdx = decoGroups.create();
  DecoGroup* specialGroup = decoGroups.get(specialGroupNdx);
  specialGroup->code = strdup_s("Special::DecoGrpSpecial");
  specialGroup->name = strdup_s("Special");
  specialGroup->flags = _decoGroupExists | _decoGroupVisible;
  decoGroupsByCode[specialGroup->code] = specialGroupNdx;

  statueDecoTypeNdx = decoTypes.create() + numLegacyDecoTypes - 1;
  DecoType* statueDecoType = getDecoType(statueDecoTypeNdx);
  statueDecoType->flags = _decoExists | _decoScaleX | _decoScaleY | _decoScaleZ;
  statueDecoType->group = specialGroupNdx;
  statueDecoType->code = strdup_s("Special::DecoStatue");
  statueDecoType->name = strdup_s("Statue");
  statueDecoType->wind = 0;
  pushRenum(&decosRenum, statueDecoTypeNdx, statueDecoType->code);

  addLuaCallback("addBuildingDecoration", addBuildingDecoration);
  addLuaCallback("addBuildingDecorationGroup", addBuildingDecorationGroup);
  initStatueCallbacks();
}

void readDecosRenum(FileBuffer* file) {
  if (file->version >= 55) {
    renumRead(&decosRenum, file);
  } else {
    renumLegacyStart(&decosRenum);
    #define RENUM(N) renumLegacy(&decosRenum, #N);
    #include "decoData.v54.renum"
    #undef RENUM
  }
}

void writeDecosRenum(FileBuffer* file) {
  renumWrite(&decosRenum, file);
}

item renumDeco(item in) {
  return renum(&decosRenum, in);
}

item sizeDecoTypes() {
  return decoTypes.size() + numLegacyDecoTypes;
}

item sizeDecoGroups() {
  return decoGroups.size();
}

item statueDecoType() {
  return statueDecoTypeNdx;
}

item getDecoTypeByName(std::string name) {
  return decosRenum.byCode[name];
}

