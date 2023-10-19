#include "toolbar.hpp"

#include "../building/building.hpp"
#include "../building/design.hpp"
#include "../building/renderBuilding.hpp"
#include "../input.hpp"
#include "../land.hpp"
#include "../game/game.hpp"
#include "../graph.hpp"
#include "../icons.hpp"
#include "../option.hpp"
#include "../pillar.hpp"
#include "../plan.hpp"
#include "../selection.hpp"
#include "../tutorial.hpp"
#include "../vehicle/vehicle.hpp"
#include "../string_proxy.hpp"
#include "../util.hpp"

#include "button.hpp"
#include "icon.hpp"
#include "label.hpp"
#include "leftPanel.hpp"
#include "hr.hpp"
#include "panel.hpp"
#include "part.hpp"
#include "root.hpp"
#include "tooltip.hpp"
#include "tutorialPanel.hpp"

const float toolPad = 0.1;

#include "../tools/tool.hpp"
#include "../tools/blueprint.hpp"
#include "../tools/road.hpp"
#include "../tools/query.cpp"
#include "../tools/bulldozer.cpp"
#include "../tools/zone.cpp"
#include "../tools/null.cpp"
#include "../tools/building.cpp"

#include "../tools/structure.cpp"
#include "../tools/deco.cpp"
#include "../tools/designBulldozer.cpp"
#include "../tools/designQuery.cpp"
#include "../tools/organizerQuery.cpp"

static int currentTool = 0;
static int blueprintToolNum = 0;
static Tool* tools[7];
static int numTools = 0;
bool showInstructions = false;
bool undergroundViewSelected = false;

static const char* charForNumber[] = {
  //GLFW_KEY_GRAVE_ACCENT,
  "#",
  //"~",
  "1",
  "2",
  "3",
  "4",
  "5",
  "6",
  "7",
  "8",
  "9",
  "0"
};

static const int keyCodesForNumber[] = {
  0,
  //GLFW_KEY_GRAVE_ACCENT,
  GLFW_KEY_1,
  GLFW_KEY_2,
  GLFW_KEY_3,
  GLFW_KEY_4,
  GLFW_KEY_5,
  GLFW_KEY_6,
  GLFW_KEY_7,
  GLFW_KEY_8,
  GLFW_KEY_9,
  GLFW_KEY_0
};

void setTool(int newTool) {
  if (currentTool) {
    item oldTool = currentTool;
    currentTool = 0;
    tools[oldTool]->reset();
  }
  if (currentTool == blueprintToolNum &&
      getSideBarMode() == BlueprintsListSideBar) {
    setSideBarMode(MessageBoardSideBar);
  }
  currentTool = newTool;
  stopBlinkingFeature(FQueryTool+newTool-1);
  if (currentTool) {
    tools[currentTool]->select();

    // Notify tutorial
    if(isQueryTool()) {
      reportTutorialUpdate(TutorialUpdateCode::SelectedQueryTool);
    } else if(isRoadTool()) {
      reportTutorialUpdate(TutorialUpdateCode::SelectedRoadTool);
    } else if(isZoneTool()) {
      reportTutorialUpdate(TutorialUpdateCode::SelectedZoneTool);
    } else if(isAmenityTool()) {
      reportTutorialUpdate(TutorialUpdateCode::SelectedAmenityTool);
    } else if(isBulldozerTool()) {
      reportTutorialUpdate(TutorialUpdateCode::SelectedBulldozerTool);
    } else if(isBlueprintTool()) {
      reportTutorialUpdate(TutorialUpdateCode::SelectedBlueprintTool);
    }
  } else {
    reportTutorialUpdate(TutorialUpdateCode::ClosedTool);
  }

  setLotsVisible();
  if (getGameMode() == ModeBuildingDesigner) {
    designRender();
  }
}

void setBlueprintTool() {
  setTool(blueprintToolNum);
}

