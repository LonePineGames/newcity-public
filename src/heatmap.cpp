#include "heatmap.hpp"

#include "amenity.hpp"
#include "building/building.hpp"
#include "draw/camera.hpp"
#include "draw/shader.hpp"
#include "game/game.hpp"
#include "graph/transit.hpp"
#include "economy.hpp"
#include "error.hpp"
#include "land.hpp"
#include "lot.hpp"
#include "option.hpp"
#include "person.hpp"
#include "string_proxy.hpp"
#include "tutorial.hpp"
#include "thread.hpp"
#include "util.hpp"
#include "weather.hpp"

#include "parts/toolbar.hpp"
#include "parts/leftPanel.hpp"

#include "spdlog/spdlog.h"

enum HeatMapBufferIndex {
  HeatMapReadFront, HeatMapReadBack, HeatMapWrite,
  HeatMapAddFront, HeatMapAddBack,
  numHeatMapBuffers
};

float heatMapInitValue[numHeatMaps];
float heatMapEffect[numHeatMaps];
float dissipationRate[numHeatMaps];

static const int blurSize = 2;
static const float blurFactor = 1.0/(blurSize*2+1);
static const float blurStart = blurFactor*blurSize;

static HeatMapTile* heatMaps[numHeatMaps][numHeatMapBuffers];
static HeatMapTile* blurBuffer;
static HeatMapTile* spareBuffer;
static float heatMapTotalValue[numHeatMaps] = {0};
static bool heatMapUpdated[numHeatMaps] = {true};
static bool shouldContinueHeatMaps = true;
static atomic<bool> hmDrawn(false);
static bool resetHMDraw;
static HeatMapIndex currentHeatMap = Pollution;
static HeatMapIndex nextHeatMap = Pollution;

static mutex heatMutex;
static mutex heatSwapMutex;
static condition_variable heatSwapCondition;
static bool heatMapsReady = false;
static GLuint heatMapTexture;
static int hmUpdateNum = 0;
static bool hmIntense = false;
static bool nextHMIntense = false;
static float durationRemaining;
static float nextDurationRemaining;
static vec3 wind;
static int popForHM;
static float hmAdjustment[numHeatMaps] = {1};
static float nextHmAdjustment[numHeatMaps] = {1};
static int stride = 5;
static int mapSize;

HeatMapTile* getHeatMapValue(HeatMapIndex heatMap, HeatMapBufferIndex buffer,
    const int ix, const int iy) {
  const int maxT = mapSize - 1;
  const int x = ix < 0 ? 0 : ix > maxT ? maxT : ix;
  const int y = iy < 0 ? 0 : iy > maxT ? maxT : iy;
  return &heatMaps[heatMap][buffer][x+y*mapSize];
}

HeatMapTile* getHeatMapValue(HeatMapIndex heatMap, HeatMapBufferIndex buffer,
    vec3 loc) {
  return getHeatMapValue(heatMap, buffer,
      loc.x/tileSize/stride, loc.y/tileSize/stride);
}

HeatMapTile interpolateHeatMapValue(HeatMapIndex heatMap,
    HeatMapBufferIndex buffer, vec3 loc) {

  HeatMapTile result = 0;
  float x = loc.x/tileSize/stride;
  float y = loc.y/tileSize/stride;

  for (int i = 0; i < 4; i++) {
    int ix = int(x) + i%2;
    int iy = int(y) + i/2;
    float fade = (1 - abs(x-ix)) * (1 - abs(y-iy));
    HeatMapTile sample = *getHeatMapValue(heatMap, buffer, ix, iy);
    result += sample*fade;
  }

  return result;
}

void heatMapAdd(HeatMapIndex ndx, vec3 loc, float amount) {
  if (amount != amount) {
    if (debugMode()) handleError("heatMapAdd(%d, ..., %f)", ndx, amount);
    return;
  }

  if (isPresimulating()) return;
  if (loc.x == 0 && loc.y == 0) return;

  // Exclude neighboring cities
  float mapDim = getMapSize();
  float offX = loc.x < 0 ? -loc.x : loc.x > mapDim ? loc.x - mapDim : 0;
  float offY = loc.y < 0 ? -loc.y : loc.y > mapDim ? loc.y - mapDim : 0;
  float off = std::max(offX, offY);
  if (off > c(CCityDistance)*3) return;
  if (off > 0) {
    //if (ndx == Crime) {
      //amount *= 0.2;
    //} 
    if (ndx == Pollution) {
      amount *= 0.5;
    } else if (ndx == Density || ndx == HealthHM || ndx == Crime) {
      return;
    }
  }

  //float mapBuffer = c(CCityDistance)*3;
  //float mapDim = getMapSize() + mapBuffer;
  //if (loc.x < -mapBuffer || loc.y < -mapBuffer) return;
  //if (loc.x > mapDim || loc.y > mapDim) return;

  *getHeatMapValue(ndx, HeatMapAddFront, loc) +=
    amount * heatMapEffect[ndx] * hmAdjustment[ndx];
}

