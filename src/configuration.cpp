#include "configuration.hpp"

#include "option.hpp"
#include "string_proxy.hpp"

Configuration makeConfig(item type, int flags, item strategy, int numLanes,
    item speedLimit) {
  Configuration result;
  result.type = type;
  result.flags = flags;
  result.strategy = strategy;
  result.numLanes = numLanes;
  result.speedLimit = speedLimit;
  return result;
}

bool configsEqual(Configuration c0, Configuration c1) {
  return c0.type == c1.type && c0.numLanes == c1.numLanes &&
    c0.speedLimit == c1.speedLimit &&
    c0.strategy == c1.strategy &&
    c0.flags == c1.flags;
}

bool configsEqualEdge(Configuration c0, Configuration c1) {
  return c0.type == c1.type && c0.numLanes == c1.numLanes &&
    c0.speedLimit == c1.speedLimit &&
    c0.flags == c1.flags;
}

bool configsEqualNode(Configuration c0, Configuration c1) {
  return c0.type == c1.type && c0.strategy == c1.strategy;
}

void writeConfiguration(FileBuffer* file, Configuration config) {
  fwrite_item(file, config.strategy);
  fwrite_int(file, config.numLanes);
  fwrite_int(file, config.flags);
  fwrite_item(file, config.type);
  fwrite_int(file, config.speedLimit);
}

Configuration readConfiguration(FileBuffer* file, int version) {
  Configuration config;
  config.strategy = fread_item(file, version);
  config.numLanes = fread_int(file);
  config.flags = fread_int(file);
  config.type = fread_item(file, version);
  config.speedLimit = fread_int(file);
  return config;
}

char* printSpeedString(const char* pre, float ms, const char* post) {
  if (useMetric()) {
    return sprintf_o("%s%3dkm/h%s", pre, int(ms*c(CMsToKmph)), post);
  } else {
    return sprintf_o("%s%3dmph%s", pre, int(ms*c(CMsToMph)), post);
  }
}