bool setTool(Part* part, InputEvent event) {
  if (part == NULL) {
    SPDLOG_ERROR("Null part in setTool");
    return false;
  }

  int ndx = part->itemData;

  // Ok, so this check and bit of code protects against an unique case
  // which prevents a tool window from closing on clicking X. 
  // 
  // If you were to do a check like the following down below, it will not. 
  // "ndx > 0 ? getKeyBind(InputAction::ActTool1+(ndx-1)).active : ndx"
  // So I catch ndx == 0 up here and we're good.
  // 
  // -supersoup
  if (ndx == 0) {
    setTool(ndx);
    return true;
  }

  if (ndx < 0 || ndx > numTools) {
    SPDLOG_ERROR("Attempted to access invalid tool by index {}", part->itemData);
    return false;
  }

  if (ndx == currentTool) {
    setTool(0);
  } else {
    setTool(ndx);
  }
  return true;
}

bool toggleUndergroundView(Part* part, InputEvent event) {
  setUndergroundViewSelected(!undergroundViewSelected);
  setUndergroundView(undergroundViewSelected);
  return true;
}

void setUndergroundViewSelected(bool val) {
  undergroundViewSelected = val;
}

bool setLabelsVisible(Part* part, InputEvent event) {
  setLabelsVisible(!areLabelsVisible());
  return true;
}

bool openCitipediaPanel(Part* part, InputEvent event) {
  setLeftPanel(CitipediaPanel);
  return true;
}

bool openTutorialPanel(Part* part, InputEvent event) {
  setTutorialPanelOpen(true);
  return true;
}

bool shiftDesignerUndoHistory(Part* part, InputEvent event) {
  shiftDesignerUndoHistory(part->itemData);
  return true;
}

bool isUndergroundViewSelected() {
  return undergroundViewSelected;
}

bool getShowInstructions() {
  return showInstructions;
}

void setShowInstructions(bool show) {
  showInstructions = show;
}

bool toggleInstructions(Part* part, InputEvent event) {
  if (tools[currentTool] == &toolQuery ||
      (currentTool == 0 && isHeatMapIntense())) {
    toggleQueryInfoPanel();

  } else if ((tools[currentTool] == getToolRoad() ||
        currentTool == blueprintToolNum) && isPlansEnabled()) {
    setPlansEnabled(false);

  } else if (currentTool &&
      (tools[currentTool]->flags & _toolForceInstructions)) {
    //no-op

  } else {
    showInstructions = !showInstructions;
    writeOptions();
  }

  return true;
}

bool isRoadTool() {
  return tools[currentTool] == getToolRoad();
}

bool isQueryTool() {
  return tools[currentTool] == getToolQuery();
}

bool isZoneTool() {
  return tools[currentTool] == getToolZone();
}

bool isAmenityTool() {
  return tools[currentTool] == getToolAmenity();
}

bool isBulldozerTool() {
  return tools[currentTool] == getToolBulldozer();
}

bool isBlueprintTool() {
  return tools[currentTool] == getToolBlueprint();
}

int getCurrentTool() {
  return currentTool;
}

Part* toolbarButton(vec2 loc, vec3 ico, std::string text, InputCallback callback, InputAction action, item itemData, bool highlight, bool blink) {

  Part* butt;
  float toolBarSizeInnerX = 1;
  char* str = sprintf_o(" %s", currentTool ? "" : text.c_str());

  if (!currentTool) {
    toolBarSizeInnerX = stringWidth(str)*0.7 + 1.1;
  }

  butt = superButton(loc, vec2(toolBarSizeInnerX, 1), ico, str, callback, itemData, action, highlight);
  if (currentTool == 0 && getHeatMap() == TransitHeatMap) butt->contents[1]->foregroundColor = PickerPalette::Black;

  if (blink) {
    butt->flags |= _partBlink;
  }

  return butt;
}