float heatMapGet(HeatMapIndex ndx, vec3 loc) {
  //float result = *getHeatMapValue(ndx, HeatMapReadFront, loc);
  float result = interpolateHeatMapValue(ndx, HeatMapReadFront, loc);
  return pow(abs(result),0.5);
}

float heatMapTotal(HeatMapIndex ndx) {
  return heatMapTotalValue[ndx];
}

void checkStride() {
  if(stride <= 0 || stride >= 1000)
  {
    SPDLOG_ERROR("Caught invalid stride value ({}), defaulting to 5", stride);
    stride = 5;
  }
}

void swapHeatMaps() {
  if (heatSwapMutex.try_lock()) {
    durationRemaining += nextDurationRemaining;
    nextDurationRemaining = 0;
    if (durationRemaining > 240/numHeatMaps) {
      SPDLOG_WARN("heatMaps are too slow!");
      durationRemaining = 1;
    }

    if (nextHeatMap != currentHeatMap) {
      hmDrawn = false;
      currentHeatMap = nextHeatMap;
    }
    hmIntense = nextHMIntense;
    wind = getWeather().wind;
    popForHM = numPeople(ourCityEconNdx());

    for (int i = 0; i < numHeatMaps; i++) {
      nextHmAdjustment[i] = getHeatMapAdjustment((HeatMapIndex)i);
      if (heatMapUpdated[i]) {
        HeatMapTile* swap = heatMaps[i][HeatMapReadBack];
        heatMaps[i][HeatMapReadBack] = heatMaps[i][HeatMapReadFront];
        heatMaps[i][HeatMapReadFront] = swap;

        swap = heatMaps[i][HeatMapAddFront];
        heatMaps[i][HeatMapAddFront] = heatMaps[i][HeatMapAddBack];
        heatMaps[i][HeatMapAddBack] = swap;

        if (i == currentHeatMap) {
          hmDrawn = false;
        }
        heatMapUpdated[i] = false;
      }
    }

    heatSwapMutex.unlock();
    heatSwapCondition.notify_one();
  }
}

void killHeatMaps() {
  heatSwapMutex.lock();
  shouldContinueHeatMaps = false;
  heatSwapMutex.unlock();
  heatSwapCondition.notify_one();
}

void resetHeatMaps() {
  heatSwapMutex.lock();

  currentHeatMap = nextHeatMap = Pollution;
  hmIntense = nextHMIntense = false;
  durationRemaining = nextDurationRemaining = 0;
  resetHMDraw = true;
  hmDrawn = false;
  heatMapsReady = false;
  popForHM = 0;
  stride = 5;

  for (int i = 0; i < numHeatMaps; i++) {
    heatMapUpdated[i] = false;
    for (int j = 0; j < numHeatMapBuffers; j++) {
      if (heatMaps[i][j] != 0) {
        free(heatMaps[i][j]);
        heatMaps[i][j] = 0;
      }
    }
  }

  if (blurBuffer != 0) {
    free(blurBuffer);
    blurBuffer = 0;
  }
  if (spareBuffer != 0) {
    free(spareBuffer);
    spareBuffer = 0;
  }
  heatSwapMutex.unlock();
}

