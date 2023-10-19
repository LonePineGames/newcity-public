#include "lane.hpp"

#include "error.hpp"
#include "graph.hpp"
#include "graph/transitRouter.hpp"
#include "option.hpp"
#include "pool.hpp"
#include "renderGraph.hpp"
#include "serialize.hpp"
#include "route/broker.hpp"
#include "route/router.hpp"
#include "thread.hpp"
#include "time.hpp"
#include "util.hpp"
#include "vehicle/update.hpp"

#include "spdlog/spdlog.h"

#include <algorithm>
#include <atomic>

const int laneBlockIndexMultiplier = 10;

Pool<LaneBlock>* laneBlocks = Pool<LaneBlock>::newPool(20000);
static bool updateLaneCache = false;
static atomic<bool> laneMemoryBarrier(false);
static atomic<bool> routerMemoryBarrier(false);
static atomic<bool> lanesReady(false);

// for router
Cup<vec3> laneBlockStarts_r;
Cup<vec3> laneBlockEnds_r;
Cup<unsigned char> laneBlockOpen;
Cup<float> laneTimeEstimate;
Cup<item> laneDrainsSlot;
Cup<item> laneBlockDrains;
Cup<item> laneBlockSpeeds_r;
item numLaneBlocks_r = 0;
item numLaneBlocks_v = 0;

// for vehicles
Cup<item> laneDrains;
Cup<unsigned char> laneBlockActive;
Cup<unsigned char> laneBlockRequested;
Cup<Spline> laneSplines;
Cup<float> laneLength;
Cup<item> laneTableNdx;
Cup<float> blockMaxSpeed;
Cup<float> traversalRecord;
Cup<float> staticTimeEstimate;
Cup<float> blockCapacity;

void computeLengths(LaneBlock* block) {
  int numLanes = block->numLanes;
  for (int i = 0; i < numLanes; i ++) {
    Lane* lane = &block->lanes[i];
    float lngth = 0;
    vec3 s = lane->ends[0];
    const int numSamples = 24;
    for (int j = 1; j <= numSamples; j++) {
      vec3 c = interpolateSpline(lane->spline, float(j)/numSamples);
      lngth += length(c-s);
      s = c;
    }
    lane->length = lngth;
    //lane->length = length(end1 - end0);
  }
  updateLaneCache = true;
}

void positionLanes(LaneBlock* block, EndDescriptor source, EndDescriptor drain) {
  int numLanes = block->numLanes;
  float hand = trafficHandedness();
  for (int i = 0; i < numLanes; i ++) {
    Lane* lane = &block->lanes[i];
    float offset = (float(i) + 0.5) * c(CLaneWidth);
    vec3 end0 = source.location + source.normal * offset + source.median;
    vec3 end1 = drain.location - drain.normal * offset - drain.median;
    lane->ends[0] = end0;
    lane->ends[1] = end1;
    lane->spline = spline(line(end0, end0 - hand*zNormal(source.normal)),
      line(end1, end1 - hand*zNormal(drain.normal)));
  }
  computeLengths(block);
}

void positionLanesSplines(LaneBlock* block) {
  EndDescriptor source;
  EndDescriptor drain;
  if (block->graphElements[1] < 0) {
    source = getNodeEndDescriptor(block->graphElements[0],
        block->graphElements[1]);
    drain = getNodeEndDescriptor(block->graphElements[2],
        block->graphElements[1]);
  } else {
    source = getEdgeEndDescriptor(block->graphElements[1],
        block->graphElements[0]);
    drain = getEdgeEndDescriptor(block->graphElements[1],
        block->graphElements[2]);
  }

  int numLanes = block->numLanes;
  float hand = trafficHandedness();
  for (int i = 0; i < numLanes; i ++) {
    Lane* lane = &block->lanes[i];
    vec3 end0 = lane->ends[0];
    vec3 end1 = lane->ends[1];
    lane->spline = spline(line(end0, end0 - hand*zNormal(source.normal)),
      line(end1, end1 - hand*zNormal(drain.normal)));
  }
}

void setDefaultTimeEstimate(item ndx) {
  LaneBlock* b = getLaneBlock(ndx);
  computeLengths(b);
  float length = b->lanes[0].length;
  float speed = b->speedLimit * c(CVehicleSpeed);
  float estimate = length / speed / gameDayInRealSeconds;
  if (b->graphElements[1] < 0) {
    Node* node = getNode(b->graphElements[1]);
    if (node->config.type == ConfigTypeRoad && node->edges.size() > 2) {
      //SPDLOG_INFO("adding time to intersection laneblk: {} + {}",
          //estimate, c(CStopLightMaxPhaseDuration)*.25f/gameDayInRealSeconds);
      estimate += c(CStopLightMaxPhaseDuration)*.25f/gameDayInRealSeconds;
    }
  }
  estimate = clamp(estimate, 0.00000000001f, 1.0f/24.f);
  b->staticTimeEstimate = estimate;
}

