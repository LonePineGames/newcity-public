#pragma once

#include "../lane.hpp"

enum LaneLocType {
  LLPhysical, LLPilot,
  llnum
};

struct LaneLocInfo {
  item vNdx;
  item ahead;
  LaneLocType type;
  LaneLocType aheadType;
  GraphLocation loc;
};

void removeLaneVehiclesActual(item lane);
void enterLane(item vNdx, GraphLocation target);
void leaveLane(item vNdx);
LaneLocInfo moveInLane(item vNdx, LaneLocType type, float dist);
LaneLocInfo moveToLane(item vNdx, LaneLocType type, GraphLocation target);
LaneLocInfo moveTo(item vNdx, LaneLocType type, GraphLocation target);
LaneLocInfo getLaneLoc(item vNdx, LaneLocType type);
LaneLocInfo getVehicleAhead(item vNdx, LaneLocType type);
LaneLocInfo firstInLane(item lane);
LaneLocInfo getNearestLaneLoc(GraphLocation target);
item numVehiclesInLaneBack(item lane);
item numVehiclesInBlock(item blkNdx);
item numVehiclesInBlock_v(item blkNdx);
void swapLaneLocs();
void resizeLaneLocs(item num);
void resetLaneLocs();

