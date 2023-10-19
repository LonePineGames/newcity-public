#include "designConfigPanel.hpp"

#include "../building/building.hpp"
#include "../building/buildingTexture.hpp"
#include "../building/design.hpp"
#include "../building/designPackage.hpp"
#include "../building/renderBuilding.hpp"
#include "../business.hpp"
#include "../color.hpp"
#include "../draw/camera.hpp"
#include "../draw/entity.hpp"
#include "../draw/texture.hpp"
#include "../economy.hpp"
#include "../game/game.hpp"
#include "../graph.hpp"
#include "../icons.hpp"
#include "../platform/file.hpp"
#include "../renderLand.hpp"
#include "../selection.hpp"
#include "../steam/steamws_core.hpp"
#include "../string_proxy.hpp"
#include "../string.hpp"
#include "../zone.hpp"

#include "button.hpp"
#include "hr.hpp"
#include "icon.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "slider.hpp"
#include "steamWorkshop.hpp"
#include "span.hpp"
#include "textBox.hpp"
#include "textureSelect.hpp"

#include "spdlog/spdlog.h"
#include <algorithm>

enum DCPTab {
  selectionTab, paintTab, mainTab, detailsTab, effectsTab,
  numDCPTabs
};

enum DesignParameters {
  DPLocationX, DPLocationY, DPLocationZ,
  DPSizeX, DPSizeY, DPSizeZ,
  DPScale, DPYaw, DPRoofSlope,
  DPMinYear, DPMaxYear,
  DPHomes, DPBiz0, DPBiz1, DPBiz2, DPBiz3,
  DPLowTide, DPHighTide,
  DPConstructionCost, DPMaintenanceCost,
  numDesignParameters
};

const float radToDeg = 180.f/pi_o;
const float degToRad = pi_o/180.f;
const float floatSteps = 10;
const float maxCost = 5000000000;
const float maxMaint = 5000000000;
TextBoxState designTextBox;
bool wasEditing = false;
static bool doingNameInput = false;
static int currentDCPTab = mainTab;
char* designParameterStr[numDesignParameters] = {0};
TextBoxState designParamTextBox[numDesignParameters];
static bool wasEditingParam[numDesignParameters] = {false};
static const float dcpWidth = 9.7;
static const float dcpPadding = 0.25;
static const float dcpScale = 0.75;
static const float dcpHeight = 20; //isGov ? 21.5 : 13;
item textureSelectionState = TSSClosed;

const char* designParameterNames[] = {
  "Location X",
  "Location Y",
  "Location Z",
  "Size X",
  "Size Y",
  "Size Z",
  "Scale",
  "Yaw",
  "Roof Slope",
  "Min Year",
  "Max Year",
  "Homes",
  "Storefronts",
  "Offices",
  "Farm Fields",
  "Factories",
  "Low Tide",
  "High Tide",
  "Construction",
  "Maintenance/yr",
};

vec3 designParameterIcon[] = {
  iconLeftRight, iconUpDown, iconDensity,
  iconLeftRight, iconUpDown, iconDensity,
  iconMergeRight, iconTurnRight,
  iconRoof, iconWait, iconWait,
  iconHouse,
  iconZoneMonoRetail,
  iconZoneMonoOffice,
  iconZoneMonoFarm,
  iconZoneMonoFactory,
  iconTide, iconTide,
  iconCash, iconCash,
};

const char* universityTypeNames[5] = {
  "Bunks", "Storefronts", "Colleges", "Students", "Professors"
};

vec3 universityIcons[5] = {
  iconZoneMonoMixedUse, iconZoneMonoRetail,
  iconZoneMonoGovernment, iconPersonMan, iconEducation
};

const float costSteps[] = {
  0.0f, 0.0001f, 0.0002f,
  0.0005f,
  0.001f, 0.0015f, 0.002f, 0.0025f, 0.003f, 0.004f, 0.005f,
  0.006f, 0.007f, 0.008f, 0.009f,
  0.01f, 0.015f, 0.02f, 0.025f, 0.03f, 0.035f, 0.04f, 0.045f, 0.05f,
  0.06f, 0.07f, 0.08f, 0.09f,
  0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.8f, 1.0f, 2.0f, 4.0f

  /*
  0.0f, 0.01f, 0.015f, 0.02f, 0.025f, 0.03f, 0.035f, 0.04f, 0.045f, 0.05f,
  0.055f, 0.06f, 0.065f, 0.07f, 0.075f, 0.08f, 0.085f, 0.09f, 0.095f, 0.1f,
  0.15f, 0.2f, 0.25f, 0.3f, 0.35f, 0.4f, 0.45f, 0.5f, 0.55f,
  0.6f, 0.65f, 0.7f, 0.75f, 0.8f, 0.85f, 0.9f, 0.95f, 1.0f
  */
};

void updateDesignParameterStr(item param);

money sliderToMoney(float x, float mx) {
  int k = int(x*37);
  k = k > 37 ? 37 : k;
  return mx*costSteps[k];
  //if (k <= 20) {
    //return k*mx/200.f;
  //} else {
    //return ((k-20)/20.f*.9f+0.1f)*mx;
  //}
}

float moneyToSlider(money m, float mx) {
  float x = m/mx;
  for (int i = 0; i < 37; i++) {
    if (x <= costSteps[i]) {
      return i/37.0;
    }
  }
  return 1.0f;
}

static bool setMinLandValue(Part* part, InputEvent event) {
  Design* d = getSelectedDesign();
  float val = int(part->vecData.x*floatSteps)/floatSteps;
  if (d->minLandValue != val) {
    d->minLandValue = val;
    pushDesignerUndoHistory();
  }
  return true;
}

static bool setMinDensity(Part* part, InputEvent event) {
  float val = int(part->vecData.x*floatSteps)/floatSteps;
  Design* d = getSelectedDesign();
  if (d->minDensity != val) {
    d->minDensity = val;
    setSpawnProbGlobal(d->minDensity+0.1f);
    rerenderGraph();
    renderLand();
    pushDesignerUndoHistory();
  }
  return true;
}

static bool setColor(Part* part, InputEvent event) {
  Building* b = getBuilding(getSelectedDesignNdx());
  item numTex = numMatchingTexturesForBuilding(getSelectedDesignNdx());
  b->color = int(part->vecData.x*(numTex-1));
  designRender();
  return true;
}