void broker_addLaneBlock_g(item ndx) {
  LaneBlock* laneBlock = getLaneBlock(ndx);
  Lane* lane0 = &laneBlock->lanes[0];
  vec3 center = .5f * (lane0->ends[0] + lane0->ends[1]);
  broker_addLaneBlock_g(ndx, center);
}

item addLaneBlock(item graphElement,
  EndDescriptor source, EndDescriptor drain, int numLanes) {

  item ndx = laneBlocks->create() * laneBlockIndexMultiplier;
  LaneBlock* laneBlock = getLaneBlock(ndx);
  laneBlock->flags = _laneActive | _laneExists;
  laneBlock->numLanes = numLanes;
  laneBlock->speedLimit = getSpeedLimit(graphElement);
  laneBlock->graphElements[0] = source.graphElement;
  laneBlock->graphElements[1] = graphElement;
  laneBlock->graphElements[2] = drain.graphElement;
  laneBlock->sources.clear();
  laneBlock->drains.clear();

  if (graphElement < 0) {
    Node* node = getNode(graphElement);
    if (node->config.type == ConfigTypeRoad ||
      node->config.type == ConfigTypeHeavyRail) {
      laneBlock->flags |= _laneZeroSpace;
    }
  }

  bool isRail = true;
  bool isRoad = true;
  for (int i = 0; i < 3; i++) {
    item elemNdx = laneBlock->graphElements[i];
    if (elemNdx < 0) continue;
    Edge* edge = getEdge(elemNdx);
    item type = edge->config.type;
    if (type != ConfigTypeHeavyRail) {
      isRail = false;
    }
    if (type != ConfigTypeRoad && type != ConfigTypeExpressway) {
      isRoad = false;
    }
  }

  if (isRail) laneBlock->flags |= _laneRailway;
  if (isRoad) laneBlock->flags |= _laneRoadway;

  positionLanes(laneBlock, source, drain);

  for (int i = 0; i < numLanes; i ++) {
    Lane* lane = &laneBlock->lanes[i];
    lane->drains.clear();
    lane->sources.clear();
  }

  broker_addLaneBlock_g(ndx);
  setDefaultTimeEstimate(ndx);
  laneBlock->timeEstimate = laneBlock->staticTimeEstimate;
  return ndx;
}

void repositionLanes(item laneBlockNdx, EndDescriptor source,
  EndDescriptor drain) {

  LaneBlock* block = getLaneBlock(laneBlockNdx);
  positionLanes(block, source, drain);
}

void removeLane(item ndx, Lane* lane);

void reconfigureLaneBlock(item ndx, EndDescriptor source,
  EndDescriptor drain, int newNumLanes) {

  LaneBlock* laneBlock = getLaneBlock(ndx);
  laneBlock->speedLimit = getSpeedLimit(laneBlock->graphElements[1]);
  int oldNumLanes = laneBlock->numLanes;
  laneBlock->numLanes = newNumLanes;

  for (int i = newNumLanes; i < oldNumLanes; i ++) {
    Lane* lane = &laneBlock->lanes[i];
    removeLane(ndx+i, lane);
  }

  for (int i = oldNumLanes; i < newNumLanes; i ++) {
    Lane* lane = &laneBlock->lanes[i];
    lane->drains.clear();
    lane->sources.clear();
  }

  positionLanes(laneBlock, source, drain);
}

void clearSources(item blockNdx) {
  LaneBlock* block = getLaneBlock(blockNdx);
  block->sources.clear();

  for(int i= 0; i < maxLanesPerBlock; i++) {
    block->lanes[i].sources.clear();
  }
}

void clearDrains(item blockNdx) {
  LaneBlock* block = getLaneBlock(blockNdx);
  block->drains.clear();

  for(int i= 0; i < maxLanesPerBlock; i++) {
    block->lanes[i].drains.clear();
  }
}

void removeLaneBlock(item ndx) {
  LaneBlock* laneBlock = getLaneBlock(ndx);
  if (!(laneBlock->flags & _laneExists)) {
    return;
  }

  broker_removeLaneBlock_g(ndx);

  laneBlock->flags = 0;

  for(int i = 0; i < laneBlock->sources.size(); i++) {
    item sourceNdx = laneBlock->sources[i];
    if (sourceNdx < laneBlockIndexMultiplier ||
        sourceNdx >= (numLaneBlocks()+1)*laneBlockIndexMultiplier ||
        sourceNdx%laneBlockIndexMultiplier >= 6) {
      continue;
    }
    LaneBlock* source = getLaneBlock(sourceNdx);
    source->drains.removeByValue(ndx);
  }

  for(int i = 0; i < laneBlock->drains.size(); i++) {
    item drainNdx = laneBlock->drains[i];
    if (drainNdx < laneBlockIndexMultiplier ||
        drainNdx >= (numLaneBlocks()+1)*laneBlockIndexMultiplier ||
        drainNdx%laneBlockIndexMultiplier >= 6) {
      continue;
    }
    LaneBlock* drain = getLaneBlock(drainNdx);
    drain->sources.removeByValue(ndx);
  }

  for (int k = 0; k < laneBlock->numLanes; k ++) {
    Lane* lane = &laneBlock->lanes[k];
    removeLane(ndx+k, lane);
  }

  laneBlocks->settle(ndx / laneBlockIndexMultiplier);
  updateLaneCache = true;
}

