#include "renderLand.hpp"

#include "building/building.hpp"
#include "building/design.hpp"
#include "color.hpp"
#include "draw/buffer.hpp"
#include "draw/entity.hpp"
#include "draw/texture.hpp"
#include "game/game.hpp"
#include "graph.hpp"
#include "heatmap.hpp"
#include "icons.hpp"
#include "import/mesh-import.hpp"
#include "land.hpp"
#include "lot.hpp"
#include "pool.hpp"
#include "renderUtils.hpp"
#include "thread.hpp"
#include "time.hpp"
#include "util.hpp"

struct TileData {
  vec3 l;
  vec3 lw;
  vec3 n;
  vec3 x;
};

//Colors
static vec3 waterColorCurrent = colorWaterBlue;

//Tree render
const bool renderElevators = false; //LP_DEBUG;

// Render detail
const int simpleLandStride = 5;
const float landBottom = -4000;

const int ltrQueueSize = maxLandSize * maxLandSize + 100;
RenderChunkIndex ltrQueue[ltrQueueSize];
int ltrQueueRead = 0;
int ltrQueueWrite = 0;
const static auto ltrSleepTime = std::chrono::microseconds(20);
bool renderLoopRunning = false;
bool continueRenderLoop = false;
static bool treesVisible = true;
static bool waterVisible = true;
static bool undergroundView = false;
static vector<char> isOnlyWater;
static vector<char> isAnyWater;
static float zOffset = 0;
static Cup<TileData> tileDataCache;

static Pool<float*> zsPool;

void resetLandRender() {
  seemHiderEntity = 0;
  tileDataCache.clear();

  for (int i = 1; i <= zsPool.size(); i++) {
    float** zs = zsPool.get(i);
    free(*zs);
  }
  zsPool.clear();
}

void clearTileDataCache() {
  tileDataCache.clear();
}

bool getTreesVisible() {
  return treesVisible;
}

void setTreesVisible(bool visible) {
  treesVisible = visible;
  setUndergroundView(undergroundView);
}

item numRenderChunksQueued() {
  return (ltrQueueWrite - ltrQueueRead + ltrQueueSize) % ltrQueueSize;
}

void renderChunk(RenderChunkIndex ndx);
void renderLoop() {
  renderLoopRunning = true;
  while (continueRenderLoop) {
    if (ltrQueueRead != ltrQueueWrite) {
      renderChunk(ltrQueue[ltrQueueRead]);
      ltrQueueRead = (ltrQueueRead + 1) % ltrQueueSize;
    }
    //std::this_thread::sleep_for(ltrSleepTime);
  }
  renderLoopRunning = false;
}

void renderLandQueue() {
  if (renderLoopRunning) return;
  for (; ltrQueueRead != ltrQueueWrite;
      ltrQueueRead = (ltrQueueRead + 1) % ltrQueueSize) {
    renderChunk(ltrQueue[ltrQueueRead]);
  }
  renderSeemHider();
}

void renderLandStep() {
  if (ltrQueueRead != ltrQueueWrite) {
    renderChunk(ltrQueue[ltrQueueRead]);
    ltrQueueRead = (ltrQueueRead + 1) % ltrQueueSize;
  }
}

void queueRenderChunk(RenderChunkIndex ndx) {
  // Prevent duplicates
  for (int i = ltrQueueRead; i != ltrQueueWrite; i = (i + 1) % ltrQueueSize) {
    RenderChunkIndex ic = ltrQueue[i];
    if (ic.x == ndx.x && ic.y == ndx.y) {
      return;
    }
  }

  ltrQueue[ltrQueueWrite] = ndx;
  ltrQueueWrite = (ltrQueueWrite + 1) % ltrQueueSize;
}

void queueRenderChunk(ChunkIndex ndx) {
  queueRenderChunk(renderChunkIndex(ndx));
}

void startRenderLoop() {
  //continueRenderLoop = false;
  //while (renderLoopRunning) {
    //std::this_thread::sleep_for(ltrSleepTime);
  //}
  //continueRenderLoop = true;
  startThread("RENDER THREAD", renderLoop);
}

void stopRenderLoop() {
  continueRenderLoop = false;
  while (renderLoopRunning) {
    std::this_thread::sleep_for(ltrSleepTime);
  }
}

bool getAnyWaterForChunk(RenderChunkIndex ndx) {
  int n = ndx.x*getLandSize() + ndx.y;
  int minSize = n/8+1;
  if (isAnyWater.size() < minSize) {
    isAnyWater.resize(minSize);
  }
  return isAnyWater[n/8] & (1 << (n%8));
}

bool getOnlyWaterForChunk(RenderChunkIndex ndx) {
  int n = ndx.x*getLandSize() + ndx.y;
  int minSize = n/8+1;
  if (isOnlyWater.size() < minSize) {
    isOnlyWater.resize(minSize);
  }
  return isOnlyWater[n/8] & (1 << (n%8));
}

void setAnyWaterForChunk(RenderChunkIndex ndx, bool val) {
  int n = ndx.x*getLandSize() + ndx.y;
  int minSize = n/8+1;
  if (isAnyWater.size() < minSize) {
    isAnyWater.resize(minSize);
  }

  if (val) {
    isAnyWater[n/8] |= (1 << (n%8));
  } else {
    isAnyWater[n/8] &= ~(1 << (n%8));
  }
}

void setOnlyWaterForChunk(RenderChunkIndex ndx, bool val) {
  int n = ndx.x*getLandSize() + ndx.y;
  int minSize = n/8+1;
  if (isOnlyWater.size() < minSize) {
    isOnlyWater.resize(minSize);
  }

  if (val) {
    isOnlyWater[n/8] |= (1 << (n%8));
  } else {
    isOnlyWater[n/8] &= ~(1 << (n%8));
  }
}

void setLandVisible(RenderChunkIndex ndx) {
  if (renderChunks == 0) {
    //SPDLOG_ERROR("Tried to call setLandVisible for null chunk at index ({},{})", ndx.x, ndx.y);
    //logStacktrace();
    return;
  }
  RenderChunk* chunk = getRenderChunk(ndx);
  bool renderWater = //!undergroundView &&
    waterVisible && getAnyWaterForChunk(ndx);
  bool renderLandEnt = !undergroundView &&
    (!c(CHideUnderwater) || !waterVisible || !getOnlyWaterForChunk(ndx) ||
     getGameMode() == ModeBuildingDesigner);
  bool renderTreeEnt = treesVisible && renderLandEnt;
  setEntityVisible(chunk->waterEntity, renderWater);
  setEntityVisible(chunk->landEntity, renderLandEnt);
  if (chunk->dirtEntity != 0) {
    setEntityVisible(chunk->dirtEntity, renderLandEnt);
  }

  for (int i = 0; i < chunk->treeEntity.size(); i++) {
    setEntityVisible(chunk->treeEntity[i], renderTreeEnt);
  }
}