bool openTextureSelect(Part* part, InputEvent event) {
  textureSelectionState = part->itemData;
  return true;
}

bool closeTextureSelect(Part* part, InputEvent event) {
  textureSelectionState = TSSClosed;
  return true;
}

bool selectCurrentTexture(Part* part, InputEvent event) {
  Building* b = getBuilding(getSelectedDesignNdx());
  Entity* e = getEntity(b->entity);
  item texId = e->texture;
  item illumTexId = getTexture(getTextureIllumFilename(texId));
  addTextureToPackage(b->design, texId, 0);
  addTextureToPackage(b->design, illumTexId, 1);
  b->color = numMatchingTexturesForBuilding(getSelectedDesignNdx())-1;
  paintBuilding(getSelectedDesignNdx());
  return true;
}

bool deleteTexture(Part* part, InputEvent event) {
  Building* b = getBuilding(getSelectedDesignNdx());
  item texId = getEntity(b->entity)->texture;
  bool illum = part->itemData > 0;
  const char* fn = 0;
  if (!illum) {
    fn = getTextureFilename(texId);
  } else {
    fn = getTextureIllumFilename(texId);
  }

  if (fn != 0) {
    deleteFile(fn);
    if (stringContainsCaseInsensitive(fn, "designs/")) {
      deleteDesignPackageTexture(getSelectedDesignNdx(), illum);
    } else {
      deleteBuildingTexture(getSelectedDesignNdx(), illum);
    }
  }

  return true;
}

static bool setLightLevel(Part* part, InputEvent event) {
  float val = part->vecData.x;
  setBuildingTextureLightLevel(int(val*10.f));
  designRender();
  return true;
}

static bool setZone(Part* part, InputEvent event) {
  Design* d = getSelectedDesign();
  Building* b = getBuilding(getSelectedDesignNdx());
  if (d->zone != part->itemData) {
    d->zone = part->itemData;
    b->zone = part->itemData;
    designRender();
    pushDesignerUndoHistory();
  }
  return true;
}

static bool setDCPTab(Part* part, InputEvent event) {
  currentDCPTab = part->itemData;
  return true;
}

static bool setDesignCategory(Part* part, InputEvent event) {
  Design* d = getSelectedDesign();
  if (d->category != part->itemData) {
    d->category = part->itemData;
    pushDesignerUndoHistory();
  }
  return true;
}

static bool toggleInstitution(Part* part, InputEvent event) {
  Design* d = getSelectedDesign();
  d->numBusinesses[Institution] = !(d->numBusinesses[Institution]);
  pushDesignerUndoHistory();
  return true;
}

static bool toggleDesignFlag(Part* part, InputEvent event) {
  Design* d = getSelectedDesign();
  int flag = part->itemData;
  if (d->flags & flag) {
    d->flags &= ~flag;
  } else {
    d->flags |= flag;
  }
  pushDesignerUndoHistory();
  return true;
}

static bool setCost(Part* part, InputEvent event) {
  Design* d = getSelectedDesign();
  money cost = sliderToMoney(part->vecData.x, maxCost);
  if (d->cost != cost) {
    d->cost = cost;
    pushDesignerUndoHistory();
  }
  return true;
}

static bool setMaintanence(Part* part, InputEvent event) {
  Design* d = getSelectedDesign();
  money val = sliderToMoney(part->vecData.x, maxMaint);
  if (d->maintenance != val) {
    d->maintenance = val;
    pushDesignerUndoHistory();
  }
  return true;
}

static bool setBuildingEffect(Part* part, InputEvent event) {
  Design* d = getSelectedDesign();
  int amount = (part->vecData.x*2 - 1)*c(CMaxEffect);
  if (d->effect[part->itemData] != amount) {
    d->effect[part->itemData] = amount;
    pushDesignerUndoHistory();
  }
  return true;
}

static bool toggleAquatic(Part* part, InputEvent event) {
  Design* d = getSelectedDesign();
  if (isDesignAquatic(getSelectedDesignNdx())) {
    d->flags &= ~_designAquatic;
  } else {
    d->flags |= _designAquatic;
    if (d->highTide == 0) d->highTide = 100;
    if (d->lowTide == 0) d->lowTide = d->highTide*2;
  }
  pushDesignerUndoHistory();
  designRender();
  return true;
}

void alignTideValues(item param) {
  Design* d = getSelectedDesign();
  if (d->highTide > d->lowTide-c(CTileSize)) {
    if (param == DPLowTide) {
      d->highTide = d->lowTide - c(CTileSize);
      updateDesignParameterStr(DPHighTide);
    } else if (param == DPHighTide) {
      d->lowTide = d->highTide + c(CTileSize);
      updateDesignParameterStr(DPLowTide);
    }
  }
}

float getDesignParameterValue(item param) {
  Design* d = getSelectedDesign();
  switch (param) {
    case DPMinYear: return d->minYear;
    case DPMaxYear: return d->maxYear;
    case DPHighTide: return d->highTide;
    case DPLowTide: return d->lowTide;
    case DPConstructionCost: return d->cost * c(CAmenityCostMult);
    case DPMaintenanceCost: return d->maintenance * c(CAmenityMaintMult);
    case DPHomes: return d->numFamilies;
    case DPBiz0: case DPBiz1: case DPBiz2: case DPBiz3: {
      int i = param-DPBiz0;
      float result = d->numBusinesses[i];
      return result; } break;
    default: break;
  }

  if (getSelectionType() == SelectionDeco) {
    Deco* deco = &d->decos[getSelection()];
    switch (param) {
      case DPLocationX: return deco->location.y;
      case DPLocationY: return deco->location.x;
      case DPLocationZ: return deco->location.z;
      case DPScale: return deco->scale;
      case DPYaw: return deco->yaw * radToDeg;
      default: return 0;
    }

  } else if (getSelectionType() == SelectionStructure) {
    Structure* s = &d->structures[getSelection()];
    switch (param) {
      case DPLocationX: return s->location.y;
      case DPLocationY: return s->location.x;
      case DPLocationZ: return s->location.z;
      case DPSizeX: return s->size.x;
      case DPSizeY: return s->size.y;
      case DPSizeZ: return s->size.z;
      case DPYaw: return s->angle * radToDeg;
      case DPRoofSlope: return s->roofSlope;
      default: return 0;
    }

  } else {
    return 0;
  }
}