void removeLane(item ndx, Lane* lane) {
  for(int i = 0; i < lane->sources.size(); i++) {
    item sourceNdx = lane->sources[i];
    if (sourceNdx < laneBlockIndexMultiplier ||
        sourceNdx >= (numLaneBlocks()+1)*laneBlockIndexMultiplier ||
        sourceNdx%laneBlockIndexMultiplier >= 6) {
      continue;
    }
    Lane* source = getLane(sourceNdx);
    source->drains.removeByValue(ndx);
  }

  for(int i = 0; i < lane->drains.size(); i++) {
    item drainNdx = lane->drains[i];
    if (drainNdx < laneBlockIndexMultiplier ||
        drainNdx >= (numLaneBlocks()+1)*laneBlockIndexMultiplier ||
        drainNdx%laneBlockIndexMultiplier >= 6) {
      continue;
    }
    Lane* drain = getLane(drainNdx);
    drain->sources.removeByValue(ndx);
  }

  removeLaneVehicles(ndx);
  updateLaneCache = true;
}

void freeLaneBlocks() {
  for (int i = laneBlocks->settling.size()-1; i >= 0; i--) {
    laneBlocks->free(laneBlocks->settling[i]);
  }
}

item laneIndexInBlock(item ndx) {
  return ndx % laneBlockIndexMultiplier;
}

item getLaneBlockIndex(item ndx) {
  return ndx - ndx%laneBlockIndexMultiplier;
}

Lane* getLane(GraphLocation location) {
  return getLane(location.lane);
}

LaneBlock* getLaneBlock(GraphLocation location) {
  return laneBlocks->get(location.lane / laneBlockIndexMultiplier);
}

LaneBlock* getLaneBlock(item ndx) {
  return laneBlocks->get(ndx / laneBlockIndexMultiplier);
}

Lane* getLane(item ndx) {
  LaneBlock* block = getLaneBlock(ndx);
  item laneNum = ndx % laneBlockIndexMultiplier;
  Lane* lane = &block->lanes[laneNum];
  return lane;
}

vec3 getLocation(GraphLocation location) {
  item lbNdx = location.lane/10;
  if (lbNdx < 1 || lbNdx > laneBlocks->size()) return vec3(0,0,0);
  Lane* lane = getLane(location.lane);
  float length = lane->length;
  location.dap = clamp(location.dap, 0.f, length);
  return interpolateSpline(lane->spline, location.dap/length);
}

GraphLocation graphLocation(item laneNdx) {
  GraphLocation result;
  result.lane = laneNdx;
  result.dap = 0;
  return result;
}

GraphLocation graphLocation(item laneNdx, float dap) {
  GraphLocation result;
  result.lane = laneNdx;
  result.dap = dap;
  return result;
}

GraphLocation graphLocation(item laneNdx, vec3 loc) {
  Lane* lane = getLane(laneNdx);
  vec3 p = nearestPointOnLine(loc, line(lane->ends[0], lane->ends[1]));

  GraphLocation result;
  result.lane = laneNdx;
  result.dap = vecDistance(p, lane->ends[0]);
  return result;
}

void connectLaneBlocks(item sourceNdx, item drainNdx,
  item sourceLaneMin, item drainLaneMin, item numLanes) {

  LaneBlock* source = getLaneBlock(sourceNdx);
  LaneBlock* drain = getLaneBlock(drainNdx);

  source->drains.push_back(drainNdx);
  drain->sources.push_back(sourceNdx);

  for (int i = 0; i < numLanes; i++) {
    Lane* lane0 = &source->lanes[i + sourceLaneMin];
    Lane* lane1 = &drain->lanes[i + drainLaneMin];
    lane0->drains.push_back(getLaneIndex(drainNdx, i + drainLaneMin));
    lane1->sources.push_back(getLaneIndex(sourceNdx,
          i + sourceLaneMin));
  }

  updateLaneCache = true;
}

