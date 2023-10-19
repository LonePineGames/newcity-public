#include "../amenity.hpp"
#include "../building/design.hpp"
#include "../person.hpp"
#include "../sound.hpp"

#include "../parts/amenityInfo.hpp"
#include "../parts/block.hpp"
#include "../parts/mainMenu.hpp"
#include "../parts/root.hpp"
#include "../parts/scrollbox.hpp"
#include "../parts/textBox.hpp"

static item buildingPlan = 0;
static item lastLot = 0;
static item currentDesign = 0;
static item currentDesignByCategory[] = {0,0,0,0,0};
static item currentCategory = 0;
static ScrollState buildingScroll;
static bool anyBuildingPlop;
static TextBoxState searchTB;
static char* searchText = 0;
static vector<item> emDomBuildingsB;
const bool highlightEmDom = false;
static bool editButtonHover = false;

void setAnyBuildingPlop(bool val) {
  anyBuildingPlop = val;
}

void setAmenityInTool(item amenity) {
  currentDesign = amenity;
  if (currentDesign != 0) {
    Design* d = getDesign(currentDesign);
    currentCategory = d->category;
    currentDesignByCategory[currentCategory] = currentDesign;
  }
}

void setHeatMapAndHighlight(bool on) {
  //if (isQueryHeatmapSet()) return;
  if (on) {
    if (buildingPlan != 0) {
      Plan* p = getPlan(buildingPlan);
      Building* b = getBuilding(p->element);
      setGuide(b->location, getAmenityThrow(b->design));
    }

  } else {
    //highlightGovernmentBuildings(-1, false);
    //setHeatMap(Pollution, false);
    stopGuide();
  }
}

void building_mouse_button_callback(InputEvent event) {
  int button = event.button;
  int action = event.action;

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    if (buildingPlan != 0) {
      playSound(_soundClickComplete);
      reportTutorialUpdate(TutorialUpdateCode::BuildAmenity);
      if (!buyPlan(buildingPlan)) {
        discardPlan(buildingPlan);
      }
      buildingPlan = 0;
    }
  }

  setHeatMapAndHighlight(true);
}

void building_mouse_move_callback(InputEvent event) {
  item lot = 0;
  if (currentDesign > 0 && buildingPlan != 0) {
    lot = nearestLot(event.mouseLine, true);
    if (lot == lastLot) {
      return; // same as last frame, skip this frame for efficiency
    }

  } else {
    lastLot = 0;
  }

  for (int i = 0; i < emDomBuildingsB.size(); i++) {
    setRedHighlight(false, SelectionBuilding, emDomBuildingsB[i]);
  }
  emDomBuildingsB.clear();
  setHeatMapAndHighlight(true);

  if (buildingPlan != 0) {
    discardPlan(buildingPlan);
    buildingPlan = 0;
  }

  if (currentDesign > 0) {
    lot = nearestLot(event.mouseLine, true);
    //vec3 mouseGroundIntersection = landIntersect(event.mouseLine);
    if (lot != 0) {
      item b = addGovernmentBuilding(lot, currentDesign);
      if (b != 0) {
        buildingPlan = addPlan(BuildingPlan, b);
        if (highlightEmDom) {
          emDomBuildingsB = collideBuilding(b);
        }

        Building* building = getBuilding(b);
        if (getGameMode() == ModeGame) {
          building->flags |= _buildingHistorical;
          building->flags |= _buildingWasPlopped;
        }

        bool shift = event.mods & GLFW_MOD_SHIFT;
        Plan* plan = getPlan(buildingPlan);
        if (shift) {
          plan->flags |= _planForceDemolish;
        } else {
          plan->flags &= ~_planForceDemolish;
        }
      }
    }
  }

  if (highlightEmDom) {
    for (int i = 0; i < emDomBuildingsB.size(); i++) {
      setRedHighlight(true, SelectionBuilding, emDomBuildingsB[i]);
    }
  }
}

void building_select() {
  setHeatMapAndHighlight(true);
}