bool isUndergroundView() {
  return undergroundView;
}

void setUndergroundView(bool status) {
  undergroundView = status;
  setSkyVisible();
  setBuildingsVisible();
  setTunnelsVisible();
  setLotsVisible();
  setEntityVisible(seemHiderEntity,
      getHeatMap() != TransitHeatMap && getHeatMap() != RoadHeatMap);
  for (int y = 0; y < numRenderChunks; y ++) {
    for (int x = 0; x < numRenderChunks; x ++) {
      RenderChunkIndex ndx = renderChunkIndex(x, y);
      setLandVisible(ndx);
    }
  }
}

void setWaterVisible(bool status) {
  waterVisible = status;
  for (int y = 0; y < numRenderChunks; y ++) {
    for (int x = 0; x < numRenderChunks; x ++) {
      RenderChunkIndex ndx = renderChunkIndex(x, y);
      setLandVisible(ndx);
    }
  }
}

void renderSeemHider() {
  if (seemHiderEntity == 0) return;

  Entity* entity = getEntity(seemHiderEntity);
  entity->texture = paletteTexture;
  entity->flags |= _entityNoHeatmap;
  placeEntity(seemHiderEntity, vec3(0,0,0), 0, 0);
  setCull(seemHiderEntity, 1e12, 1e12);
  setEntityVisible(seemHiderEntity, true);

  Mesh* mesh = getMesh(entity->mesh);
  float z = -c(CMaxHeight) - 10;
  float mapSize = getMapSize();

  makeQuad(mesh, vec3(0,0,z),
      vec3(mapSize,0,z),
      vec3(0,mapSize,z),
      vec3(mapSize,mapSize,z),
      colorBrown, colorBrown);

  bufferMesh(entity->mesh);
}

bool isTileDense(RenderChunkIndex cNdx, item x, item y) {
  if (getGameMode() == ModeBuildingDesigner) {
    return getDesign(1)->minDensity >= c(CPavementDensity);
  } else {
    TileIndex ndx = normalize(tileIndex(cNdx, x, y));
    vec3 loc = getTileLocation(ndx);
    return heatMapGet(Density, loc) > c(CPavementDensity) &&
        loc.z > 0 && !hasTrees(ndx.x, ndx.y);
  }
}

float getTerrainColorU(float z, float nz) {
  // Color
  float u;
  if (z < -100) {
    u = 1.f;

  } else if (z < 0) {
    u = (z + 100)/100;
    u = clamp(u*2+1, 1.f, 3.f);

  } else if (z < beachLine) {
    u = (z - beachLine + 1) / beachLine;
    u = clamp(u*2+3, 3.f, 5.f);

  } else if (z > grassLine) {
    if (z > snowLine) {
      z *= nz;
      /*
      float nz = td.n.z;
      nz = 1-nz;
      nz = clamp(nz*3, 0, 1);
      //nz = 1-nz*nz;
      nz = 1-nz;
      */

      u = (z - snowLine) / 500;
      u = clamp(u, 0.f, 2.f);
      //u *= nz;
      u = clamp(u*2+9, 9.f, 11.f);
    } else {
      u = (z - grassLine) / (snowLine - grassLine);
      u = clamp(u*2+7, 7.f, 9.f);
    }

  } else if (z > dryGrassLine) {
    z *= nz;
    u = (z - dryGrassLine) / (grassLine - dryGrassLine);
    u = clamp(u*2+5, 5.f, 7.f);

  } else {
    u = 5;
  }

  return u;
}

TileData computeTileData(RenderChunkIndex ndx, item x, item y) {
  TileData td;

  // Location
  td.l = getTileLocation(ndx, x, y);
  td.lw = td.l;
  td.lw.z = 0;
  float z = getHeight(ndx, x, y);
  td.l.z = z;

  // Normal
  float z10 = getHeight(ndx, x+1, y);
  float z01 = getHeight(ndx, x,   y+1);
  float z90 = getHeight(ndx, x-1, y);
  float z09 = getHeight(ndx, x,   y-1);
  td.n = normalize(vec3(z90 - z10, z09 - z01, 20));

  // Adjust color for slope
  float nz = td.n.z;
  nz = clamp(pow(nz*2, 0.2), 0.25, 1.5);
  float u = getTerrainColorU(z, nz);
  td.x = getTerrainColor(u, getLandConfig().flags & _landDesert);

  return td;
}

TileData getTileData(RenderChunkIndex ndx, item x, item y) {
  TileData td;
  td.x.y = 0;
  int renderChunkSize = getRenderChunkSize();

  item x0 = ndx.x*renderChunkSize + x;
  item y0 = ndx.y*renderChunkSize + y;
  item cacheNdx = x0 + y0*(getMapTiles()+2) + 4;

  if (cacheNdx < 0) handleError("TileData less than zero");
  if (cacheNdx < tileDataCache.size()) {
    td = tileDataCache.at(cacheNdx);
  }

  if (td.x.y == 0) {
    td = computeTileData(ndx, x, y);
    tileDataCache.ensureSize(cacheNdx+1);
    tileDataCache.set(cacheNdx, td);
  }

  int chunkSize = getRenderChunkSize();
  float chunkRenderSize = chunkSize * tileSize;
  vec3 offset = vec3((ndx.x + 0.5f) * chunkRenderSize,
      (ndx.y + 0.5f) * chunkRenderSize, 0);
  td.l -= offset;
  td.lw = td.l;
  td.lw.z = 0;

  return td;
}

TileData getTileData(RenderChunkIndex ndx, item x, item y, float z) {
  TileData td = getTileData(ndx, x, y);
  td.l.z = z;

  // Adjust color for slope
  float nz = td.n.z;
  nz = clamp(pow(nz*2, 0.2), 0.25, 1.5);
  float u = getTerrainColorU(z, nz);
  td.x = getTerrainColor(u, getLandConfig().flags & _landDesert);

  return td;
}