item createConnectingLaneBlock(item nodeNdx, EndDescriptor source,
  EndDescriptor drain, int sourceOffset, int drainOffset, int numLanes) {

  source.location += source.normal * float(sourceOffset) * c(CLaneWidth);
  drain.location -= drain.normal * float(drainOffset) * c(CLaneWidth);

  item newLaneBlock = addLaneBlock(
    nodeNdx, source, drain, numLanes);
  connectLaneBlocks(source.source, newLaneBlock,
    sourceOffset, 0, numLanes);
  connectLaneBlocks(newLaneBlock, drain.drain,
    0, drainOffset, numLanes);
  return newLaneBlock;
}

item getLaneIndex(item laneBlockNdx, int laneNum) {
  return laneBlockNdx + laneNum;
}

GraphLocation nullGraphLoc() {
  GraphLocation result;
  result.lane = 0;
  result.dap = 0;
  return result;
}

item getElement(GraphLocation loc) {
  if (loc.lane < 10) return 0;
  LaneBlock* blk = getLaneBlock(loc);
  return blk->graphElements[1];
}

GraphLocation graphLocationForEdge(item edgeNdx, vec3 loc) {
  if (edgeNdx <= 0) {
    return nullGraphLoc();
  }
  Edge* edge = getEdge(edgeNdx);
  int laneInBlock = edge->config.numLanes - 1;
  item laneNdx = 0;
  float best = FLT_MAX;
  for (int i=0; i < 2; i++) {
    if (edge->laneBlocks[i] >= 10) {
      Lane* lane = getLane(edge->laneBlocks[i] + laneInBlock);
      float dist = pointLineDistance(loc, line(lane->ends[0], lane->ends[1]));
      if (dist < best) {
        best = dist;
        laneNdx = edge->laneBlocks[i] + laneInBlock;
      }
    }
  }

  if (laneNdx <= 0) {
    return nullGraphLoc();
  }

  return graphLocation(laneNdx, loc);
}

GraphLocation graphLocation(vec3 loc) {
  item edgeNdx = nearestEdge(loc, false);
  return graphLocationForEdge(edgeNdx, loc);
}

GraphLocation graphLocation(vec3 loc, Configuration config) {
  item edgeNdx = nearestEdge(loc, false, config);
  return graphLocationForEdge(edgeNdx, loc);
}

GraphLocation graphLocation(Line l, item edgeNdx) {
  if (edgeNdx <= 0) {
   return nullGraphLoc();
  }

  Edge* edge = getEdge(edgeNdx);
  if (edge->laneBlocks[0] == 0) return nullGraphLoc();
  int laneInBlock = edge->config.numLanes - 1;
  item laneNdx;
  float best = FLT_MAX;
  for (int i=0; i < 2; i++) {
    if (edge->laneBlocks[i] != 0) {
      Lane* lane = getLane(edge->laneBlocks[i] + laneInBlock);
      float dist = lineDistance(l, line(lane->ends[0], lane->ends[1]));
      if (dist < best) {
        best = dist;
        laneNdx = edge->laneBlocks[i] + laneInBlock;
      }
    }
  }

  Lane* lane = getLane(laneNdx);
  vec3 p = pointOfIntersection(line(lane->ends[0], lane->ends[1]), l);

  GraphLocation result;
  result.lane = laneNdx;
  result.dap = vecDistance(p, lane->ends[0]);
  return result;
}

GraphLocation graphLocation(Line l) {
  item edgeNdx = nearestEdge(l, false);
  return graphLocation(l, edgeNdx);
}

GraphLocation graphLocation(Line l, Configuration config) {
  item edgeNdx = nearestEdge(l, false, config);
  return graphLocation(l, edgeNdx);
}

GraphLocation getRandomGraphLoc() {
  if (laneBlocks->size() == 0) {
    return nullGraphLoc();
  }

  item laneBlockNdx;
  LaneBlock* block;
  int iter = 0;

  do {
    if (iter > laneBlocks->size()) {
      return nullGraphLoc();
    }
    laneBlockNdx = (randItem(laneBlocks->size())+1) * laneBlockIndexMultiplier;
    block = getLaneBlock(laneBlockNdx);
    iter ++;
  } while (!(block->flags & _laneExists) || !(block->flags & _laneOpen) ||
    block->graphElements[1] < 0);

  item laneNum = 0;
  item laneNdx = getLaneIndex(laneBlockNdx, laneNum);
  Lane* lane = getLane(laneNdx);

  GraphLocation result;
  result.lane = laneNdx;
  result.dap = .5f * lane->length * randFloat();
  return result;
}

void setLaneBlockState(item ndx) {
  if (ndx <= 0) return;
  LaneBlock* block = getLaneBlock(ndx);
  if (!(block->flags & _laneExists)) return;
  bool open = isOpen(block->graphElements[0]) &&
    isOpen(block->graphElements[1]) &&
    isOpen(block->graphElements[2]);
  if (open) {
    block->flags |= _laneOpen;
  } else {
    block->flags &= ~_laneOpen;
  }
  updateLaneCache = true;
}

