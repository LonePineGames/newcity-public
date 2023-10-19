#include "../parts/scrollbox.hpp"
#include "../sound.hpp"

static item decoType = 0;
static item decoGroup = -1;
static item decoButtons[numLegacyDecoTypes+1];
static item currentDeco = -1;
static ScrollState decoScroll;
static bool decoGrab = false;
static double startYaw = 0;
static float startScale = 1;
static TextBoxState decoSearchTB;
static char* decoSearchText = 0;
bool isDecoSearching = false;

void deco_mouse_button_callback(InputEvent event) {
  int button = event.button;
  int action = event.action;
  decoGrab = false;

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    Design* d = getDesign(1);
    Building* b = getBuilding(1);
    int numDecos = d->decos.size();
    float bestDistance = FLT_MAX;
    item bestDeco = -1;
    item bestHandleType = -1;
    Line l = event.mouseLine;
    l.start -= b->location;
    l.end   -= b->location;
    mouseDown = true;

    for (int i=0; i < numDecos; i++) {
      Deco* s = &d->decos[i];
      if (!isDecoVisible(s->decoType) &&
          (getSelectionType() != SelectionDeco || i != getSelection())) {
        continue;
      }

      vec3 loc = s->location;

      float cangle = cos(s->yaw);
      float sangle = sin(s->yaw);
      vec3 ualong = vec3(cangle, sangle, 0);
      vec3 along = ualong * s->scale * 10.f;

      {
        float dist = pointLineDistance(loc, l);
        if (dist <= bestDistance) {
          bestDistance = dist;
          bestDeco = i;
          bestHandleType = 1;
        }
      }

      if (s->decoType >= numLegacyDecoTypes) {
        float dist = pointLineDistance(loc-along, l);
        if (dist <= bestDistance) {
          bestDistance = dist;
          bestDeco = i;
          bestHandleType = 2;
        }
      }
    }

    if (bestDeco >= 0 && bestDistance < 5) {
      playSound(_soundClickCancel);
      currentDeco = bestDeco;
      currentHandleType = bestHandleType;
      setSelection(SelectionDeco, bestDeco);
      updateDesignDimensionStrings();

      Deco* s = &d->decos[currentDeco];
      startYaw = s->yaw;
      startScale = s->scale;

    } else {
      playSound(_soundClickComplete);
      currentDeco = numDecos;
      currentHandleType = 1;
      Deco s;
      s.location = designIntersect(event.mouseLine, 0);
      s.yaw = 0;
      s.scale = 1;
      s.decoType = decoType;
      d->decos.push_back(s);
      setDesignHasStatue(d);
      setSelection(SelectionDeco, d->decos.size()-1);
      designRender();
    }
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    if (mouseDown) {
      playSound(_soundClickPrimary);
      pushDesignerUndoHistory();
    }
    mouseDown = false;
    currentHandleType = 0;
    currentDeco = -1;
  }
}

void deco_mouse_move_callback(InputEvent event) {
  if (mouseDown || decoGrab) {
    Design* d = getDesign(1);
    Deco* s = &d->decos[currentDeco];
    bool shift = event.mods & GLFW_MOD_SHIFT;

    if (currentHandleType == 1) {
      if (shift) {
        s->location.z = designIntersectZ(event.mouseLine, s->location);
      } else {
        vec3 ml = designIntersect(event.mouseLine, s->location.z);
        s->location = vec3(vec2(ml), s->location.z);
      }

    } else if (currentHandleType == 2) {
      vec3 ml = designIntersect(event.mouseLine, s->location.z);
      vec3 sa = ml - s->location;
      float len = length(sa);

      if (len > maxDist) {
        sa = sa*(maxDist/len);
        len = maxDist;
      }

      if (len > 0) {
        double yaw = atan(-sa.y, -sa.x);
        yaw = yaw - 2*pi_o * floor(yaw/(2*pi_o));
        double yawDiff = abs(yaw-startYaw);
        if (yawDiff > pi_o) yawDiff = 2*pi_o - yawDiff;
        float lenDiff = abs(startScale*10 - len);
        bool doYaw = yawDiff > pi_o*0.25 || lenDiff < 2;

        if (designerGridMode) {
          yaw = yaw/(2.*pi_o);
          yaw = round(yaw*72.)/72.;
          yaw = yaw * (2.*pi_o);

          if (len > 9 && len < 11) {
            len = 10;
          } else {
            len = round(len);
          }
        }

        if (doYaw) {
          s->yaw = yaw;
          s->scale = startScale;
        } else {
          s->yaw = startYaw;
          s->scale = len/10.f;
        }
      }
    }

    designRender();
  }
}

void deco_select() {
  mouseDown = false;
  decoGrab = false;
}

void deco_reset() {
  if (mouseDown) {
    pushDesignerUndoHistory();
  }
  mouseDown = false;
  decoGrab = false;
}

bool setDecoType(Part* part, InputEvent event) {
  decoType = part->itemData;
  decoSearchTB.flags &= ~_textBoxEditing;
  focusTextBox(0);
  return true;
}

bool setSelectedDecoType(Part* part, InputEvent event) {
  if (getSelectionType() == SelectionDeco) {
    Design* d = getDesign(1);
    Deco* deco = &d->decos[getSelection()];
    deco->decoType = part->itemData;
    designRender();
    pushDesignerUndoHistory();
  }
  return true;
}

bool setDecoGroup(Part* part, InputEvent event) {
  decoGroup = part->itemData;
  decoType = 0;
  decoScroll.target = 0;
  return true;
}

static bool toggleDecoGrab(Part* part, InputEvent event) {
  item ndx = part->itemData;
  item key = event.key;
  decoGrab = !decoGrab;
  currentDeco = getSelection();
  currentHandleType = 1;
  return true;
}

