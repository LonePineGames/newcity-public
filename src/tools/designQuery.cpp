#include "../sound.hpp"

void designQuery_mouse_button_callback(InputEvent event) {
  int button = event.button;
  int action = event.action;

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    Line mouseLine = event.mouseLine;
    Design* d = getDesign(1);
    Building* b = getBuilding(1);
    float bestDist = FLT_MAX;
    item bestElement = -1;
    bool elementIsDeco = false;

    if (isStructuresVisible()) {
      for(int i=0; i < d->structures.size(); i++) {
        if (getSelectionType() == SelectionStructure && getSelection() == i) {
          continue;
        }
        Structure* s = &d->structures[i];
        float cangle = cos(s->angle);
        float sangle = sin(s->angle);
        vec3 loc = s->location + b->location;
        vec3 along = vec3(sangle, cangle, 0) * s->size.y;

        if (s->size.z >= s->size.y) {
          loc += along*.5f;
          along = vec3(0,0,s->size.z);
        }

        float dist = lineDistance(line(loc, loc+along), mouseLine);
        //dist -= s->size.x/2;
        if (dist <= bestDist) {
          bestDist = dist;
          bestElement = i;
        }
      }
    }

    for(int i=0; i < d->decos.size(); i++) {
      if (!isDecoVisible(d->decos[i].decoType)) continue;
      if (getSelectionType() == SelectionDeco && getSelection() == i) {
        continue;
      }

      float dist = pointLineDistance(d->decos[i].location + b->location,
        mouseLine);
      if (dist <= bestDist) {
        bestDist = dist;
        bestElement = i;
        elementIsDeco = true;
      }
    }

    if (bestDist > c(CBulldozerSnapDistance)) {
      playSound(_soundClickCancel);
      clearSelection();

    } else if (bestElement >= 0) {
      playSound(_soundClickComplete);
      if (elementIsDeco) {
        setSelection(SelectionDeco, bestElement);
      } else {
        setSelection(SelectionStructure, bestElement);
      }
    }
    designRender();
  }
}

void designQuery_mouse_move_callback(InputEvent event) {}
void designQuery_select() {}
void designQuery_reset() {}

Part* designQuery_render(Line dim) {
  Part* result = panel(dim);
  r(result, label(vec2(0,0), 1, strdup_s("Select")));
  //r(result, hr(vec2(0,1), dim.end.x-toolPad*2));
  r(result, button(vec2(4.5f,3.5f), iconPointer, 0));
  return result;
}

void designQueryInstructionPanel(Part* panel) {
  r(panel, label(vec2(0,0), 1, strdup_s(
    "Use left click to select\n"
    "a structure or decoration.")));
}

std::string designQuery_name() {
  return "Select";
}

bool designQuery_visible() {
  return true;
}

Tool toolDesignQuery = {
  designQuery_mouse_button_callback,
  designQuery_mouse_move_callback,
  designQuery_select,
  designQuery_reset,
  designQuery_visible,
  iconPointer,
  designQuery_render,
  designQueryInstructionPanel,
  designQuery_name,
};

