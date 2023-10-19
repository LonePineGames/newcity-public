#pragma once

#include "../item.hpp"
#include "../main.hpp"

#include <cstdint>
#include <vector>
using namespace std;

enum LocationType {
  LocLaneBlock = 0, LocPathBlock, LocDap, LocTransitLeg, LocTransitStop,
  LocVehicle, LocPedestrian, LocTravelGroup, LocBuilding, LocFail,
  numStepTypes
};

typedef uint32_t Location;

const uint32_t _locationTypeShift = 24;
const uint32_t _locationTypeMask = 255 << _locationTypeShift;
const uint32_t _locationLegLineShift = 12;

const uint32_t _routeInfoValid = 1 << 0;

Location transitLegStopLocation(item lineNdx, item stopNdx);
Location buildingLocation(item buildingNdx);
Location vehicleLocation(item vNdx);
Location travelGroupLocation(item vNdx);
Location dapLocation(item dap);
Location transitStopLocation(item ndx);

item locationType(Location location);
item locationNdx(Location location);
item locationLineNdx(Location location);
item locationLegNdx(Location location);
item getStopForLeg_g(Location location);

vec3 locationToWorldspace_g(Location loc);
char* format(const Location& loc);