Part* toolbar() {
  int gameMode = getGameMode();
  bool designer = gameMode == ModeBuildingDesigner;
  float toolPanelSizeX = 10+toolPad*2;
  float toolPanelSizeY = 7+toolPad*2;
  float toolBarSizeX = (currentTool ? 1 : 1) + toolPad*2;
  float toolBarSizeY = (numTools + (designer ? 3 : 2))*(1+toolPad) + toolPad*1;
  float toolBarX = 0.0;
  float toolPanelX = toolBarSizeX + toolPad;
  float toolPanelY = toolBarSizeY-toolPanelSizeY-toolPad;
  float toolBarY = uiGridSizeX - toolBarSizeY - toolPad*3;
  if (!designer) toolBarY -= 1 + toolPad;

  Part* toolbar = panel(vec2(toolBarX, toolBarY),
      vec2(toolBarSizeX, toolBarSizeY));
  toolbar->padding = toolPad;

  for (int i = 1; i <= numTools; i++) {
    if (gameMode == ModeGame && !isFeatureEnabled(FQueryTool+i-1)) continue;
    if (tools[i] == 0) continue;
    Tool* tool = tools[i];
    if (!tool->isVisible()) continue;

    bool blink = blinkFeature(FQueryTool+i-1);

    if (getGameMode() == ModeGame) {
      if (i == 1) {
        for (int j = 0; j < numHeatMaps; j++) {
          if (blinkFeature(featureForHeatmap(j))) blink = true;
        }

      } else if (i == 2) {
        for (int j = FRoadAve; j <= FRoadPlanner; j++) {
          if (blinkFeature(j)) blink = true;
        }

      } else if (i == 3) {
        for (int j = FZoneResidential; j <= FZoneMixedUse; j++) {
          if (blinkFeature(j)) blink = true;
        }
      }
    }

    InputAction action = (InputAction)(((int)InputAction::ActTool1)+(i-1));
    float buttY = (i-1)*(1+toolPad);
    Part* butt = toolbarButton(vec2(0, buttY), tool->icon, tools[i]->name(), setTool, action, i, i == currentTool, blink);

    /*
    if (currentTool) {
      butt = button(vec2(0,(i-1)*(1+toolPad)), tool->icon, setTool);
      butt->inputAction = action;
      butt->itemData = i;
      if (i == currentTool) {
        butt->flags |= _partHighlight;
      }
    } else {
      butt = superButton(vec2(0,(i-1)*(1+toolPad)), vec2(toolBarSizeInnerX, 1), tool->icon, tools[i]->name, setTool, i, action, i == currentTool);
    }
    */
    setPartTooltipValues(butt,
      designer ? TooltipType::DesignerSelect+(i-1) 
      : TooltipType::TbQuery+(i-1));

    r(toolbar, butt);
  }

  bool forceInstructions = currentTool &&
      (tools[currentTool]->flags & _toolForceInstructions);
  bool showInstructionPanel = showInstructions || forceInstructions;
  bool allowClose = currentTool == 0 ||
    (tools[currentTool] != &toolBuilding && !isRouteTool());
  if (currentTool && (tools[currentTool] == getToolRoad() ||
    tools[currentTool] == getToolBlueprint()) && isPlansEnabled()) {
    allowClose = false;
  }

  if (currentTool) {
    Line toolMenuSize =
      line(vec3(toolPanelX, toolPanelY, 0),
          vec3(toolPanelSizeX, toolPanelSizeY, 0));
    Part* toolMenu = tools[currentTool]->render(toolMenuSize);
    toolMenu->padding = toolPad;
    Part* closeButt = button(vec2(9,0), iconX, setTool);
    closeButt->itemData = 0;
    r(toolMenu, closeButt);
    r(toolbar, toolMenu);

    if (allowClose) {
      Part* iButt = button(vec2(8,0), iconI, toggleInstructions);
      r(toolMenu, iButt);
      setPartTooltipValues(iButt,
        TooltipType::GenInfo);
      if (showInstructionPanel) {
        iButt->flags |= _partHighlight;
      }
    }
  }

  bool hmInfoPanel = isHeatMapIntense() && isShowHeatmapInfoPanel() &&
    getHeatMap() < 250;

  if (showInstructionPanel) {
    float instructionPosition = (currentTool ? toolPanelSizeX + toolPanelX : 7) + toolPad*2;
    Part* instructions = panel(
      vec2(instructionPosition, toolPanelY),
      vec2(14,toolPanelSizeY));
    instructions->padding = 0.25;
    r(toolbar, instructions);

    if (allowClose) {
      r(instructions, button(vec2(12.5,0), iconX, toggleInstructions));
    }

    if (currentTool && showInstructionPanel) {
      tools[currentTool]->instructionPanel(instructions);

    } else if (hmInfoPanel) {
      tools[1]->instructionPanel(instructions);

    } else if (designer) {
      r(instructions, label(vec2(0,1), .85, strdup_s(
        "Use Structure Tool (2),\n"
        "Decoration Tool (3).\n"
        "Set zone on left panel.\n"
        "Designs will spawn automatically\n"
        "in game if conditions are met.\n"
        "Press escape to access Main Menu.")));

    } else {
      r(instructions, label(vec2(0,1), 0.85f, strdup_s(
        "Use WASD keys to move camera.\n"
        "Use scroll wheel to zoom in and out.\n"
        "Press and hold wheel to rotate\n"
        "camera, or use < > keys to rotate.\n"
        "Press 2 to open Transportation Tool.\n"
        "Press escape to access Main Menu.")));
    }
  }

  if (!designer) {
    Part* tutorialButt = r(toolbar, toolbarButton(vec2(0, toolBarSizeY-2-toolPad*3), iconQuestion, "Tutorial", openTutorialPanel, ActNone, 0, isTutorialPanelOpen(), false));
    Part* citipediaButt = r(toolbar, toolbarButton(vec2(0, toolBarSizeY-1-toolPad*2), iconCitipedia, "Citipedia", openCitipediaPanel, ActNone, 0, getLeftPanel() == CitipediaPanel, false));
  }

  if (designer) {
    float buttWidth = toolPad+1;

    if (getSelectionType() != NoSelection) {
      Part* delButt = r(toolbar, toolbarButton(vec2(0,(numTools-2)*buttWidth),
            iconTrash, "Delete", deleteSelected, ActDesignDelete, 0, false, false));
      setPartTooltipValues(delButt, TooltipType::DesignerDelete);
    }

    Part* undoButt = r(toolbar, toolbarButton(vec2(0,(numTools+0)*buttWidth), iconUndo, "Undo", shiftDesignerUndoHistory, ActDesignUndo, -1, false, false));
    if (!hasDesignerUndoHistory()) {
      undoButt->foregroundColor = PickerPalette::GrayDark;
    }
    setPartTooltipValues(undoButt, TooltipType::DesignerUndo);

    Part* redoButt = r(toolbar, toolbarButton(vec2(0,(numTools+1)*buttWidth), iconRedo, "Redo", shiftDesignerUndoHistory, ActDesignRedo, 1, false, false));
    if (!hasDesignerRedoHistory()) {
      redoButt->foregroundColor = PickerPalette::GrayDark;
    }
    setPartTooltipValues(redoButt, TooltipType::DesignerRedo);

    Part* underButt = r(toolbar, toolbarButton(vec2(0,(numTools+2)*buttWidth), iconUnderground, "Underground", toggleUndergroundView, ActNone, 0, undergroundViewSelected, false));
    setPartTooltipValues(underButt,TooltipType::InfUnder);

    return toolbar;
  }

  if (!isFeatureEnabled(FHeatmaps) ||
      getGameMode() == ModeDesignOrganizer) return toolbar;

  Part* infoviewPanel = panel(vec2(-toolPad, toolBarSizeY+toolPad),
      vec2(numHeatMaps+6+toolPad*2, 1+toolPad*2));
  infoviewPanel->padding = toolPad;
  r(toolbar, infoviewPanel);
  for (int i = 0; i <= numHeatMaps; i++) {
    int hm = heatmapOrder[i];
    int hmn = hm < 0 ? numHeatMaps : hm;
    if (!isFeatureEnabled(featureForHeatmap(hm))) continue;
    vec3 hmIcon = hm == TrafficHeatMap ? iconCar : iconHeatmap[hm];

    Part* hmButt = r(infoviewPanel, button(vec2(i,0), hmIcon, setHeatMap));
    hmButt->itemData = hm;
    hmButt->inputAction = (InputAction)(((int)InputAction::ActHeatmapPollution)+i);
    setPartTooltipValues(hmButt, TooltipType::QuePollu+i);

    if (getHeatMap() == hm && isHeatMapIntense()) {
      hmButt->flags |= _partHighlight;
    }
    if (blinkFeature(featureForHeatmap(hm))) {
      hmButt->flags |= _partBlink;
    }
  }

  if (isFeatureEnabled(featureForHeatmap(RoadHeatMap))) {
    Part* roadButt = r(infoviewPanel,
        button(vec2(numHeatMaps+1,0), iconRoad, setHeatMap));
    roadButt->itemData = RoadHeatMap;
    roadButt->inputAction = InputAction::ActHeatmapRoad;
    setPartTooltipValues(roadButt,TooltipType::InfRoad);
    if (getHeatMap() == RoadHeatMap) roadButt->flags |= _partHighlight;
  }

  if (isFeatureEnabled(featureForHeatmap(TransitHeatMap))) {
    Part* transitButt = r(infoviewPanel,
        button(vec2(numHeatMaps+2,0), iconBus, setHeatMap));
    transitButt->itemData = TransitHeatMap;
    transitButt->inputAction = InputAction::ActHeatmapTransit;
    setPartTooltipValues(transitButt,TooltipType::InfTransit);
    if (getHeatMap() == TransitHeatMap) transitButt->flags |= _partHighlight;
  }

  if (isFeatureEnabled(featureForHeatmap(ZoneHeatMap))) {
    Part* zoneButt = r(infoviewPanel,
        button(vec2(numHeatMaps+3,0), iconHouse, setHeatMap));
    zoneButt->itemData = ZoneHeatMap;
    zoneButt->inputAction = InputAction::ActHeatmapZone;
    setPartTooltipValues(zoneButt,TooltipType::InfZone);
    if (getHeatMap() == ZoneHeatMap) zoneButt->flags |= _partHighlight;
  }

  if (isFeatureEnabled(FObserveUnderground)) {
    Part* underButt = r(infoviewPanel,
        button(vec2(numHeatMaps+4,0), iconUnderground, toggleUndergroundView));
    setPartTooltipValues(underButt,TooltipType::InfUnder);
    if (undergroundViewSelected) underButt->flags |= _partHighlight;
  }

  if (isFeatureEnabled(FLabel)) {
    Part* labelsButt = r(infoviewPanel,
        button(vec2(numHeatMaps+5,0), iconMessage, setLabelsVisible));
    setPartTooltipValues(labelsButt,TooltipType::InfLabels);
    if (areLabelsVisible()) labelsButt->flags |= _partHighlight;
  }

  return toolbar;
}

