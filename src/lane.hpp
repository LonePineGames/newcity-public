#ifndef LANE_H
#define LANE_H

#include "item.hpp"
#include "configuration.hpp"
#include "cup.hpp"
#include "main.hpp"
#include "spline.hpp"
#include "route/location.hpp"

#include <stdio.h>

const int _laneExists       = 1 <<  0;
const int _laneOpen         = 1 <<  1;
const int _laneActive       = 1 <<  2;
const int _laneRequested    = 1 <<  3;
const int _laneZeroSpace    = 1 <<  4;
const int _laneAlwaysActive = 1 <<  5;

const int _laneRight        = 1 <<  6;
const int _laneStraight     = 1 <<  7;
const int _laneLeft         = 1 <<  8;
const int _laneU            = 1 <<  9;
const int _laneTurnMask     = _laneStraight | _laneRight | _laneLeft | _laneU;
const int _laneTurnShift    = 6;
const int _laneYellow       = 1 <<  10;

const int _laneRoadway      = 1 <<  16;
const int _laneRailway      = 1 <<  17;

const int maxLanesPerBlock = 5;
const int maxLaneDrains = 5;

struct Lane {
  float length;
  vec3 ends[2];
  Spline spline;
  Cup<item> drains;
  Cup<item> sources;
};

struct LaneBlock {
  uint32_t flags;
  item graphElements[3];
  int numLanes;
  float speedLimit;
  float timeEstimate;
  float staticTimeEstimate;
  Cup<item> drains;
  Cup<item> sources;
  Lane lanes[maxLanesPerBlock];
};

struct EndDescriptor {
  vec3 location;
  vec3 normal;
  vec3 median;
  item graphElement;
  item source;
  item drain;
};

item addLaneBlock(item graphElement, EndDescriptor source, EndDescriptor drain, int numLanes);
void removeLaneBlock(item ndx);
void connectLaneBlocks(item source, item drain,
  item sourceLaneMin, item drainLaneMin, item numLanes);
item createConnectingLaneBlock(item nodeNdx, EndDescriptor source,
  EndDescriptor drain, int startOffset, int drainOffset, int numLanes);

LaneBlock* getLaneBlock(item ndx);
LaneBlock* getLaneBlock(GraphLocation location);
Lane* getLane(GraphLocation location);
Lane* getLane(item ndx);
item getLaneIndex(item blockNdx, int laneNum);
item getLaneBlockIndex(item laneNdx);
vec3 getLocation(GraphLocation location);
item laneIndexInBlock(item ndx);
GraphLocation getRandomGraphLoc();
GraphLocation graphLocation(item laneNdx, float dap);
GraphLocation graphLocation(item laneNdx, vec3 loc);
GraphLocation graphLocation(item laneNdx);
GraphLocation graphLocation(vec3 loc);
GraphLocation graphLocation(vec3 loc, Configuration config);
GraphLocation graphLocation(Line l);
GraphLocation graphLocation(Line l, Configuration config);
item getElement(GraphLocation loc);
GraphLocation graphLocationForEdge(item edgeNdx, vec3 loc);
item numLaneBlocks();
float getStaticTimeEstimate(item blockNdx);
float getTraversalRecord(item blockNdx);

// For router
void startLaneRouterMemoryBarrier_r();
void endLaneRouterMemoryBarrier_r();
item getNumLaneBlocks_r();
item encodeLane_r(item i);
item decodeLane_r(item i);
item* getDrains_r(item i, item* num);
float laneBlockCost_r(item a);
bool blockIsActive_r(item i);
bool blockIsOpen_r(item i);
float laneRoutingEstimate_r(item blockNdx, item endNdx);
void markRouterDataDirty_g();
vec3 getBlockLoc_r(item blockNdx);

// For vehicles
item getNumLaneBlocks_v();
float getLaneLength(item lane);
item* getLaneDrains(item lane);
vec3 getLocationV(GraphLocation loc);
Spline getLaneSpline(item lane);
item getBlockLanes(item i);
float getBlockSpeed(item lane);
void activateBlock_v(item block, bool activate, bool yellow);
void requestBlock_v(item blockNdx);
bool detectBlock_v(item blockNdx);
float getBlockCapacity_v(item blockNdx);
bool areLanesReady();

void clearSources(item blockNdx);
void clearDrains(item blockNdx);
void setLaneBlockState(item ndx);
void recordTraversal_v(item blockNdx, float time);
void repositionLanes(item laneBlockNdx, EndDescriptor source,
  EndDescriptor drain);
void reconfigureLaneBlock(item ndx, EndDescriptor source,
  EndDescriptor drain, int newNumLanes);

void readLaneBlocks(FileBuffer* file, int version);
void writeLaneBlocks(FileBuffer* file);
void resetLanes();
void finishLanes();
void freeLaneBlocks();
void swapLanesForVehicles(bool fromSave);
vector<item> routeToLaneBlock_g(item start, item end);
vector<item> routeToLaneBlock_r(item start, item end);

#endif
