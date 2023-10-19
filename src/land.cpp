#include "land.hpp"

#include "building/building.hpp"
#include "city.hpp"
#include "draw/entity.hpp"
#include "game/game.hpp"
#include "graph.hpp"
#include "plan.hpp"
#include "renderLand.hpp"
#include "serialize.hpp"
#include "simplex.hpp"
#include "terrain/tree.hpp"
#include "util.hpp"

#include "spdlog/spdlog.h"
//#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <random>
#include <set>
using namespace std;

const int defaultChunkSize = 25;
const float heightFudgeMultiplier = 15;
const int hillOctaves = 6;
const float hillSize = defaultChunkSize;
const int treeOctaves = 3;
const int forestSize = defaultChunkSize / 4;
const float heightAboveSeaLevel = 32;

struct RenderChunk {
  item landEntity;
  item dirtEntity;
  item waterEntity;
  vector<item> treeEntity;
};

float* heightMap;
float* naturalHeightMap;
LandConfiguration landConfig;
LandConfiguration nextLandConfig;
set<item> toAssignHeight;
static int chunkSize = defaultChunkSize;
static item numRenderChunks = 0;
item seemHiderEntity = 0;
item landSize = 24;
Chunk* chunks = 0;
RenderChunk* renderChunks = 0;

//To kill
ChunkIndex chunkIndex(item x, item y) {
  ChunkIndex result;
  result.x = x;
  result.y = y;
  return result;
}

ChunkIndex chunkIndex(TileIndex ndx) {
  ChunkIndex result;
  result.x = ndx.x / chunkSize;
  result.y = ndx.y / chunkSize;
  return result;
}

RenderChunkIndex renderChunkIndex(item x, item y) {
  RenderChunkIndex result;
  result.x = x;
  result.y = y;
  return result;
}

RenderChunkIndex renderChunkIndex(ChunkIndex ndx) {
  RenderChunkIndex result;
  result.x = ndx.x / c(CTerrainChunking);
  result.y = ndx.y / c(CTerrainChunking);
  return result;
}

TileIndex tileIndex(item xc, item yc, item xt, item yt) {
  TileIndex result;
  result.x = xc * chunkSize + xt;
  result.y = yc * chunkSize + yt;
  return result;
}

TileIndex tileIndex(ChunkIndex chunk, item x, item y) {
  TileIndex result;
  result.x = chunk.x * chunkSize + x;
  result.y = chunk.y * chunkSize + y;
  return result;
}

TileIndex tileIndex(RenderChunkIndex chunk, item x, item y) {
  TileIndex result;
  result.x = chunk.x * chunkSize * c(CTerrainChunking) + x;
  result.y = chunk.y * chunkSize * c(CTerrainChunking) + y;
  return result;
}

TileIndex tileIndex(item x, item y) {
  TileIndex result;
  result.x = x;
  result.y = y;
  return result;
}

const TileIndex nullLocation = tileIndex(-1, -1);

item getLandSize() {
  return landSize;
}

item getChunkSize() {
  return chunkSize;
}

item getRenderChunkSize() {
  return chunkSize*c(CTerrainChunking);
}

item getMapTiles() {
  return landSize*chunkSize;
}

float getMapSize() {
  return getMapTiles()*tileSize;
}

RenderChunkIndex normalize(RenderChunkIndex loc) {
  if (loc.x < 0) loc.x = 0;
  if (loc.y < 0) loc.y = 0;
  if (loc.x >= numRenderChunks) loc.x = numRenderChunks - 1;
  if (loc.y >= numRenderChunks) loc.y = numRenderChunks - 1;
  return loc;
}

ChunkIndex normalize(ChunkIndex loc) {
  if (loc.x < 0) loc.x = 0;
  if (loc.y < 0) loc.y = 0;
  if (loc.x >= landSize) loc.x = landSize - 1;
  if (loc.y >= landSize) loc.y = landSize - 1;
  return loc;
}

TileIndex normalize(TileIndex loc) {
  int mapTiles = getMapTiles();
  if (loc.x < 0) loc.x = 0;
  if (loc.y < 0) loc.y = 0;
  if (loc.x >= mapTiles) loc.x = mapTiles - 1;
  if (loc.y >= mapTiles) loc.y = mapTiles - 1;
  return loc;
}

Chunk* getChunk(item x, item y) {
  return getChunk(chunkIndex(x,y));
}

Chunk* getChunk(ChunkIndex loc) {
  loc = normalize(loc);
  return &chunks[loc.x+loc.y*landSize];
}

Chunk* getChunk(TileIndex loc) {
  return getChunk(loc.x/chunkSize, loc.y/chunkSize);
}

RenderChunk* getRenderChunk(RenderChunkIndex loc) {
  loc = normalize(loc);
  return &renderChunks[loc.x+loc.y*numRenderChunks];
}

RenderChunk* getRenderChunk(item x, item y) {
  return getRenderChunk(renderChunkIndex(x, y));
}

TileIndex getTileIndex(vec3 point) {
  return tileIndex(point.x / tileSize, point.y / tileSize);
}

vec3 getChunkCenter(item cx, item cy) {
  float chunkWidth = chunkSize*tileSize;
  return vec3((cx+0.5f)*chunkWidth, (cy+0.5f)*chunkWidth, 0);
}

void addTrees(Chunk* chunk, int x, int y) {
  int ndx = x*chunkSize + y;
  chunk->trees[ndx/8] |= (1 << (ndx%8));
}

void removeTrees(Chunk* chunk, int x, int y) {
  int ndx = x*chunkSize + y;
  chunk->trees[ndx/8] &= ~(1 << (ndx%8));
}