/*
bool laneVehiclesComparator(item a, item b) {
  Vehicle* va = getVehicle(a);
  Vehicle* vb = getVehicle(b);
  return va->pilot.dap > vb->pilot.dap;
}

void sortVehiclesInLane(item laneNdx) {
  Lane* lane = getLane(laneNdx);
  vector<item>* list = &lane->vehicles;
  sort(list->begin(), list->end(), laneVehiclesComparator);
  int listSize = list->size();
  for(int i = 0; i < listSize; i++) {
    Vehicle* v = getVehicle(list->operator[](i));
    if (v->pilot.lane == laneNdx) {
      item nextVehicle = i > 0 ?
        list->operator[](i-1) : 0;
      v->vehicleAhead = nextVehicle;
    }
  }
}
*/

/*
float spaceInLane(item laneNdx, bool fastForYellow) {
  if (numVehicles[laneNdx] > maxVehiclesInLane) {
    return 0;
  }

  LaneBlock* laneBlock = getLaneBlock(laneNdx);
  if (!(laneBlock->flags & _laneActive) &&
      !(fastForYellow && laneBlock->flags & _laneYellow)) {
    return 0;
  }

  int num = numVehicles[laneNdx];
  if (num >= maxVehiclesInLane) {
    return 0;
  }

  Lane* lane = getLane(laneNdx);
  float nextLaneSpace = tileSize*25; //c(CmaxRoadLength);
  if (laneBlock->flags & _laneZeroSpace) {
    nextLaneSpace = spaceInLane(lane->drains[0], false);
  }

  if (num < 2) {
    return nextLaneSpace; //std::max(lane->length, minLaneSpace);
  }
  //Vehicle* endVehicle = getVehicle(
    //laneVehicles[laneNdx][num-1]);
  //if (endVehicle->vehicleAhead == 0) return maxRoadLength;
  //Vehicle * vA = getVehicle(endVehicle->vehicleAhead);
  Vehicle * vA = getVehicle(laneVehicles[laneNdx][num-2]);
  return std::min(nextLaneSpace, vA->pilot.dap - vehicleLength(vA));
}
*/

float getBlockCapacity_v(item blockNdx) {
  item ndx = blockNdx/laneBlockIndexMultiplier;
  return blockCapacity[ndx];
}

float getStaticTimeEstimate(item blockNdx) {
  item ndx = blockNdx/laneBlockIndexMultiplier;
  if (ndx >= staticTimeEstimate.size()) return 0;
  return staticTimeEstimate[ndx];
}

float getTraversalRecord(item blockNdx) {
  item ndx = blockNdx/laneBlockIndexMultiplier;
  if (ndx >= traversalRecord.size()) return 0;
  return traversalRecord[ndx];
}

void recordTraversal_v(item blockNdx, float time) {
  item ndx = blockNdx/laneBlockIndexMultiplier;
  time /= gameDayInRealSeconds;
  float lastTime = traversalRecord[ndx];
  float estimate = mix(lastTime, time, c(CTraversalRecordDynamism));
  //estimate = std::max(estimate, 0.00000000001f);
  traversalRecord.set(ndx, estimate);
}

item numLaneBlocks() {
  return laneBlocks->size();
}

item getNumLaneBlocks_r() {
  return numLaneBlocks_r;
}

item getNumLaneBlocks_v() {
  return numLaneBlocks_v;
}

void resetLanes() {
  SPDLOG_INFO("resetLanes");
  for (int i = 1; i <= laneBlocks->size(); i++) {
    LaneBlock* block = getLaneBlock(i*laneBlockIndexMultiplier);
    block->sources.empty();
    block->drains.empty();
    for (int j = 0; j < maxLanesPerBlock; j++) {
      Lane* lane = &block->lanes[j];
      lane->sources.empty();
      lane->drains.empty();
    }
  }
  laneBlocks->clear();

  updateLaneCache = true;
  lanesReady = false;
  numLaneBlocks_r = 0;
  numLaneBlocks_v = 0;

  // for router
  laneBlockStarts_r.clear();
  laneBlockEnds_r.clear();
  laneBlockOpen.clear();
  laneTimeEstimate.clear();
  laneBlockSpeeds_r.clear();
  laneDrainsSlot.clear();
  laneBlockDrains.clear();

  // for vehicles
  laneDrains.clear();
  laneBlockActive.clear();
  laneBlockRequested.clear();
  laneSplines.clear();
  laneLength.clear();
  laneTableNdx.clear();
  blockMaxSpeed.clear();
  traversalRecord.clear();
  staticTimeEstimate.clear();
  blockCapacity.clear();
}

/*************
 * Save/Load *
 *************/

void writeLane(FileBuffer* file, item ndx) {
  Lane* lane = getLane(ndx);
  fwrite_float(file, lane->length);
  fwrite_vec3 (file, lane->ends[0]);
  fwrite_vec3 (file, lane->ends[1]);
  writeSpline(file, lane->spline);
  lane->sources.write(file);
  lane->drains.write(file);
}

