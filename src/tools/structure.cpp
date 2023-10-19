#include "../parts/designConfigPanel.hpp"

#include <algorithm>

const float floorSize = 2;
const float maxDist = 2500.0f;
const float maxDistZ = 1000.0f;
bool designerGridMode = true;
bool roofOnly = false;

const vec2 structureIcon = vec2(2,6);
static item structureType = 1;
static item structureButtons[numRoofTypes+1];
static item currentStructure = -1;
static item currentHandleType = -1;

void setStructureType(item zt);

float designerSnap(float val) {
  if (designerGridMode) {
    return round(val / c(CDesignerGridSize)) * c(CDesignerGridSize);
  } else {
    return val;
  }
}

vec3 designIntersect(Line l, float z) {
  vec3 buildingLoc = getBuilding(1)->location;
  vec3 pol = lineAtZ(l, z + buildingLoc.z);
  //vec3 pol = landIntersect(l);
  vec3 result = pol - buildingLoc;
  result.x = designerSnap(result.x);
  result.y = designerSnap(result.y);
  result.z = designerSnap(result.z);
  return result;
}

float designIntersectZ(Line l, vec3 point) {
  vec3 buildingLoc = getBuilding(1)->location;
  point += buildingLoc;
  Line axis = line(point, point);
  axis.start.z = -10000;
  axis.end.z = 10000;
  vec3 intersect = pointOfIntersection(axis, l);
  float result = intersect.z - buildingLoc.z;
  result = designerSnap(result);
  return result;
}

void structure_mouse_button_callback(InputEvent event) {
  int button = event.button;
  int action = event.action;

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    Design* d = getDesign(1);
    Building* b = getBuilding(1);
    int numStructures = d->structures.size();
    float bestDistance = FLT_MAX;
    item bestStructure = -1;
    item bestHandleType = -1;
    Line l = event.mouseLine;
    l.start -= b->location;
    l.end   -= b->location;
    mouseDown = true;

    for (int i=0; i < numStructures; i++) {
      if (!isStructuresVisible() &&
          (getSelectionType() != SelectionStructure || i != getSelection())) {
        continue;
      }

      Structure* s = &d->structures[i];
      vec3 size = s->size;
      float cangle = cos(s->angle);
      float sangle = sin(s->angle);
      vec3 right = vec3(-cangle, sangle, 0) * size.x * .5f;
      vec3 along = vec3(sangle, cangle, 0) * size.y;
      vec3 loc = s->location;

      {
        float dist = pointLineDistance(loc, l);
        if (dist <= bestDistance) {
          bestDistance = dist;
          bestStructure = i;
          bestHandleType = 1;
        }
      }

      {
        float dist = pointLineDistance(loc+along, l);
        if (dist <= bestDistance) {
          bestDistance = dist;
          bestStructure = i;
          bestHandleType = 2;
        }
      }

      {
        float dist = pointLineDistance(loc+right+vec3(0,0,size.z), l);
        if (dist <= bestDistance) {
          bestDistance = dist;
          bestStructure = i;
          bestHandleType = 3;
        }
      }
    }

    if (bestStructure >= 0 && bestDistance < 5) {
      playSound(_soundClickCancel);
      currentStructure = bestStructure;
      currentHandleType = bestHandleType;
      setSelection(SelectionStructure, bestStructure);
      updateDesignDimensionStrings();

    } else {
      playSound(_soundClickComplete);
      currentStructure = numStructures;
      currentHandleType = 2;
      Structure s;
      s.location = designIntersect(event.mouseLine, 0);
      s.size = vec3(12,12,floorSize*2);
      s.roofType = structureType;
      if (roofOnly) s.roofType = -s.roofType-1;
      s.angle = 0;
      s.roofSlope = defaultRoofSlopeForType(structureType);
      d->structures.push_back(s);
      setSelection(SelectionStructure, d->structures.size()-1);
      pushDesignerUndoHistory();
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
    currentStructure = -1;
    pushDesignerUndoHistory();
  }
}

void structure_mouse_move_callback(InputEvent event) {
  if (mouseDown) {
    Design* d = getDesign(1);
    Structure* s = &d->structures[currentStructure];
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
        s->size.y = len;
        s->angle = atan(sa.x, sa.y);
        s->angle = s->angle - 2*pi_o * floor(s->angle/(2*pi_o));
      }

    } else if (currentHandleType == 3) {
      Building* b = getBuilding(1);
      Line l = event.mouseLine;
      l.start -= b->location;
      l.end   -= b->location;
      float cangle = cos(s->angle);
      float sangle = sin(s->angle);
      vec3 loc = s->location;
      vec3 along = vec3(sangle, cangle, 0) * s->size.y;
      vec3 right = vec3(-cangle, sangle, 0) * s->size.x * .5f;

      if (shift) {
        float z = designIntersectZ(event.mouseLine,
            loc+right+vec3(0,0,s->size.z)) - s->size.z;
        float slope = z/12.f + defaultRoofSlopeForType(s->roofType);
        if (slope < 0) slope = 0;
        s->roofSlope = slope;
        //s->roofSlope = clamp(slope, 0.f, 2.f);

      } else {
        vec3 pl = linePlaneIntersect(l, loc, along);

        if (pl.x != -1) {
          vec3 pv = pl - loc;
          float x = designerSnap(length(vec2(pv))*2);
          x = clamp(x, c(CDesignerGridSize), maxDist);
          s->size.x = x;
          float z = round(pv.z / floorSize)*floorSize;
          z = clamp(z, floorSize*.5f, maxDistZ);
          s->size.z = z;
        }
      }
    }

    designRender();
  }
}