bool hasTrees(Chunk* chunk, int x, int y) {
  int ndx = x*chunkSize + y;
  return chunk->trees[ndx/8] & (1 << (ndx%8));
}

bool hasTrees(int x, int y) {
  TileIndex ndx = normalize(tileIndex(x, y));
  Chunk* c = getChunk(ndx);
  return hasTrees(c, ndx.x%chunkSize, ndx.y%chunkSize);
}

bool approveLand() {
  if (getLandConfig().flags & _landSuperflat) {
    return true;
  }

  item numTiles = getMapTiles();

  item numFlat = 0;

  for (int y = 0; y < numTiles; y ++) {
    int yStride = y*numTiles;
    for (int x = 0; x < numTiles; x ++) {
      float h = naturalHeightMap[x + yStride];
      if (h > beachLine && h < grassLine) {
        numFlat ++;
      }
    }
  }

  float proportion = float(numFlat)/(numTiles*numTiles);
  bool approved = proportion > 0.25;
  SPDLOG_INFO("Land is {}% flat - {}", int(proportion*100),
      approved ? "Approved" : "Regenerate");
  return approved;
}

void generateLandInner();

LandConfiguration getLandConfig() {
  return landConfig;
}

// Generates a random seed, and ensures it's positive
// Requires an existing random number generator passed as arg
int64_t generateSeed(std::mt19937 &randomGen) {
  int64_t randomVal = randomGen();
  if(randomVal < 0) randomVal *= -1;
  return randomVal;
}

// Generates a random seed, and ensures it's positive
// Will instantiate a random number generator
int64_t generateSeed() {
  std::mt19937 randomGen(time(NULL));
  int64_t randomVal = randomGen();
  if(randomVal < 0) randomVal *= -1;
  return randomVal;
}

void randomizeNextLandConfig() {
  // Seed the 32-bit random number generator with time()
  std::mt19937 randomGen(time(NULL));

  // Generate height seed
  nextLandConfig.heightSeed = generateSeed(randomGen);
  nextLandConfig.treeSeed = nextLandConfig.heightSeed;
}

void resetNextLandConfig() {
  nextLandConfig = landConfig;
}

LandConfiguration* getNextLandConfig() {
  return &nextLandConfig;
}

LandConfiguration defaultLandConfig() {
  LandConfiguration conf;
  conf.flags = _landHills | _landWater | _landTrees | _landGenerateNeighbors;
  conf.landSize = c(CDefaultLandSize);
  conf.chunkSize = chunkSize;
  return conf;
}

void setDefaultLandConfig() {
  landConfig = defaultLandConfig();
  nextLandConfig = landConfig;
  SPDLOG_INFO("setDefaultLandConfig {}", nextLandConfig.landSize);
}

void generateLand() {
  SPDLOG_INFO("Generating new land, landSize {}", nextLandConfig.landSize);

  resetLand();
  landConfig = nextLandConfig;
  generateLandInner();

  while (!approveLand()) {
    randomizeNextLandConfig();
    resetLand();
    landConfig = nextLandConfig;
    generateLandInner();
  }

  // Once we have a good land generated, set up the nextLandConfig again
  randomizeNextLandConfig();
}

void resetLand() {
  SPDLOG_INFO("resetLand {} {}", landSize,
      chunks == 0 ? "(unallocated)" : "(allocated)");
  SPDLOG_INFO("nextLandConfig.landSize {}", nextLandConfig.landSize);

  resetLandRender();

  if (chunks != 0) {
    for (int x = 0; x < landSize; x ++) {
      for (int y = 0; y < landSize; y ++) {
        Chunk* chunk = getChunk(x,y);
        free(chunk->trees);
        free(chunk->elevationData);
      }
    }

    for (int x = 0; x < numRenderChunks; x ++) {
      for (int y = 0; y < numRenderChunks; y ++) {
        RenderChunk* chunk = getRenderChunk(x,y);
        vector<item> swap;
        chunk->treeEntity.swap(swap);
      }
    }

    free(chunks);
    free(renderChunks);
    free(naturalHeightMap);
    free(heightMap);
    renderChunks = 0;
    chunks = 0;
  }

  chunkSize = defaultChunkSize;
  toAssignHeight.clear();
}

void allocLand() {
  SPDLOG_INFO("allocLand {}", landSize);
  item numTiles = getMapTiles();
  numRenderChunks = ceil(landSize*1.0/c(CTerrainChunking));

  heightMap = (float*) calloc(numTiles*numTiles, sizeof(float));
  naturalHeightMap = (float*) calloc(numTiles*numTiles, sizeof(float));
  chunks = (Chunk*) calloc(landSize*landSize, sizeof(Chunk));
  renderChunks = (RenderChunk*) calloc(numRenderChunks*numRenderChunks,
      sizeof(RenderChunk));

  for (int x = 0; x < landSize; x ++) {
    for (int y = 0; y < landSize; y ++) {
      Chunk* chunk = getChunk(x,y);
      int tilesPerChunk = chunkSize*chunkSize;
      chunk->trees = (char*) calloc((tilesPerChunk-1)/8+1, 1);
      chunk->elevationData = (ElevationData*) calloc(tilesPerChunk,
          sizeof(ElevationData));
    }
  }
}

void initMaps() {
  item numTiles = getMapTiles();

  for (int x = 0; x < numTiles; x ++) {
    for (int y = 0; y < numTiles; y ++) {
      int cx = x/chunkSize;
      int cy = y/chunkSize;
      int tx = x%chunkSize;
      int ty = y%chunkSize;
      int mi = x + numTiles*y;
      Chunk* chunk = &chunks[cx + cy*landSize];
      ElevationData elevation = chunk->elevationData[tx*chunkSize + ty];

      heightMap[mi] = elevation.height;
      naturalHeightMap[mi] = elevation.naturalHeight;
    }
  }
}