void designParameterBulkStep(item param, float step) {
  Design* d = getSelectedDesign();
  for (int i = 0; i < d->decos.size(); i++) {
    Deco* deco = &d->decos[i];
    switch (param) {
      case DPLocationX: {deco->location.y += step;} break;
      case DPLocationY: {deco->location.x += step;} break;
      case DPLocationZ: {deco->location.z += step;} break;
      case DPScale: {deco->scale += step;} break;
      case DPYaw: {deco->yaw += step;} break;
      default: break;
    }
  }

  for (int i = 0; i < d->structures.size(); i++) {
    Structure* s = &d->structures[i];
    switch (param) {
      case DPLocationX: {s->location.y += step;} break;
      case DPLocationY: {s->location.x += step;} break;
      case DPLocationZ: {s->location.z += step;} break;
      case DPSizeX: {s->size.x += step;} break;
      case DPSizeY: {s->size.y += step;} break;
      case DPSizeZ: {s->size.z += step;} break;
      case DPYaw: {s->angle += step;} break;
      case DPRoofSlope: {s->roofSlope += step;} break;
      default: break;
    }
  }

  designRender();
}

bool setDesignParameterValue(item param, float val) {
  if (param == DPYaw) {
    //val = fmod(val, 360);
    val = val - 360.f * floor(val/360.f);
    val *= degToRad;
  //} else if (param == DPRoofSlope) {
    //val = clamp(val, 0.f, 2.f);
  } else if (param > DPLocationZ && param < DPConstructionCost) {
    if (val < 0) val = 0;
  }

  if (param >= DPMinYear && param <= DPBiz3) val = round(val);

  Design* d = getSelectedDesign();
  switch (param) {
    case DPMinYear: {d->minYear = val;} break;
    case DPMaxYear: {d->maxYear = val;} break;
    case DPLowTide: {d->lowTide = val;} break;
    case DPHighTide: {d->highTide = val; } break;
    case DPConstructionCost: {d->cost = val / c(CAmenityCostMult); } break;
    case DPMaintenanceCost: {d->maintenance = val / c(CAmenityMaintMult); } break;
    case DPHomes: {d->numFamilies = val;} break;
    case DPBiz0: case DPBiz1: case DPBiz2: case DPBiz3: {
      int i = param-DPBiz0;
      d->numBusinesses[i] = val;
    } break;
    default: break;
  }

  alignTideValues(param);

  if (getSelectionType() == SelectionDeco) {
    Deco* deco = &d->decos[getSelection()];
    switch (param) {
      case DPLocationX: {deco->location.y = val;} break;
      case DPLocationY: {deco->location.x = val;} break;
      case DPLocationZ: {deco->location.z = val;} break;
      case DPScale: {deco->scale = val;} break;
      case DPYaw: {deco->yaw = val;} break;
      default: return false;
    }

  } else if (getSelectionType() == SelectionStructure) {
    Structure* s = &d->structures[getSelection()];
    switch (param) {
      case DPLocationX: {s->location.y = val;} break;
      case DPLocationY: {s->location.x = val;} break;
      case DPLocationZ: {s->location.z = val;} break;
      case DPSizeX: {s->size.x = val;} break;
      case DPSizeY: {s->size.y = val;} break;
      case DPSizeZ: {s->size.z = val;} break;
      case DPYaw: {s->angle = val;} break;
      case DPRoofSlope: {s->roofSlope = val;} break;
      default: return false;
    }
  }

  designRender();
  return true;
}

void updateDesignParameterStr(item param) {
  if (designParamTextBox[param].flags & _textBoxEditing) return;
  if (designParameterStr[param]) free(designParameterStr[param]);
  if (param >= DPMinYear && param <= DPBiz3) {
    designParameterStr[param] = sprintf_o("% 4d",
      int(getDesignParameterValue(param)));
  } else if (param >= DPConstructionCost && param <= DPMaintenanceCost) {
    designParameterStr[param] = printMoneyString(getDesignParameterValue(param));
  } else {
    designParameterStr[param] = sprintf_o(
        param == DPRoofSlope ? "% 7.3f"  :
        param == DPYaw ?       "% 7.3fdeg" :
        param == DPScale ?     "% 7.3fx"   :
                               "% 7.3fm",
        getDesignParameterValue(param));
  }
}

void updateDesignDimensionStrings() {
  if (getSelectedDesignNdx() == 0) return;
  //currentDCPTab = selectionTab;

  for (int i = 0; i < numDesignParameters; i ++) {
    updateDesignParameterStr(i);
  }
}

void designConfigPanelSelection() {
  currentDCPTab = selectionTab;
}

static bool designParameterStep(Part* part, InputEvent event) {
  item param = part->itemData;
  float step = part->vecData.x;
  char* str = designParameterStr[param];
  float val = getDesignParameterValue(param);
  val += step;
  setDesignParameterValue(param, val);
  updateDesignParameterStr(param);
  alignTideValues(param);
  pushDesignerUndoHistory();
  designRender();
  return true;
}

static bool designParameterBulkStep(Part* part, InputEvent event) {
  item param = part->itemData;
  float step = part->vecData.x;
  char* str = designParameterStr[param];
  designParameterBulkStep(param, step);
  updateDesignParameterStr(param);
  pushDesignerUndoHistory();
  return true;
}

static bool designParameterQuickset(Part* part, InputEvent event) {
  item param = part->itemData;
  float val = part->vecData.x;
  char* str = designParameterStr[param];
  setDesignParameterValue(param, val);
  updateDesignParameterStr(param);
  alignTideValues(param);
  pushDesignerUndoHistory();
  designRender();
  return true;
}

static float parseDesignParamter(item param) {
  float val = 0;
  float multiplier = 1;
  char* str = designParameterStr[param];
  bool neg = false;

  for (int i = 0; i < 100; i++) {
    char c = str[i];
    if (c == '\0') break;
    if (c == 'k' || c == 'K') multiplier = 1000;
    if (c == 'm' || c == 'M') multiplier = 1000000;
    if (c == 'b' || c == 'B') multiplier = 1000000000;
    if (c == 't' || c == 'T') multiplier = 1000000000000;
    if (c == '(' || c == ')') neg = true;
  }

  for (int i = 0; i < 100; i++) {
    if (str[i] == '\0') break;
    if (str[i] == '-') neg = true;
    if ((str[i] >= '0' && str[i] <= '9') || str[i] == '.') {
      str = &str[i];
      break;
    }
  }
  sscanf(str, "%f", &val);
  if (neg) multiplier *= -1;
  val *= multiplier;
  return val;
}

