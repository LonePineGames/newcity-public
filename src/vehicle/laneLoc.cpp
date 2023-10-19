#include "laneLoc.hpp"

#include "vehicle.hpp"
#include "physics.hpp"
#include "update.hpp"
#include "../cup.hpp"
#include "../game/game.hpp"
#include "../option.hpp"

#include "spdlog/spdlog.h"

struct LaneLoc {
  GraphLocation loc;
  item ahead;
  item behind;
};

Cup<LaneLoc> laneLocs;
Cup<item> firstLaneLocs;
Cup<item> numInLane;
Cup<item> numInBlockBack;
Cup<item> numInBlock;

item compactLaneNdx(item lane) {
  return (lane/10)*5 + lane%10;
}

item numVehiclesInLaneBack(item lane) {
  item ndx = compactLaneNdx(lane);
  numInLane.ensureSize(ndx+1);
  return numInLane[ndx];
}

item numVehiclesInBlock_v(item blkNdx) {
  item ndx = blkNdx/10;
  if (numInBlockBack.size() <= ndx) {
    return 0;
  }
  return numInBlockBack[ndx];
}

item numVehiclesInBlock(item blkNdx) {
  item ndx = blkNdx/10;
  if (numInBlock.size() <= ndx) {
    return 0;
  }
  return numInBlock[ndx];
}

LaneLocInfo getLaneLoc(item vNdx, LaneLocType type) {
  LaneLocInfo result;

  if (vNdx == 0) {
    result.vNdx = 0;
    result.type = type;
    result.ahead = 0;
    result.aheadType = type;
    result.loc.lane = 0;
    result.loc.dap = 0;

  } else {
    LaneLoc ll = laneLocs[vNdx*llnum+type];
    result.vNdx = vNdx;
    result.type = type;
    result.ahead = ll.ahead/llnum;
    result.aheadType = (LaneLocType)(ll.ahead%llnum);
    result.loc = ll.loc;
  }

  return result;
}

LaneLocInfo getLaneLoc(item vNdx) {
  return getLaneLoc(vNdx/llnum, (LaneLocType)(vNdx%llnum));
}

item fllGet(item lane) {
  item ndx = compactLaneNdx(lane);
  firstLaneLocs.ensureSize(ndx+1);
  return firstLaneLocs[ndx];
}

void fllSet(item lane, item llNdx) {
  item ndx = compactLaneNdx(lane);
  firstLaneLocs.ensureSize(ndx+1);
  firstLaneLocs.set(ndx, llNdx);
}

LaneLocInfo firstInLane(item lane) {
  return getLaneLoc(fllGet(lane));
}

void llValidate(item llNdx) {
  #ifdef LP_DEBUG
  if (llNdx != 0 && debugMode()) {
    LaneLoc ll = laneLocs[llNdx];
    item ahead = ll.ahead;

    if (ll.ahead == llNdx) {
      SPDLOG_ERROR("llValidate: {} ahead of itself", llNdx);
      selectAndPauseBack(llNdx/llnum);
    }

    int depth = 0;
    while (ahead != 0) {
      depth ++;
      LaneLoc lla = laneLocs[ahead];
      if (ll.ahead == llNdx) {
        SPDLOG_ERROR("llValidate: {} in a loop {} deep",
            llNdx, depth);
        selectAndPauseBack(llNdx/llnum);
        break;
      }
      ahead = lla.ahead;
    }

    if (ll.behind == llNdx) {
      SPDLOG_ERROR("llValidate: {} behind itself", llNdx);
      selectAndPauseBack(llNdx/llnum);
    }

    if (ll.loc.lane == 0) {
      if (ll.behind != 0) {
        SPDLOG_ERROR("llValidate: {} ll.loc.lane==0 but ll.behind=={}",
            llNdx, ll.behind);
        selectAndPauseBack(llNdx/llnum); //, "Not in lane but has behind");
      }

      if (ll.ahead != 0) {
        SPDLOG_ERROR("llValidate: {} ll.loc.lane==0 but ll.ahead=={}",
            llNdx, ll.ahead);
        selectAndPauseBack(llNdx/llnum); //, "Not in lane but has ahead");
      }

    } else {
      if (ll.behind == 0) {
        if (fllGet(ll.loc.lane) != llNdx) {
          SPDLOG_ERROR("llValidate: {} behind==0 but fllGet({})=={}",
              llNdx, ll.loc.lane, fllGet(ll.loc.lane));
          selectAndPauseBack(llNdx/llnum);
            //"Nothing behind but not first vehicle");
        }

      } else {
        LaneLoc behind = laneLocs[ll.behind];
        if (behind.ahead != llNdx) {
          SPDLOG_ERROR("llValidate: {} behind=={} but {} ahead=={}",
              llNdx, ll.behind, ll.behind, behind.ahead);
          selectAndPauseBack(llNdx/llnum); //, "Bad vehicle behind");
        }
      }

      if (ll.ahead != 0) {
        LaneLoc ahead = laneLocs[ll.ahead];
        if (ahead.behind != llNdx) {
          SPDLOG_ERROR("llValidate: {} ahead=={} but {} behind=={}",
              llNdx, ll.ahead, ll.ahead, ahead.behind);
          selectAndPauseBack(llNdx/llnum); //, "Bad vehicle ahead");

        }
      }
    }
  }
  #endif
}

item llNear(GraphLocation target) {
  item llNdx = fllGet(target.lane);
  if (llNdx == 0) return 0;
  float dap = target.dap;

  while (true) {
    LaneLoc ll = laneLocs[llNdx];
    if (ll.ahead == 0 || ll.loc.dap >= dap) return llNdx;
    llNdx = ll.ahead;
  }
}