void building_reset() {
  for (int i = 0; i < emDomBuildingsB.size(); i++) {
    setRedHighlight(false, SelectionBuilding, emDomBuildingsB[i]);
  }

  emDomBuildingsB.clear();
  setHeatMapAndHighlight(false);
  editButtonHover = false;
  lastLot = 0;

  if (buildingPlan != 0) {
    discardPlan(buildingPlan);
    buildingPlan = 0;
  }
}

bool setCategory(Part* part, InputEvent event) {
  currentCategory = part->itemData;
  setHeatMapAndHighlight(true);
  buildingScroll.amount = 0;

  /*
  if (currentDesignByCategory[currentCategory] < 1) {
    for (int i=1; i <= sizeDesigns(); i++) {
      Design* d = getDesign(i);
      if (d->category == currentCategory) {
        currentDesignByCategory[currentCategory] = i;
        break;
      }
    }
  }

  currentDesign = currentDesignByCategory[currentCategory];
  */

  return true;
}

bool setCurrentDesign(Part* part, InputEvent event) {
  currentDesign = part->itemData;

  if (currentDesign > 0) {
    Design* d = getDesign(currentDesign);
    string name = d->name;
    if (stringContainsCaseInsensitive(name, "school")) {
      reportTutorialUpdate(TutorialUpdateCode::SelectedSchoolInAmenityTool);
    }
  }

  return true;
}

bool editInDesigner(Part* part, InputEvent event) {
  if (currentDesign <= 0) return false;
  closeMenus();
  Design* d = getDesign(currentDesign);
  gameLoad(LoadTargetFilename, ModeBuildingDesigner, strdup_s(d->name));
  return true;
}

bool setEditButtonHover(Part* part, InputEvent event) {
  if (event.action != MOUSE_MOVED_INPUT) return false;
  if (!event.isMouse) return false;
  editButtonHover = true;
  return true;
}

Part* building_render(Line dim) {
  int iter = 0;
  bool anyPlop = currentCategory == numBuildingCategories;
  /*
  while (!anyPlop &&
      getCategoryCount(currentCategory) == 0) {
    currentCategory = (currentCategory+1)%numBuildingCategories;
    iter ++;
    if (iter > numBuildingCategories + 2) {
      if (anyBuildingPlop) {
        currentCategory = numBuildingCategories;
        break;
      } else {
        Part* result = panel(dim);
        r(result, labelCenter(vec2(0,3), vec2(dim.end.x,1),
              strdup_s("No Amenities Enabled")));
        return result;
      }
    }
  }
  */

  float k = 0;
  float scale = 0.75;
  vector<item> buildingList;
  vector<money> buildingCost;

  for (int i=1; i <= sizeDesigns(); i++) {
    Design* d = getDesign(i);
    if (!(d->flags & _designEnabled)) continue;
    if (d->flags & _designDisableSpawning) continue;

    if (anyPlop) {
      if (d->zone == GovernmentZone) continue;
      if (searchText != 0 &&
          strlength(searchText) > 0) {
        std::string nameStr = d->name;
        if (nameStr.find(searchText) == std::string::npos) continue;
      }

    } else if (
      //!getGovBuildingAvailable(i) ||
      d->zone != GovernmentZone ||
      d->category != currentCategory ||
      d->minYear > getCurrentYear() ||
      d->maxYear < getCurrentYear() ||
      ((d->flags & _designSingleton) && getGovBuildingsPlaced(i) >= 1)) {
      if (i == currentDesign) {
        currentDesign = currentDesignByCategory[currentCategory] = 0;
      }
      continue;
    }

    float cost = anyPlop ? getDesignValue(i) : d->cost;
    if (!getGovBuildingAvailable(i)) cost += 1e10;
    buildingList.push_back(i);
    buildingCost.push_back(cost);
  }

  // Insertion sort
  item numB = buildingList.size();
  item* indices = (item*) alloca(sizeof(item)*numB);
  for (int i = 0; i < numB; i++) {
    indices[i] = i;
  }
  for (int i = 1; i < numB; i++) {
    item x = indices[i];
    float xCost = buildingCost[x];
    int j = i-1;
    for (; j >= 0 && buildingCost[indices[j]] > xCost; j--) {
      indices[j+1] = indices[j];
    }
    indices[j+1] = x;
  }

  if (currentDesign == 0 && numB > 0) {
    currentDesign = buildingList[indices[0]];
    currentDesignByCategory[currentCategory] = currentDesign;
  }

  Part* result = panel(dim);
  r(result, label(vec2(0,0), 1,
        strdup_s(getBuildingCategoryString(currentCategory))));
  Part* scroll = scrollbox(vec2(0,0), vec2(10,4.75));

  for (int i = 0; i < numB; i++) {
    item ndx = buildingList[indices[i]];
    Design* d = getDesign(ndx);
    Part* butt = button(vec2(0,k), vec2(9,scale),
      strdup_s(anyPlop ? d->name : d->displayName), setCurrentDesign);
    k += scale;
    butt->itemData = ndx;

    if (ndx == currentDesign) butt->flags |= _partHighlight;
    if (!anyPlop && !getGovBuildingAvailable(ndx)){
      butt->foregroundColor = PickerPalette::GrayDark;
    }

    r(scroll, butt);
  }

  r(result, scrollboxFrame(vec2(0,1), vec2(10,4.75), &buildingScroll, scroll));

  Part* tabPanel = panel(vec2(0,6), vec2(8,1));
  tabPanel->renderMode = RenderTransparent;
  r(result, tabPanel);

  for (int i=0; i < numBuildingCategories + anyBuildingPlop; i++) {
    //if (i != numBuildingCategories && getCategoryCount(i) == 0) {
      //continue;
    //}

    Part* butt = button(vec2(i,0), iconBuildingCategory[i], setCategory, i);
    if (i == currentCategory) {
      butt->flags |= _partHighlight;
    }
    setPartTooltipValues(butt,
      TooltipType::AmeEdu+i);
    butt->inputAction = (InputAction)(ActATEducation+i);
    r(tabPanel, butt);
  }

  if (anyPlop) {
    searchTB.text = &searchText;
    Part* tb = r(result, textBoxLabel(vec2(5,0), vec2(4, 0.85), &searchTB,
          iconQuery, "Search"));
  }

  return result;
}