static bool submitDesignParamter(Part* part, InputEvent event) {
  item param = part->itemData;
  float val = parseDesignParamter(param);
  SPDLOG_WARN("submitDesignParamter {} {}", param, val);
  setDesignParameterValue(param, val);
  updateDesignParameterStr(param);
  pushDesignerUndoHistory();
  return true;
}

static bool duplicate(Part* part, InputEvent event) {
  Design* d = getSelectedDesign();

  if (getSelectionType() == SelectionStructure) {
    Structure structure = d->structures[getSelection()];
    d->structures.push_back(structure);
    setSelection(SelectionStructure, d->structures.size()-1);
    pushDesignerUndoHistory();
    designRender();
    return true;

  } else if (getSelectionType() == SelectionDeco) {
    Deco deco = d->decos[getSelection()];
    d->decos.push_back(deco);
    setSelection(SelectionDeco, d->decos.size()-1);
    pushDesignerUndoHistory();
    designRender();
    return true;

  } else {
    return false;
  }
}

bool deleteSelected(Part* part, InputEvent event) {
  Design* d = getSelectedDesign();

  if (getSelectionType() == SelectionDeco) {
    if (d->decos.size() <= getSelection()) return true;
    d->decos.erase(d->decos.begin() + getSelection());
    setDesignHasStatue(d);
  } else if (getSelectionType() == SelectionStructure) {
    if (d->structures.size() <= getSelection()) return true;
    d->structures.erase(d->structures.begin() + getSelection());
  } else {
    return false;
  }

  pushDesignerUndoHistory();
  clearSelection();
  designRender();
  return true;
}

/*
static bool changeRoofType(Part* part, InputEvent event) {
  Design* d = getSelectedDesign();
  Structure* s = &d->structures[getSelection()];
  s->roofType = (s->roofType+1) % numRoofTypes;
  s->roofSlope = defaultRoofSlopeForType(s->roofType);
  updateDesignParameterStr(DPRoofSlope);
  pushDesignerUndoHistory();
  designRender();
  return true;
}

static bool changeDecoType(Part* part, InputEvent event) {
  Design* d = getSelectedDesign();
  Deco* deco = &d->decos[getSelection()];
  if (part->itemData > 0) {
    deco->decoType = (deco->decoType+1) % sizeDecoTypes();
  } else {
    deco->decoType = (deco->decoType+sizeDecoTypes()-1) % sizeDecoTypes();
  }
  pushDesignerUndoHistory();
  designRender();
  return true;
}
*/

void designParameterTextBox(Part* result, vec2 loc, vec2 size, item param) {
  const char* designParameterName = designParameterNames[param];
  bool isEdu = isDesignEducation(getSelectedDesignNdx());
  bool isHotel = getSelectedDesign()->flags & _designIsHotel;
  if (isEdu && param >= DPHomes && param <= DPBiz3) {
    designParameterName = universityTypeNames[param - DPHomes];
  }
  if (isHotel && param == DPHomes) {
    designParameterName = "Hotel Rooms";
  }
  r(result, label(loc, 0.7, strdup_s(designParameterName)));

  if (designParameterStr[param] == 0) {
    updateDesignParameterStr(param);
  }

  bool editing = designParamTextBox[param].flags & _textBoxEditing;
  bool was = wasEditingParam[param];
  if (editing && !was) {
    if (designParameterStr[param] != 0) free(designParameterStr[param]);
    designParameterStr[param] = sprintf_o("%.2f", getDesignParameterValue(param));
    wasEditingParam[param] = true;

  } else if (editing && was) {
    float val = parseDesignParamter(param);
    setDesignParameterValue(param, val);

  } else if (!editing && was) {
    wasEditingParam[param] = false;
    updateDesignParameterStr(param);
    pushDesignerUndoHistory();
  }

  designParamTextBox[param].text = &designParameterStr[param];
  Part* tb = r(result, textBoxLabel(loc + vec2(0,0.7), size - vec2(0,.7),
        &designParamTextBox[param]));
  tb->itemData = param;
}