void allocHeatMaps() {
  checkStride();

  mapSize = getMapTiles()/stride;
  const int numTiles = mapSize*mapSize;

  for (int i = 0; i < numHeatMaps; i++) {
    for (int j = 0; j < numHeatMapBuffers; j++) {
      heatMaps[i][j] = (HeatMapTile*)
        malloc(numTiles*sizeof(HeatMapTile));
    }
  }

  blurBuffer = (HeatMapTile*) malloc(numTiles*sizeof(HeatMapTile));
  spareBuffer = (HeatMapTile*) malloc(numTiles*sizeof(HeatMapTile));

  heatMapInitValue[Pollution] = pow(c(CPollutionInit),2);
  heatMapInitValue[Value] = pow(c(CValueInit),2);
  heatMapInitValue[Density] = pow(c(CDensityInit),2);
  heatMapInitValue[Crime] = pow(c(CCrimeInit),2);
  heatMapInitValue[Education] = pow(c(CEducationInit),2);
  heatMapInitValue[Prosperity] = pow(c(CProsperityInit),2);
  heatMapInitValue[CommunityHM] = pow(c(CCommunityInit), 2);
  heatMapInitValue[HealthHM] = pow(c(CHealthInit), 2);
  // SPDLOG_WARN("health init {} {}", heatMapInitValue[HealthHM]); // Will throw and crash on MSVC!

  heatMapEffect[Pollution] = c(CPollutionEffect);
  heatMapEffect[Value] = c(CValueEffect);
  heatMapEffect[Density] = c(CDensityEffect);
  heatMapEffect[Crime] = c(CCrimeEffect);
  heatMapEffect[Education] = c(CEducationEffect);
  heatMapEffect[Prosperity] = c(CProsperityEffect);
  heatMapEffect[CommunityHM] = c(CCommunityEffect);
  heatMapEffect[HealthHM] = c(CHealthEffect);

  dissipationRate[Pollution] = c(CPollutionDissipationRate);
  dissipationRate[Value] = c(CValueDissipationRate);
  dissipationRate[Density] = c(CDensityDissipationRate);
  dissipationRate[Crime] = c(CCrimeDissipationRate);
  dissipationRate[Education] = c(CEducationDissipationRate);
  dissipationRate[Prosperity] = c(CProsperityDissipationRate);
  dissipationRate[CommunityHM] = c(CCommunityDissipationRate);
  dissipationRate[HealthHM] = c(CHealthDissipationRate);
}

void fillHeatMap(HeatMapTile* map, int numTiles, HeatMapTile value) {
  for (int i = 0; i < numTiles; i++) {
    map[i] = value;
  }
}

void initHeatMaps() {
  heatSwapMutex.lock();

  allocHeatMaps();
  const int numTiles = mapSize*mapSize;

  for (int i = 0; i < numHeatMaps; i++) {
    float init = heatMapInitValue[i];
    fillHeatMap(heatMaps[i][HeatMapReadBack], numTiles, init);
    fillHeatMap(heatMaps[i][HeatMapReadFront], numTiles, init);
    fillHeatMap(heatMaps[i][HeatMapWrite], numTiles, init);
    fillHeatMap(heatMaps[i][HeatMapAddFront], numTiles, 0);
    fillHeatMap(heatMaps[i][HeatMapAddBack], numTiles, 0);
    heatMapTotalValue[i] = pow(init, 0.5f);
  }

  heatMapsReady = true;
  heatSwapMutex.unlock();
}

void applyWindToPollution() {
  const int maxT = mapSize - 1;
  const int numTiles = mapSize*mapSize;
  HeatMapTile* write = heatMaps[Pollution][HeatMapWrite];
  const float alpha = 0.02; //1.0 * heatMapEffect[Pollution];
  const float beta = 1 - alpha;
  const float init = heatMapInitValue[Pollution];

  int xOff = wind.x * 3;
  int yOff = wind.y * 3;
  if (xOff == 0 && yOff == 0) {
    return;
  }

  int xStart = xOff < 0 ? -xOff : 0;
  int yStart = yOff < 0 ? -yOff : 0;
  int xEnd = xOff > 0 ? maxT - xOff : maxT;
  int yEnd = yOff > 0 ? maxT - yOff : maxT;
  int offset = xOff + yOff*mapSize;

  for (int y0 = 0; y0 < mapSize; y0++) {
    int yStride = y0*mapSize;
    bool outsideY = y0 < yStart || y0 > yEnd;
    for (int x0 = 0; x0 < mapSize; x0++) {
      int loc = x0 + yStride;
      HeatMapTile from = outsideY || x0 < xStart || x0 > xEnd ?
        init : write[loc + offset];
      blurBuffer[loc] = alpha * from + beta * write[loc];
    }
  }

  memcpy_s(write, blurBuffer, numTiles*sizeof(HeatMapTile));
}

