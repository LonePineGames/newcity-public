#ifndef LAND_H
#define LAND_H

#include "main.hpp"
#include "item.hpp"
#include "box.hpp"
#include "game/constants.hpp"
#include "line.hpp"

#include "serialize.hpp"

// Elevations
const float beachLine = 5;
const float dryGrassLine = 150;
const float grassLine = 300;
const float treeLine = 900;
const float snowLine = 600;

const int maxLandSize = 200;
const float maxMound = tileSize*6;
const float maxSlope = 1.5;
const float maxMoundHeight = maxMound*maxSlope;

const int _landHills = 1 << 0;
const int _landWater = 1 << 1;
const int _landRivers = 1 << 2;
const int _landTrees = 1 << 3;
const int _landSC2K = 1 << 4;
const int _landSuperflat = 1 << 5;
const int _landDesert = 1 << 6;
const int _landTundra = 1 << 7;
const int _landGenerateNeighbors = 1 << 8;
const int _landCustomSeed = 1 << 9;

struct LandConfiguration {
  int flags;
  item landSize;
  item chunkSize;
  int64_t treeSeed;
  int64_t heightSeed;
};

struct ElevationData {
  float height;
  float naturalHeight;
  item items[8];
  float dist[8];
  float targetHeight[8];
  char itemType;
};

bool isUndergroundView();
void setUndergroundView(bool status);
void setWaterVisible(bool status);
LandConfiguration getLandConfig();
void generateLand();
void setDefaultLandConfig();
LandConfiguration* getNextLandConfig();
void randomizeNextLandConfig();
void resetNextLandConfig();
void generateFlatLand(item landSize);
void generateBuildingDesignerLand(float buildX,
    float lowTide, float highTide);
void resetLand();
item getLandSize();
item getMapTiles();
item getChunkSize();
float getMapSize();
void initLandEntities();
void renderLand();
vec3 pointOnLand(vec3 point);
vec3 pointOnLandNatural(vec3 point);
vec3 unitize(vec3 vector);
vec3 unitizeFine(vec3 vector);
vec3 getChunkCenter(item x, item y);
float getNaturalHeight(item x, item y);
float getNaturalHeight(vec3 loc);
const float* getNaturalHeightMap();
float getHeight(item x, item y);
vec3 getTileLocation(item x, item y);
vec3 landIntersect(Line line);

enum BridgeSegmentType {
  BSTTunnel, BSTBridge, BSTLand,
  numBridgeSegmentType
};

struct BridgeSegment {
  float length;
  item type;
};

vector<BridgeSegment> getBridgeSegments(Line line);

void clearTrees(Box b);
bool addTrees(vec3 p);
bool clearTrees(vec3 p);
float setNaturalHeight(vec3 p, float radius, bool mound);
void addBuildingElevator(item it, bool assign);
void addGraphElevator(item it, bool assign);
void removeBuildingElevator(item it);
void removeGraphElevator(item it);
void repairElevators();
void assignHeights();
item numHeightsToAssign();

void writeLand(FileBuffer* file);
void readLand(FileBuffer* file, int version);

// To kill
struct TileIndex {
  item x, y;
};
struct ChunkIndex {
  item x, y;
};
struct RenderChunkIndex {
  item x, y;
};
ChunkIndex chunkIndex(item x, item y);
ChunkIndex chunkIndex(TileIndex ndx);

struct Chunk {
  char* trees;
  ElevationData* elevationData;
};

vec3 getTileLocation(ChunkIndex cNdx, item xt, item yt);
float getHeight(ChunkIndex ndx, item x, item y);
Chunk* getChunk(ChunkIndex location);

#endif
