#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "money.hpp"
#include "serialize.hpp"

enum ConfigType {
  ConfigTypeRoad, ConfigTypeExpressway, ConfigTypeHeavyRail,
  ConfigTypePedestrian, ConfigTypeLightRail, ConfigTypeMonorail,
  ConfigTypeMonoHang, ConfigTypeMagLev,
  numConfigTypes
};

const int numIntersectionStrategies = 2;
enum IntersectionStrategies {
  StopSignStrategy, TrafficLightStrategy, RoundaboutStrategy,
  UnregulatedStrategy, SpawnStrategy, JointStrategy = 10,
  JunctionStrategy,
  numIntersectionStrategyTypes
};

/* Notes on speedLimits:
 *   speeds are in m/s
 *   they are represented to the user as mph/kmh
 *   By fiat, 10m/s == 15mph
 *   By fiat, 10m/s == 25kmh
 *   By fiat, 15mph == 25kmh
 *   This is so that all numbers are clean in all units of measurement
 */
const int numSpeedLimits = 5;
const int maxRoadSpeedLimit = 2;
const int minExpresswaySpeedLimit = 2;
const float speedLimits[] = {10, 20, 30, 40, 50};

const uint32_t _configOneWay = 1 << 0;
const uint32_t _configMedian = 1 << 1;
const uint32_t _configPlatform = 1 << 2;
const uint32_t _configDontMoveEarth = 1 << 3;
const uint32_t _configToll = 1 << 4;
const uint32_t _configElevationOffset = 16;
const uint32_t _configElevationMask = 15 << _configElevationOffset;

struct Configuration {
  int numLanes = 1;
  item speedLimit = 0;
  item type = ConfigTypeRoad;
  item strategy = 0;
  uint32_t flags;
};

Configuration makeConfig(item type, int flags, item strategy, int numLanes,
    item speedLimit);
bool configsEqualNode(Configuration c0, Configuration c1);
bool configsEqualEdge(Configuration c0, Configuration c1);
bool configsEqual(Configuration c0, Configuration c1);
Configuration readConfiguration(FileBuffer* file, int version);
void writeConfiguration(FileBuffer* file, Configuration config);

char* printSpeedString(const char* pre, float ms, const char* post);

#endif