void makeTileBox(RenderChunkIndex ndx, int xs, int xe, int ys, int ye,
    float tz00, float tz09, float tz90, float tz99, float zAdj,
    Mesh* mesh, vec3 xp) {
  if (!c(CShowLandTiling)) return;
  int chunkSize = getRenderChunkSize();
  float chunkRenderSize = chunkSize * tileSize;
  vec3 offset = vec3((ndx.x + 0.5f) * chunkRenderSize,
      (ndx.y + 0.5f) * chunkRenderSize, 0);
  zAdj -= zOffset;
  vec3 xAdjust = vec3(2, 0, 0);
  vec3 yAdjust = vec3(0, 2, 0);

  vec3 loc[4] = {
    getTileLocation(ndx, xs, ys) - offset + xAdjust + yAdjust,
    getTileLocation(ndx, xs, ye) - offset + xAdjust - yAdjust,
    getTileLocation(ndx, xe, ye) - offset - xAdjust - yAdjust,
    getTileLocation(ndx, xe, ys) - offset - xAdjust + yAdjust,
  };

  loc[0].z = tz00 + zAdj;
  loc[1].z = tz09 + zAdj;
  loc[2].z = tz99 + zAdj;
  loc[3].z = tz90 + zAdj;

  for (int i = 0; i < 4; i++) {
    vec3 start = loc[i];
    vec3 end = loc[(i+1)%4];
    vec3 along = start - end;
    vec3 right = -uzNormal(along)*1.f;
    makeAngledCube(mesh, end, right, along, vec3(0,0,2),
      true, xp);
  }
}

void renderTile(RenderChunkIndex ndx, int xs, int xe, int ys, int ye,
    float tz00, float tz09, float tz90, float tz99,
    Mesh* landMesh, Mesh* waterMesh, Mesh* dirtMesh) {

  //SPDLOG_INFO("renderTile {},{} {},{} {},{}",
   //   ndx.x, ndx.y, xs, ys, xe, ye);
  int chunkSize = getRenderChunkSize();
  if (xs < 0) xs = 0;
  if (ys < 0) ys = 0;
  if (xe > chunkSize) xe = chunkSize;
  if (ye > chunkSize) ye = chunkSize;
  if (xe-xs <= 0 || ye-ys <= 0) return;

  TileData td[4] = {
    getTileData(ndx, xs, ys, tz00),
    getTileData(ndx, xs, ye, tz09),
    getTileData(ndx, xe, ys, tz90),
    getTileData(ndx, xe, ye, tz99)
  };

  td[0].l.z -= zOffset;
  td[1].l.z -= zOffset;
  td[2].l.z -= zOffset;
  td[3].l.z -= zOffset;

  bool dense = isTileDense(ndx, xs, ys) && isTileDense(ndx, xs, ye) &&
    isTileDense(ndx, xe, ys) && isTileDense(ndx, xe, xe);

  //makeQuad(landMesh, td[1].l, td[0].l, td[3].l, td[2].l,
      //td[0].x, td[0].x);

  // Render the tile, either as pavement or not
  if (abs(td[0].l.z - td[3].l.z) > abs(td[2].l.z - td[1].l.z)) {
    if (dense) {
      makeQuad(landMesh, td[3].l, td[1].l, td[2].l, td[0].l,
          colorDensityPavement, colorDensityPavement);

    } else {
      makeQuad(landMesh, td[3].l, td[1].l, td[2].l, td[0].l,
          td[3].n, td[1].n, td[2].n, td[0].n,
          td[3].x, td[1].x, td[2].x, td[0].x);
    }

  } else {
    if (dense) {
      makeQuad(landMesh, td[1].l, td[0].l, td[3].l, td[2].l,
          colorDensityPavement, colorDensityPavement);

    } else {
      makeQuad(landMesh, td[1].l, td[0].l, td[3].l, td[2].l,
          td[1].n, td[0].n, td[3].n, td[2].n,
          td[1].x, td[0].x, td[3].x, td[2].x);
    }
  }

  for (int i = 0; i < 4; i++) {
    int v0 = 0;
    int v1 = 0;

    if (i == 0) {
      if (xs > 0) continue;
      if (ndx.x > 0) continue;
      v0 = 1;
      v1 = 0;

    } else if (i == 1) {
      if (xe + ndx.x*chunkSize < getMapTiles()) continue;
      v0 = 2;
      v1 = 3;

    } else if (i == 2) {
      if (ys > 0) continue;
      if (ndx.y > 0) continue;
      v0 = 0;
      v1 = 2;

    } else if (i == 3) {
      if (ye + ndx.y*chunkSize < getMapTiles()) continue;
      //if (ye < chunkSize) continue;
      //if (ndx.y < getLandSize()-1) continue;
      v0 = 3;
      v1 = 1;
    }

    vec3 l0 = td[v0].l;
    vec3 l1 = td[v1].l;
    vec3 l2 = l0;
    vec3 l3 = l1;
    l2.z = landBottom - zOffset;
    l3.z = landBottom - zOffset;
    if (dirtMesh != 0) {
      makeQuad(dirtMesh, l1, l0, l3, l2, colorBrown, colorBrown);
    }

    l0.z += zOffset;
    l1.z += zOffset;
    if (waterMesh != 0 && (l0.z < 0 || l1.z < 0)) {
      vec3 along = l1-l0;
      float lngth = length(along);
      int tiles = lngth/tileSize;
      if (waterColorCurrent.z < 0) tiles /= simpleLandStride;
      vec3 tAlong = along/float(tiles);
      for (int k = 0; k < tiles; k++) {
        vec3 lt0 = l0+tAlong*float(k);
        vec3 lt1 = lt0+tAlong;
        vec3 ltw0 = lt0;
        vec3 ltw1 = lt1;
        ltw0.z = 0;
        ltw1.z = 0;
        makeQuad(waterMesh, lt0, lt1, ltw0, ltw1,
            waterColorCurrent, waterColorCurrent);
      }
    }
  }
}

