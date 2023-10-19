#pragma once

#include "../serialize.hpp"

enum LegacyDecoTypes {
  TreeDeco, VerticalFence, HorizontalFence, ShrubsV, ShrubsH,
  PathV, PathH, RoadV, RoadH, SwingSet, DecoPool, DecoBigPool,
  ShortSign, ColumnSign, TallSign, HangingSign, Awning,
  Smokestack, ShippingContainers, SmallTanks, BigTank,
  VerticalCropsGreen, HorizontalCropsGreen,
  VerticalCropsYellow, HorizontalCropsYellow,
  BleachersN, BleachersE, BleachersS, BleachersW,
  BaseballDiamond, Facade, Pavilion, Parking,
  ParkingSuper, VerticalFenceLong, HorizontalFenceLong,
  numLegacyDecoTypes
};

struct DecoGroup {
  uint32_t flags;
  char* code;
  char* name;
};

struct DecoType {
  uint32_t flags;
  item meshImport;
  item group;
  char* code;
  char* name;
  float wind;
  item spawns;
};

struct Deco {
  item decoType;
  vec3 location;
  float yaw;
  float scale;
};

const uint32_t _decoGroupExists = 1 << 0;
const uint32_t _decoGroupVisible = 1 << 1;

const uint32_t _decoExists = 1 << 0;
const uint32_t _decoScaleX = 1 << 1;
const uint32_t _decoScaleY = 1 << 2;
const uint32_t _decoScaleZ = 1 << 3;
const uint32_t _decoWindAssigned = 1 << 4;
const uint32_t _decoUseBuildingTexture = 1 << 5;

void initDecoCallbacks();
void readDecosRenum(FileBuffer* file);
void writeDecosRenum(FileBuffer* file);
item renumDeco(item in);
item sizeDecoTypes();
item sizeDecoGroups();
item statueDecoType();
DecoType* getDecoType(item ndx);
DecoGroup* getDecoGroup(item ndx);
const char* getDecoTypeName(item type);
item getDecoTypeByName(std::string name);
bool isDecoVisible(item decoType);
bool isDecoGroupVisible(item groupNdx);
void toggleDecoGroupVisible(item group);