void initEntities(RenderChunkIndex ndx) {
  RenderChunk* chunk = getRenderChunk(ndx);
  if (chunk->landEntity == 0) {
    bool simpleOnly = c(CLandSimpleDistance) <= 0;
    bool treeSimpleOnly = c(CTreeSimpleDistance) <= 0;
    bool waterSimpleOnly = c(CWaterSimpleDistance) <= 0;

    chunk->landEntity = addEntity(LandShader);
    chunk->waterEntity = addEntity(WaterShader);

    chunk->treeEntity.clear();
    for (int i = 0; i < sizeTreeTypes(); i++) {
      item entity = addEntity(TreeShader);
      chunk->treeEntity.push_back(entity);
      setEntityVisible(entity, false);
      createSimpleMeshForEntity(entity);
      if (!treeSimpleOnly) {
        createMeshForEntity(entity);
      }
    }

    if (!simpleOnly) {
      createMeshForEntity(chunk->landEntity);
    }

    if (!waterSimpleOnly) {
      createMeshForEntity(chunk->waterEntity);
    }

    createSimpleMeshForEntity(chunk->landEntity);
    createSimpleMeshForEntity(chunk->waterEntity);

    setEntityVisible(chunk->landEntity, false);
    setEntityVisible(chunk->waterEntity, false);

    int ls1 = numRenderChunks - 1;
    if (ndx.x == 0 || ndx.y == 0 || ndx.x == ls1 || ndx.y == ls1) {
      chunk->dirtEntity = addEntity(PaletteShader);
      createSimpleMeshForEntity(chunk->dirtEntity);
      if (!simpleOnly) {
        createMeshForEntity(chunk->dirtEntity);
      }
      setEntityVisible(chunk->dirtEntity, false);
    }
  }
}

void generateFlatLand(item size) {
  landSize = size;
  landConfig.flags = _landSuperflat;
  nextLandConfig.flags = _landSuperflat;
  landConfig.landSize = size;
  SPDLOG_INFO("generateFlatLand {}", landSize);
  allocLand();

  for (int x = 0; x < landSize; x ++) {
    for (int y = 0; y < landSize; y ++) {
      Chunk* chunk = getChunk(x,y);

      for (int x0 = 0; x0 < chunkSize; x0 ++) {
        for (int y0 = 0; y0 < chunkSize; y0 ++) {
          int x1 = x * chunkSize + x0;
          int y1 = y * chunkSize + y0;

          ElevationData* data = &chunk->elevationData[x0*chunkSize + y0];
          data->height = c(CSuperflatHeight);
          data->naturalHeight = c(CSuperflatHeight);
        }
      }
    }
  }

  initMaps();
}

void generateBuildingDesignerLand(float buildX,
    float lowTide, float highTide) {
  if (getGameMode() == ModeGame) return;
  SPDLOG_INFO("generateBuildingDesignerLand");

  if (chunks == 0) {
    allocLand();
  }

  for (int x = 0; x < landSize; x ++) {
    for (int y = 0; y < landSize; y ++) {
      Chunk* chunk = getChunk(x,y);

      for (int x0 = 0; x0 < chunkSize; x0 ++) {
        for (int y0 = 0; y0 < chunkSize; y0 ++) {
          int x1 = x * chunkSize + x0;
          int y1 = y * chunkSize + y0;
          ElevationData* data = &chunk->elevationData[x0*chunkSize + y0];
          vec3 loc = getTileLocation(x1, y1);
          float buildDist = loc.x - buildX;
          float height = buildDist > lowTide ? -40 :
            buildDist > highTide ? -5 : c(CSuperflatHeight);

          data->height = height;
          data->naturalHeight = height;
        }
      }
    }
  }

  initMaps();
  renderLand();
}