static bool stopDecoSearch(Part* part, InputEvent event) {
  decoSearchTB.flags &= ~_textBoxEditing;
  focusTextBox(0);
  return true;
}

static bool toggleDecoSearch(Part* part, InputEvent event) {
  item key = event.key;
  if (decoSearchTB.flags & _textBoxEditing) {
    decoSearchTB.flags &= ~_textBoxEditing;
    focusTextBox(0);
    isDecoSearching = false;
  } else {
    focusTextBox(&decoSearchTB);
    isDecoSearching = true;
  }

  return true;
}

static bool toggleGroupVisible(Part* part, InputEvent event) {
  item ndx = part->itemData;
  toggleDecoGroupVisible(ndx);
  designRender();
  return true;
}

Part* decoTypeButton(vec2 loc, item type) {
  Part* butt = button(loc, vec2(8,0.75),
    strdup_s(getDecoTypeName(type)), setDecoType);
  butt->itemData = type;
  butt->flags |= _partIsPanel;
  if (type == decoType) {
    butt->flags |= _partHighlight;
  }

  if (getSelectionType() == SelectionDeco) {
    r(butt, button(vec2(8.25,0), iconPointer, vec2(0.75,0.75),
      setSelectedDecoType, type));
  }

  return butt;
}

Part* deco_render(Line dim) {
  Part* result = panel(dim);
  float scrollSize = 6;
  bool isDecoReallySearching = isDecoSearching && decoSearchText != 0 && strlength(decoSearchText) > 0;

  if (isDecoSearching) {
    decoSearchTB.text = &decoSearchText;
    Part* tb = r(result, textBox(vec2(0,0), vec2(5,1), &decoSearchTB));
    tb->onCustom = stopDecoSearch;

  } else {
    r(result, label(vec2(0,0), 1, strdup_s("Decorations")));
  }

  if (!isDecoReallySearching) {
    if (decoGroup >= 0) {
      scrollSize = 5;
      const char* groupName = decoGroup > 0 ? getDecoGroup(decoGroup)->name :
        "Legacy Decorations";
      r(result, button(vec2(0,1), iconLeft, vec2(10,0.75),
        strdup_s(groupName), setDecoGroup, -1));
    }
  }

  Part* gridButt = r(result, button(vec2(7, 0),
        iconGrid, vec2(1,1),
        toggleDesignerGridMode, 0));
  gridButt->inputAction = ActDesignGridMode;
  setPartTooltipValues(gridButt,
    TooltipType::DesignerGrid);
  if (designerGridMode) gridButt->flags |= _partHighlight;

  Part* searchButt = r(result, button(vec2(5, 0),
        iconQuery, vec2(1,1), toggleDecoSearch, 0));
  searchButt->inputAction = ActDesignSearch;

  if (getSelectionType() == SelectionDeco) {
    Part* grabButt = r(result, button(vec2(6, 0),
          iconHand, vec2(1,1),
          toggleDecoGrab, 0));
    grabButt->inputAction = ActDesignGrab;
    setPartTooltipValues(grabButt,
      TooltipType::DesignerDecoGrab);
    if (decoGrab || mouseDown) grabButt->flags |= _partHighlight;
  }

  Part* scroll = scrollbox(vec2(0,0), vec2(10,scrollSize));

  if (isDecoReallySearching) {
    float y = 0;
    for (int i = 0; i < sizeDecoTypes(); i++) {
      std::string name = getDecoTypeName(i);
      if (!stringContainsCaseInsensitive(name, decoSearchText)) continue;

      r(scroll, decoTypeButton(vec2(0, y), i));
      y ++;
    }

  } else if (decoGroup < 0) {
    for (int i=0; i <= sizeDecoGroups(); i++) {
      const char* groupName = i > 0 ? getDecoGroup(i)->name :
        "Legacy Decorations";
      Part* butt = button(vec2(0,i), vec2(8,0.75),
        strdup_s(groupName), setDecoGroup);
      butt->itemData = i;
      r(scroll, butt);

      bool viz = isDecoGroupVisible(i);
      Part* vizButt = r(scroll, button(vec2(8.25, i),
            viz ? iconEye : iconEyeClosed, vec2(.75,.75),
            toggleGroupVisible, i));
      setPartTooltipValues(vizButt,
        TooltipType::DesignerDecoGroupVisible);
      //if (isDecoGroupVisible(i)) vizButt->flags |= _partHighlight;
    }

  } else {
    float y = 0;
    bool legacy = decoGroup == 0;
    item start = legacy ? 0 : numLegacyDecoTypes;
    item end = legacy ? numLegacyDecoTypes : sizeDecoTypes();

    for (int i = start; i < end; i++) {
      if (!legacy) {
        DecoType* type = getDecoType(i);
        if (type->group != decoGroup) continue;
      }

      r(scroll, decoTypeButton(vec2(0,y), i));
      y ++;
    }
  }

  r(result, scrollboxFrame(vec2(0, (isDecoReallySearching || decoGroup < 0) ? 1 : 2),
        vec2(10,scrollSize), &decoScroll, scroll));

  return result;
}

void decoInstructionPanel(Part* panel) {
  r(panel, label(vec2(0,0), 1,
    strdup_s("Use left click to place\n"
    "a decorative object.\n"
    "Use scrollbox to choose between\n"
    "trees, signs and smokestacks.")));
}

std::string deco_name() {
  return "Decorations";
}

bool deco_visible() {
  return true;
}

Tool toolDeco = {
  deco_mouse_button_callback,
  deco_mouse_move_callback,
  deco_select,
  deco_reset,
  deco_visible,
  iconTree,
  deco_render,
  decoInstructionPanel,
  deco_name,
};

