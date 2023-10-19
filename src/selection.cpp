#include "selection.hpp"

#include "building/building.hpp"
#include "building/design.hpp"
#include "building/renderBuilding.hpp"
#include "business.hpp"
#include "draw/entity.hpp"
#include "game/game.hpp"
#include "graph.hpp"
#include "graph/stop.hpp"
#include "graph/transit.hpp"
#include "graph/transitRouter.hpp"
#include "heatmap.hpp"
#include "lot.hpp"
#include "person.hpp"
#include "pillar.hpp"
#include "renderGraph.hpp"
#include "renderLot.hpp"
#include "renderSelection.hpp"
#include "tutorial.hpp"
#include "vehicle/renderVehicle.hpp"
#include "vehicle/travelGroup.hpp"
#include "vehicle/vehicle.hpp"

#include "parts/designConfigPanel.hpp"
#include "parts/designOrganizerPanel.hpp"
#include "parts/leftPanel.hpp"
#include "parts/messageBoard.hpp"
#include "parts/root.hpp"
#include "parts/selectionPanel.hpp"

#include "tools/query.hpp"

#include "spdlog/spdlog.h"

item selected, selectionType;
item selectionEntity;
vector<item> lastSelectionLanes;
bool followingSelection = false;

void resetSelection() {
  lastSelectionLanes.clear();
  selectionEntity = 0;
  clearSelection();
  setFixBuildingMode(false);
}

void setHighlight(bool highlight, item querySelectionType,
    item querySelection
) {
  if (querySelection == 0) {
    return;
  }

  if (querySelectionType == SelectionGraphElement) {
    setElementHighlight(querySelection, highlight);
  } else if (querySelectionType == SelectionVehicle) {
    setVehicleHighlight(querySelection, highlight);
  } else if (querySelectionType == SelectionBuilding) {
    setBuildingHighlight(querySelection, highlight);
  } else if (querySelectionType == SelectionLot) {
    setLotHighlight(querySelection, highlight);
  } else if (querySelectionType == SelectionPillar) {
    setPillarHighlight(querySelection, highlight);
  } else if (querySelectionType == SelectionStop) {
    setStopHighlight(querySelection, highlight);
  }
}

void setRedHighlight(bool highlight, item selectionType, item selection) {
  if (selection == 0) {
    return;
  }

  setHighlight(highlight, selectionType, selection);

  if (selectionType == SelectionPillar) {
    Pillar* pillar = getPillar(selection);
    setEntityRedHighlight(pillar->entity, highlight);
  } else if (selectionType == SelectionBuilding) {
    setBuildingRedHighlight(selection, highlight);
  } else if (selectionType == SelectionStop) {
    setStopRedHighlight(selection, highlight);
  } else if (selection < 0) {
    Node* node = getNode(selection);
    setEntityRedHighlight(node->entity, highlight);
  } else {
    Edge* edge = getEdge(selection);
    setEntityRedHighlight(edge->entity, highlight);
  }
}

void renderSelection() {
  if (selectionType == NoSelection) {
    if (selectionEntity != 0) {
      setEntityVisible(selectionEntity, false);
    }
  } else {
    if (selectionEntity == 0) {
      selectionEntity = addEntity(PaletteShader);
    }
    bool hideUI = getMenuMode() == HideUI;
    setEntityVisible(selectionEntity, !hideUI);
    if (!hideUI) {
      renderSelection(selectionEntity, selectionType, selected,
          lastSelectionLanes);
    }
  }
}

void clearSelection() {
  setSelection(NoSelection, 0);
}

void setSelection(item st, item element, bool andDisableVehicleFollowMode) {
  if (andDisableVehicleFollowMode) {
    stopVehicleFollowMode();
  }
  if (element == 0 && st != SelectionStructure && st != SelectionDeco) {
    st = NoSelection;

  } else if (st == SelectionVehicle) {
    /*
    Vehicle* v = getVehicle(element);
    if (v->trailing != 0) {
      setSelection(SelectionVehicle, v->trailing, false);
      return;
    }
    */
  }

  clearSelectionPanel();
  setFixBuildingMode(false);
  setHighlight(false, selectionType, selected);
  selected = element;
  selectionType = st;
  bool hideUI = getMenuMode() == HideUI;

  if (selectionType == NoSelection) {
    if (selectionEntity != 0) setEntityVisible(selectionEntity, false);
    followingSelection = false;

  } else {
    if (selectionEntity != 0) setEntityVisible(selectionEntity, !hideUI);
    setLeftPanel(NoPanel);
    followingSelection = selectionType != SelectionStructure &&
      selectionType != SelectionDeco;
    setHighlight(!hideUI, selectionType, selected);

    if (selectionType == SelectionPerson) {
      reportTutorialUpdate(TutorialUpdateCode::QueryPerson);
    } else if (selectionType == SelectionBuilding) {
      reportTutorialUpdate(TutorialUpdateCode::QueryBuilding);
    } else if (selectionType == SelectionBusiness) {
      reportTutorialUpdate(TutorialUpdateCode::QueryBusiness);
    }
  }

  if (getHeatMap() == TransitHeatMap) {
    paintTransit();
  }

  if (getGameMode() == ModeBuildingDesigner) {
    designRender();
    if (getSelectionType() != NoSelection) {
      designConfigPanelSelection();
    }
  }
}

void setSelection(item st, item element) {
  setSelection(st, element, true);
}

void deselect(item st, item element) {
  if (selectionType == st && selected == element) {
    setSelection(NoSelection, 0, false);
  }
}