void generateLandInner() {
  landSize = landConfig.landSize;
  SPDLOG_INFO("generateLandInner {}", landSize);
  allocLand();

  // Instantiate a random number generator, in case we need it
  std::mt19937 randomGen(time(NULL));

  // If we're not using a custom seed, generate a seed based on the current time
  if(!(landConfig.flags & _landCustomSeed)) {
    landConfig.heightSeed = generateSeed(randomGen);
    landConfig.treeSeed = landConfig.heightSeed;
  }

  // Reset custom seed bitflags
  // This means only one attempt will be made to generate a land with a custom seed
  // as the flags will be reset on the next pass
  landConfig.flags &= ~_landCustomSeed;

  struct osn_context* treeGenerator;
  open_simplex_noise(landConfig.treeSeed, &treeGenerator);

  struct osn_context* heightGenerator;
  open_simplex_noise(landConfig.heightSeed, &heightGenerator);
  float maxHeight = c(CMaxHeight);

  for (int x = 0; x < landSize; x ++) {
    for (int y = 0; y < landSize; y ++) {
      Chunk* chunk = getChunk(x,y);

      for (int x0 = 0; x0 < chunkSize; x0 ++) {
        for (int y0 = 0; y0 < chunkSize; y0 ++) {
          int x1 = x * chunkSize + x0;
          int y1 = y * chunkSize + y0;

          if (landConfig.flags & _landTrees) {
            float treeProb = 0;
            int power = 2;
            int totalPower = pow(2, treeOctaves) - 1;
            for (int i = 0; i < treeOctaves; i ++) {
              float h0 = open_simplex_noise4(treeGenerator,
                (double) (x1 + 1) / (power * forestSize),
                (double) (y1 + 1) / (power * forestSize), 50.0, 50.0);
              treeProb += (h0 * power) / totalPower;
              power *= 2;
            }
            treeProb += pow(randFloat(), 2)*c(CTreeOuterFrequency);
            if (randFloat() < treeProb) {
              addTrees(chunk, x0, y0);
            }
          }

          if (landConfig.flags & _landSuperflat) {
            ElevationData* data = &chunk->elevationData[x0*chunkSize + y0];
            data->height = c(CSuperflatHeight);
            data->naturalHeight = c(CSuperflatHeight);

          } else {
            float height = 0;
            int power = 1;
            int totalPower = pow(2, hillOctaves) - 1;
            for (int i = 0; i < hillOctaves; i ++) {
              float h0 = open_simplex_noise4(heightGenerator,
                (double) (x1 + 1) / (power * hillSize),
                (double) (y1 + 1) / (power * hillSize),
                i, power);
              float add = (h0 * power) / totalPower;
              if (validate(add)) {
                height += add;
              } else {
                SPDLOG_INFO("add:{} at ({}-{},{}-{}) h0:{} power:{} tot:{}",
                    add, x, x0, y, y0, h0, power, totalPower);
              }
              power *= 2;
            }

            //sign-preserving square, creates flat lowlands and dramatic hills
            height = abs(height) * height;
            //height = pow(abs(height), 2 + 1) / height;
            height *= maxHeight * heightFudgeMultiplier;
            height += heightAboveSeaLevel;

            if (height < -maxHeight) {
              height = -maxHeight;
            }
            if (!(landConfig.flags & _landHills) && height > c(CSuperflatHeight)) {
              height = c(CSuperflatHeight);
            }
            if (!(landConfig.flags & _landWater) && height < beachLine*2) {
              height = beachLine * 2;
            }

            if ((landConfig.flags & _landSC2K)
                && height > beachLine*2 && height < 100) {
              const float step = c(CZTileSize);
              height = round(height/step)*step;
            }

            /*
            if (x % 5 == 0 && y % 5 == 0 && abs(height-beachLine) < 2) {
              height = beachLine+2;
            }
            */

            if (height > 2 && height < 8) {
              height = 8;
              /*
              const float a = (height-2.f)/6.f;
              const float b = 1.f-a;
              const float c = (1.f-b*b);
              const float d = c*6.f+2.f;
              height = d;
              */
            }

            ElevationData* data = &chunk->elevationData[x0*chunkSize + y0];
            data->height = height;
            data->naturalHeight = height;
          }
        }
      }
    }
  }

  initMaps();
  open_simplex_noise_free(treeGenerator);
  open_simplex_noise_free(heightGenerator);
}

void initLandEntities() {
  seemHiderEntity = addEntity(PaletteShader);
  createMeshForEntity(seemHiderEntity);

  for (int x = 0; x < numRenderChunks; x ++) {
    for (int y = 0; y < numRenderChunks; y ++) {
      RenderChunkIndex loc = renderChunkIndex(x, y);
      initEntities(loc);
    }
  }
}

void renderLand() {
  //for (int y = landSize-1; y >= 0; y --) {
    //for (int x = 0; x < landSize; x ++) {
  for (int x = 0; x < numRenderChunks; x ++) {
    for (int y = numRenderChunks-1; y >= 0; y --) {
      RenderChunkIndex loc = renderChunkIndex(x, y);
      initEntities(loc);
      queueRenderChunk(loc);
    }
  }
}

vec3 pointOnLand(vec3 point) {
  if (chunks == 0) return point;
  int x = point.x / tileSize;
  int y = point.y / tileSize;
  float mx = (point.x - x*tileSize)/tileSize;
  float my = (point.y - y*tileSize)/tileSize;
  float z00 = getHeight(x,   y);
  float z10 = getHeight(x+1, y);
  float z01 = getHeight(x,   y+1);
  float z11 = getHeight(x+1, y+1);

  point.z = lerp(lerp(z00, z10, mx), lerp(z01, z11, mx), my);
  return point;
}

vec3 pointOnLandNatural(vec3 point) {
  int x = point.x / tileSize;
  int y = point.y / tileSize;
  float mx = (point.x - x*tileSize)/tileSize;
  float my = (point.y - y*tileSize)/tileSize;
  float z00 = getNaturalHeight(x,   y);
  float z10 = getNaturalHeight(x+1, y);
  float z01 = getNaturalHeight(x,   y+1);
  float z11 = getNaturalHeight(x+1, y+1);

  point.z = lerp(lerp(z00, z10, mx), lerp(z01, z11, mx), my);
  return point;
}

vec3 landIntersect(Line line) {
  vec3 orig = line.start;
  vec3 dir = line.end - line.start;
  float lx = dir.x;
  float ly = dir.z;
  float du = tileSize / sqrt(lx * lx + ly * ly);
  int mapTiles = getMapTiles();

  item bx = 0;
  item by = 0;
  float bestDistance = FLT_MAX;
  for(float u = 0; u < 1.0; u += du) {
    vec3 p = orig + dir * u;
    item px = p.x/tileSize;
    item py = p.y/tileSize;
    vec3 t00 = getTileLocation(px, py);
    float dist = vecDistance(t00, p);
    if (dist < bestDistance) {
      bestDistance = dist;
      bx = px;
      by = py;
    }
  }

  if (bx < 0) bx = 0;
  if (by < 0) by = 0;
  if (bx > mapTiles+1) bx = mapTiles+1;
  if (by > mapTiles+1) by = mapTiles+1;

  for (int x = std::max(0, bx-2);
    x < std::min(mapTiles+1, bx+3); x++) {
    for (int y = std::max(0, by-2);
      y < std::min(mapTiles+1, by+3); y++) {

      vec3 t00 = getTileLocation(x,   y);
      vec3 t01 = getTileLocation(x,   y+1);
      vec3 t10 = getTileLocation(x+1, y);
      vec3 t11 = getTileLocation(x+1, y+1);

      float ru = std::max(triangle_intersection(line, t00, t01, t10),
        triangle_intersection(line, t01, t11, t10));
      if (ru > 0) {
        return orig + dir * ru;
      }
    }
  }

  return getTileLocation(bx, by);
}