void renderWaterTile(RenderChunkIndex ndx, int x, int y, int stride,
    float uvz, Mesh* waterMesh) {
  int numWater = 0;
  int chunkSize = getRenderChunkSize();

  TileData td[4] = {
    getTileData(ndx, x, y),
    getTileData(ndx, x, y+stride),
    getTileData(ndx, x+stride, y),
    getTileData(ndx, x+stride, y+stride),
  };

  bool isWater[4] = {false, false, false, false};

  if (getGameMode() == ModeBuildingDesigner) {
    for (int i = 0; i < 4; i++) {
      isWater[i] = true;
    }
    setOnlyWaterForChunk(ndx, true);
    setAnyWaterForChunk(ndx, true);

  } else {
    for (int i = 0; i < 4; i++) {
      if (td[i].l.z > -beachLine) setOnlyWaterForChunk(ndx, false);
      if (td[i].l.z < 1) {
        isWater[i] = true;
        int ix = i%2 + (!(i/2))*2;
        int iy = (i/2)*2 + !(i%2);
        isWater[ix] = true;
        isWater[iy] = true;
      }
    }
  }

  for (int i = 0; i < 4; i++) {
    if (isWater[i]) numWater ++;
  }

  if (numWater > 2 && waterMesh != 0) {
    setAnyWaterForChunk(ndx, true);
    vec3 tx = colorWaterBlue;
    tx.z = uvz;
    if (numWater == 3) {
      vector<vec3> waters;
      for (int i = 0; i < 4; i++) {
        if (isWater[i]) waters.push_back(td[i].lw);
      }
      if (cross(waters[0]-waters[1], waters[0]-waters[2]).z < 0) {
        makeTriangle(waterMesh, waters[0], waters[2], waters[1], tx);
      } else {
        makeTriangle(waterMesh, waters[0], waters[1], waters[2], tx);
      }

    } else if ((x+y)%2 == 0) {
      makeQuad(waterMesh, td[3].lw, td[1].lw, td[2].lw, td[0].lw, tx, tx);
    } else {
      makeQuad(waterMesh, td[1].lw, td[0].lw, td[3].lw, td[2].lw, tx, tx);
    }
  }
}

struct TileDiff {
  int x, y;
  float diff;
};

TileDiff getMaxTileDiff(RenderChunkIndex ndx, int xs, int xe, int ys, int ye,
    float tz00, float tz09, float tz90, float tz99, int ee) {

  TileDiff result;
  result.x = xs;
  result.y = ys;
  result.diff = 0;

  for (int x = xs+ee; x <= xe-ee; x ++) {
    for (int y = ys+ee; y <= ye-ee; y ++) {
      float z = getHeight(ndx, x, y);
      float tz0 = mix(tz00, tz09, float(y-ys)/nonZero(ye-ys));
      float tz1 = mix(tz90, tz99, float(y-ys)/nonZero(ye-ys));
      float tz2 = mix(tz0, tz1, float(x-xs)/nonZero(xe-xs));
      float diff = z < tz2 ? c(CLandErrorConcaveFactor)*(tz2-z) : abs(tz2-z);
      /*
      if ((z < beachLine-1) != (tz2 < beachLine-1)) {
        diff *= c(CLandErrorConcaveFactor);
      }
      */

      float u = getTerrainColorU(z, 1);
      float tz2u = getTerrainColorU(tz2, 1);
      float uError = abs(u-tz2u);
      diff += c(CLandErrorColorFactor) * uError;

      if (diff > result.diff) {
        result.diff = diff;
        result.x = x;
        result.y = y;
      }
    }
  }

  return result;
}

void renderTileRecursive(RenderChunkIndex ndx, int xs, int xe, int ys, int ye,
    float tz00, float tz09, float tz90, float tz99,
    float maxDiff, Mesh* landMesh, Mesh* waterMesh, Mesh* dirtMesh);

void renderInnerTileRecursive(RenderChunkIndex ndx, int xs, int xe, int ys, int ye,
    float tz00, float tz09, float tz90, float tz99,
    float maxDiff, Mesh* landMesh, Mesh* waterMesh, Mesh* dirtMesh) {

  if (xs >= xe || ys >= ye) return;
  makeTileBox(ndx, xs, xe, ys, ye, tz00, tz09, tz90, tz99, 2,
      landMesh, colorWaterBlue);

  if (xe-xs > 1 || ye-ys > 1) {

    TileDiff td = getMaxTileDiff(ndx, xs, xe, ys, ye,
        tz00, tz09, tz90, tz99, 1);

    if (td.diff > maxDiff) {
      // Split the tile into four
      float tz55 = getHeight(ndx, td.x, td.y);
      float tz05 = mix(tz00, tz09, float(td.y-ys)/(ye-ys));
      float tz95 = mix(tz90, tz99, float(td.y-ys)/(ye-ys));
      float tz50 = mix(tz00, tz90, float(td.x-xs)/(xe-xs));
      float tz59 = mix(tz09, tz99, float(td.x-xs)/(xe-xs));

      renderTileRecursive(ndx, xs, td.x, ys, td.y,
          tz00, tz05, tz50, tz55,
          maxDiff, landMesh, waterMesh, dirtMesh);
      renderTileRecursive(ndx, xs, td.x, td.y, ye,
          tz05, tz09, tz55, tz59,
          maxDiff, landMesh, waterMesh, dirtMesh);
      renderTileRecursive(ndx, td.x, xe, ys, td.y,
          tz50, tz55, tz90, tz95,
          maxDiff, landMesh, waterMesh, dirtMesh);
      renderTileRecursive(ndx, td.x, xe, td.y, ye,
          tz55, tz59, tz95, tz99,
          maxDiff, landMesh, waterMesh, dirtMesh);
      return;
    }
  }

  // Make the tile
  renderTile(ndx, xs, xe, ys, ye,
      tz00, tz09, tz90, tz99,
      landMesh, waterMesh, dirtMesh);
}

vector<item> getBreakpoints(RenderChunkIndex ndx, int ks, int ke, bool isY, int j,
    float tz0, float tz9, float maxDiff) {

  if (ks == ke) return vector<item>();

  TileDiff td = isY ?
    getMaxTileDiff(ndx, j, j, ks, ke, tz0, tz9, tz0, tz9, 0) :
    getMaxTileDiff(ndx, ks, ke, j, j, tz0, tz0, tz9, tz9, 0);

  if (td.diff > maxDiff) {
    float tz5 = getHeight(ndx, td.x, td.y);
    vector<item> result0 = getBreakpoints(ndx, ks, isY ? td.y : td.x, isY,
        j, tz0, tz5, maxDiff);
    vector<item> result1 = getBreakpoints(ndx, isY ? td.y : td.x, ke, isY,
        j, tz5, tz9, maxDiff);
    int k = isY ? td.y : td.x;
    result0.push_back(k);
    result0.insert(result0.end(), result1.begin(), result1.end());
    return result0;
  }

  return vector<item>();
}