void applyElevationToLandValue(float strength) {
  HeatMapTile* write = heatMaps[Value][HeatMapWrite];
  const float* naturalHeightMap = getNaturalHeightMap();
  const float coastHeight = c(CValueCoastEnd);
  const float coastDiffInv = 1.0 / coastHeight;
  const float coastTarget = pow(c(CValueCoastTarget),2);
  const float hillStart = c(CValueHillStart);
  const float hillEnd = c(CValueHillEnd);
  const float hillDiff = hillEnd - hillStart;
  const float hillDiffInv = 1.0 / hillDiff;
  const bool isSuperflat = getLandConfig().flags & _landSuperflat;
  const float hillTarget = pow(c(CValueHillTarget), 2);
  const item tiles = getMapTiles();

  for (int y0 = 0; y0 < mapSize; y0++) {
    const int yStride = y0*mapSize;
    const int yMap = y0*stride*tiles;
    for (int x0 = 0; x0 < mapSize; x0++) {
      const int xMap = x0*stride;
      const int loc = x0 + yStride;
      const float height = naturalHeightMap[xMap + yMap];
      const float target = isSuperflat ? c(CValueSuperflat) :
        (height < hillStart ? coastTarget : hillTarget);
      const float tStrength = isSuperflat ? strength :
        strength * (
        height < 0           ? 1                                     :
        height > hillEnd     ? 1                                     :
        height < coastHeight ? (coastHeight - height) * coastDiffInv :
        height > hillStart   ? (height - hillStart)   * hillDiffInv  :
                               0                                    );

      const float residue = 1 - tStrength;
      //write[loc] = 1.0;
      write[loc] = write[loc]*residue + tStrength*target;
    }
  }
}

void applyElevationToLandValue() {
  float strength = c(CHeatMapTime) * c(CValueTerrainEffect) *
    heatMapEffect[Value];
  applyElevationToLandValue(strength);
}

void initLandValue() {
  applyElevationToLandValue(1);
}

void applyElevationToPollution() {
  HeatMapTile* write = heatMaps[Pollution][HeatMapWrite];
  const float* naturalHeightMap = getNaturalHeightMap();
  const float strength = c(CHeatMapTime) * heatMapEffect[Pollution];
  const float coastHeight = c(CPollutionCoastEnd);
  const float coastEffectMax = c(CPollutionCoastEffect) * strength;
  const float coastEffect = coastEffectMax / coastHeight;
  const float hillStart = c(CPollutionHillStart);
  const float hillEnd = c(CPollutionHillEnd);
  const float hillDiff = hillEnd - hillStart;
  const float hillEffectMax = c(CPollutionHillEffect) * strength;
  const float hillEffect = hillEffectMax / hillDiff;
  const item tiles = getMapTiles();

  for (int y0 = 0; y0 < mapSize; y0++) {
    const int yStride = y0*mapSize;
    const int yMap = y0*stride*tiles;
    for (int x0 = 0; x0 < mapSize; x0++) {
      const int xMap = x0*stride;
      const int loc = x0 + yStride;
      const float height = naturalHeightMap[xMap + yMap];
      const float effect =
        height < 0           ? coastEffectMax                       :
        height > hillEnd     ? hillEffectMax                        :
        height < coastHeight ? (coastHeight - height) * coastEffect :
        height > hillStart   ? (height - hillStart)   * hillEffect  :
                               0                                    ;
      write[loc] += effect;
    }
  }
}

