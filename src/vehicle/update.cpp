#include "update.hpp"

#include "../graph/transit.hpp"
#include "../graph/transitRouter.hpp"
#include "../game/constants.hpp"
#include "../game/game.hpp"
#include "../game/task.hpp"
#include "../game/update.hpp"
#include "../money.hpp"
#include "../option.hpp"
#include "../thread.hpp"

#include "interpolator.hpp"
#include "laneLoc.hpp"
#include "physics.hpp"
#include "transitPhysics.hpp"
#include "travelGroup.hpp"
#include "vehicle.hpp"
#include "wanderer.hpp"

#include "spdlog/spdlog.h"

#include <atomic>
#include <vector>
using namespace std;

enum VehicleCommandType {
  VCReset, VCKill, VCSwapBack, VCSwap, VCUpdate, VCRemoveLane,
  VCAdd, VCRemove, VCFree, VCRecieveRoute, VCSelectAndPause,
  VCReroute, VCInvalidateRoute, VCAddGroup, VCRemoveGroup,
  numVehicleCommandTypes
};

const char* commandName[] = {
  "VCReset", "VCKill", "VCSwapBack", "VCSwap", "VCUpdate", "VCRemoveLane",
  "VCAdd", "VCRemove", "VCFree", "VCRecieveRoute", "VCSelectAndPause",
  "VCReroute", "VCInvalidateRoute", "VCAddGroup", "VCRemoveGroup",
  "VCInvalid", "VCInvalid", "VCInvalid"
};

struct VehicleCommand {
  VehicleCommandType type;
  item ndx;
  //Route route;

  VehicleCommand(VehicleCommandType t, item n):
    type(t), ndx(n) {}
};

vector<VehicleCommand> nextCommands;
vector<VehicleCommand> backCommands;

static double vehicleDurationRemaining = 0;
static atomic<bool> vehicleMemoryBarrier(false);
static atomic<bool> resetVehiclesFlag(false);
static atomic<bool> killVehiclesFlag(false);
static atomic<bool> isVehicleThreadWaiting(false);
static atomic<bool> doFullSwap(true);
static double vehicleTime = 0;
static atomic<int> updatesPending(0);
static int framesSinceSwap = 0;
static int framesSinceFullSwap = 0;

void swapVehicles();

int getVehicleUpdatesPending() {
  return updatesPending;
}

double getVehicleTime() {
  return vehicleTime;
}

void validateSwapVehicles(char* msg) {
  validateVehicles(msg);
  validatePhysicalVehicles(msg);
  validateSwapPhysicalVehicles(msg);
}

void runVehicleBackCommand(VehicleCommand command) {
  if (resetVehiclesFlag) {
    return;
  }
  //SPDLOG_INFO("runVehicleCommandBack {} {}",
      //commandName[command.type], command.ndx);

  if (command.type == VCRemove) {
    removeVehicle(command.ndx);
  } else if (command.type == VCFree) {
    //freeVehicle(command.ndx);
  } else if (command.type == VCSelectAndPause) {
    selectVehicleAndPause(command.ndx);
  } else if (command.type == VCReroute) {
    rerouteVehicle(command.ndx);
  } else if (command.type == VCRemoveGroup) {
    //SPDLOG_WARN("removeTravelGroup_g (command) {}", command.ndx);
    removeTravelGroup_g(command.ndx);
  } else if (command.type == VCInvalidateRoute) {
    invalidateVehicleRoute(command.ndx);
  }
}

void runVehicleCommand(item ndx) {
  VehicleCommand* command = &nextCommands[ndx];
  if (resetVehiclesFlag && command->type != VCReset) {
    return;
  }
  //SPDLOG_INFO("runVehicleCommand {} {}",
      //commandName[command->type], command->ndx);

  if (command->type == VCReset) {
    updatesPending = 0;
    resetVehiclePhysics();
    resetTransitPhysics();
    backCommands.clear();

  } else if (command->type == VCUpdate) {
    //updatesPending += command->ndx;

  } else if (command->type == VCSwap) {
    //swapVehicles();

  } else if (command->type == VCSwapBack) {
    swapVehiclePhysicsBack();
    swapTransitPhysics(true);

  } else if (command->type == VCAdd) {
    addPhysicalVehicle(command->ndx);

  } else if (command->type == VCRemove) {
    removePhysicalVehicle(command->ndx);
    backCommands.push_back(VehicleCommand(VCFree, command->ndx));

  } else if (command->type == VCAddGroup) {
    swapOneTravelGroup(command->ndx, true);

  } else if (command->type == VCRecieveRoute) {
    recieveVehicleRoute(command->ndx); //, &command->route);
    //clearRoute(&command->route);

  } else if (command->type == VCRemoveLane) {
    removeLaneVehiclesActual(command->ndx);
  }
}

void vehicleLoop() {
  resetVehiclesFlag = false;
  killVehiclesFlag = false;

  while (!killVehiclesFlag) {
    if (vehicleMemoryBarrier) {
      swapVehicles();
    }
    isVehicleThreadWaiting = false;

    for (int i = 0; i < 10 && updatesPending > 0; i++) {
      oneVehicleUpdate();
      updatesPending --;
    }
    sleepMilliseconds(1);

    //if (updatesPending == 0) {
      isVehicleThreadWaiting = true;
    //}
  }

  nextCommands.clear();
  backCommands.clear();
}

