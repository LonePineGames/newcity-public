#include "tree.hpp"

#include "../import/mesh-import.hpp"
#include "../land.hpp"
#include "../platform/lua.hpp"
#include "../pool.hpp"
#include "../util.hpp"

#include "spdlog/spdlog.h"
#include <unordered_map>
#include <string>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

Pool<TreeType> treeTypes;
unordered_map<string, item> treeTypesByCode;
float totalTreeWeight = 0;

int addTreeType(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  char* code = luaFieldString(L, "code");
  if (code == 0) {
    handleError("Must provide code for treeType");
  }

  item ndx = treeTypesByCode[code];
  if (ndx == 0) {
    ndx = treeTypes.create();
    treeTypesByCode[code] = ndx;
  }

  TreeType* type = treeTypes.get(ndx);
  type->code = code;
  type->weight = luaFieldNumber(L, "weight");
  type->flags = _treeExists;
  if(luaFieldBool(L, "wind")) type->flags |= _treeWind;

  type->meshImport = readMeshImportData(L, false);
  MeshImport* import = getMeshImport(type->meshImport);
  if (import->objFile == 0) {
    handleError("Must provide mesh obj file for treeType %s", type->code);
  }

  char* biomes = luaFieldString(L, "biomes");
  if (biomes == 0) {
    type->flags |= _treeForAlpine | _treeForDesert;
  } else {
    boost::char_separator<char> sep(",", "");
    string str(biomes);
    tokenizer tokens(str, sep);
    BOOST_FOREACH(std::string const& token, tokens) {
      if (token == "alpine") type->flags |= _treeForAlpine;
      if (token == "desert") type->flags |= _treeForDesert;
    }
  }

  return 0;
}

void initTreeCallbacks() {
  treeTypes.clear();
  treeTypesByCode.clear();
  totalTreeWeight = 0;
  addLuaCallback("addTreeType", addTreeType);
}

item sizeTreeTypes() {
  return treeTypes.size();
}

item randomTreeType(item ndx) {
  item treeTypeS = treeTypes.size();
  if (treeTypeS == 0) return 0;

  LandConfiguration config = getLandConfig();
  bool isDesert = config.flags & _landDesert;
  uint32_t _biomeFlag = isDesert ? _treeForDesert : _treeForAlpine;

  if (totalTreeWeight == 0) {
    for (int i = 1; i <= treeTypeS; i++) {
      TreeType* type = treeTypes.get(i);
      if (!(type->flags & _biomeFlag)) continue;
      totalTreeWeight += treeTypes.get(i)->weight;
    }
  }

  float rand = randFloat(&ndx) * totalTreeWeight;
  for (int i = 1; i <= treeTypeS; i++) {
    TreeType* type = treeTypes.get(i);
    if (!(type->flags & _biomeFlag)) continue;
    rand -= treeTypes.get(i)->weight;
    if (rand < 0) return i;
  }

  return 1;
}

TreeType* getTreeType(item ndx) {
  return treeTypes.get(ndx);
}