void renderTileRecursive(RenderChunkIndex ndx, int xs, int xe, int ys, int ye,
    float tz00, float tz09, float tz90, float tz99,
    float maxDiff, Mesh* landMesh, Mesh* waterMesh, Mesh* dirtMesh) {

  if (xs >= xe || ys >= ye) return;
  makeTileBox(ndx, xs, xe, ys, ye, tz00, tz09, tz90, tz99, 0,
      landMesh, colorRed);

  vector<item> bps[4] = {
    getBreakpoints(ndx, xs, xe, false, ys, tz00, tz90, maxDiff),
    getBreakpoints(ndx, xs, xe, false, ye, tz09, tz99, maxDiff),
    getBreakpoints(ndx, ys, ye, true, xs, tz00, tz09, maxDiff),
    getBreakpoints(ndx, ys, ye, true, xe, tz90, tz99, maxDiff),
  };

  vector<item> bpx = bps[0];
  bpx.insert(bpx.end(), bps[1].begin(), bps[1].end());
  bpx.push_back(xs);
  bpx.push_back(xe);
  sort(bpx.begin(), bpx.end());

  vector<item> bpy = bps[2];
  bpy.insert(bpy.end(), bps[3].begin(), bps[3].end());
  bpy.push_back(ys);
  bpy.push_back(ye);
  sort(bpy.begin(), bpy.end());

  if (bpx.size() > 2 || bpy.size() > 2) {
    int cs = getRenderChunkSize() + 1;
    item zsNdx = zsPool.create(false);
    float** zsRef = zsPool.get(zsNdx);
    float* zs = *zsRef;
    if (zs == 0) {
      zs = (float*) calloc(cs*cs, sizeof(float));
      *zsRef = zs;
    }

    for (int x = 0; x < bpx.size(); x++) {
      for (int y = 0; y < bpy.size(); y++) {
        item x0 = bpx[x];
        item y0 = bpy[y];
        zs[x0*cs + y0] = getHeight(ndx,x0,y0);
      }
    }

    zs[xs*cs + ys] = tz00;
    zs[xs*cs + ye] = tz09;
    zs[xe*cs + ys] = tz90;
    zs[xe*cs + ye] = tz99;

    for (int i = 0; i < 4; i++) {
      bool isY = i >= 2;
      bool isEnd = i%2;
      vector<item> a = bps[i];
      vector<item> b = bps[isY*2 + !isEnd];
      int aS = a.size();

      for (int k = 0; k < aS; k++) {
        int x = isY ? (isEnd ? xe : xs) : a[k];
        int y = isY ? a[k] : (isEnd ? ye : ys);
        zs[x*cs + y] = getHeight(ndx, x, y);
      }

      int an = -1;
      float tz0 = isEnd ? (isY ? tz90 : tz09) : tz00;
      float tz9 = tz0;
      int k0 = isY ? ys : xs;
      int k9 = k0; //aS > 0 ? a[0] : (isY ? ye : xe);
      for (int k = 0; k < b.size(); k++) {
        int x = isY ? (isEnd ? xe : xs) : b[k];
        int y = isY ? b[k] : (isEnd ? ye : ys);
        while (an < aS && (an < 0 || a[an] < b[k])) {
          an ++;
          k0 = k9;
          tz0 = tz9;
          if (an < aS) {
            k9 = a[an];
            int ax = isY ? x : k9;
            int ay = isY ? k9 : y;
            tz9 = getHeight(ndx, ax, ay);
          } else {
            k9 = isY ? ye : xe;
            tz9 = isEnd ? tz99 : (isY ? tz09 : tz90);
          }
        }
        zs[x*cs + y] = mix(tz0, tz9, (b[k]-k0)/nonZero(k9-k0));
      }
    }

    for (int x = 0; x < bpx.size()-1; x++) {
      for (int y = 0; y < bpy.size()-1; y++) {
        int xsa = bpx[x];
        int xea = bpx[x+1];
        int ysa = bpy[y];
        int yea = bpy[y+1];

        renderTileRecursive(ndx, xsa, xea, ysa, yea,
        //renderInnerTileRecursive(ndx, xsa, xea, ysa, yea,
        //renderTile(ndx, xsa, xea, ysa, yea,
          zs[xsa*cs + ysa],
          zs[xsa*cs + yea],
          zs[xea*cs + ysa],
          zs[xea*cs + yea],
          maxDiff, landMesh, waterMesh, dirtMesh);
          //landMesh, waterMesh, dirtMesh);
      }
    }

    zsPool.free(zsNdx);

  } else {
    renderInnerTileRecursive(ndx, xs, xe, ys, ye,
        tz00, tz09, tz90, tz99,
        maxDiff, landMesh, waterMesh, dirtMesh);
  }
}

void renderTileRecursive(RenderChunkIndex ndx, int xs, int xe, int ys, int ye,
    float maxDiff, Mesh* landMesh, Mesh* waterMesh, Mesh* dirtMesh) {

  renderTileRecursive(ndx, xs, xe, ys, ye,
    getHeight(ndx, xs, ys),
    getHeight(ndx, xs, ye),
    getHeight(ndx, xe, ys),
    getHeight(ndx, xe, ye),
    maxDiff, landMesh, waterMesh, dirtMesh);
}