void readLane(FileBuffer* file, int version, item ndx) {
  Lane* lane = getLane(ndx);

  lane->length = fread_float(file);
  lane->ends[0] = fread_vec3(file);
  lane->ends[1] = fread_vec3(file);
  /*
  if (version < 4) {
    numVehicles[ndx] = fread_int(file);
    for (int j = 0; j < numVehicles[ndx]; j++) {
      laneVehicles[ndx][j] = fread_item(file, version);
    }
    for (int j = numVehicles[ndx]; j < maxVehiclesInLane; j++) {
      laneVehicles[ndx][j] = 0;
    }
  }
  */
  if (version >= 24) {
    lane->spline = readSpline(file, version);
  }
  lane->sources.read(file, version);
  lane->drains.read(file, version);
}

void writeLaneBlock(FileBuffer* file, item ndx) {
  LaneBlock* laneBlock = getLaneBlock(ndx);

  fwrite_int  (file, laneBlock->flags);
  if (laneBlock->flags & _laneExists) {
    fwrite_float(file, laneBlock->speedLimit);
    fwrite_float(file, laneBlock->timeEstimate);
    fwrite_item(file, laneBlock->graphElements[0]);
    fwrite_item(file, laneBlock->graphElements[1]);
    fwrite_item(file, laneBlock->graphElements[2]);
    laneBlock->sources.write(file);
    laneBlock->drains.write(file);
    fwrite_int  (file, laneBlock->numLanes);
    for(int i=0; i < laneBlock->numLanes; i ++) {
      writeLane(file, ndx+i);
    }
  }
}

void readLaneBlock(FileBuffer* file, int version, item ndx) {
  LaneBlock* laneBlock = getLaneBlock(ndx);

  laneBlock->flags = fread_int(file);
  if (laneBlock->flags & _laneExists) {
    laneBlock->speedLimit = fread_float(file);
    if (version >= 31) {
      laneBlock->timeEstimate = fread_float(file);
    }
    laneBlock->graphElements[0] = fread_item(file, version);
    laneBlock->graphElements[1] = fread_item(file, version);
    laneBlock->graphElements[2] = fread_item(file, version);
    laneBlock->sources.read(file, version);
    laneBlock->drains.read(file, version);
    laneBlock->numLanes = fread_int(file);
    for(int i=0; i < laneBlock->numLanes; i ++) {
      readLane(file, version, ndx+i);
    }
    if (version < 24) {
      positionLanesSplines(laneBlock);
    }
    broker_addLaneBlock_g(ndx);
    setDefaultTimeEstimate(ndx);

  } else {
    laneBlock->speedLimit = 0;
    //laneBlock->timeEstimate = 0;
    laneBlock->graphElements[0] = 0;
    laneBlock->graphElements[1] = 0;
    laneBlock->graphElements[2] = 0;
    laneBlock->sources.clear();
    laneBlock->drains.clear();
    laneBlock->numLanes = 0;
  }
}

void writeLaneBlocks(FileBuffer* file) {
  laneBlocks->write(file);

  for (int i=1; i <= laneBlocks->size(); i++) {
    writeLaneBlock(file, i * laneBlockIndexMultiplier);
  }

  /*
  for (int i = 0; i <= laneBlocks->size() * laneBlockIndexMultiplier; i++) {
    fwrite_item(file, numVehicles[i]);
    for (int j = 0; j < numVehicles[i]; j++) {
      fwrite_item(file, laneVehicles[i][j]);
    }
  }
  */
}

void readLaneBlocks(FileBuffer* file, int version) {
  laneBlocks->read(file, version);

  for (int i=1; i <= laneBlocks->size(); i++) {
    readLaneBlock(file, version, i*laneBlockIndexMultiplier);
  }

  if (version >= 4 && version < 48) { // legacy laneVehicles
    int mult = version < 41 ? 6 : laneBlockIndexMultiplier;
    for (int i=0; i <= laneBlocks->size()*mult; i++) {
      int numVehicles = fread_item(file, version);
      for (int j = 0; j < numVehicles; j++) {
        fread_item(file, version);
      }
    }
  }

  /*
  if (version >= 4) {
    int mult = version < 41 ? 6 : laneBlockIndexMultiplier;
    for (int i=0; i <= laneBlocks->size()*mult; i++) {
      numVehicles[i] = fread_item(file, version);
      for (int j = 0; j < numVehicles[i]; j++) {
        laneVehicles[i][j] = fread_item(file, version);
      }
      //for (int j = numVehicles[i]; j < maxVehiclesInLane; j++) {
       // laneVehicles[i][j] = 0;
      //}
    }
  }
  */

  updateLaneCache = true;
}

/***********
 * Routing *
 ***********/