void buildingInstructionPanel(Part* pnl) {
  if (currentDesign < 1 || currentDesign > sizeDesigns()) {
    return;
  }

  item buildingNdx = 0;
  if (buildingPlan != 0) {
    Plan* p = getPlan(buildingPlan);
    buildingNdx = p->element;
  }

  vec2 partSize = vec2(pnl->dim.end) -
    vec2(pnl->padding*2, pnl->padding*2);
  r(pnl, amenityInfoPart(vec2(0,0), partSize, currentDesign,
        buildingNdx, true));

  Part* editButt = r(pnl, button(vec2(12.5,0), iconPencil, saveConfirm, 0));
  editButt->ptrData = (void*) editInDesigner;
  editButt->flags &= ~_partFreePtr;
  editButt->onHover = setEditButtonHover;
  setPartTooltipValues(editButt, TooltipType::EditInDesigner);

  if (editButtonHover) {
    Design* design = getDesign(currentDesign);
    float hoverTextScale = 0.75;
    vec2 pos = vec2(14,0.5-hoverTextScale*0.5);
    float txtWidth = stringWidth(design->name)*(hoverTextScale-0.25f) + 0.25f;
    Part* ttPanel = panel(pos, vec2(txtWidth, hoverTextScale));
    ttPanel->dim.start.z += 10;
    ttPanel->renderMode = RenderDarkBox;

    Part* ttTxt = label(vec2(0,0), hoverTextScale, strdup_s(design->name));
    r(ttPanel, ttTxt);

    r(pnl, ttPanel);
  }
  editButtonHover = false;
}

std::string building_name() {
  return "Amenities";
}

bool building_visible() {
  return getGameMode() != ModeTest; // Prevent the amenity tool from being visible in the Scenario Editor
  //return getCategoryCount(numBuildingCategories) > 0;
}

Tool toolBuilding = {
  building_mouse_button_callback,
  building_mouse_move_callback,
  building_select,
  building_reset,
  building_visible,
  iconZoneMono[GovernmentZone],
  building_render,
  buildingInstructionPanel,
  building_name,
  _toolForceInstructions,
};

Tool* getToolAmenity() {
  return &toolBuilding;
}

