#include "../graph/transit.hpp"
#include "../graph/stop.hpp"
#include "../heatmap.hpp"

#include "../parts/leftPanel.hpp"

static bool isAddingStop = false;
static item lastLegAdded = -1;
static item lastStopAdded = 0;
static item highlightStop = 0;

void resetAddedStop() {
  if (getCurrentLine() != 0 && lastLegAdded >= 0) {
    removeStopFromLineByIndex(getCurrentLine(), lastLegAdded);
  }

  discardAllUnsetPlans();
  lastLegAdded = -1;
  lastStopAdded = 0;
}

void removeLastAddedStop(item stopNdx) {
  if (stopNdx == lastStopAdded) {
    lastLegAdded = -1;
    lastStopAdded = 0;
  }
}

void setHeatmapForTransit() {
  if (currentRoadTab == transitTab) {
    setHeatMap(TransitHeatMap, true);
  } else {
    setHeatMap(Pollution, false);
  }
}

void stopAddingStop() {
  resetAddedStop();
  isAddingStop = false;
  setHeatmapForTransit();
}

void setTransitSystem(item tab) {
  if (tab != getCurrentTransitSystem()) {
    resetAddedStop();
  }

  //if (currentRoadTab != transitTab) setRoadTab(transitTab);
  setCurrentTransitSystem(tab);
  setRoadSystem(transitSystems);
  setHeatmapForTransit();
  elevationSelect(true, &elevation);

  if (tab == 1) {
    reportTutorialUpdate(SelectedBusesInTT);
  } else if (tab == 2) {
    reportTutorialUpdate(SelectedTrainsInTT);
  }

  //if (tab == busTab) {
    //stopBlinkingFeature(FBus);
  //} else if (tab == railTab) {
    //stopBlinkingFeature(FRail);
  //} else if (tab == pedTab) {
    //stopBlinkingFeature(FPedestrian);
  //}
}

item getLastLegAdded() {
  return lastLegAdded;
}

static bool setTransitSystem(Part* part, InputEvent event) {
  item ndx = part->itemData;
  setTransitSystem(ndx);
  return true;
}

bool newTransitSystemWithKey(Part* part, InputEvent event) {

  item ndx = part->itemData;
  item key = event.key;
  setTransitSystem(ndx);
  return true;
}

bool addTransitLine(Part* part, InputEvent event) {
  isAddingStop = true;
  setCurrentLine(addTransitLine(getCurrentTransitSystem()));
  setHeatmapForTransit();
  reportTutorialUpdate(AddedTransitLineInTT);
  return true;
}

bool toggleAddingStop(Part* part, InputEvent event) {
  if (isAddingStop) {
    isAddingStop = false;
    reportTutorialUpdate(StopAddingStopsInTT);
  } else {
    isAddingStop = true;
  }

  setHeatmapForTransit();

  return true;
}

void transit_cursor(item cursorEntityNdx, item textEntityNdx) {
}

