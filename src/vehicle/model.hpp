#pragma once

#include "../serialize.hpp"

enum VehicleModelType {
  #define RENUM(N) N,
  #include "modelTypes.renum"
  #undef RENUM

  numModelTypes
};

const int _modelWanderer = 1 << 1;
const int _modelSpawned = 1 << 2;
const int _modelColorized = 1 << 3;
const int _modelSea = 1 << 24;
const int _modelAir = 1 << 25;

struct VehicleModel {
  uint32_t flags;
  item type;
  item meshImport;
  item count;
  float length;
  float density;
  float maxAge;
  float speed;
  item passengers;
  char* code;
  int updateFunc;
};

VehicleModel* getVehicleModel(item ndx);
item vehicleModelByCode(const char* code);
void initVehicleModelCallbacks();
item vehicleModelsSize();
item numVehicleModels(VehicleModelType type);
item randomVehicleModel(VehicleModelType type);
void swapVehicleModels();
float modelLength_v(item model);
float modelPassengers_v(item model);
void resetVehicleModels();

void writeVehicleModels(FileBuffer* file);
void readVehicleModels(FileBuffer* file);
item renumVehicleModel(item model);