vector<BridgeSegment> getBridgeSegments(Line line) {
  vec3 transverse = line.end-line.start;
  float l = length(transverse);
  float unitSize = 1;
  vec3 unit = normalize(transverse)*unitSize;
  vec3 cursor = line.start;
  vector<BridgeSegment> result;
  float amount = 0;
  float along = 0;
  item type = BSTLand;

  for(; along < l; along+=unitSize) {
    float elevation = cursor.z - pointOnLand(cursor).z;
    item newType = elevation < 0 ? BSTTunnel :
      elevation > c(CRoadRise)*8 ? BSTBridge : BSTLand;

    if (along == 0) {
      type = newType;

    } else if (newType != type) {
      if (newType < type) {
        if (amount > 0) {
          result.push_back(BridgeSegment {amount-unitSize, type});
          amount = unitSize;
        } else {
          result.push_back(BridgeSegment {0, type});
        }

      } else {
        result.push_back(BridgeSegment {amount+unitSize, type});
        along+=unitSize;
        amount = 0;
      }
      type = newType;
    }
    amount += unitSize;
    cursor += unit;
  }

  result.push_back(BridgeSegment {amount+l-along, type});
  //if (!onLand && amount != 0) {
    //result.push_back(0);
  //}
  return result;
}

vec3 unitize(vec3 vector) {
  float unit = tileSize * 5;
  return vec3(
    round(vector.x / unit) * unit,
    round(vector.y / unit) * unit,
    vector.z
  );
}

vec3 unitizeFine(vec3 vector) {
  float unit = tileSize;
  return vec3(
    round(vector.x / unit) * unit,
    round(vector.y / unit) * unit,
    vector.z
  );
}

float getNaturalHeight(const item x, const item y) {
  const int mapTiles = getMapTiles();
  const int maxT = mapTiles - 1;
  const int nx = x < 0 ? 0 : x > maxT ? maxT : x;
  const int ny = y < 0 ? 0 : y > maxT ? maxT : y;
  return naturalHeightMap[nx + ny*mapTiles];
}

float getNaturalHeight(vec3 loc) {
  return getNaturalHeight(loc.x/tileSize, loc.y/tileSize);
}

const float* getNaturalHeightMap() {
  return naturalHeightMap;
}

float getHeight(TileIndex ndx) {
  ndx = normalize(ndx);
  return heightMap[ndx.x + ndx.y*getMapTiles()];
}

float getHeight(ChunkIndex cNdx, item xt, item yt) {
  item x = cNdx.x*chunkSize + xt;
  item y = cNdx.y*chunkSize + yt;
  return getHeight(x, y);
}

float getHeight(RenderChunkIndex cNdx, item xt, item yt) {
  item x = cNdx.x*chunkSize*c(CTerrainChunking) + xt;
  item y = cNdx.y*chunkSize*c(CTerrainChunking) + yt;
  return getHeight(x, y);
}

float getHeight(item x, item y) {
  return getHeight(tileIndex(x, y));
}

vec3 getTileLocation(const item x, const item y) {
  return vec3(x * tileSize, y * tileSize, getHeight(x,y));
}

vec3 getTileLocation(ChunkIndex cNdx, item xt, item yt) {
  item x = cNdx.x*chunkSize + xt;
  item y = cNdx.y*chunkSize + yt;
  return getTileLocation(x, y);
}

vec3 getTileLocation(RenderChunkIndex cNdx, item xt, item yt) {
  item x = cNdx.x*chunkSize*c(CTerrainChunking) + xt;
  item y = cNdx.y*chunkSize*c(CTerrainChunking) + yt;
  return getTileLocation(x, y);
}

vec3 getTileLocation(TileIndex ndx) {
  return getTileLocation(ndx.x, ndx.y);
}

void clearChunkTrees(ChunkIndex cNdx, Box b) {
  Chunk* chunk = getChunk(cNdx);
  bool render = false;

  for (int x0 = 0; x0 < chunkSize; x0++) {
    for (int y0 = 0; y0 < chunkSize; y0++) {
      float dist = boxDistance(b, getTileLocation(cNdx, x0, y0));
      if (dist < tileSize*.75f) {
        render |= hasTrees(chunk, x0, y0);
        removeTrees(chunk, x0, y0);
      }
    }
  }

  if (render) {
    queueRenderChunk(cNdx);
  }
}

void clearTrees(Box b) {
  float chunkWidth = chunkSize*tileSize;
  for (int cx = 0; cx < landSize; cx++) {
    for (int cy = 0; cy < landSize; cy++) {
      vec3 center = vec3((cx+0.5f)*chunkWidth, (cy+0.5f)*chunkWidth, 0);
      if (boxDistance(b, center) < chunkWidth) {
        ChunkIndex ndx = chunkIndex(cx, cy);
        clearChunkTrees(ndx, b);
      }
    }
  }
}