void transit_move(InputEvent event) {
  resetAddedStop();

  if (!isAddingStop || getCurrentLine() == 0) {
    stopGuide();
    highlightStop = 0;
    return;
  }

  highlightStop = nearestStop(event.mouseLine);
  Stop* stop = 0;
  if (highlightStop != 0) {
    stop = getStop(highlightStop);
  }

  Configuration config;
  item desiredType = getSystemGraphType(getCurrentTransitSystem());
  config.type = desiredType;
  config.flags = 0;
  if (desiredType != ConfigTypeRoad) {
    config.flags |= _configPlatform;
  }
  GraphLocation loc = graphLocation(event.mouseLine, config);
  if (loc.lane < 10) {
    highlightStop = 0;
    return;
  }

  Lane* lane = getLane(loc.lane);
  LaneBlock* blk = getLaneBlock(loc.lane);
  loc.dap = clamp(loc.dap, c(CTileSize), lane->length - c(CTileSize));
  loc.dap = round(loc.dap/c(CTileSize)) * c(CTileSize);

  if (blk->graphElements[1] > 0) {
    Edge* edge = getEdge(blk->graphElements[1]);
    if (edge->config.type != desiredType) {
      highlightStop = 0;
      return;
    }

    if (desiredType != ConfigTypeRoad &&
        !(edge->config.flags & _configPlatform)) {
      highlightStop = 0;
      return;
    }

  } else {
    highlightStop = 0;
    return;
  }

  vec3 location = getLocation(loc);
  float locMouseDist = pointLineDistance(location, event.mouseLine);
  //float stopMouseDist = 0;
  float stopLocDist = 100000;
  if (stop != 0) {
    stopLocDist = vecDistance(stop->location, location);
    //stopMouseDist = pointLineDistance(stop->location, event.mouseLine);
  }

  if ((stop == 0 && locMouseDist < c(CTileSize)*5) ||
      stopLocDist > 2.5*c(CLaneWidth)) {
      //locMouseDist < stopMouseDist - 0.001) {
    highlightStop = addStop(loc);
    addPlan(StopPlan, highlightStop);
    stop = getStop(highlightStop);

  } else if (highlightStop != 0) {
    TransitLine* tl = getTransitLine(getCurrentLine());
    int tlS = tl->legs.size();
    for (int i = 0; i < 2; i++) {
      item legNdx = getTransitCursor() + i -1;
      if (legNdx < 0 || legNdx >= tlS) continue;
      item stopNdx = tl->legs[legNdx].stop;
      if (stopNdx == highlightStop) {
        highlightStop = 0;
        break;
      }
    }
  }

  if (highlightStop != 0) {
    lastLegAdded = insertStopIntoLine(getCurrentLine(), highlightStop);
    lastStopAdded = highlightStop;
  }

  if (stop != 0) {
    roadCursorLoc = stop->location;
    setGuide(stop->location, c(CStopRadius));
  } else {
    roadCursorLoc = landIntersect(event.mouseLine);
    stopGuide();
  }
}

void transit_click(InputEvent event) {
  bool isRMB = event.button == GLFW_MOUSE_BUTTON_RIGHT;
  if (lastLegAdded >= 0) markRouterDataDirty_g();

  if (isRMB) {
    if (getCurrentLine() == 0) return;
    resetAddedStop();
    TransitLine* tl = getTransitLine(getCurrentLine());
    int sS = getTransitCursor()-1;
    playSound(_soundClickPrimary);

    if (tl->legs.size() == 0) {
      removeTransitLine(getCurrentLine());
    } else if (sS >= 0) {
      removeStopFromLineByIndex(getCurrentLine(), sS);
    }

  } else {
    lastLegAdded = -1;
    lastStopAdded = 0;
    buyAllUnsetPlans();
    playSound(_soundClickCancel);
    reportTutorialUpdate(BuildTransitStop);
  }
}

void transit_select() {
  setLeftPanel(TransitPanel);
  elevationSelect(true, &elevation);
  isAddingStop = false;
  lastLegAdded = -1;
  lastStopAdded = 0;
  setHeatmapForTransit();
}

void transit_reset() {
  resetAddedStop();
  stopGuide();
  if (getHeatMap() == TransitHeatMap) {
    setHeatMap(Pollution, false);
  }
  if (getLeftPanel() == TransitPanel) setLeftPanel(NoPanel);
  highlightStop = 0;
}

bool transit_isVisible() {
  return true;
}

void transit_render(Part* pnl) {
  if (getCurrentTransitSystem() != 0) {
    TransitSystem* system = getTransitSystem(getCurrentTransitSystem());
    if ((system->flags & _transitExists) &&
        (system->flags & _transitComplete)) {

      Part* newLineButt = r(pnl,
          superButton(vec2(2.f, 1.5f),
            vec2(7, .9f), iconAddTransitLine, strdup_s("Add Transit Line"),
            addTransitLine, 0, ActTTAddTransitLine, false));
      setPartTooltipValues(newLineButt, TooltipType::TransNewLine);

      if (getCurrentLine() != 0) {
        Part* addingButt = r(pnl,
            superButton(vec2(2.f, 2.5f),
              vec2(7, .9f), iconAddBusStop, strdup_s("Add Stops to Line"),
              toggleAddingStop, 0, ActTTAddStopsToLine, isAddingStop));
        setPartTooltipValues(addingButt, TooltipType::TransAddStops);
      }
      return;
    }
  }

  r(pnl, labelCenter(vec2(1,2.5f), vec2(8,.85),
        strdup_s("Select a transportation")));
  r(pnl, labelCenter(vec2(1,3.5f), vec2(8,.85),
        strdup_s("system below.")));
}

void transit_instructionPanel(Part* pnl) {
}