void capDensity() {
  float popRatio = popForHM / c(CPopForMaxDensity);
  popRatio = clamp(popRatio, 0.f, 1.f);
  const float globalCap = mix(0.1f, 1.f, pow(popRatio,0.5));
  HeatMapTile* densi = heatMaps[Density][HeatMapWrite];
  HeatMapTile* prosp = heatMaps[Prosperity][HeatMapWrite];
  HeatMapTile* crime = heatMaps[Crime][HeatMapWrite];
  HeatMapTile* pollu = heatMaps[Pollution][HeatMapWrite];
  HeatMapTile* educa = heatMaps[Education][HeatMapWrite];
  HeatMapTile* commu = heatMaps[CommunityHM][HeatMapWrite];
  HeatMapTile* healt = heatMaps[HealthHM][HeatMapWrite];
  const float* naturalHeightMap = getNaturalHeightMap();
  const item tiles = getMapTiles();

  const float coastHeight = c(CDensityCapCoastEnd);
  const float coastEffectMax = c(CDensityCapCoastEffect);
  const float coastEffect = coastEffectMax / coastHeight;
  const float hillStart = c(CDensityCapHillStart);
  const float hillEnd = c(CDensityCapHillEnd);
  const float hillDiff = hillEnd - hillStart;
  const float hillEffectMax = c(CDensityCapHillEffect);
  const float hillEffect = hillEffectMax / hillDiff;

  for (int y0 = 0; y0 < mapSize; y0++) {
    const int yStride = y0*mapSize;
    const int yMap = y0*stride*tiles;
    for (int x0 = 0; x0 < mapSize; x0++) {
      const int loc = x0 + yStride;

      const float densiT = densi[loc];
      const float prospT = prosp[loc];
      const float crimeT = crime[loc];
      const float polluT = pollu[loc];
      const float educaT = educa[loc];
      const float commuT = commu[loc];
      const float healtT = healt[loc];

      const int xMap = x0*stride;
      const float height = naturalHeightMap[xMap + yMap];
      const float terrainT =
        height < 0           ? coastEffectMax                       :
        height > hillEnd     ? hillEffectMax                        :
        height < coastHeight ? (coastHeight - height) * coastEffect :
        height > hillStart   ? (height - hillStart)   * hillEffect  :
                               0                                    ;

      float cap = c(CDensityCapBase)
        + prospT * c(CDensityCapProsperity)
        + educaT * c(CDensityCapEducation)
        + crimeT * c(CDensityCapCrime)
        + polluT * c(CDensityCapPollution)
        + commuT * c(CDensityCapCommunity)
        + healtT * c(CDensityCapHealth)
        + terrainT;

      cap = cap > densiT ? densiT : cap;
      cap = cap > globalCap ? globalCap : cap;
      //cap = cap < 0.0 ? 0.0 : cap;
      //cap = cap > 1.0 ? 1.0 : cap;
      densi[loc] = cap;
    }
  }
}

void capValue() {
  HeatMapTile* value = heatMaps[Value][HeatMapWrite];
  HeatMapTile* prosp = heatMaps[Prosperity][HeatMapWrite];
  HeatMapTile* crime = heatMaps[Crime][HeatMapWrite];
  HeatMapTile* pollu = heatMaps[Pollution][HeatMapWrite];
  HeatMapTile* educa = heatMaps[Education][HeatMapWrite];
  HeatMapTile* densi = heatMaps[Density][HeatMapWrite];
  HeatMapTile* commu = heatMaps[CommunityHM][HeatMapWrite];
  HeatMapTile* healt = heatMaps[HealthHM][HeatMapWrite];
  float prospInit = heatMapInitValue[Prosperity];

  for (int y0 = 0; y0 < mapSize; y0++) {
    const int yStride = y0*mapSize;
    for (int x0 = 0; x0 < mapSize; x0++) {
      const int loc = x0 + yStride;

      const float valueT = value[loc];
      const float prospT = prosp[loc] - prospInit;
      const float crimeT = crime[loc];
      const float polluT = pollu[loc];
      const float educaT = educa[loc];
      const float densiT = densi[loc];
      const float commuT = commu[loc];
      const float healtT = healt[loc];

      float cap = c(CValueCapBase)
        + prospT * c(CValueCapProsperity)
        + educaT * c(CValueCapEducation)
        + crimeT * c(CValueCapCrime)
        + polluT * c(CValueCapPollution)
        + densiT * c(CValueCapDensity)
        + commuT * c(CValueCapCommunity)
        + healtT * c(CValueCapHealth);

      cap = cap > valueT ? valueT : cap;
      //cap = cap < 0.0 ? 0.0 : cap;
      //cap = cap > 1.0 ? 1.0 : cap;
      value[loc] = cap;
    }
  }
}

void influenceHeatMap(HeatMapIndex writeNdx, HeatMapIndex readNdx,
    float effect) {
  if (effect == 0) return;
  const int numTiles = mapSize*mapSize;
  const HeatMapTile* read = heatMaps[readNdx][HeatMapWrite];
  HeatMapTile* write = heatMaps[writeNdx][HeatMapWrite];
  const float alpha = effect * heatMapEffect[writeNdx] *
    hmAdjustment[writeNdx];

  for (int i = 0; i < numTiles; i++) {
    write[i] += read[i]*alpha;
  }
}