bool addTrees(vec3 p) {
  TileIndex ndx = getTileIndex(p);
  ChunkIndex cNdx = chunkIndex(ndx);
  Chunk* chunk = getChunk(cNdx);
  item x0 = ndx.x%chunkSize;
  item y0 = ndx.y%chunkSize;

  ElevationData* data = &(chunk->elevationData[x0*chunkSize + y0]);
  for (int i = 0; i < 8; i++) {
    if (data->items[i] != 0 && data->dist[i] < tileSize*.25f) {
      return false;
    }
  }

  if (hasTrees(chunk, x0, y0)) return false;

  addTrees(chunk, x0, y0);
  queueRenderChunk(cNdx);
  return true;
}

bool clearTrees(vec3 p) {
  TileIndex ndx = getTileIndex(p);
  ChunkIndex cNdx = chunkIndex(ndx);
  Chunk* chunk = getChunk(cNdx);
  item x0 = ndx.x%chunkSize;
  item y0 = ndx.y%chunkSize;
  removeTrees(chunk, x0, y0);
  queueRenderChunk(cNdx);
  return true;
}

/************
 * Elevator *
 ************/
struct Elevator {
  Box b;
  Line l;
  item it;
  bool isBuilding;
};

void setElevator(ElevationData* data, int ndx, item it, bool isBuilding,
    float dist, float targetHeight) {

  char itemType = data->itemType;
  char flag = (1 << ndx);
  data->itemType = isBuilding ? (itemType | flag) : (itemType & ~flag);

  data->items[ndx] = it;
  data->dist[ndx] = dist;
  data->targetHeight[ndx] = targetHeight;
}

bool getElevatorType(ElevationData* data, int ndx) {
  return data->itemType & (1 << ndx);
}

Elevator getElevator(item it, bool isBuilding) {
  Elevator e;
  e.it = it;
  e.isBuilding = isBuilding;

  if (isBuilding) {
    Building* building = getBuilding(it);
    e.b = getBuildingBox(it);
    e.l = line(building->location, building->location + vec3(e.b.axis0, 0));

  } else {
    e.b = getGraphBox(it);
    e.l = getLine(it);

    if (it > 0) {
      // Lower the road a bit to prevent the land rising above
      vec3 a = e.l.end - e.l.start;
      float slope = abs(a.z/(a.x*a.x + a.y*a.y));
      float lower = 2*c(CRoadRise) + tileSize * slope;
      e.l.start.z -= lower;
      e.l.end.z -= lower;
    } else {
      float lower = 2*c(CRoadRise);
      e.l.start.z -= lower;
      e.l.end.z -= lower;
    }
  }

  return e;
}

Elevator getElevator(ElevationData* data, int ndx) {
  bool isBuilding = getElevatorType(data, ndx);
  return getElevator(data->items[ndx], isBuilding);
}

bool assignTileHeight(TileIndex ndx) {
  Chunk* chunk = getChunk(ndx);
  int x0 = ndx.x%chunkSize;
  int y0 = ndx.y%chunkSize;

  ElevationData* data = &(chunk->elevationData[x0*chunkSize + y0]);
  vec3 location = getTileLocation(ndx.x, ndx.y);
  float min = 100000;
  float max = -100000;

  for (int i=0; i < 8; i++) {
    if (data->items[i] == 0) {
      continue;
    }

    float dist = data->dist[i];
    float height = data->targetHeight[i];
    float myMin = height + dist*maxSlope;
    float myMax = height - dist*maxSlope;

    if (myMin < min) {
      min = myMin;
    }
    if (myMax > max) {
      max = myMax;
    }
  }

  float natHeight = data->naturalHeight;
  float newHeight = 0;
  if (natHeight < 1) {
    newHeight = natHeight;
  } else {
    newHeight = std::min(min, std::max(max, natHeight));
  }

  if (abs(data->height - newHeight) > 0.001) {
    data->height = newHeight;
    heightMap[ndx.x + ndx.y*getMapTiles()] = newHeight;

    for (int i=0; i < 8; i++) {
      if (data->items[i] != 0 && !getElevatorType(data, i)) {
        renderGraphElement(data->items[i]);
      }
    }

    return true;

  } else {
    return false;
  }
}

void assignChunkHeight(ChunkIndex cNdx, vector<ChunkIndex>* chunksTouched) {
  Chunk* chunk = getChunk(cNdx);
  int ml = landSize-1;
  int mc = chunkSize-1;

  for (int x0 = 0; x0 < chunkSize; x0++) {
    for (int y0 = 0; y0 < chunkSize; y0++) {
      bool modified = assignTileHeight(tileIndex(cNdx, x0, y0));
      if (modified) {
        chunksTouched->push_back(cNdx);
        for (int x1 = -1; x1 <= 1; x1 ++) {
          for (int y1 = -1; y1 <= 1; y1 ++) {
            if (x1 == 0 && y1 == 0) continue;
            if (x1 == -1 && (x0 != 0  || cNdx.x == 0)) continue;
            if (x1 ==  1 && (x0 != mc || cNdx.x == ml)) continue;
            if (y1 == -1 && (y0 != 0  || cNdx.y == 0)) continue;
            if (y1 ==  1 && (y0 != mc || cNdx.y == ml)) continue;
            chunksTouched->push_back(chunkIndex(cNdx.x+x1, cNdx.y+y1));
          }
        }
      }
    }
  }
}

void assignHeight(Elevator e, vector<ChunkIndex> toRender) {
  float chunkWidth = chunkSize*tileSize;

  for (int cx = 0; cx < landSize; cx++) {
    for (int cy = 0; cy < landSize; cy++) {
      vec3 center = vec3((cx+0.5f)*chunkWidth, (cy+0.5f)*chunkWidth, 0);
      if (boxDistance(e.b, center) < chunkWidth*2) {
        toAssignHeight.insert(cy*landSize + cx);
      }
    }
  }
}