void llLink(item one, item two) {
  if (one == two && one != 0) {
	  handleError("laneLoc llLink(): one == two && one != 0");
  }

  //SPDLOG_INFO("llLink {} {}", one, two);
  if (one != 0) {
    LaneLoc ll = laneLocs[one];
    ll.ahead = two;
    laneLocs.set(one, ll);
  }

  if (two != 0) {
    LaneLoc ll = laneLocs[two];
    ll.behind = one;
    laneLocs.set(two, ll);
  }
}

void llInject(item one, item two, item three) {
  llLink(one, two);
  llLink(two, three);
}

void updateLaneCounts(item llNdx, item lane, item delta) {
  if (lane == 0) return;

  item type = llNdx%llnum;
  item otNdx = llNdx - type + !type;
  LaneLoc ot = laneLocs[otNdx];

  if (ot.loc.lane != lane) {
    item ndx = compactLaneNdx(lane);
    numInLane.ensureSize(ndx+1);
    numInLane.set(ndx, numInLane[ndx] + delta);
  }

  if (ot.loc.lane/10 != lane/10) {
    item ndx = lane/10;
    numInBlockBack.ensureSize(ndx+1);
    numInBlockBack.set(ndx, numInBlockBack[ndx] + delta);
  }
}

void llRemove(item llNdx) {
  laneLocs.ensureSize(llNdx+1);
  LaneLoc ll = laneLocs[llNdx];
  //SPDLOG_INFO("llRemove {} {} {}", llNdx, ll.loc.lane, ll.loc.dap);

  updateLaneCounts(llNdx, ll.loc.lane, -1);
  if (fllGet(ll.loc.lane) == llNdx) {
    fllSet(ll.loc.lane, ll.ahead);
  }

  llLink(ll.behind, ll.ahead);
  llValidate(ll.behind);
  llValidate(ll.ahead);
  llInject(0, llNdx, 0);

  ll = laneLocs[llNdx];
  ll.loc.lane = 0;
  ll.loc.dap = 0;
  laneLocs.set(llNdx, ll);
  llValidate(llNdx);
}

void llAdd(item llNdx, GraphLocation target) {
  llRemove(llNdx);
  //SPDLOG_INFO("llAdd {} {} {}", llNdx, target.lane, target.dap);

  LaneLoc ll = laneLocs[llNdx];
  ll.loc = target;
  laneLocs.set(llNdx, ll);
  updateLaneCounts(llNdx, target.lane, 1);

  if (ll.loc.lane == 0) {
    return;
  }

  item nearNdx = llNear(target);
  item behind = 0;
  item ahead = 0;

  if (nearNdx != 0) {
    LaneLoc lnear = laneLocs[nearNdx];
    if (lnear.loc.dap < target.dap) {
      behind = nearNdx;
      ahead = lnear.ahead;
    } else {
      behind = lnear.behind;
      ahead = nearNdx;
    }
  }

  llInject(behind, llNdx, ahead);
  if (behind == 0) {
    fllSet(target.lane, llNdx);
  }

  llValidate(llNdx);
  llValidate(ll.behind);
  llValidate(ll.ahead);
}

void enterLane(item vNdx, GraphLocation target) {
  llAdd(vNdx*llnum, target);
  llAdd(vNdx*llnum+1, target);
}

LaneLocInfo moveInLane(item vNdx, LaneLocType type, float dist) {
  LaneLoc ll = laneLocs[vNdx*llnum+type];
  ll.loc.dap += dist;
  laneLocs.set(vNdx*llnum+type, ll);
  return getLaneLoc(vNdx, type);
}

LaneLocInfo moveToLane(item vNdx, LaneLocType type, GraphLocation target) {
  llAdd(vNdx*llnum+type, target);
  return getLaneLoc(vNdx, type);
}

LaneLocInfo moveTo(item vNdx, LaneLocType type, GraphLocation target) {
  item llndx = vNdx*llnum+type;
  LaneLoc ll = laneLocs[llndx];
  if (ll.loc.lane == target.lane) {
    ll.loc.dap = target.dap;
    laneLocs.set(llndx, ll);
  } else {
    llAdd(llndx, target);
  }
  return getLaneLoc(vNdx, type);
}

void leaveLane(item vNdx) {
  llRemove(vNdx*llnum);
  llRemove(vNdx*llnum+1);
}

LaneLocInfo getVehicleAhead(item vNdx, LaneLocType type) {
  item ahead = laneLocs[vNdx*llnum+type].ahead;
  if (ahead/llnum == vNdx) {
    return getVehicleAhead(vNdx, (LaneLocType)(ahead%llnum));
  } else {
    return getLaneLoc(ahead);
  }
}

LaneLocInfo getNearestLaneLoc(GraphLocation target) {
  return getLaneLoc(llNear(target));
}

void swapLaneLocs() {
  numInBlockBack.copy(&numInBlock);
}

void removeLaneVehiclesActual(item lane) {
  item ndx = compactLaneNdx(lane);
  firstLaneLocs.ensureSize(ndx+1);
  item llNdx = firstLaneLocs[ndx];
  while (llNdx != 0) {
    removeVehicleBack(llNdx/llnum);
    llNdx = firstLaneLocs[ndx];
  }
}

void resizeLaneLocs(item num) {
  laneLocs.ensureSize(num);
}

void resetLaneLocs() {
  laneLocs.clear();
  firstLaneLocs.clear();
  numInLane.clear();
  numInBlockBack.clear();
  numInBlock.clear();
}

