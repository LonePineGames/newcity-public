#include "model.hpp"

#include "../cup.hpp"
#include "../import/mesh-import.hpp"
#include "../platform/lua.hpp"
#include "../pool.hpp"
#include "../renum.hpp"
#include "../util.hpp"

RenumTable modelTypes;
RenumTable modelsRenum;
Pool<VehicleModel> models;
vector<vector<item>> modelsByType;
Cup<float> modelLength;
Cup<item> modelPassengers;
bool vehicleModelsDirty = true;

void resetVehicleModels() {
  renumClear(&modelTypes);
  renumClear(&modelsRenum);
  modelsByType.clear();

  for (int i = 1; i <= models.size(); i++) {
    free(models.get(i)->code);
  }
  models.clear();
}

int addVehicleModel(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  char* code = luaFieldString(L, "code");
  item ndx = modelsRenum.byCode[code];
  if (ndx == 0) {
    ndx = models.create();
  }

  VehicleModel* model = models.get(ndx);
  model->flags = 0;
  model->code = code;
  model->count = 0;
  model->length = luaFieldNumber(L, "length");
  model->passengers = luaFieldNumber(L, "passengers");
  model->maxAge = luaFieldNumber(L, "maxAge");
  model->speed = luaFieldNumber(L, "speed");
  model->density = luaFieldNumber(L, "density");
  model->meshImport = readMeshImportData(L, true);
  model->updateFunc = luaFieldFunction(L, "update");

  if (luaFieldBool(L, "colorized")) model->flags |= _modelColorized;
  if (luaFieldBool(L, "wanderer")) model->flags |= _modelWanderer;
  if (luaFieldBool(L, "spawned")) model->flags |= _modelSpawned;
  if (luaFieldBool(L, "sea")) model->flags |= _modelSea;
  if (luaFieldBool(L, "air")) model->flags |= _modelAir;

  char* type = luaFieldString(L, "type");
  model->type = modelTypes.byCode[type];
  modelsByType[model->type].push_back(ndx);

  pushRenum(&modelsRenum, ndx, model->code);

  vehicleModelsDirty = true;
  return ndx;
}

int getVehicleModel(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  item mNdx = luaL_checknumber(L, 1);
  if (mNdx <= 0 || mNdx > vehicleModelsSize()) return 0;

  VehicleModel* m = getVehicleModel(mNdx);

  lua_newtable(L);
  luaSetTableNumber(L, "ndx", mNdx);
  luaSetTableNumber(L, "type", m->type);
  luaSetTableNumber(L, "count", m->count);
  luaSetTableNumber(L, "length", m->length);
  luaSetTableNumber(L, "density", m->density);
  luaSetTableNumber(L, "maxAge", m->maxAge);
  luaSetTableNumber(L, "speed", m->speed);
  luaSetTableNumber(L, "passengers", m->passengers);
  luaSetTableString(L, "code", m->code);

  luaSetTableBool(L, "colorized", bool(m->flags & _modelColorized));
  luaSetTableBool(L, "wanderer", bool(m->flags & _modelWanderer));
  luaSetTableBool(L, "spawned", bool(m->flags & _modelSpawned));
  luaSetTableBool(L, "sea", bool(m->flags & _modelSea));
  luaSetTableBool(L, "air", bool(m->flags & _modelAir));

  return 1;
}

int sizeVehicleModels(lua_State* L) {
  lua_pushnumber(L, vehicleModelsSize());
  return 1;
}

item randomVehicleModel(VehicleModelType type) {
  if (type < 0 || type > modelsByType.size()) return 0;
  item num = modelsByType[type].size();
  if (num == 0) return 0;
  return modelsByType[type][randItem(num)];
}

item vehicleModelsSize() {
  return models.size();
}

item numVehicleModels(VehicleModelType type) {
  if (type < 0 || type > modelsByType.size()) return 0;
  item num = modelsByType[type].size();
  return num;
}

item renumVehicleModel(item model) {
  return renum(&modelsRenum, model);
}

item vehicleModelByCode(const char* code) {
  return modelsRenum.byCode[code];
}

void initVehicleModelCallbacks() {
  renumClear(&modelTypes);
  #define RENUM(N) pushRenum(&modelTypes, N, #N);
  #include "modelTypes.renum"
  #undef RENUM
  modelsByType.resize(numModelTypes);

  addLuaCallback("addVehicleModel", addVehicleModel);
  addLuaCallback("getVehicleModel", getVehicleModel);
  addLuaCallback("maxVehicleModelNdx", sizeVehicleModels);
}

void swapVehicleModels() {
  if (vehicleModelsDirty) {
    modelLength.clear();
    modelLength.resize(models.size()+1);
    modelLength.set(0, 0);

    modelPassengers.clear();
    modelPassengers.resize(models.size()+1);
    modelPassengers.set(0, 0);
    for (int i = 1; i <= models.size(); i++) {
      VehicleModel* model = models.get(i);
      modelLength.set(i, model->length);
      modelPassengers.set(i, model->passengers);
    }
    vehicleModelsDirty = false;
  }
}

float modelLength_v(item model) {
  if (model >= 0 && model < modelLength.size()) {
    return modelLength[model];
  } else {
    return 0;
  }
}

float modelPassengers_v(item model) {
  if (model >= 0 && model < modelPassengers.size()) {
    return modelPassengers[model];
  } else {
    return 0;
  }
}

VehicleModel* getVehicleModel(item ndx) {
  return models.get(ndx);
}

void readVehicleModels(FileBuffer* file) {
  if (file->version < 54) {
    renumLegacyStart(&modelsRenum);
    #define RENUM(N) renumLegacy(&modelsRenum, #N);
    #include "modelData.v53.renum"
    #undef RENUM
  } else {
    renumRead(&modelsRenum, file);
  }
}

void writeVehicleModels(FileBuffer* file) {
  renumWrite(&modelsRenum, file);
}