void updateHeatMaps(double duration) {
  nextDurationRemaining += duration;
}

void computeHeatMapTotal(HeatMapIndex ndx, HeatMapBufferIndex buffer) {
  const int numTiles = mapSize*mapSize;
  HeatMapTile* read = heatMaps[ndx][buffer];
  float init = heatMapInitValue[ndx];
  float total = 0;
  int count = 0;
  for (int i = 0; i < numTiles; i++) {
    float val = read[i];
    if (abs(val - init) > 0.01) {
      total += val;
      count++;
    }
  }
  //const float standardDeviation =
    //clamp(pow(deviation / (numTiles-1), .5f), 0.001f, 1.0f);
  const float mean = total / count;
  total = clamp(mean, 0.f, 1.f);
  heatMapTotalValue[ndx] = total;
}

void updateHeatMaps() {
  heatMutex.lock();

  const int numTiles = mapSize*mapSize;
  const int mapT = mapSize-1;
  const HeatMapTile init = heatMapInitValue[hmUpdateNum];
  const HeatMapTile dissRate = dissipationRate[hmUpdateNum] * c(CHeatMapTime);
  const HeatMapTile dissipation = 1 - dissRate;
  const HeatMapTile initDissipated = init * dissRate;
  HeatMapTile* read = heatMaps[hmUpdateNum][HeatMapReadBack];
  HeatMapTile* write = heatMaps[hmUpdateNum][HeatMapWrite];
  HeatMapTile* add = heatMaps[hmUpdateNum][HeatMapAddBack];

  if (write == 0) return;

  if (hmUpdateNum == Density) {
    memcpy_s(spareBuffer, write, numTiles*sizeof(HeatMapTile));
  }

  // Apply add buffer, then erase add buffer
  for (int i = 0; i < numTiles; i++) {
    write[i] += add[i];
  }
  fillHeatMap(add, numTiles, 0);

  // Apply Influence
  switch (hmUpdateNum) {
    case Pollution: {
      //applyWindToPollution();
      applyElevationToPollution();
    } break;

    case Value: {
      applyElevationToLandValue();
      influenceHeatMap(Value, Crime, c(CCrimeInfluenceOnValue));
      influenceHeatMap(Value, Pollution, c(CPollutionInfluenceOnValue));
      influenceHeatMap(Value, Education, c(CEducationInfluenceOnValue));
      influenceHeatMap(Value, Density, c(CDensityInfluenceOnValue));
      capValue();
    } break;

    case Density: {
      capDensity();
    } break;

    case Crime: {
    } break;

    case Prosperity: {
      influenceHeatMap(Prosperity, CommunityHM, c(CCommunityProsperityInfluence));
    }

    case CommunityHM: {
      influenceHeatMap(CommunityHM, Density, c(CCommunityDensityInfluence));
    }

    case HealthHM: {
      influenceHeatMap(HealthHM, Pollution, c(CHealthPollutionInfluence));
      influenceHeatMap(HealthHM, CommunityHM, c(CHealthCommunityInfluence));
      influenceHeatMap(HealthHM, Crime, c(CHealthCrimeInfluence));
    }
  }

  // Blur X
  for (int y0 = 0; y0 < mapSize; y0++) {
    int yStride = y0*mapSize;
    float rval2 = write[yStride] * (blurSize+1);
    for (int x0 = 0; x0 < blurSize; x0++) {
      rval2 += write[x0 + yStride];
    }
    for (int x0 = 0; x0 < mapSize; x0++) {
      const int x2 = x0 + blurSize;
      const int x2c = x2 > mapT ? mapT : x2;
      rval2 += write[x2c + yStride];
      const int x3 = x0 - blurSize - 1;
      const int x3c = x3 < 0 ? 0 : x3;
      rval2 -= write[x3c + yStride];
      blurBuffer[x0 + yStride] = rval2 * blurFactor;
    }
  }

  // Blur Y
  for (int x0 = 0; x0 < mapSize; x0++) {
    float rval2 = blurBuffer[x0] * (blurSize+1);
    for (int y0 = 0; y0 < blurSize; y0++) {
      rval2 += blurBuffer[y0*mapSize + x0];
    }
    for (int y0 = 0; y0 < mapSize; y0++) {
      const int y2 = y0 + blurSize;
      const int y2c = y2 > mapT ? mapT : y2;
      rval2 += blurBuffer[y2c*mapSize + x0];
      const int y3 = y0 - blurSize - 1;
      const int y3c = y3 < 0 ? 0 : y3;
      rval2 -= blurBuffer[y3c*mapSize + x0];
      write[y0*mapSize + x0] = rval2 * blurFactor;
    }
  }

  // Cap, Dissipate, Prevent Infinite/NaN Values
  float total = 0;
  for (int i = 0; i < numTiles; i++) {
    const HeatMapTile source = write[i];
    const HeatMapTile dissipated = source * dissipation +
      initDissipated;
    const HeatMapTile capped =
      !isfinite(dissipated) ? init :
      dissipated < 0 ? 0 :
      dissipated > 1 ? 1 :
      dissipated;
    total += capped;
    write[i] = capped;
  }

  if (hmUpdateNum == Density) {
    for (int i = 0; i < numTiles; i++) {
      const float densiPrev = spareBuffer[i];
      const float densiT = write[i];
      const float res = densiT < 0.1 && densiPrev < 0.1 ? densiT :
        mix(densiT, densiPrev, c(CDensityInertia));
      blurBuffer[i] = res == res ? res : 0;
    }
    memcpy_s(write, blurBuffer, numTiles*sizeof(HeatMapTile));
  }

  computeHeatMapTotal((HeatMapIndex)hmUpdateNum, HeatMapWrite);
  heatMutex.unlock();

  heatSwapMutex.lock();
  memcpy_s(read, write, numTiles*sizeof(HeatMapTile));
  heatMapUpdated[hmUpdateNum] = true;
  hmUpdateNum = (hmUpdateNum+1)%numHeatMaps;
  heatSwapMutex.unlock();
}