void renderChunkInner(RenderChunkIndex ndx, int s, vec3 offset,
    Mesh* landMesh, Mesh* waterMesh, Mesh* dirtMesh) {
  bool renderWater;
  int chunkSize = getRenderChunkSize();
  int paddedChunkSize = chunkSize + s;
  float maxHeight = c(CMaxHeight);

  // Remember to dealloc! No early returns
  vec3** ns = new vec3*[paddedChunkSize+1];
  vec3** tx = new vec3*[paddedChunkSize+1];
  vec3** lc = new vec3*[paddedChunkSize+1];
  for (int i = 0; i < paddedChunkSize+1; i++) {
    ns[i] = new vec3[paddedChunkSize+1];
    tx[i] = new vec3[paddedChunkSize+1];
    lc[i] = new vec3[paddedChunkSize+1];
  }

  for(int x = 0; x <= paddedChunkSize; x ++) {
    for(int y = 0; y <= paddedChunkSize; y ++) {

      // Normal
      float z10 = getHeight(ndx, x+1, y);
      float z01 = getHeight(ndx, x,   y+1);
      float z90 = getHeight(ndx, x-1, y);
      float z09 = getHeight(ndx, x,   y-1);
      ns[x][y] = vec3(z90 - z10, z09 - z01, tileSize*2);
      float z = getHeight(ndx, x, y);
      //float adjZ = z - offset.z;
      //if (adjZ < beachLine && adjZ != 0) {
        //adjZ = z < 0 ? z : beachLine;
      //}
      vec3 loc = getTileLocation(ndx, x,y) - offset;

      loc.z = z - offset.z;
      lc[x][y] = loc;

      // Color
      float u;
      if (z < -100) {
        u = 1.f;

      } else if (z < 0) {
        u = (z + 100)/100;
        u = clamp(u*2+1, 1.f, 3.f);

      } else if (z < beachLine) {
        u = (z - beachLine + 1) / 1;
        u = clamp(u*2+3, 3.f, 5.f);
        renderWater = true;

      } else if (z > grassLine) {
        if (z > snowLine) {
          u = (z - snowLine) / 10;
          u = clamp(u*2+9, 9.f, 11.f);
        } else {
          u = (z - grassLine) / (snowLine - grassLine);
          u = clamp(u*2+7, 7.f, 9.f);
        }

      } else if (z > dryGrassLine) {
        u = (z - dryGrassLine) / (grassLine - dryGrassLine);
        u = clamp(u*2+5, 5.f, 7.f);

      } else {
        u = 5;
      }

      tx[x][y] = getTerrainColor(u, getLandConfig().flags & _landDesert);
    }
  }

  if (s > 1) {
    for(int x = 0; x <= paddedChunkSize; x ++) {
      for(int y = 0; y <= paddedChunkSize; y ++) {

        float minZ = lc[x][y].z;
        float totalZ = 0;
        int num = 0;
        for(int x1 = -2; x1 <= 2; x1 ++) {
          for(int y1 = -2; y1 <= 2; y1 ++) {
            int x2 = x+x1;
            int y2 = y+y1;
            //if (x2 < 0 || y2 < 0 ||
                //x2 >= paddedChunkSize || y2 >= paddedChunkSize) {
              //continue;
            //}

            //float z = lc[x2][y2].z;
            float z = getHeight(ndx, x2, y2);
            totalZ += z;
            num ++;
            if (z < minZ) minZ = z;
          }
        }

        float adjZ = minZ;
        //if (offset.z != 0) {
          //adjZ -= offset.z;
          if (adjZ < beachLine) {
            adjZ = totalZ/num;
          }
        //}
        lc[x][y].z = adjZ;
      }
    }
  }


  for(int x = 0; x < chunkSize; x += s) {
    for(int y = 0; y < chunkSize; y += s) {
      vec3 br = lc[x  ][y  ];
      vec3 tr = lc[x  ][y+s];
      vec3 bl = lc[x+s][y  ];
      vec3 tl = lc[x+s][y+s];

      bool dense = isTileDense(ndx, x, y) && isTileDense(ndx, x, y+s) &&
        isTileDense(ndx, x+s, y) && isTileDense(ndx, x+s, y+s);

      if (dense) {
        vec3 bru = br;
        vec3 blu = bl;
        vec3 tru = tr;
        vec3 tlu = tl;
        bru.z += c(CDensePavementZ);
        tru.z += c(CDensePavementZ);
        blu.z += c(CDensePavementZ);
        tlu.z += c(CDensePavementZ);

        if (!isTileDense(ndx, x-s, y) || !isTileDense(ndx, x-s, y+s)) {
          makeQuad(landMesh, bru, tru, br, tr,
              colorDensityPavement, colorDensityPavement);
        }

        if (!isTileDense(ndx, x, y-s) || !isTileDense(ndx, x+s, y-s)) {
          makeQuad(landMesh, blu, bru, bl, br,
              colorDensityPavement, colorDensityPavement);
        }

        if (!isTileDense(ndx, x+s*2, y) || !isTileDense(ndx, x+s*2, y+s)) {
          makeQuad(landMesh, tlu, blu, tl, bl,
              colorDensityPavement, colorDensityPavement);
        }

        if (!isTileDense(ndx, x, y+s*2) || !isTileDense(ndx, x+s, y+s*2)) {
          makeQuad(landMesh, tru, tlu, tr, tl,
              colorDensityPavement, colorDensityPavement);
        }

        br = bru;
        bl = blu;
        tr = tru;
        tl = tlu;
      }

      if (abs(br.z - tl.z) > abs(bl.z - tr.z)) {
        if (dense) {
          makeQuad(landMesh, tl, tr, bl, br,
              colorDensityPavement, colorDensityPavement);
        } else {
          makeQuad(landMesh, tl, tr, bl, br,
            ns[x+s][y+s], ns[x][y+s], ns[x+s][y], ns[x][y],
            tx[x+s][y+s], tx[x][y+s], tx[x+s][y], tx[x][y]);
        }

      } else {
        if (dense) {
          makeQuad(landMesh, tr, br, tl, bl,
              colorDensityPavement, colorDensityPavement);
        } else {
          makeQuad(landMesh, tr, br, tl, bl,
            ns[x][y+s], ns[x][y], ns[x+s][y+s], ns[x+s][y],
            tx[x][y+s], tx[x][y], tx[x+s][y+s], tx[x+s][y]);
        }
      }

      if (x == 0 && ndx.x == 0) {
        vec3 ubr = br;
        vec3 utr = tr;
        ubr.z = landBottom;
        utr.z = landBottom;
        if (dirtMesh != 0) {
          makeQuad(dirtMesh, br, tr, ubr, utr, colorBrown, colorBrown);
        }

        if (waterMesh != 0 && (br.z < 0 || tr.z < 0)) {
          makeQuad(waterMesh, tr, br,
            vec3(tr.x, tr.y, 0), vec3(br.x, br.y, 0),
            colorWaterBlue, colorWaterBlue);
        }
      }

      if (y == 0 && ndx.y == 0) {
        vec3 ubr = br;
        vec3 ubl = bl;
        ubr.z = landBottom;
        ubl.z = landBottom;
        if (dirtMesh != 0) {
          makeQuad(dirtMesh, bl, br, ubl, ubr, colorBrown, colorBrown);
        }
        if (waterMesh != 0 && (br.z < 0 || bl.z < 0)) {
          makeQuad(waterMesh, br, bl,
            vec3(br.x, br.y, 0), vec3(bl.x, bl.y, 0),
            colorWaterBlue, colorWaterBlue);
        }
      }

      if (x+s >= chunkSize && ndx.x == numRenderChunks - 1) {
        vec3 ubl = bl;
        vec3 utl = tl;
        ubl.z = landBottom;
        utl.z = landBottom;
        if (dirtMesh != 0) {
          makeQuad(dirtMesh, tl, bl, utl, ubl, colorBrown, colorBrown);
        }
        if (waterMesh != 0 && (bl.z < 0 || tl.z < 0)) {
          makeQuad(waterMesh, bl, tl,
            vec3(bl.x, bl.y, 0), vec3(tl.x, tl.y, 0),
            colorWaterBlue, colorWaterBlue);
        }
      }

      if (y+s >= chunkSize && ndx.y == numRenderChunks - 1) {
        vec3 utr = tr;
        vec3 utl = tl;
        utr.z = landBottom;
        utl.z = landBottom;
        if (dirtMesh != 0) {
          makeQuad(dirtMesh, tr, tl, utr, utl, colorBrown, colorBrown);
        }
        if (waterMesh != 0 && (bl.z < 0 || tl.z < 0)) {
          makeQuad(waterMesh, tl, tr,
            vec3(tl.x, tl.y, 0), vec3(tr.x, tr.y, 0),
            colorWaterBlue, colorWaterBlue);
        }
      }

      if (br.z > -beachLine || tr.z > -beachLine ||
          bl.z > -beachLine || tl.z > -beachLine) {
        setOnlyWaterForChunk(ndx, false);
      }

      if (waterMesh != 0 && (br.z < beachLine || tr.z < beachLine ||
          bl.z < beachLine || tl.z < beachLine)) {
        br.z = 0;
        tr.z = 0;
        bl.z = 0;
        tl.z = 0;
        makeQuad(waterMesh, tl, tr, bl, br, colorWaterBlue, colorWaterBlue);
        setAnyWaterForChunk(ndx, true);
      }

    }
  }

  // Deallocating ns[] tx[] lc[]
  for (int d = 0; d < paddedChunkSize; d++)
  {
    delete[] ns[d];
    delete[] tx[d];
    delete[] lc[d];
  }

  delete[] ns;
  delete[] tx;
  delete[] lc;
}

