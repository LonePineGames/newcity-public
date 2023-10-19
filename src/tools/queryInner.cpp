
SelectionTypes querySelectionType = NoSelection;
item querySelection = 0;
item vehicleSelection = 0;
float vehicleSelectionTime = 0;
static bool isVehicleFollowMode = false;
static bool isQueryMode = true;

void setQueryHighlight(bool highlight) {
  if (querySelectionType != getSelectionType() ||
    querySelection != getSelection()
  ) {
    setHighlight(highlight, querySelectionType, querySelection);
  }
}

void query_inner_mouse_button_callback(InputEvent event) {
  if (!isQueryMode) return;

  int button = event.button;
  int action = event.action;

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    playSound(_soundClickCancel);
    setSelection(querySelectionType, querySelection);
  }
}

void query_inner_mouse_move_callback(InputEvent event) {
  setQueryHighlight(false);

  if (isVehicleFollowMode) {
    if (getSelection() == 0) {
      //setSelection(SelectionVehicle, getRandomVehicle());
      if (sizePeople() > 0) {
        setSelection(SelectionPerson, randItem(sizePeople())+1);
      }

    } else if (!isFollowingSelection()) {
      isVehicleFollowMode = false;

    } else if (getSelectionType() == SelectionVehicle) {
      Vehicle* vehicle = getVehicle(getSelection());
      float angle = pi_o*3/2 - vehicle->yaw;
      if (angle < 0) {
        angle += pi_o*2;
      }
      setHorizontalCameraAngle(angle);
    }
    return;
  }

  if (!isQueryMode) return;

  Line mouseLine = event.mouseLine;
  float minDist = tileSize;
  querySelection = 0;
  querySelectionType = NoSelection;

  item vehicleNdx = nearestVehicle(mouseLine);
  if (vehicleNdx > 0) {
    Vehicle* vehicle = getVehicle(vehicleNdx);
    // Selecting vehicles is preferred
    float dist = pointLineDistance(vehicle->location, mouseLine);
    if (dist < tileSize*.5f) {
      if (vehicleSelection != vehicleNdx) {
        vehicleSelection = vehicleNdx;
        vehicleSelectionTime = getCameraTime();
      }

      if (vehicleSelectionTime < getCameraTime() - 0.1) {
        minDist = dist*.25f;
        querySelection = vehicleSelection;
        querySelectionType = SelectionVehicle;
      }
    }
  }

  item buildingNdx = nearestBuilding(mouseLine);
  if (buildingNdx > 0) {
    vec3 landPoint = landIntersect(mouseLine);
    float dist = boxDistance(getBuildingBox(buildingNdx), landPoint);
    if (dist < minDist) {
      minDist = dist;
      querySelection = buildingNdx;
      querySelectionType = SelectionBuilding;
    }
  }

  item stopNdx = nearestStop(mouseLine);
  if (stopNdx > 0) {
    Stop* s = getStop(stopNdx);
    float dist = pointLineDistance(s->location, mouseLine) *.25f;
    if (dist < minDist) {
      querySelection = stopNdx;
      querySelectionType = SelectionStop;
      minDist = dist;
    }
  }

  item graphElemNdx = nearestElement(mouseLine, true);
  if (graphElemNdx != 0) {
    float dist = lineDistance(getLine(graphElemNdx), mouseLine);
    if (dist < minDist) {
      minDist = dist;
      querySelection = graphElemNdx;
      querySelectionType = SelectionGraphElement;
    }
  }

  if (areLotsVisible()) {
    item lotNdx = nearestLot(mouseLine);
    if (lotNdx > 0) {
      Lot* lot = getLot(lotNdx);
      float dist = pointLineDistance(lot->loc, mouseLine);
      if (dist < minDist) {
        minDist = dist;
        querySelection = lotNdx;
        querySelectionType = SelectionLot;
      }
    }
  }

  setQueryHighlight(true);
}

void query_inner_select() {
}

void query_inner_reset() {
  isVehicleFollowMode = false;
  setQueryHighlight(false);
  querySelectionType = NoSelection;
  querySelection = 0;
}

bool startVehicleFollowMode(Part* part, InputEvent event) {
  setQueryHighlight(false);
  //setHeatMap(Pollution, false);
  //setQueryInfoForce();
  querySelectionType = NoSelection;
  querySelection = 0;
  clearSelection();
  //setMenuMode(HideUI);
  isVehicleFollowMode = true;
  return true;
}

bool stopVehicleFollowMode(Part* part, InputEvent event) {
  isVehicleFollowMode = false;
  setFollowingSelection(false);
  return true;
}

void stopVehicleFollowMode() {
  isVehicleFollowMode = false;
}

Part* query_inner_render(Line dim) {
  Part* result = panel(dim);

  r(result, label(vec2(0,0), 1, strdup_s("Query")));
  //r(result, hr(vec2(0,1.0), dim.end.x-toolPad*2));

  /*
  // Toggle Query Mode
  Part* queryButtContainer = panel(vec2(0.f, 1.f), vec2(3.33f, 1.f));
  Part* queryIco = button(vec2(0.f, 0.f), iconQuery, toggleQueryTool);
  queryIco->text = strdup_s("Q");
  queryButtContainer->renderMode = RenderTransparent;
  queryButtContainer->onKeyDown = toggleQueryTool;
  queryButtContainer->onClick = toggleQueryTool;
  queryButtContainer->flags |= _partHover;
  if (isQueryMode) {
    queryButtContainer->flags |= _partHighlight;
  }
  r(queryButtContainer, label(vec2(1.f, 0.125f), 0.75, strdup_s("Query")));
  r(queryButtContainer, queryIco);
  r(result, queryButtContainer);
  */

  // Demo Mode
  Part* carButtContainer = panel(vec2(3.34f, 2.5f), vec2(3.33f, 1.f));
  Part* carIco = icon(vec2(0.f, 0.f), iconFamily);
  carButtContainer->renderMode = RenderTransparent;
  carButtContainer->onClick = startVehicleFollowMode;
  carButtContainer->flags |= _partHover;
  setPartTooltipValues(carButtContainer,
    TooltipType::QueDemo);
  if (isVehicleFollowMode) {
    carButtContainer->flags |= _partHighlight;
  }
  r(carButtContainer, label(vec2(1.f, 0.125f), 0.75f,
        strdup_s("Random")));
  r(carButtContainer, carIco);
  r(result, carButtContainer);

  return result;
}

bool query_inner_visible() {
  return true;
}

Tool toolQueryInner = {
  query_inner_mouse_button_callback,
  query_inner_mouse_move_callback,
  query_inner_select,
  query_inner_reset,
  query_inner_visible,
  iconQuery,
  query_inner_render,
  queryInstructionPanel,
};