void heatMapLoop() {
  while(true) {
    unique_lock<mutex> heatLock(heatSwapMutex);
    heatSwapCondition.wait(heatLock, []{
      return !shouldContinueHeatMaps ||
        (!heatMapUpdated[hmUpdateNum] &&
         heatMapsReady &&
         durationRemaining >= c(CHeatMapTime)/numHeatMaps);
    });
    if (!shouldContinueHeatMaps) {
      return;
    }
    durationRemaining -= c(CHeatMapTime)/numHeatMaps;
    for (int i = 0; i < numHeatMaps; i++) {
      hmAdjustment[i] = nextHmAdjustment[i];
    }
    heatLock.unlock();

    updateHeatMaps();
  }
}

HeatMapIndex getHeatMap() {
  return nextHeatMap;
}

HeatMapIndex getHeatMap_d() {
  return currentHeatMap;
}

void setHeatMap(HeatMapIndex ndx, bool intense) {
  item lastHeatMap = nextHeatMap;
  nextHeatMap = ndx;
  nextHMIntense = intense;
  setIssuesIconsVisible();
  setUndergroundView(isUndergroundViewSelected() ||
      ndx == TransitHeatMap || ndx == RoadHeatMap);
  setAmenityHighlights();

  if (lastHeatMap == ZoneHeatMap || ndx == ZoneHeatMap) {
    renderLots();
  }

  if (lastHeatMap == TransitHeatMap && ndx != TransitHeatMap) {
    setTransitVisible(false);
    if (getLeftPanel() == TransitPanel) setLeftPanel(NoPanel);

  } else if (lastHeatMap != TransitHeatMap && ndx == TransitHeatMap) {
    setTransitVisible(true);
    if (getLeftPanel() == NoPanel) setLeftPanel(TransitPanel);
  }

  reportTutorialHeatmapSelected(ndx);
}

bool isHeatMapIntense() {
  return nextHMIntense;
}

void setupHeatmap(GLuint programID, item hm, bool intense, bool showPollution) {
  int chunkSize = getChunkSize();
  uint8_t number = 256+hm;
  if (!intense && hm == Pollution) number = 250;
  int hmData = chunkSize*getLandSize() * 256 * 256 + number;
  if (!c(CShowPollution)) showPollution = false;
  if (!isShowPollution()) showPollution = false;
  if (!intense && !showPollution) hmData = 0;
  //SPDLOG_INFO("setupHeatmap {} {}", number, hmData);
  glUniform1i(glGetUniformLocation(programID, "heatmapData"), hmData);
  glUniform1i(glGetUniformLocation(programID, "heatMapTexture"), 4);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, hm >= 0 ? heatMapTexture : 0);
}