Part* designConfigPanel() {
  Building* b = getBuilding(getSelectedDesignNdx());
  Design* d = getSelectedDesign();
  bool isGov = d->zone == GovernmentZone;
  bool isEdu = isDesignEducation(getSelectedDesignNdx());
  bool isHotel = d->flags & _designIsHotel;
  bool hasSelection = getSelectionType() != NoSelection;

  if (!isGov && currentDCPTab > mainTab) {
    currentDCPTab = mainTab;
  }

  if (!hasSelection && currentDCPTab == selectionTab) {
    currentDCPTab = mainTab;
  }

  if (designTextBox.flags & _textBoxEditing) {
    wasEditing = true;
  } else if (wasEditing) {
    pushDesignerUndoHistory();
    wasEditing = false;
  }

  Part* result = panel(vec2(0,0),
      vec2(dcpWidth+dcpPadding*2, (dcpHeight+dcpPadding*2)));
  result->padding = dcpPadding;

  r(result, labelCenter(vec2(0,0), vec2(dcpWidth,1),
        strdup_s("Building Designer")));

  #ifdef INCLUDE_STEAM
    // Even if Steam's included, only show buttons if it has an active connection
    if (steam_isActive()) {
      Part* steamButt = button(vec2(dcpWidth-1, 0.f), iconSteam, openDesignInWorkshop, getSelectedDesignNdx());
      setPartTooltipValues(steamButt, TooltipType::DesignerOpenInWorkshop);
      r(result, steamButt);
    }
  #endif

  float y = 1;

  if (textureSelectionState != TSSClosed) {
    r(result, textureSelectPanel(vec2(dcpWidth+dcpPadding*2, 1.25), vec2(17.4f, dcpHeight-1.25f+dcpPadding*1), textureSelectionState));
  }

  Part* tabPanel = panel(vec2(-dcpPadding,y), vec2(dcpWidth+dcpPadding*2,1));
  tabPanel->flags |= _partLowered;
  r(result, tabPanel);
  y += 1.25f;

  int numDCPTabsAvailable = (isGov ? numDCPTabs : (mainTab+1)) + hasSelection;
  float tabx = (dcpWidth+dcpPadding*2 - numDCPTabsAvailable)*.5f;

  if (hasSelection) {
    Part* selectButt = button(vec2(tabx, 0.f), iconPointer,
        setDCPTab, selectionTab);
    setPartTooltipValues(selectButt,
      TooltipType::DesignerConfigSelect);
    if (currentDCPTab == selectionTab) {
      selectButt->flags |= _partHighlight;
    }
    r(tabPanel, selectButt);
    tabx++;
  }

  Part* paintButt = button(vec2(tabx, 0.f), iconPaint,
      setDCPTab, paintTab);
  setPartTooltipValues(paintButt,
    TooltipType::DesignerConfigVisual);
  if (currentDCPTab == paintTab) {
    paintButt->flags |= _partHighlight;
  }
  r(tabPanel, paintButt);
  tabx++;

  Part* mainButt = button(vec2(tabx, 0.f), iconHouse,
      setDCPTab, mainTab);
  setPartTooltipValues(mainButt,
    TooltipType::DesignerConfigGame);
  if (currentDCPTab == mainTab) {
    mainButt->flags |= _partHighlight;
  }
  r(tabPanel, mainButt);
  tabx++;

  if (d->zone == GovernmentZone) {
    Part* detailsButt = button(vec2(tabx, 0.f), iconWrench,
        setDCPTab, detailsTab);
    if (currentDCPTab == detailsTab) {
      detailsButt->flags |= _partHighlight;
    }
    r(tabPanel, detailsButt);
    tabx++;

    Part* effectsButt = button(vec2(tabx, 0.f), iconBusiness,
        setDCPTab, effectsTab);
    if (currentDCPTab == effectsTab) {
      effectsButt->flags |= _partHighlight;
    }
    r(tabPanel, effectsButt);
    tabx++;
  }

  if (currentDCPTab == selectionTab) {
    bool isDeco = getSelectionType() == SelectionDeco;
    if (getSelection() < 0) clearSelection();

    char* name = 0;
    item decoType = 0;
    if (isDeco) {
      if (getSelection() >= d->decos.size()) {
        clearSelection();
      } else {
        Deco* deco = &d->decos[getSelection()];
        decoType = deco->decoType;
        name = strdup_s(getDecoTypeName(decoType));
      }
    } else {
      if (getSelection() >= d->structures.size()) {
        clearSelection();
      } else {
        Structure* s = &d->structures[getSelection()];
        if (s->roofType < 0) {
          name = sprintf_o("%s Roof",
              getRoofTypeName(s->roofType));
        } else {
          name = sprintf_o("%s Roof Structure",
              getRoofTypeName(s->roofType));
        }
      }
    }

    r(result, labelCenter(vec2(0,y), vec2(dcpWidth, 1), name));
    y += 1 + dcpPadding;

    float buttScale = 0.6f;
    r(result, button(vec2(0,y), iconCopy, vec2(dcpWidth*.5f,buttScale), strdup_s(isDeco ? "Duplicate Decoration" : "Duplicate Structure"), duplicate, 0));
    Part* keyButt = r(result, button(vec2(0,y), iconNull, vec2(buttScale, buttScale), duplicate, 0));
    keyButt->inputAction = ActDesignDuplicate;

    r(result, button(vec2(dcpWidth*.55f,y), iconTrash, vec2(dcpWidth*.5f,buttScale), strdup_s(isDeco ? "Delete Decoration" : "Delete Structure"), deleteSelected, 0));
    y += dcpScale + dcpPadding;

    /*
    if (isDeco) {
      r(result, button(vec2(0,y), iconLeft, vec2(dcpScale,dcpScale),
          changeDecoType, -1));
      const char* lblStr = "Change Decoration Type";
      float lblSize = stringWidth(lblStr) * (dcpScale-.25f);
      r(result, label(vec2(dcpScale,y), dcpScale, strdup_s(lblStr)));
      r(result, button(vec2(lblSize+dcpScale+.2f,y), iconRight,
            vec2(dcpScale,dcpScale), changeDecoType, 1));

    } else {
      r(result, button(vec2(0,y), iconRoof, vec2(dcpWidth,dcpScale),
        strdup_s("Change Roof Type"), changeRoofType, 0));
    }
    y += dcpScale + dcpPadding;
    */

    for (int param = 0; param <= DPRoofSlope; param++) {
      if (isDeco) {
        if (param >= DPSizeX) {
          if (decoType < numLegacyDecoTypes) {
            continue;
          } else if (param != DPYaw && param != DPScale) {
            continue;
          }
        }
      } else if (param == DPScale) {
        continue;
      }

      if (param > 0) {
        r(result, hr(vec2(.25f,y-.05f), dcpWidth-.5f));
      }
      y += dcpPadding;

      float paramLabelX = 0.3f;

      r(result, label(vec2(paramLabelX,y-.1f), 0.6f, strdup_s(
              param == DPRoofSlope ? "1.00"  :
              param == DPYaw ?       "10deg" :
              param == DPScale ?     "10.0x"   :
                                     "10.0m")));
      r(result, label(vec2(paramLabelX,y+.4f), 0.6f, strdup_s(
              param == DPRoofSlope ? "0.1"  :
              param == DPYaw ?       " 1deg" :
              param == DPScale ?     " 1.0x"   :
                                     " 1.0m")));
      r(result, label(vec2(paramLabelX,y+.9f), 0.6f, strdup_s(
              param == DPRoofSlope ? "0.01"  :
              param == DPYaw ?       ".1deg" :
              param == DPScale ?     " 0.1x"   :
                                     " 0.1m")));

      for (int u = 0; u < 6; u ++) {
        float df = u/2 == 0 ? 10 : u/2 == 1 ? 1 : 0.1;
        if (param == DPRoofSlope) df *= 0.1;
        if (u%2 == 1) df *= -1;
        float xOff = u % 2 == 0 ? 1.6f : 0;
        float yOff = (u/2) * .5f - .1f;
        Part* dfButt = r(result, button(vec2(xOff,y+yOff),
              vec2(0.6f, 0.6f), strdup_s(u%2==0 ? "+" : "-"),
              designParameterStep));
        dfButt->itemData = param;
        dfButt->vecData.x = df;
      }

      designParameterTextBox(result, vec2(2,y),
          vec2(dcpWidth-2,dcpScale+0.7), param);
      y += dcpScale + dcpPadding + 0.7;
    }

  } else if (currentDCPTab == paintTab) {
    r(result, icon(vec2(0,y), vec2(dcpScale, dcpScale), iconPaint));
    item numTex = numMatchingTexturesForBuilding(getSelectedDesignNdx());
    float colorSliderLoc = numTex <= 1 ? 0.f : b->color*1.0f/(numTex-1);
    float sliderWidth = (dcpWidth*.72f)-dcpScale-dcpPadding;
    colorSliderLoc = clamp(colorSliderLoc, 0.f, 1.f);
    b->color = colorSliderLoc*(numTex-1);
    r(result, slider(vec2(dcpScale,y), vec2(sliderWidth,dcpScale), colorSliderLoc, setColor));
    Part* tspAlbedoBtn = r(result, button(vec2(dcpWidth*.72f,y), iconMenu, vec2(dcpWidth*.28f, dcpScale), strdup_s("Select"), openTextureSelect, TSSAlbedo));
    if (textureSelectionState == TSSAlbedo) tspAlbedoBtn->flags |= _partHighlight;
    y += dcpScale;

    r(result, label(vec2(0,y), dcpScale, sprintf_o("Texture - %d/%d", b->color+1, numTex)));
    r(result, button(vec2(dcpWidth*.72f,y), iconTrash, vec2(dcpWidth*.28f, dcpScale), strdup_s("Delete"), deleteTexture, 0));
    if (!designHasPackageTextures(getSelectedDesignNdx())) {
      r(result, button(vec2(dcpWidth*.72f-dcpScale-dcpPadding,y), iconPlus, vec2(dcpScale, dcpScale), selectCurrentTexture, 0));
    }
    y += dcpScale * 1.f;

    if (b->entity != 0) {
      item texNdx = getEntity(b->entity)->texture;
      r(result, label(vec2(0, y), vec2(dcpWidth*.72f, 0.6), sprintf_o("%s", getTextureFilename(texNdx))));

      y += 0.6*2;
    }

    float l = getBuildingTextureLightLevel();
    r(result, icon(vec2(0,y), vec2(dcpScale, dcpScale), iconPaint));
    r(result, slider(vec2(dcpScale,y), vec2(sliderWidth,dcpScale),
          l/10.f, setLightLevel));
    Part* tspIllumBtn = r(result, button(vec2(dcpWidth*.72f,y), iconMenu, vec2(dcpWidth*.28f, dcpScale), strdup_s("Select"), openTextureSelect, TSSIllumination));
    if (textureSelectionState == TSSIllumination) tspIllumBtn->flags |= _partHighlight;
    y += dcpScale;

    r(result, label(vec2(0,y), dcpScale,
          sprintf_o("Illumination - %d", int(l))));
    r(result, button(vec2(dcpWidth*.72f,y), iconTrash, vec2(dcpWidth*.28f, dcpScale), strdup_s("Delete"), deleteTexture, 1));
    if (!designHasPackageTextures(getSelectedDesignNdx())) {
      r(result, button(vec2(dcpWidth*.72f-dcpScale-dcpPadding,y), iconPlus, vec2(dcpScale, dcpScale), selectCurrentTexture, 1));
    }
    y += dcpScale;

    if (b->entity != 0) {
      item texNdx = getEntity(b->entity)->texture;
      r(result, label(vec2(0, y), vec2(dcpWidth*.72f, 0.6), sprintf_o("%s", getTextureIllumFilename(texNdx))));

      y += 0.6*2;
    }

    y += dcpScale * 1.f;

    // Aquatic Buildings
    bool isAquatic = isDesignAquatic(getSelectedDesignNdx()) ||
      wasEditingParam[DPLowTide];
    r(result, icon(vec2(0,y), vec2(dcpScale, dcpScale), iconTide));
    r(result, button(vec2(1,y), isAquatic ? iconCheck : iconNull,
          vec2(dcpWidth-1, dcpScale), strdup_s("Aquatic Building"),
          toggleAquatic, 0));
    y += dcpScale;

    vec2 dpSize = vec2(dcpWidth*.5f-1,dcpScale + 0.7);
    if (isAquatic) {
      for (int param = DPLowTide; param <= DPHighTide; param++) {
        float x = (param == DPHighTide) * (dpSize.x+1);
        designParameterTextBox(result, vec2(x+1,y), dpSize, param);

        for (int u = 0; u < 2; u ++) {
          float df = 10;
          if (u%2 == 1) df*=-1;
          float yOff = u % 2 == 0 ? 0.6f : 0.f;
          Part* dfButt = r(result, button(vec2(x+0.4,y+yOff),
                vec2(0.6f, 0.6f), strdup_s(u%2==0 ? "+" : "-"),
                designParameterStep));
          dfButt->itemData = param;
          dfButt->vecData.x = df;
        }
      }

      y += dpSize.y + dcpPadding;
      r(result, span(vec2(0,y), 0, vec2(dcpWidth,0.6),
        strdup_s("When the building is placed, everything before High Tide"
          " will be land. Everything after Low Tide will be water. Anything" 
          " between High Tide and Low Tide can be either land or water."
          " The line of red buoys represents Low Tide."),
        &y));
    }
    y += dcpScale;

    // Move Design
    r(result, label(vec2(0,y), dcpScale, strdup_s("Move Design")));
    y += dcpScale;
    for (int param = DPLocationX; param <= DPLocationZ; param++) {
      r(result, label(vec2(1.3f,y-.1f), 0.6f, strdup_s("10.0")));
      r(result, label(vec2(1.3f,y+.4f), 0.6f, strdup_s(" 1.0")));
      r(result, label(vec2(1.3f,y+.9f), 0.6f, strdup_s(" 0.1")));

      for (int u = 0; u < 6; u ++) {
        float df = u/2 == 0 ? 10 : u/2 == 1 ? 1 : 0.1;
        if (param == DPRoofSlope) df *= 0.1;
        if (u%2 == 1) df *= -1;
        float xOff = u % 2 == 1 ? 1 : 2.3f;
        float yOff = (u/2) * .5f - .1f;
        Part* dfButt = r(result, button(vec2(xOff,y+yOff),
              vec2(0.6f, 0.6f), strdup_s(u%2==0 ? "+" : "-"),
              designParameterBulkStep));
        dfButt->itemData = param;
        dfButt->vecData.x = df;
      }
      r(result, label(vec2(3,y+0.55f), dcpScale,
            strdup_s(designParameterNames[param])));
      y += 1.5;
    }

  } else if (currentDCPTab == mainTab) {
    vec2 dpSize = vec2(dcpWidth*.5f-2,dcpScale + 0.7);
    vec2 zbSize = vec2(dcpWidth*.5f,dcpScale);

    r(result, label(vec2(0,y), dcpScale, strdup_s("Zone")));
    y += dcpScale;
    Part* zones = panel(vec2(0,y),
        vec2(dcpWidth, dcpScale*(numZoneTypes-1)*.5f));
    y += zbSize.y*(numZoneTypes/2) + 0.7;
    for (int i=1; i < numZoneTypes; i++) {
      Part* butt = button(vec2(((i-1)%2)*dcpWidth*.5f,((i-1)/2)*dcpScale), 
          zbSize, strdup_s(zoneName[i]), setZone);
      butt->itemData = i;
      if (i == d->zone) {
        butt->flags |= _partHighlight;
      }
      r(zones, butt);
    }
    r(result, zones);

    for (int param = DPMinYear; param <= DPBiz3; param++) {
      bool odd = ((param-DPMinYear) % 2);
      float x = odd * (dpSize.x+2);

      vec3 ico = designParameterIcon[param];
      if (isEdu&& param >= DPHomes) ico = universityIcons[param-DPHomes];
      if (isHotel && param == DPHomes) ico = iconHotelRoom;
      r(result, icon(vec2(x,y), vec2(dcpScale, dcpScale), ico));
      for (int u = 0; u < 2; u ++) {
        float df = 1;
        if (u%2 == 1) df*=-1;
        float xOff = u % 2 == 0 ? 0.6f : 0.f;
        Part* dfButt = r(result, button(vec2(x+xOff,y+dcpScale+0.1),
              vec2(0.6f, 0.6f), strdup_s(u%2==0 ? "+" : "-"),
              designParameterStep));
        dfButt->itemData = param;
        dfButt->vecData.x = df;
      }

      designParameterTextBox(result, vec2(x+1,y), dpSize, param);
      if (odd || param == DPBiz3) y += dpSize.y + dcpPadding;
    }

    r(result, icon(vec2(0,y), vec2(dcpScale, dcpScale), iconHeatmap[1]));
    Line valueLine = getHeatmapGradient(Value);
    Part* valSlide = r(result, slider(vec2(dcpScale,y),
          vec2(dcpWidth-dcpScale,dcpScale),
          d->minLandValue, setMinLandValue, valueLine));
    valSlide->renderMode = RenderGradientRotated;
    y += dcpScale;
    r(result, label(vec2(0,y), dcpScale, sprintf_o("Minimum Land Value - %d",
            (int)round(d->minLandValue*10))));
    y += dcpScale * 1.5f;

    r(result, icon(vec2(0,y), vec2(dcpScale, dcpScale), iconHeatmap[2]));
    Line densLine = getHeatmapGradient(Density);
    Part* densSlide = r(result, slider(vec2(dcpScale,y),
          vec2(dcpWidth-dcpScale,dcpScale),
          d->minDensity, setMinDensity, densLine));
    densSlide->renderMode = RenderGradientRotated;
    y += dcpScale;
    r(result, label(vec2(0,y), dcpScale,
          sprintf_o("Minimum Density - %d", (int)round(d->minDensity*10))));
    y += dcpScale * 1.5f;

    if (!isGov) {
      float value = getDesignValue(1,
          ourCityEconNdx(), d->minLandValue, d->minDensity);
      char* valueStr = printMoneyString(value);
      r(result, label(vec2(0,y), dcpScale,
            sprintf_o("%s Value (1950)", valueStr)));
      free(valueStr);
      y += dcpScale * 1.5f;
    }

    r(result, button(vec2(0,y),
          d->flags & _designIsHotel ? iconCheck : iconNull,
          vec2(dcpWidth, dcpScale), strdup_s("Is a Hotel"),
          toggleDesignFlag, _designIsHotel));
    y += dcpScale;

    r(result, button(vec2(0,y),
          !(d->flags & _designDisableSpawning) ? iconCheck : iconNull,
          vec2(dcpWidth, dcpScale), strdup_s("Spawnable"),
          toggleDesignFlag, _designDisableSpawning));
    y += dcpScale;

  } else if (currentDCPTab == detailsTab && isGov) {
    r(result, label(vec2(0,y), dcpScale, strdup_s("Name")));
    y += 1;

    if (d->displayName == 0) d->displayName = strdup_s("");
    designTextBox.text = &d->displayName;
    Part* tb = r(result, textBoxLabel(vec2(0,y),
          vec2(dcpWidth,1), &designTextBox));
    y += 1;

    r(result, label(vec2(0,y), dcpScale,
        sprintf_o("Category: %s", getBuildingCategoryString(d->category))));
    y += dcpScale;

    for (int i=0; i < numBuildingCategories; i++) {
      Part* butt = button(vec2(i*dcpScale,y), iconBuildingCategory[i],
          vec2(dcpScale,dcpScale), setDesignCategory, i);
      if (i == d->category) {
        butt->flags |= _partHighlight;
      }
      r(result, butt);
    }
    y += dcpScale * 1.5f;

    float totalEffects = 0;
    for (int i = 0; i < numEffects; i++) {
      totalEffects += d->effect[i];
    }

    r(result, label(vec2(0,y), dcpScale, strdup_s("Costs (1950 Dollars)")));
    y += 1;

    vec2 dpSize = vec2(dcpWidth*.5f - 1, dcpScale + 0.7);
    float dy = y;
    for (int param = DPConstructionCost; param <= DPMaintenanceCost; param++) {
      dy = y;
      bool isMaint = param == DPMaintenanceCost;
      float dx = isMaint * dcpWidth*.5f;
      r(result, icon(vec2(dx, dy), vec2(dcpScale, dcpScale), isMaint ? iconWrench : iconHeatmap[2]));

      float costStep = 1;
      float costEffectStep = isMaint ? c(CRecommendedAmenityMaint)*c(CAmenityMaintMult) :
        c(CRecommendedAmenityCost) * c(CAmenityCostMult);
      float costVal = abs(getDesignParameterValue(param));
      if (costVal < costEffectStep / 20) costVal = costEffectStep / 20;
      while (costStep * 20 <= costVal) {
        costStep *= 10;
      }

      for (int u = 0; u < 2; u++) {
        float df = costStep;
        if (u % 2 == 1) df *= -1;
        float xOff = u % 2 == 0 ? 0.4f : 0;
        Part* dfButt = r(result, button(vec2(dx+xOff, dy + dcpScale+0.05f),
          vec2(0.6f, 0.6f), strdup_s(u % 2 == 0 ? "+" : "-"),
          designParameterStep));
        dfButt->itemData = param;
        dfButt->vecData.x = df;
      }

      designParameterTextBox(result, vec2(dx+dcpScale, dy), dpSize, param);
      dy += dpSize.y;

      float recCost = totalEffects * costEffectStep;
      char* recCostStr = printMoneyString(recCost);
      Part* recPart = r(result, button(vec2(dx+dcpScale, dy), vec2(dpSize.x, 1.5f), strdup_s(""),
        designParameterQuickset));
      r(result, label(vec2(dx+dcpScale, dy), 0.7f, sprintf_o("Recommended:\n%s", recCostStr)));
      recPart->itemData = param;
      recPart->vecData.x = recCost;
      free(recCostStr);
      dy += 2;
    }
    y = dy;

    r(result, button(vec2(0,y),
          d->flags & _designSingleton ? iconCheck : iconNull,
          vec2(dcpWidth, dcpScale), strdup_s("Limit One Per City"),
          toggleDesignFlag, _designSingleton));
    y += dcpScale;

    r(result, button(vec2(0,y),
          d->flags & _designAlwaysAvailable ? iconCheck : iconNull,
          vec2(dcpWidth, dcpScale), strdup_s("Always Available"),
          toggleDesignFlag, _designAlwaysAvailable));
    y += dcpScale;

    r(result, button(vec2(0,y),
          d->numBusinesses[Institution] > 0 ? iconCheck : iconNull,
          vec2(dcpWidth, dcpScale), strdup_s("Is an Institution"),
          toggleInstitution, 0));
    y += dcpScale;

    r(result, button(vec2(0,y),
          d->flags & _designNoEminentDomain ? iconCheck : iconNull,
          vec2(dcpWidth, dcpScale), strdup_s("No Eminent Domain Costs"),
          toggleDesignFlag, _designNoEminentDomain));
    y += dcpScale;

    y += dcpScale;

    r(result, button(vec2(0,y),
          d->flags & _designProvidesHSDiploma ? iconCheck : iconNull,
          vec2(dcpWidth, dcpScale), strdup_s("Grants High School Diplomas"),
          toggleDesignFlag, _designProvidesHSDiploma));
    y += dcpScale;

    r(result, button(vec2(0,y),
          d->flags & _designProvidesBclDegree ? iconCheck : iconNull,
          vec2(dcpWidth, dcpScale), strdup_s("Grants Bachelor's Degrees"),
          toggleDesignFlag, _designProvidesBclDegree));
    y += dcpScale;

    r(result, button(vec2(0,y),
          d->flags & _designProvidesPhd ? iconCheck : iconNull,
          vec2(dcpWidth, dcpScale), strdup_s("Grants Doctorates"),
          toggleDesignFlag, _designProvidesPhd));
    y += dcpScale;

    r(result, button(vec2(0,y),
          d->flags & _designProvidesHealthCare ? iconCheck : iconNull,
          vec2(dcpWidth, dcpScale), strdup_s("Provides Health Care"),
          toggleDesignFlag, _designProvidesHealthCare));
    y += dcpScale;

  } else if (isGov && currentDCPTab == effectsTab) {
    //Second column
    //Part* effectsPanel = panel(vec2(dcpWidth+dcpPadding*2,-dcpPadding),
        //vec2(dcpWidth+dcpPadding*2, dcpHeight+dcpPadding*2));
    //effectsPanel->padding = dcpPadding;

    float totalEffects = 0;
    int numEffectsSelected = 0;
    for (int i = 0; i < numEffects; i++) {
      totalEffects += d->effect[i];
      if (d->effect[i] != 0) {
        numEffectsSelected ++;
      }
    }
    r(result, label(vec2(0,y), dcpScale, sprintf_o("Effects (%s%d/%d)",
            totalEffects < 0 ? "" : "+",
            int(totalEffects),
            numEffectsSelected)));
    y += dcpScale;
    r(result, hr(vec2(0,y), dcpWidth));
    y += 0.25f;

    /*
    Part* maxLbl = label(vec2(0,y), 0.75,
        strdup_s("Recommended: 4 Effects Max"));
    if (numEffectsSelected > 4) {
      maxLbl->foregroundColor = PickerPalette::RedLight;
    }
    r(result, maxLbl);
    y += 0.75f;
    */

    for (int i = 0; i < numEffects; i++) {
      float val = d->effect[i];
      int number = abs(val);
      bool negative = val < 0;
      float sliderX = val / c(CMaxEffect) * .5f + .5f;

      r(result, icon(vec2(0, y), vec2(dcpScale, dcpScale),
            getEffectIcon(i)));

      Part* slide = slider(vec2(dcpScale,y),
          vec2(dcpWidth*.5f-dcpScale,dcpScale),
            sliderX, setBuildingEffect);
      slide->itemData = i;
      r(result, slide);

      r(result, label(vec2(dcpWidth*.5f, y), dcpScale,
        sprintf_o("%s%d %s", negative ? "-" : "+",
          number, getEffectString(i))));
      y += dcpScale * 1.1f;

      if (number != 0) {
        char* desc = getEffectDescriptor(i, val, d->flags);
        r(result, label(vec2(dcpScale, y), 0.75, desc));
        y += 1;
      }
    }
  }

  return result;
}

#include "designOrganizerPanel.cpp"

