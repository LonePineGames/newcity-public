#include "../sound.hpp"

void organizerQuery_mouse_button_callback(InputEvent event) {
  int button = event.button;
  int action = event.action;

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    playSound(_soundClickCancel);
    setSelection(querySelectionType, querySelection);
    updateDesignDimensionStrings();
  }
}

void organizerQuery_mouse_move_callback(InputEvent event) {
  setQueryHighlight(false);

  Line mouseLine = event.mouseLine;
  float minDist = tileSize;
  item buildingNdx = nearestBuilding(mouseLine);
  querySelection = 0;
  querySelectionType = NoSelection;
  if (buildingNdx > 0) {
    vec3 landPoint = landIntersect(mouseLine);
    float dist = boxDistance(getBuildingBox(buildingNdx), landPoint);
    if (dist < minDist) {
      minDist = dist;
      querySelection = buildingNdx;
      querySelectionType = SelectionBuilding;
    }
  }

  setQueryHighlight(true);
}

void organizerQuery_select() {}
void organizerQuery_reset() {
  setQueryHighlight(false);
  querySelectionType = NoSelection;
  querySelection = 0;
}

Part* organizerQuery_render(Line dim) {
  Part* result = panel(dim);
  r(result, label(vec2(0,0), 1, strdup_s("Select")));
  r(result, hr(vec2(0,1), dim.end.x-toolPad*2));
  r(result, button(vec2(4.5f,3.5f), iconPointer, 0));
  return result;
}

void organizerQueryInstructionPanel(Part* panel) {
  r(panel, label(vec2(0,0), 1, strdup_s(
    "Use left click to select\n"
    "a building.")));
}

std::string organizerQuery_name() {
  return "Select";
}

bool organizerQuery_visible() {
  return true;
}

Tool toolOrganizerQuery = {
  organizerQuery_mouse_button_callback,
  organizerQuery_mouse_move_callback,
  organizerQuery_select,
  organizerQuery_reset,
  organizerQuery_visible,
  iconPointer,
  organizerQuery_render,
  organizerQueryInstructionPanel,
  organizerQuery_name,
};