void swapVehicles() {
  if (!resetVehiclesFlag && doFullSwap) {
    #ifdef LP_DEBUG
      if (debugMode()) validateSwapVehicles("before swapVehicles()");
    #endif

    swapVehiclePhysics();
    //logStage("swapLaneLocs");
    swapLaneLocs();
    swapTransitPhysics(false);
    //logStage("swapTransitStopData_v");
    swapTransitStopData_v();

    /*
    #ifdef LP_DEBUG
      if (debugMode()) validateSwapVehicles("before commands swapVehicles()");
    #endif
    */
  }

  if (!doFullSwap) {
    //swapVehicleKeyframe();
  }

  //logStage("run vehicle commands");
  while (nextCommands.size() > 0 || backCommands.size() > 0) {
    for (int i = 0; i < nextCommands.size(); i++) {
      runVehicleCommand(i);
    }
    nextCommands.clear();

    for (int i = 0; i < backCommands.size(); i++) {
      runVehicleBackCommand(backCommands[i]);
    }
    backCommands.clear();
  }

  //logStage("freeTravelGroups");
  freeTravelGroups();
  vehicleMemoryBarrier = false;
}

void swapVehiclesCommand(bool doFull) {
  if (!doFull && !isVehicleThreadWaiting) return;

  doFullSwap = doFull;
  if (doFull) framesSinceFullSwap = 0;
  framesSinceSwap = 0;
  vehicleMemoryBarrier = true;
  while (vehicleMemoryBarrier && !killVehiclesFlag) {
    sleepMilliseconds(1);
  }
}

void swapVehiclesCommand() {
  swapVehiclesCommand(true);
}

void updateVehicles(double duration) {
  //logStage("updateVehicles bookkeeping");
  framesSinceSwap ++;
  framesSinceFullSwap ++;
  vehicleTime += duration;
  vehicleDurationRemaining += duration;

  int numUpdates = vehicleDurationRemaining / c(CVehicleUpdateTime);
  updatesPending += numUpdates;
  nextCommands.push_back(VehicleCommand(VCUpdate, numUpdates));
  bool doSwap = numUpdates > 0;
  vehicleDurationRemaining -= c(CVehicleUpdateTime) * numUpdates;

  //while (vehicleDurationRemaining > c(CVehicleUpdateTime)) {
    //vehicleDurationRemaining -= c(CVehicleUpdateTime);
    //nextCommands.push_back(VehicleCommand(VCUpdate, 0));
    //doSwap = true;
  //}

  doSwap = (doSwap && framesSinceSwap > 10); // || nextCommands.size() > 0;

  if (doSwap) {
    /*
    SPDLOG_INFO("swapVehicles {}", framesSinceSwap, framesSinceFullSwap);
    //logStage("vehicle swap start");
    swapVehiclesCommand(false); //framesSinceFullSwap > 30);
    //logStage("vehicle swap end");
    */
    if (framesSinceFullSwap > 30) {
      queueTask_g(TaskFullVehicleSwap, 0);
    } else {
      queueTask_g(TaskPartialVehicleSwap, 0);
    }
  }

  logStage("updateVehicles interpolate");
  updateWanderers_g(duration);
  logStage("updateVehicles interpolate");
  interpolateVehicles(duration);
  //logStage("updateVehicles place");
  //placeVehicles();
  logStage("updateVehicles settle");
  settleVehicles();
}

void resetVehiclesCommand() {
  resetVehiclesFlag = true;
  nextCommands.clear();
  swapVehiclesCommand();
  swapVehiclesCommand();
  nextCommands.push_back(VehicleCommand(VCReset, 0));
  sleepMilliseconds(100);
  swapVehiclesCommand();
  sleepMilliseconds(100);
  swapVehiclesCommand();
  sleepMilliseconds(100);
  framesSinceSwap = 10;
  framesSinceFullSwap = 30;
  resetVehiclesFlag = false;
}

void swapVehiclesBack() {
  nextCommands.push_back(VehicleCommand(VCSwapBack, 0));
}

void addVehicleCommand(item ndx) {
  nextCommands.push_back(VehicleCommand(VCAdd, ndx));
}

void addTravelGroupCommand(item ndx) {
  nextCommands.push_back(VehicleCommand(VCAddGroup, ndx));
}

void removeTravelGroupCommand_v(item ndx) {
  //SPDLOG_WARN("removeTravelGroupCommand_v {}", ndx);
  backCommands.push_back(VehicleCommand(VCRemoveGroup, ndx));
}

void removeVehicleCommand(item ndx) {
  nextCommands.push_back(VehicleCommand(VCRemove, ndx));
}

void removeVehicleCommandBack(item ndx) {
  backCommands.push_back(VehicleCommand(VCRemove, ndx));
}

void rerouteCommandBack(item ndx) {
  backCommands.push_back(VehicleCommand(VCReroute, ndx));
}

void invalidateRouteCommandBack(item ndx) {
  backCommands.push_back(VehicleCommand(VCInvalidateRoute, ndx));
}

void finishVehicleRoute(item ndx, Route* route) {
  if (route->steps.size() == 0) {
    removeVehicle(ndx);
    return;
  }

  //SPDLOG_INFO("finishVehicleRoute {}", ndx);
  VehicleCommand comm(VCRecieveRoute, ndx);
  nextCommands.push_back(comm);
  //copyRoute(route, &nextCommands[nextCommands.size()-1].route);

  Vehicle* v = getVehicle(ndx);
  copyRoute(route, &v->route);
  if ((v->flags & _vehicleIsTransit) && v->transitLine > 0) {
    updateTransitRoute_g(v->transitLine, route);
  }
}

void removeLaneVehicles(item ndx) {
  nextCommands.push_back(VehicleCommand(VCRemoveLane, ndx));
}

void selectAndPauseBack(item ndx) {
  backCommands.push_back(VehicleCommand(VCSelectAndPause, ndx));
}

void killVehicles() {
  resetVehiclesFlag = true;
  nextCommands.clear();
  //nextCommands.push_back(VehicleCommand(VCKill, 0));
  killVehiclesFlag = true;
  vehicleMemoryBarrier = true;
  sleepMilliseconds(100);
}

