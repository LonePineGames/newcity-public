
void design_bulldozer_mouse_button_callback(InputEvent event) {
  int button = event.button;
  int action = event.action;

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    Line mouseLine = event.mouseLine;
    Design* d = getDesign(1);
    Building* b = getBuilding(1);
    float bestDist = FLT_MAX;
    item bestElement = -1;
    bool elementIsDeco = false;

    for(int i=0; i < d->structures.size(); i++) {
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
      if (dist < bestDist) {
        bestDist = dist;
        bestElement = i;
      }
    }

    for(int i=0; i < d->decos.size(); i++) {
      float dist = pointLineDistance(d->decos[i].location + b->location,
        mouseLine);
      if (dist < bestDist) {
        bestDist = dist;
        bestElement = i;
        elementIsDeco = true;
      }
    }

    if (bestElement >= 0 && bestDist < c(CBulldozerSnapDistance)) {
      playSound(_soundClickComplete);

      if (elementIsDeco) {
        d->decos.erase(d->decos.begin() + bestElement);
        setDesignHasStatue(d);
        if (getSelectionType() == SelectionDeco &&
            getSelection() == bestElement) {
          clearSelection();
        }

      } else {
        d->structures.erase(d->structures.begin() + bestElement);
        if (getSelectionType() == SelectionStructure &&
            getSelection() == bestElement) {
          clearSelection();
        }
      }

      pushDesignerUndoHistory();
      designRender();
    }
  }
}

void design_bulldozer_mouse_move_callback(InputEvent event) {}
void design_bulldozer_select() {}
void design_bulldozer_reset() {}

Part* design_bulldozer_render(Line dim) {
  Part* result = panel(dim);
  r(result, label(vec2(0,0), 1, strdup_s("Bulldozer")));
  //r(result, hr(vec2(0,1), dim.end.x-toolPad*2));
  r(result, button(vec2(4.5f,3.5f), iconBulldozer, 0));
  return result;
}

void designBulldozerInstructionPanel(Part* panel) {
  r(panel, label(vec2(0,0), 1, strdup_s(
    "Use left click to remove\n"
    "a structure or decoration.")));
}

std::string design_bulldozer_name() {
  return "Bulldozer";
}

bool design_bulldozer_visible() {
  return true;
}

Tool toolDesignBulldozer = {
  design_bulldozer_mouse_button_callback,
  design_bulldozer_mouse_move_callback,
  design_bulldozer_select,
  design_bulldozer_reset,
  design_bulldozer_visible,
  iconBulldozer,
  design_bulldozer_render,
  designBulldozerInstructionPanel,
  design_bulldozer_name,
};