item encodeLane_r(item i) {
  return i/laneBlockIndexMultiplier;
}

item decodeLane_r(item i) {
  return i*laneBlockIndexMultiplier;
}

item* getDrains_r(item i, item* num) {
  i /= laneBlockIndexMultiplier;
  item ndx = laneDrainsSlot[i];
  *num = laneDrainsSlot[i+1] - ndx;
  return laneBlockDrains.get(ndx);
}

float laneBlockCost_r(item a) {
  a /= laneBlockIndexMultiplier;
  return laneTimeEstimate[a];
}

vec3 getBlockLoc_r(item blockNdx) {
  blockNdx /= laneBlockIndexMultiplier;
  return laneBlockStarts_r[blockNdx];
}

float laneRoutingEstimate_r(item blockNdx, item endNdx) {
  blockNdx /= laneBlockIndexMultiplier;
  endNdx /= laneBlockIndexMultiplier;
  if (endNdx >= numLaneBlocks_r || blockNdx >= numLaneBlocks_r) return 1;
  float dist = vecDistance(laneBlockEnds_r[endNdx],
      laneBlockStarts_r[blockNdx]);
  const float maxSpeed = laneBlockSpeeds_r[blockNdx]*c(CVehicleSpeed);
  return dist / maxSpeed / gameDayInRealSeconds
    * c(CAStarFactor);
}

bool blockIsOpen_r(item i) {
  i /= laneBlockIndexMultiplier;
  return laneBlockOpen[i/8] & (1 << i%8);
}

bool blockIsActive_r(item i) {
  i /= laneBlockIndexMultiplier;
  return laneBlockActive[i/8] & (1 << i%8);
}

void activateBlock_v(item block, bool activate, bool yellow) {
  block /= laneBlockIndexMultiplier;
  laneBlockActive.ensureSize(block/8+1);
  unsigned char byte = laneBlockActive[block/8];
  if (activate) {
    byte |= (1 << block%8);
  } else {
    byte &= ~(1 << block%8);
  }
  laneBlockActive.set(block/8, byte);
}

void requestBlock_v(item block) {
  block /= laneBlockIndexMultiplier;
  unsigned char byte = laneBlockRequested[block/8];
  byte |= (1 << block%8);
  laneBlockRequested.set(block/8, byte);
}

bool detectBlock_v(item block) {
  block /= laneBlockIndexMultiplier;
  laneBlockRequested.ensureSize(block/8+1);
  unsigned char byte = laneBlockRequested[block/8];
  unsigned char flag = (1 << block%8);
  bool result = byte & flag;
  if (result) laneBlockRequested.set(block/8, byte & ~flag);
  return result;
}

void startLaneRouterMemoryBarrier_r() {
  routerMemoryBarrier = true;
  while (laneMemoryBarrier) {
    routerMemoryBarrier = false;
    sleepMilliseconds(1);
    routerMemoryBarrier = true;
  }
}

void endLaneRouterMemoryBarrier_r() {
  routerMemoryBarrier = false;
}

Spline getLaneSpline(item lane) {
  int ndx = laneTableNdx[lane/laneBlockIndexMultiplier];
  return laneSplines[ndx+laneIndexInBlock(lane)];
}

vec3 getLocationV(GraphLocation loc) {
  float length = getLaneLength(loc.lane);
  float dap = clamp(loc.dap/length, 0.f, 1.f);
  return interpolateSpline(getLaneSpline(loc.lane), dap);
}

float getLaneLength(item lane) {
  int ndx = laneTableNdx[lane/laneBlockIndexMultiplier];
  return laneLength[ndx+laneIndexInBlock(lane)];
}

item* getLaneDrains(item lane) {
  int ndx = laneTableNdx[lane/laneBlockIndexMultiplier];
  return laneDrains.get(maxLaneDrains*(ndx+laneIndexInBlock(lane)));
}

item getBlockLanes(item block) {
  item ndx = block/laneBlockIndexMultiplier;
  if (ndx < 0 || ndx >= laneTableNdx.size()) return 0;
  return laneTableNdx[ndx+1] - laneTableNdx[ndx];
}

float getBlockSpeed(item lane) {
  return blockMaxSpeed[lane/laneBlockIndexMultiplier];
}

void markRouterDataDirty_g() {
  updateLaneCache = true;
}