void selectLocation(Location loc) {
  SPDLOG_INFO("selectLocation {} {}",
      locationType(loc), locationNdx(loc));
  if (loc < 10) return;

  item type = locationType(loc);
  if (type == LocLaneBlock) {
    LaneBlock* blk = getLaneBlock(loc);
    setSelection(SelectionGraphElement, blk->graphElements[1]);
  } else if (type == LocTransitLeg) {
    setSelection(SelectionStop, getStopForLeg_r(loc));
  } else if (type == LocTransitStop) {
    setSelection(SelectionStop, locationNdx(loc));
  } else if (type == LocVehicle) {
    setSelection(SelectionVehicle, locationNdx(loc));
  } else if (type == LocTravelGroup) {
    TravelGroup* tg = getTravelGroup_g(locationNdx(loc));
    selectLocation(tg->location);
  } else if (type == LocBuilding) {
    setSelection(SelectionBuilding, locationNdx(loc));
  }
}

void setFollowingSelection(bool follow) {
  followingSelection = follow;
}

bool isFollowingSelection() {
  return followingSelection;
}


void updateSelection(double duration) {
  bool hideUI = getMenuMode() == HideUI;
  setHighlight(!hideUI, selectionType, selected);
  lastSelectionLanes.clear();

  /*
  if (selectionType == SelectionVehicle) {
    Vehicle* vehicle = getVehicle(selected);
    //lastSelectionLanes = vehicle->route.steps;
    //if (lastSelectionLanes.size() > 0) {
      //lastSelectionLanes.erase(lastSelectionLanes.begin());
    //}

  } else
  */
  if (selectionType == SelectionGraphElement) {
    setElementHighlight(selected, !hideUI);
    if (selected < 0) {
      lastSelectionLanes = getNode(selected)->laneBlocks;
    } else if (selected > 0) {
      Edge* edge = getEdge(selected);
      for (int i = 0; i < 2; i++) {
        item ndx = edge->laneBlocks[i];
        if (ndx != 0) {
          lastSelectionLanes.push_back(ndx);
        }
      }
    }

    /*
  } else if (selectionType == SelectionPerson) {
    Person* person = getPerson(selected);
    if (person->flags & _personTraveling) {
      //lastSelectionLanes = getVehicle(person->location)->route;
      //if (lastSelectionLanes.size() > 0) {
        //lastSelectionLanes.erase(lastSelectionLanes.begin());
      //}
    }
    */

  } else if (selectionType == SelectionBuilding) {
    Building* building = getBuilding(selected);
    if (building->graphLoc.lane != 0) {
      lastSelectionLanes.push_back(building->graphLoc.lane);
    }

  } else if (selectionType == SelectionStop) {
    Stop* stop = getStop(selected);
    if (stop->graphLoc.lane != 0) {
      lastSelectionLanes.push_back(stop->graphLoc.lane);
    }

  } else if (selectionType == SelectionLaneBlock) {
    lastSelectionLanes.push_back(selected);
  }

  renderSelection();

  if (followingSelection) {
    vec3 loc = getSelectionLocation();
    if (loc.x != 0 || loc.y != 0 || loc.z != 0) {
      setCameraTarget(getSelectionLocation());
    }
  }
}

item getSelectionType() { return selectionType; }
item getSelection() { return selected; }
bool isSelected(item st, item element) {
  return selectionType == selectionType && selected == element;
}

bool isMonitored(item st, item element) {
  if (isSelected(st, element)) return true;
  MessageType mt =
    st == SelectionPerson ? PersonMessage :
    st == SelectionBusiness ? BusinessMessage :
    st == SelectionBuilding ? BuildingMessage :
    st == SelectionGraphElement ? GraphMessage :
    st == SelectionVehicle ? VehicleMessage :
    (MessageType)-1;
  if (mt < 0) return false;
  return hasMessage(mt, element);
}

vec3 getSelectionLocation() {
  if (selectionType == SelectionVehicle) {
    return getVehicleCenter_g(selected);
  } else if (selectionType == SelectionBuilding) {
    return getBuildingTop(selected);
  } else if (selectionType == SelectionGraphElement) {
    return getElementLoc(selected);

  } else if (selectionType == SelectionPerson) {
    Person* person = getPerson(selected);
    if (person->location == 0) {
      return vec3(0,0,0);
    } else if (person->flags & _personTraveling) {
      return locationToWorldspace_g(person->location);
    } else {
      return getBuildingTop(person->location);
    }

  } else if (selectionType == SelectionFamily) {
    item home = getFamily(selected)->home;
    if (home == 0) {
      return vec3(0,0,0);
    } else {
      return getBuildingTop(home);
    }

  } else if (selectionType == SelectionBusiness) {
    item building = getBusiness(selected)->building;
    if (building == 0) {
      return vec3(0,0,0);
    } else {
      return getBuildingTop(building);
    }

  } else if (selectionType == SelectionStop) {
    Stop* stop = getStop(selected);
    return stop->location;

  } else if (selectionType == SelectionLaneBlock) {
    vec3 loc(0,0,0);
    LaneBlock* blk = getLaneBlock(selected);
    if (blk->numLanes > 0) {
      for (int i = 0; i < blk->numLanes; i++) {
        for (int j = 0; j < 2; j++) {
          loc += blk->lanes[i].ends[j];
        }
      }
      loc /= blk->numLanes*2;
    }
    return loc;

  } else if (selectionType == SelectionLot) {
    Lot* lot = getLot(selected);
    return lot->loc + tileSize*.5f*lot->normal;

  } else if (selectionType == SelectionStructure) {
    return getStructureHandleLocation(selected, SelectionPointHandle);

  } else if (selectionType == SelectionDeco) {
    return getDecoHandleLocation(selected, SelectionPointHandle);

  } else {
    return vec3(-1, -1, -1);
  }
}