void setupHeatmap(GLuint programID, bool showPollution) {
  setupHeatmap(programID, currentHeatMap, hmIntense, showPollution);
}

void setupNoHeatmap(GLuint programID) {
  setupHeatmap(programID, Pollution, false, false);
}

void drawHeatMaps() {
  heatSwapMutex.lock();
  if (resetHMDraw) {
    resetHMDraw = false;
    hmDrawn = false;
    if (heatMapTexture != 0) {
      glDeleteTextures(1, &heatMapTexture);
      heatMapTexture = 0;
    }
  }

  bool toDraw = !hmDrawn;
  heatSwapMutex.unlock();

  if (currentHeatMap < 0) return;
  if (heatMaps[currentHeatMap][HeatMapReadFront] == 0) return;

  hmDrawn = true;

  if (toDraw) {
    HeatMapTile* map = heatMaps[currentHeatMap][HeatMapReadFront];

    glActiveTexture(GL_TEXTURE4);
    if (heatMapTexture == 0) {
      glGenTextures(1, &heatMapTexture);
      glBindTexture(GL_TEXTURE_2D, heatMapTexture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, mapSize, mapSize,
        0, GL_RED, GL_FLOAT, map);

    } else {
      glBindTexture(GL_TEXTURE_2D, heatMapTexture);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mapSize, mapSize,
        GL_RED, GL_FLOAT, map);
    }
  }
}

void readHeatMaps(FileBuffer* file, int version) {
  unique_lock<mutex> lock(heatMutex);
  int numHeatMapsInFile = fread_item(file, version);
  if (version < 46) {
    stride = 1;
    mapSize = getMapTiles();
  } else {
    stride = fread_item(file, version);
    mapSize = fread_item(file, version);
  }

  checkStride();

  const int numTiles = mapSize*mapSize;
  allocHeatMaps();

  for (int i = 0; i < numHeatMaps; i++) {
    fillHeatMap(heatMaps[i][HeatMapAddFront], numTiles, 0);
    fillHeatMap(heatMaps[i][HeatMapAddBack], numTiles, 0);
  }

  for (int i = 0; i < numHeatMapsInFile; i++) {
    if (version >= 30) {
      fread(heatMaps[i][HeatMapWrite], sizeof(float), numTiles, file);

    } else {
      int ls = getLandSize();
      int chunkSize = getChunkSize();
      for (int cx = 0; cx < ls; cx++) {
        for (int cy = 0; cy < ls; cy++) {
          for (int x1 = 0; x1 < chunkSize; x1++) {
            for (int y1 = 0; y1 < chunkSize; y1++) {
              int x = cx*chunkSize + x1;
              int y = cy*chunkSize + y1;
              heatMaps[i][HeatMapWrite][x + y*mapSize] = fread_float(file);
            }
          }
        }
      }
    }

    memcpy_s(heatMaps[i][HeatMapReadFront], heatMaps[i][HeatMapWrite],
        numTiles*sizeof(HeatMapTile));
    memcpy_s(heatMaps[i][HeatMapReadBack], heatMaps[i][HeatMapWrite],
        numTiles*sizeof(HeatMapTile));
  }

  for (int i = numHeatMapsInFile; i < numHeatMaps; i++) {
    float init = heatMapInitValue[i];
    SPDLOG_WARN("heatmap {} init {}", i, init);
    fillHeatMap(heatMaps[i][HeatMapReadBack], numTiles, init);
    fillHeatMap(heatMaps[i][HeatMapReadFront], numTiles, init);
    fillHeatMap(heatMaps[i][HeatMapWrite], numTiles, init);
  }

  for (int i = 0; i < numHeatMaps; i++) {
    computeHeatMapTotal((HeatMapIndex)i, HeatMapWrite);
  }

  lock.unlock();
  heatSwapMutex.lock();
  heatMapsReady = true;
  heatSwapMutex.unlock();
  heatSwapCondition.notify_one();
}

void writeHeatMaps(FileBuffer* file) {
  const int numTiles = mapSize*mapSize;

  fwrite_item(file, numHeatMaps);
  fwrite_item(file, stride);
  fwrite_item(file, mapSize);
  for (int i = 0; i < numHeatMaps; i++) {
    fwrite(heatMaps[i][HeatMapReadFront], sizeof(float), numTiles, file);
  }
}