void assignHeights() {
  vector<ChunkIndex> toRender;
  for (set<item>::iterator it = toAssignHeight.begin();
      it != toAssignHeight.end(); it++) {
    item ndx = *it;
    ChunkIndex cNdx = chunkIndex(ndx%landSize, ndx/landSize);
    assignChunkHeight(cNdx, &toRender);
  }
  toAssignHeight.clear();

  //if (chunks[0].landEntity == 0) {
   // return; // Still loading, don't render anything
  //}

  for (int i = 0; i < toRender.size(); i++) {
    ChunkIndex ci = toRender[i];
    bool isDuplicate = false;
    for (int j = 0; j < i; j++) {
      ChunkIndex cj = toRender[j];
      if (ci.x == cj.x && ci.y == cj.y) {
        isDuplicate = true;
        break;
      }
    }

    if (!isDuplicate) {
      queueRenderChunk(ci);
    }
  }
}

item numHeightsToAssign() {
  return toAssignHeight.size();
}

void addElevatorToTile(TileIndex tileNdx, ElevationData* data, Elevator e) {
  vec3 location = getTileLocation(tileNdx.x, tileNdx.y);
  float dist = boxDistance(e.b, location)
    - tileSize * (e.isBuilding ? .5f : e.it < 0 ? 1.f : 1.f);
  dist = dist < 0 ? 0 : dist;
  float targetHeight = nearestPointOnLine(location, e.l).z;
  int ndx = 0;

  if (dist > maxMound) {
    return;
  }

  // Find first empty
  for (; ndx < 8 && data->items[ndx] != 0; ndx++);

  // All full, see what we can kick out
  if (ndx == 8) {
    int worst = -1;
    float worstMinHeight = targetHeight + dist*maxSlope;
    for (int i = 0; i < 8; i++) {
      float myDist = data->dist[i];
      float myHeight = data->targetHeight[i];
      float myMin = myHeight + myDist*maxSlope;
      if (myMin > worstMinHeight) {
        worstMinHeight = myMin;
        worst = i;
      }
    }

    if (worst >= 0) {
      ndx = worst;
    } else {
      return;
    }
  }

  setElevator(data, ndx, e.it, e.isBuilding, dist, targetHeight);
}

void addElevatorToChunk(ChunkIndex cNdx, Elevator e) {
  Chunk* chunk = getChunk(cNdx);
  for (int x0 = 0; x0 < chunkSize; x0++) {
    for (int y0 = 0; y0 < chunkSize; y0++) {
      addElevatorToTile(tileIndex(cNdx, x0, y0),
          &chunk->elevationData[x0*chunkSize + y0], e);
    }
  }
}

void addElevator(Elevator e, bool assign) {
  float chunkWidth = chunkSize*tileSize;
  for (int cx = 0; cx < landSize; cx++) {
    for (int cy = 0; cy < landSize; cy++) {
      vec3 center = vec3((cx+0.5f)*chunkWidth, (cy+0.5f)*chunkWidth, 0);
      if (boxDistance(e.b, center) < chunkWidth + maxMound) {
        addElevatorToChunk(chunkIndex(cx, cy), e);
        if (assign) {
          toAssignHeight.insert(cy*landSize + cx);
        }
      }
    }
  }

  clearTrees(e.b);
}

void removeElevatorFromTile(TileIndex tileNdx, ElevationData* data,
    Elevator e) {

  for (int ndx = 0; ndx < 8; ndx ++) {
    if (data->items[ndx] == e.it &&
        getElevatorType(data, ndx) == e.isBuilding) {
      setElevator(data, ndx, 0, false, 0, 0);
    }
  }
}

void removeElevatorFromChunk(ChunkIndex cNdx, Elevator e) {
  Chunk* chunk = getChunk(cNdx);
  for (int x0 = 0; x0 < chunkSize; x0++) {
    for (int y0 = 0; y0 < chunkSize; y0++) {
      removeElevatorFromTile(tileIndex(cNdx, x0, y0),
          &chunk->elevationData[x0*chunkSize + y0], e);
    }
  }
}

void removeElevator(Elevator e) {
  float chunkWidth = chunkSize*tileSize;
  for (int cx = 0; cx < landSize; cx++) {
    for (int cy = 0; cy < landSize; cy++) {
      vec3 center = vec3((cx+0.5f)*chunkWidth, (cy+0.5f)*chunkWidth, 0);
      if (boxDistance(e.b, center) < chunkWidth*2.f) {
        removeElevatorFromChunk(chunkIndex(cx, cy), e);
      }
    }
  }
  vector<ChunkIndex> toRender;
  assignHeight(e, toRender);
}

void repairElevators() {
  for (int cx = 0; cx < landSize; cx++) {
    for (int cy = 0; cy < landSize; cy++) {
      Chunk* chunk = getChunk(cx, cy);
      for (int x0 = 0; x0 < chunkSize; x0++) {
        for (int y0 = 0; y0 < chunkSize; y0++) {
          ElevationData* data = &chunk->elevationData[x0*chunkSize + y0];
          for (int ndx = 0; ndx < 8; ndx ++) {
            if (data->items[ndx] == 0) continue;
            Elevator e = getElevator(data, ndx);
            vec3 location = getTileLocation(tileIndex(cx, cy, x0, y0));
            float dist = boxDistance(e.b, location) - tileSize *
              (e.isBuilding ? .5f : 1.f);
            dist = dist < 0 ? 0 : dist;
            data->dist[ndx] = dist;
          }
        }
      }
    }
  }
}

void addBuildingElevator(item it, bool assign) {
  addElevator(getElevator(it, true), assign);
}

void addGraphElevator(item it, bool assign) {
  addElevator(getElevator(it, false), assign);
}