void structure_select() {
  mouseDown = false;
}

void structure_reset() {
  if (mouseDown) {
    pushDesignerUndoHistory();
  }
  mouseDown = false;
}

bool setStructureType(Part* part, InputEvent event) {
  item nextStructureType = part->itemData;
  structureType = nextStructureType;
  return true;
}

bool setSelectedStructureType(Part* part, InputEvent event) {
  item nextStructureType = part->itemData;
  if (getSelectionType() == SelectionStructure) {
    Design* d = getDesign(1);
    Structure* s = &d->structures[getSelection()];
    if (roofOnly) nextStructureType = -nextStructureType-1;
    s->roofType = nextStructureType;
    s->roofSlope = defaultRoofSlopeForType(nextStructureType);
    designRender();
    pushDesignerUndoHistory();
  }

  return true;
}

static bool toggleDesignerGridMode(Part* part, InputEvent event) {
  item ndx = part->itemData;
  item key = event.key;
  designerGridMode = !designerGridMode;
  return true;
}

static bool toggleRoofOnly(Part* part, InputEvent event) {
  item ndx = part->itemData;
  item key = event.key;
  roofOnly = !roofOnly;
  return true;
}

static bool toggleStructuresVisible(Part* part, InputEvent event) {
  item ndx = part->itemData;
  item key = event.key;
  toggleStructuresVisible();
  designRender();
  return true;
}

Part* structure_render(Line dim) {
  Part* result = panel(dim);
  r(result, label(vec2(0,0), 1, strdup_s("Structure")));
  //r(result, hr(vec2(0,1), dim.end.x-toolPad*2));

  Part* gridButt = r(result, button(vec2(7, 0),
        iconGrid, vec2(1,1),
        toggleDesignerGridMode, 0));
  gridButt->inputAction = ActDesignGridMode;
  setPartTooltipValues(gridButt,
    TooltipType::DesignerGrid);
  if (designerGridMode) gridButt->flags |= _partHighlight;

  Part* roofButt = r(result, button(vec2(5.75, 0),
        roofOnly ? iconRoof : iconFarm, vec2(1,1),
        toggleRoofOnly, 0));
  roofButt->inputAction = ActDesignRoofOnly;
  setPartTooltipValues(roofButt,
    TooltipType::DesignerStructRoofOnly);
  if (roofOnly) roofButt->flags |= _partHighlight;

  bool viz = isStructuresVisible();
  Part* vizButt = r(result, button(vec2(4.5, 0),
        viz ? iconEye : iconEyeClosed, vec2(1,1),
        toggleStructuresVisible, 0));
  vizButt->inputAction = ActDesignHideStructures;
  setPartTooltipValues(vizButt,
    TooltipType::DesignerStructuresVisible);
  if (isStructuresVisible()) vizButt->flags |= _partHighlight;

  for (int i=0; i < numRoofTypes; i++) {
    float y = i * 0.85f + 1.5f;
    Part* butt = button(vec2(1.25,y), vec2(7.5,0.85f),
      strdup_s(getRoofTypeName(i)), setStructureType);
    butt->flags |= _partAlignCenter;
    butt->itemData = i;
    butt->inputAction = (InputAction)(ActDesignGable+i);
    setPartTooltipValues(butt,
      TooltipType::DesignerStructGable+i);
    if (i == structureType) {
      butt->flags |= _partHighlight;
    }
    r(result, butt);

    // Empty button to show hotkey helper
    /*
    KeyBind bind = getKeyBind(butt->inputAction);
    if (bind.key != GLFW_KEY_UNKNOWN) {
      Part* keyButt = r(butt, button(vec2(6,y), iconNull, vec2(1.f,0.85f),
        setStructureType, i));
      butt->text = strdup_s(getKeyStr(bind.key).c_str());
    }
    */

    if (getSelectionType() == SelectionStructure) {
      Part* changeTypeButt = button(vec2(9, y), iconPointer, vec2(0.85, 0.85),
        setSelectedStructureType, i);
      setPartTooltipValues(changeTypeButt,
        TooltipType::DesignerStructChangeGable+i);
      r(result, changeTypeButt);
    }
  }
  return result;
}

void structureInstructionPanel(Part* panel) {
  r(panel, label(vec2(0,0), 1, strdup_s(
    "Left click and to place\n"
    "a structure.\n"
    "Use grab handles to adjust\n"
    "size and position.")));
}

std::string structure_name() {
  return "Structure";
}

bool structure_visible() {
  return true;
}

Tool toolStructure = {
  structure_mouse_button_callback,
  structure_mouse_move_callback,
  structure_select,
  structure_reset,
  structure_visible,
  iconHouse,
  structure_render,
  structureInstructionPanel,
  structure_name,
};