void makeUnderDirt(Mesh* mesh, RenderChunkIndex ndx, float zOffset) {
  if (mesh == 0) return;

  float chunkRenderSize = chunkSize * tileSize;
  vec3 loc = vec3(-chunkRenderSize*.5f, -chunkRenderSize*.5f,
      landBottom - zOffset);
  vec3 along = vec3(chunkRenderSize, 0, 0);
  vec3 right = vec3(0, chunkRenderSize, 0);

  //makeQuad(mesh, loc, loc+along, loc+right, loc+along+right,
      //colorBrown, colorBrown);
}

void renderTrees(RenderChunkIndex ndx, vec3 offset) {
  float mapSize = getMapSize();
  RenderChunk* chunk = getRenderChunk(ndx);
  int chunkSize = getRenderChunkSize();
  float chunkRenderSize = chunkSize * tileSize;
  float chunkCullSize = chunkRenderSize*2;
  bool treeSimpleOnly = c(CTreeSimpleDistance) <= 0;
  float stepSize = c(CMinTreeSpacing);
  float stepDev = c(CTreeSpacingDeviation);
  vector<Cup<vec4>*> treeData;

  for (int i = 0; i < sizeTreeTypes(); i++) {
    treeData.push_back(Cup<vec4>::newCup(0));
  }

  for(int x = 0; x < chunkSize; x ++) {
    for(int y = 0; y < chunkSize; y ++) {
      item x0 = ndx.x*chunkSize + x;
      item y0 = ndx.y*chunkSize + y;
      vec3 loc = getTileLocation(x0, y0);

      if (hasTrees(x0, y0)) {
        // Adjust color for slope
        TileData td = getTileData(ndx, x, y, 0);
        float nz = td.n.z;
        nz = clamp(pow(nz*2, 0.2), 0.25, 1.5);

        item randomSeed = x0*100000 + y0*100 + 19;
        int k = 0;
        vec3 tileOff = stepSize*vec3(randFloat(&randomSeed, -1, 0),
            randFloat(&randomSeed, -1, 0), 0);
        //vec3 tileOff = vec3(0,0,0);
        for (float xm = 0; xm <= tileSize; xm +=stepSize) {
          k ++;
          for (float ym = 0; ym <= tileSize; ym +=stepSize) {
            if (randFloat(&randomSeed) < c(CTreeInnerFrequency)) {
              vec3 treeLoc = loc + vec3(
                  xm + randFloat(&randomSeed, -stepDev, stepDev),
                  ym + randFloat(&randomSeed, -stepDev, stepDev)
                   + (k%2-.5f)*stepSize*.5f,
                  0);
              treeLoc += tileOff;
              treeLoc = pointOnLand(treeLoc);
              float rnd = randFloat(&randomSeed, 0, 1);
              float treeLineZ = treeLoc.z*nz + rnd*rnd*100;
              if (treeLoc.z < beachLine-1 ||
                  treeLineZ > treeLine) {
                continue;
              }
              if (treeLoc.x < 0 || treeLoc.x > mapSize ||
                  treeLoc.y < 0 || treeLoc.y > mapSize) {
                continue;
              }

              treeLoc.z -= 3;
              float size = (1 + .75f*randFloat(&randomSeed));
              item type = randomTreeType(randomSeed);
              treeData[type-1]->push_back(vec4(treeLoc - offset, size));
            }
          }
        }
      }
    }
  }

  for (int i = 0; i < sizeTreeTypes(); i++) {
    TreeType* treeType = getTreeType(i+1);
    item entityNdx = chunk->treeEntity[i];
    Entity* treeEntity = getEntity(entityNdx);
    if (treeType->flags & _treeWind) treeEntity->flags |= _entityWind;
    placeEntity(entityNdx, offset, 0, 0);
    setCull(entityNdx, chunkCullSize, c(CTreeCull));
    setEntityVisible(entityNdx, false);
    treeEntity->simpleDistance = c(CTreeSimpleDistance);

    MeshImport* import = getMeshImport(treeType->meshImport);
    if (!(import->flags & _meshImportComplete)) {
      loadMeshImport(treeType->meshImport);
    }

    if (!treeSimpleOnly) {
      Mesh* treeMesh = getMesh(treeEntity->mesh);
      insertMesh(treeMesh, import->mesh, vec3(0,0,0), 0, 0, vec3(1,1,1));

      if (import->flags & _meshImportBillboard) {
        treeMesh->flags |= _meshBillboard;
      }
    }

    Mesh* simpleTreeMesh = getMesh(treeEntity->simpleMesh);
    insertMesh(simpleTreeMesh, import->simpleMesh, vec3(0,0,0), 0, 0,
        vec3(1,1,1));
    if (import->flags & _meshImportSimpleBillboard) {
      simpleTreeMesh->flags |= _meshBillboard;
    }

    if (import->texture == 0) {
      treeEntity->texture = paletteTexture;
    } else {
      treeEntity->texture = import->texture;
    }

    bufferMeshForEntity(entityNdx);
    writeTreeData(treeData[i], entityNdx);
  }
}