void finishLanes() {
  if (!updateLaneCache) return;
  laneMemoryBarrier = true;
  while (routerMemoryBarrier) {
    sleepMilliseconds(1);
  }

  swapTransitRouterData_g();

  item size = laneBlocks->size()+1;
  numLaneBlocks_r = laneBlocks->size()+1;
  if (laneBlockStarts_r.size() < size) {

    laneBlockStarts_r.resize(size);
    laneBlockEnds_r.resize(size);
    laneBlockOpen.resize(size/8+1);
    laneTimeEstimate.resize(size);
    laneBlockSpeeds_r.resize(size);
    laneDrainsSlot.resize(size+1);
  }

  for (int i = 0; i < size/8+1; i++) {
    laneBlockOpen.set(i, 0);
  }

  for (int i = 1; i < size; i++) {
    LaneBlock* blk = getLaneBlock(i*laneBlockIndexMultiplier);
    laneBlockStarts_r.set(i, blk->lanes[0].ends[0]);
    laneBlockEnds_r.set(i, blk->lanes[0].ends[1]);
    bool open = (blk->flags & _laneExists) && (blk->flags & _laneOpen);
    if (open) laneBlockOpen.set(i/8, laneBlockOpen[i/8] | (1 << i%8));
    float time = mix(blk->staticTimeEstimate, blk->timeEstimate,
        c(CRoutingTrafficAwareness));
    laneTimeEstimate.set(i, std::max(time, 0.00000000001f));
    laneBlockSpeeds_r.set(i, blk->speedLimit/10);
    laneDrainsSlot.set(i, laneBlockDrains.size());
    for (int j = 0; j < blk->drains.size(); j++) {
      laneBlockDrains.push_back(blk->drains[j]);
    }
  }
  laneDrainsSlot.set(size, laneBlockDrains.size());

  updateLaneCache = false;
  laneMemoryBarrier = false;
}

void swapLanesForVehicles(bool fromSave) {

  item size = laneBlocks->size()+1;
  numLaneBlocks_v = size;
  if (laneTableNdx.size() <= size) {
    laneBlockActive.resize(size*5/8+1);
    laneBlockRequested.resize(size*5/8+1);
    laneSplines.resize(size*5);
    laneLength.resize(size*5);
    laneTableNdx.resize(size+2);
    blockMaxSpeed.resize(size+1);
    traversalRecord.resize(size+1);
    staticTimeEstimate.resize(size+1);
    blockCapacity.resize(size+1);
  }

  if (fromSave) {
    for (int i = 0; i < size*5/8+1; i++) {
      laneBlockActive.set(i, 0);
      laneBlockRequested.set(i, 0);
    }
  }

  int tableNdx = 0;

  for (int i = 1; i < size; i++) {
    LaneBlock* blk = getLaneBlock(i*laneBlockIndexMultiplier);
    blockMaxSpeed.set(i, blk->speedLimit);
    staticTimeEstimate.set(i, blk->staticTimeEstimate);

    if (fromSave) {
      traversalRecord.set(i, blk->timeEstimate);
      bool active = open && (blk->flags & _laneActive);
      if (blk->flags & _laneAlwaysActive) active = true;
      if (blk->graphElements[1] > 0) active = true;
      if (active) laneBlockActive.set(i/8, laneBlockActive[i/8] | (1 << i%8));
      bool requested = open && (blk->flags & _laneRequested);
      if (requested) {
        laneBlockRequested.set(i/8, laneBlockRequested[i/8] | (1 << i%8));
      }

    } else {
      blk->timeEstimate = mix(blk->timeEstimate, traversalRecord[i], 0.5);

      blk->timeEstimate = clamp(blk->timeEstimate, 0.00000000001f,
          blk->staticTimeEstimate*c(CTraversalRecordMax));
      bool active = blockIsActive_r(i*laneBlockIndexMultiplier);
      if (blk->flags & _laneAlwaysActive) active = true;
      if (blk->graphElements[1] > 0) active = true;
      if (active) laneBlockActive.set(i/8, laneBlockActive[i/8] | (1 << i%8));
      if (active) {
        blk->flags |= _laneActive;
      } else {
        blk->flags &= ~_laneActive;
      }

      bool requested = laneBlockRequested[i/8] & (1 << i%8);
      if (requested) {
        blk->flags |= _laneRequested;
      } else {
        blk->flags &= ~_laneRequested;
      }
    }

    float blockCap = 0;
    laneTableNdx.set(i, tableNdx);
    for (int j = 0; j < blk->numLanes; j++) {
      Lane* lane = getLane(i*laneBlockIndexMultiplier + j);
      laneSplines.set(tableNdx, lane->spline);
      laneLength.set(tableNdx, lane->length);
      blockCap += lane->length / c(CVehicleLengthForCapacity);

      laneDrains.ensureSize((tableNdx+1)*maxLaneDrains);
      int ndx = tableNdx*maxLaneDrains;
      int numDrains = lane->drains.size();
      for (int i = 0; i < maxLaneDrains; i ++) {
        laneDrains.set(ndx, i < numDrains ? lane->drains[i] : 0);
        ndx ++;
      }

      tableNdx ++;
    }

    if (blk->graphElements[1] <= 0) blockCap = 0;
    blockCapacity.set(i, blockCap);

  }

  laneTableNdx.set(size, tableNdx);
  lanesReady = true;
}

bool areLanesReady() {
  return lanesReady;
}