void tool_mouse_button_callback(InputEvent event) {
  if (currentTool) {
    tools[currentTool]->mouse_button_callback(event);
  }
}

void tool_mouse_move_callback(InputEvent event) {
  if (currentTool) {
    tools[currentTool]->mouse_move_callback(event);
  }
}

void resetTools() {
  if (currentTool) {
    tools[currentTool]->reset();
  }
  resetRoadTool();
  setAnyBuildingPlop(getGameMode() != ModeGame);
}

void initTools() {
  int gameMode = getGameMode();
  currentTool = 0;

  if (gameMode == ModeGame) {
    numTools = 5;
    tools[0] = 0;
    tools[1] = &toolQuery;
    tools[2] = getToolRoad();
    tools[3] = &toolZone;
    tools[4] = &toolBuilding;
    tools[5] = &toolBulldozer;
    tools[6] = getToolBlueprint();
    blueprintToolNum = 6;

  } else if (gameMode == ModeTest) {
    numTools = 5;
    tools[0] = 0;
    tools[1] = &toolQuery;
    tools[2] = getToolRoad();
    tools[3] = &toolZone;
    tools[4] = &toolBuilding;
    tools[5] = &toolBulldozer;
    tools[6] = getToolBlueprint();
    blueprintToolNum = 6;

  } else if (gameMode == ModeBuildingDesigner) {
    numTools = 5;
    tools[0] = 0;
    tools[1] = &toolDesignQuery;
    tools[2] = &toolStructure;
    tools[3] = &toolDeco;
    tools[4] = &toolNull;
    tools[5] = &toolDesignBulldozer;
    blueprintToolNum = 0;
    designRender();

  } else if (gameMode == ModeDesignOrganizer) {
    numTools = 1;
    tools[0] = 0;
    tools[1] = &toolOrganizerQuery;
    tools[2] = 0;
    tools[3] = 0;
    tools[4] = 0;
    tools[5] = 0;
    blueprintToolNum = 0;

  } else {
    numTools = 0;
    tools[0] = 0;
    tools[1] = 0;
    tools[2] = 0;
    tools[3] = 0;
    tools[4] = 0;
    tools[5] = 0;
  }
}