void renderChunk(RenderChunkIndex ndx) {
  if (!c(CLandVisible)) return;
  if (!isRenderEnabled()) { return; }

  if (ndx.x < 0 || ndx.y < 0 ||
      ndx.x >= numRenderChunks || ndx.y >= numRenderChunks) {
    return;
  }
  RenderChunk* chunk = getRenderChunk(ndx);
  int chunkSize = getRenderChunkSize();
  float chunkRenderSize = chunkSize * tileSize;
  float chunkCullSize = chunkRenderSize*2;
  bool simpleOnly = c(CLandSimpleDistance) <= 0;
  bool waterSimpleOnly = c(CWaterSimpleDistance) <= 0;
  vec3 offset = vec3((ndx.x + 0.5f) * chunkRenderSize,
      (ndx.y + 0.5f) * chunkRenderSize, 0);

  float zOff = 0;
  for(int x = 0; x < chunkSize; x ++) {
    for(int y = 0; y < chunkSize; y ++) {
      float height = getHeight(ndx.x*chunkSize + x, ndx.y*chunkSize + y);
      if (validate(height)) {
        zOff += height;
      } else {
        SPDLOG_INFO("height:{} at ({}-{},{}-{})",
            height, ndx.x, x, ndx.y, y);
      }
    }
  }
  zOffset = zOff/(chunkSize*chunkSize);
  offset.z = zOffset;
  if (!validate(zOffset)) {
    SPDLOG_INFO("zOffset:{} zOff:{} chunkSize:{}",
        zOffset, zOff, chunkSize);
  }

  vec3 placement = offset;
  setOnlyWaterForChunk(ndx, true);
  setAnyWaterForChunk(ndx, false);

  Entity* landEntity = getEntity(chunk->landEntity);
  landEntity->texture = landTexture;
  placeEntity(chunk->landEntity, placement, 0, 0);
  setCull(chunk->landEntity, chunkCullSize, c(CLandCull));
  landEntity->simpleDistance = c(CLandSimpleDistance);
  setEntityVisible(chunk->landEntity, false);

  Entity* dirtEntity = 0;
  if (chunk->dirtEntity != 0) {
    dirtEntity = getEntity(chunk->dirtEntity);
    dirtEntity->texture = paletteTexture;
    dirtEntity->flags |= _entityNoHeatmap;
    placeEntity(chunk->dirtEntity, placement, 0, 0);
    setCull(chunk->dirtEntity, chunkCullSize, c(CLandCull));
    dirtEntity->simpleDistance = c(CLandSimpleDistance);
    setEntityVisible(chunk->dirtEntity, false);
  }

  Entity* waterEntity = getEntity(chunk->waterEntity);
  waterEntity->texture = paletteTexture;
  vec3 waterPlacement = placement;
  waterPlacement.z = 0;
  placeEntity(chunk->waterEntity, waterPlacement, 0, 0);
  setCull(chunk->waterEntity, chunkCullSize, c(CLandCull));
  setEntityVisible(chunk->waterEntity, false);
  waterEntity->simpleDistance = c(CWaterSimpleDistance);

  if (!simpleOnly) {
    createMeshForEntity(chunk->landEntity);
    if (chunk->dirtEntity != 0) {
      createMeshForEntity(chunk->dirtEntity);
    }
  }

  if (!waterSimpleOnly) {
    createMeshForEntity(chunk->waterEntity);
  }

  createSimpleMeshForEntity(chunk->landEntity);
  if (chunk->dirtEntity != 0) {
    createSimpleMeshForEntity(chunk->dirtEntity);
  }
  createSimpleMeshForEntity(chunk->waterEntity);

  Mesh* landMesh = 0;
  Mesh* dirtMesh = 0;
  Mesh* waterMesh = 0;

  if (!simpleOnly) {
    landMesh = getMesh(landEntity->mesh);
    landMesh->flags |= _meshDynDraw;
    if (dirtEntity != 0) {
      dirtMesh = getMesh(dirtEntity->mesh);
      dirtMesh->flags |= _meshDynDraw;
    }
  }

  if (!waterSimpleOnly) {
    waterMesh = getMesh(waterEntity->mesh);
    waterMesh->flags |= _meshDynDraw;
  }

  Mesh* simpleLandMesh = getMesh(landEntity->simpleMesh);
  Mesh* simpleDirtMesh = dirtEntity == 0 ? 0 : getMesh(dirtEntity->simpleMesh);
  Mesh* simpleWaterMesh = getMesh(waterEntity->simpleMesh);

  simpleLandMesh->flags |= _meshDynDraw;
  if (simpleDirtMesh != 0) {
    simpleDirtMesh->flags |= _meshDynDraw;
  }
  //simpleWaterMesh->flags |= _meshDynDraw;

  if (c(CUseOldTerrainRendering)) {
    renderChunkInner(ndx, simpleLandStride, offset - vec3(0,0,-1.9),
        simpleLandMesh, waterMesh, simpleDirtMesh);
    renderChunkInner(ndx, 1, offset,
        landMesh, 0, dirtMesh);

  } else {
    waterColorCurrent = colorWaterBlue;
    if (!simpleOnly) {
      renderTileRecursive(ndx, 0, chunkSize, 0, chunkSize,
          c(CMaxLandErrorClose), landMesh, waterMesh, dirtMesh);
    }
    waterColorCurrent.z = 0.4;
    renderTileRecursive(ndx, 0, chunkSize, 0, chunkSize,
        c(CMaxLandErrorFar), simpleLandMesh, simpleWaterMesh, simpleDirtMesh);

    makeUnderDirt(dirtMesh, ndx, zOffset);
    makeUnderDirt(simpleDirtMesh, ndx, zOffset);

    for (int i = waterSimpleOnly; i < 2; i++) {
      Mesh* m = i == 0 ? waterMesh : simpleWaterMesh;
      int stride = i == 0 ? 1 : simpleLandStride;
      for(int x = 0; x < chunkSize; x += stride) {
        for(int y = 0; y < chunkSize; y += stride) {
          renderWaterTile(ndx, x, y, stride, i ? 0.4 : 1, m);
        }
      }
    }
  }

  bufferMeshForEntity(chunk->landEntity);
  bufferMeshForEntity(chunk->waterEntity);
  if (chunk->dirtEntity != 0) {
    bufferMeshForEntity(chunk->dirtEntity);
  }

  renderTrees(ndx, offset);

  setEntityVisible(chunk->dirtEntity, true);
  setLandVisible(ndx);

  srand(getCurrentDateTime()*100000);
}