void removeBuildingElevator(item it) {
  removeElevator(getElevator(it, true));
}

void removeGraphElevator(item it) {
  removeElevator(getElevator(it, false));
}

void setNaturalHeight(TileIndex ndx, float z) {
  ChunkIndex cNdx = chunkIndex(ndx);
  Chunk* chunk = getChunk(cNdx);
  item x0 = ndx.x%chunkSize;
  item y0 = ndx.y%chunkSize;
  item numTiles = getMapTiles();

  ElevationData* data = &(chunk->elevationData[x0*chunkSize + y0]);
  data->naturalHeight = z;
  naturalHeightMap[ndx.x + ndx.y*numTiles] = z;
  bool modified = assignTileHeight(ndx);
  if (modified) queueRenderChunk(cNdx);
}

float setNaturalHeight(vec3 p, float radius, bool smooth) {
  TileIndex ndx = getTileIndex(p);
  TileIndex start = ndx;
  TileIndex end = ndx;
  float tileRadius = radius/tileSize + 125;
  start.x -= tileRadius;
  start.y -= tileRadius;
  end.x += tileRadius;
  end.y += tileRadius;
  start = normalize(start);
  end = normalize(end);
  float radiusSqrd = radius*radius;
  float change = 0;
  item numTiles = getMapTiles();

  for (int y = start.y; y < end.y; y++) {
    int yStride = y*numTiles;
    for (int x = start.x; x < end.x; x++) {
      vec3 loc = getTileLocation(x,y);
      float natZ = naturalHeightMap[x + yStride];
      float distFromCener = distance2D(p, loc);
      float z = natZ;

      if (smooth) {
        float smoothDist = distFromCener/radius*.5f;
        smoothDist = pow(smoothDist, 0.1);
        smoothDist = clamp(smoothDist, 0.9f, 1.f);
        z = mix(p.z, natZ, smoothDist);

      } else {
        float dist = distFromCener - radius;
        if (dist <= 0) dist = 0;
        float max = p.z + dist*maxSlope;
        float min = p.z - dist*maxSlope;
        z = z < min ? min : (z > max ? max : z);
      }

      setNaturalHeight(tileIndex(x, y), z);
      change += abs(natZ-z);
    }
  }

  clearTileDataCache();
  return change;
}

// Legacy
void readTile(FileBuffer *file, int version, TileIndex ndx) {
  Chunk* chunk = getChunk(ndx);
  int x0 = ndx.x%chunkSize;
  int y0 = ndx.y%chunkSize;

  if (version <= 10) {
    fread_item(file, version);
    fread_item(file, version);
  }

  const int _tileTrees = 1<<1;
  int flags        = fread_item(file, version);
  if (flags & _tileTrees) {
    int y = ndx.y%chunkSize;
    int x = ndx.x%chunkSize;
    addTrees(chunk, x, y);
  }

  if (version <= 10) {
    fread_item(file, version);
  }

  float height = fread_vec3(file).z;
  chunk->elevationData[x0*chunkSize + y0].height = height;
  chunk->elevationData[x0*chunkSize + y0].naturalHeight = height;
  fread_float(file); // maxHeight
}

void writeChunk(FileBuffer *file, int x, int y) {
  Chunk* chunk = getChunk(x, y);
  fwrite(chunk->trees, sizeof(char), chunkSize*chunkSize/8, file);
  fwrite(chunk->elevationData, sizeof(ElevationData),
      chunkSize*chunkSize, file);
}

void readChunk(FileBuffer *file, int version, int x, int y) {
  Chunk* chunk = getChunk(x, y);

  if (version < 25) {
    for (int xt = 0; xt < chunkSize; xt++) {
      for (int yt = 0; yt < chunkSize; yt++) {
        readTile(file, version, tileIndex(x, y, xt, yt));
      }
    }

  } else {
    fread(chunk->trees, sizeof(char), chunkSize*chunkSize/8, file);
    fread(chunk->elevationData, sizeof(ElevationData),
        chunkSize*chunkSize, file);
  }
}

void writeLand(FileBuffer *file) {
  fwrite_int(file, chunkSize);
  fwrite_int(file, landSize);
  fwrite_int(file, landConfig.flags);
  fwrite_int64_t(file, landConfig.treeSeed);
  fwrite_int64_t(file, landConfig.heightSeed);

  for (int x = 0; x < landSize; x++) {
    for (int y = 0; y < landSize; y++) {
      writeChunk(file, x, y);
    }
  }
}

void readLand(FileBuffer *file, int version) {
  chunkSize = fread_int(file);
  landSize = fread_int(file);
  landConfig.chunkSize = chunkSize;
  landConfig.landSize = landSize;

  if (version >= 50) {
    landConfig.flags = fread_int(file);
    landConfig.treeSeed = fread_int64_t(file);
    landConfig.heightSeed = fread_int64_t(file);
  } else if (version >= 45) {
    #ifdef _WIN32
      landConfig.flags = fread_int(file);
      landConfig.treeSeed = fread_int(file);
      landConfig.heightSeed = fread_int(file);
    #else
      landConfig.flags = fread_int(file);
      landConfig.treeSeed = fread_int64_t(file);
      landConfig.heightSeed = fread_int64_t(file);
    #endif
  } else {
    landConfig.flags = _landHills | _landWater | _landTrees;
    landConfig.treeSeed = 0;
    landConfig.heightSeed = 0;
  }

  nextLandConfig = landConfig;
  allocLand();

  for (int x = 0; x < landSize; x++) {
    for (int y = 0; y < landSize; y++) {
      readChunk(file, version, x, y);
    }
  }

  initMaps();
}

#include "renderLand.cpp"

