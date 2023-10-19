#include "blueprint.hpp"

#include "../blueprint.hpp"
#include "../color.hpp"
#include "../draw/entity.hpp"
#include "../draw/texture.hpp"
#include "../game/feature.hpp"
#include "../game/game.hpp"
#include "../graph.hpp"
#include "../icons.hpp"
#include "../input.hpp"
#include "../land.hpp"
#include "../lot.hpp"
#include "../plan.hpp"
#include "../renderGraph.hpp"
#include "../renderLot.hpp"
#include "../renderUtils.hpp"
#include "../sound.hpp"
#include "../string.hpp"
#include "../string_proxy.hpp"
#include "../time.hpp"
#include "../tutorial.hpp"
#include "../util.hpp"

#include "../parts/blueprintPanel.hpp"
#include "../parts/button.hpp"
#include "../parts/hr.hpp"
#include "../parts/icon.hpp"
#include "../parts/label.hpp"
#include "../parts/panel.hpp"
#include "../parts/root.hpp"
#include "../parts/textBox.hpp"
#include "../parts/tooltip.hpp"

#include "elevation.hpp"
#include "plansPanel.hpp"

#include "spdlog/spdlog.h"

static bool wasModified = false;
static bool isSelectingBP = false;
static bool isBPSelectionMode = true;
static bool raiseCursor = false;
static item bpCursorEntity = 0;
static item bpTextEntity = 0;
static int zOffset = 0;
static vec3 blueprintCursorLoc;
static vec3 selectionStart;
static vector<item> graphElemsSelected;
static vector<item> lotsSelected;
static vector<item> virtualLots;
static Line bpLine;
static TextBoxState draftTB;
static bool skipFrame = false;
static const char* helpTxt = "";
static float lastBPUpdateTime = 0;

void blueprint_mouse_button_callback(InputEvent event);
void blueprint_mouse_move_callback(InputEvent event);
void blueprint_select();
void blueprint_reset();
bool blueprint_visible();
Part* blueprint_render(Line dim);
void blueprintInstructionPanel(Part* panel);

std::string blueprint_name() {
  return "Blueprints";
}

Tool toolBlueprint = {
  blueprint_mouse_button_callback,
  blueprint_mouse_move_callback,
  blueprint_select,
  blueprint_reset,
  blueprint_visible,
  iconBlueprint,
  blueprint_render,
  blueprintInstructionPanel,
  blueprint_name,
};

const char* bpInstructionMessage =
  "Left click to start a blueprint.\n"
  "Left click again to complete it,\n"
  "or right click to cancel.";

const char* bpApplyInstructionMessage =
  "Left click to apply blueprint.";

void setBPHighlights(bool val) {
  for (int i = 0; i < graphElemsSelected.size(); i++) {
    setElementHighlight(graphElemsSelected[i], val);
  }
  for (int i = 0; i < lotsSelected.size(); i++) {
    item lotNdx = lotsSelected[i];
    if (val) {
      Lot* lot = getLot(lotNdx);
      renderLotAs(lotNdx, lot->zone, getLotMaxDensity(lotNdx), true);
    } else {
      renderLot(lotNdx);
    }
  }
}

void resetVirtualLots() {
  for (int i = 0; i < virtualLots.size(); i++) {
    removeEntity(virtualLots[i]);
  }
  virtualLots.clear();
}

void renderBlueprintCursor() {
  resetVirtualLots();

  Entity* entity = getEntity(bpCursorEntity);
  entity->texture = paletteTexture;
  setEntityBringToFront(bpCursorEntity, true);
  setEntityTransparent(bpCursorEntity, true);
  setEntityHighlight(bpCursorEntity, getLightLevel() < 0.5);
  createMeshForEntity(bpCursorEntity);
  vec3 offset = blueprintCursorLoc;
  placeEntity(bpCursorEntity, offset, 0, 0);

  Entity* textEntity = getEntity(bpTextEntity);
  textEntity->texture = textTexture;
  setEntityBringToFront(bpTextEntity, true);
  createMeshForEntity(bpTextEntity);

  Mesh* mesh = getMeshForEntity(bpCursorEntity);
  Mesh* textMesh = getMeshForEntity(bpTextEntity);
  float angle = getHorizontalCameraAngle() - pi_o/2;
  float pitch = -getVerticalCameraAngle();
  float dist = getCameraDistance();
  float fontSize = dist/15;
  float tx = fontSize;
  float ty = 0;
  float tz = 0; //fontSize*2;
  vec3 tup = vec3(0,0,fontSize*2);

  const float cursorZ = 10;
  const float cursorX = 20;
  const float cursorBaseX = 12;
  const float cursorBaseInner = 8;

  vec3 tex = isSelectingBP || !isBPSelectionMode ? colorWhite : colorBlue;
  float height = zOffset*c(CZTileSize);
  float baseZ = -height;

  if (height < 0) {
    baseZ += height;
  }

  vec3 bpCursorSize = vec3(10, 10, 5);
  vec3 baseLoc = blueprintCursorLoc + vec3(0,0,baseZ) - offset;
  makeCube(mesh, baseLoc, bpCursorSize, tex, true, false);

  if (isSelectingBP) {
    vec3 center = .5f*(bpLine.start + bpLine.end);
    vec3 size = vec3(
        abs(bpLine.start.x-center.x)*2.f,
        abs(bpLine.start.y-center.y)*2.f,
        5);
    makeCube(mesh, center-offset, size, colorBlue, true, false);
    makeCube(mesh, bpLine.start-offset, bpCursorSize, tex, true, false);
    setEntityVisible(bpTextEntity, false);

  } else if (!isBPSelectionMode) {
    Mesh* textMesh = getMeshForEntity(bpTextEntity);
    float angle = getHorizontalCameraAngle() - pi_o/2;
    float pitch = -getVerticalCameraAngle();
    float dist = getCameraDistance();
    float fontSize = dist/15;
    float tx = fontSize;
    float ty = 0;
    float tz = 0; //fontSize*2;
    vec3 tup = vec3(0,0,fontSize*2);

    money cost = getTotalUnsetPlansCost();
    if (cost > 0) {
      char* resString = printMoneyString(cost);
      renderString(textMesh, resString, vec3(tx,ty,tz), fontSize);
      free(resString);
    }

    bool affordable = canBuy(RoadBuildExpenses, cost);
    placeEntity(bpTextEntity, blueprintCursorLoc+tup, angle, pitch);
    setEntityRedHighlight(bpTextEntity, !affordable);
    setEntityHighlight(bpTextEntity, true);
    setEntityVisible(bpTextEntity, true);
  }

  bufferMesh(textEntity->mesh);
  bufferMesh(entity->mesh);

  Blueprint* bp = getDraftBlueprint();
  if (!isBPSelectionMode && getGameMode() == ModeGame &&
      (bp->flags & _blueprintExists)) {
    for (int i = 0; i < bp->edges.size(); i++) {

      BlueprintEdge e = bp->edges[i];
      if (e.zone[0] == 0 && e.zone[1] == 0) continue;
      BlueprintNode n0 = bp->nodes[e.ends[0]];
      BlueprintNode n1 = bp->nodes[e.ends[1]];
      vec3 n0c = pointOnLand(n0.loc + blueprintCursorLoc);
      vec3 n1c = pointOnLand(n1.loc + blueprintCursorLoc);
      vec3 along = n1c - n0c;
      float l = length(along);
      vec3 tAlong = along*(tileSize/l);
      vec3 norm = zNormal(tAlong);
      vec3 cursor = n0c + tAlong*.5f;

      for (; l > tileSize; l -= tileSize) {
        for (int s = 0; s < 2; s++) {
          item zone = e.zone[s];
          if (zone == 0) continue;
          if (!isFeatureEnabled(FZoneResidential+zone-1)) continue;
          float mult = s == 1 ? 1 : -1;
          virtualLots.push_back(
            renderVirtualLot(cursor + mult*norm, mult*norm, zone, 10, false));
        }
        cursor += tAlong;
      }
    }
  }
}

void blueprint_mouse_button_callback(InputEvent event) {
  if (event.action != GLFW_PRESS || event.button == GLFW_MOUSE_BUTTON_MIDDLE) {
    return;
  }
  bool isRMB = event.button == GLFW_MOUSE_BUTTON_RIGHT;

  if (skipFrame) {
    skipFrame = false;
    return;
  }

  if (isBPSelectionMode) {
    if (isSelectingBP) {
      if (!isRMB) {
        // Take the blueprint
        setDraftBlueprintToBuffer();
        isBPSelectionMode = false;
        reportTutorialUpdate(TutorialUpdateCode::FinishBlueprintCapture);
      }
      isSelectingBP = false;
      clearBufferBlueprint();

    } else if (!isRMB && !isSelectingBP) {
      isSelectingBP = true;
      selectionStart = blueprintCursorLoc;
      clearBufferBlueprint();
      reportTutorialUpdate(TutorialUpdateCode::StartBlueprintCapture);
    }

  } else if (isRMB) {
    // no op
    //isBPSelectionMode = true;
    //clearBufferBlueprint();

  } else {
    // Set/Buy the blueprint
    playSound(_soundClickComplete);
    reportTutorialUpdate(TutorialUpdateCode::PlaceBlueprint);
    skipFrame = true;
    if (isPlansEnabled()) {
      SPDLOG_INFO("setAllPlans()");
      setAllPlans();
    } else {
      splitAllUnsetPlans();
      if (buyAllPlans(false, true)) {
        queueBlueprintZones(getDraftBlueprint(), blueprintCursorLoc);
      }
    }
  }
}

void blueprint_mouse_move_callback(InputEvent event) {

  if (skipFrame) {
    skipFrame = false;
    return;
  }

  float time = glfwGetTime();
  if (time - lastBPUpdateTime < 0.1) return;
  lastBPUpdateTime = time;

  vec3 loc = landIntersect(event.mouseLine);
  if (isBPSelectionMode || getDraftBlueprint()->flags & _blueprintFine) {
    loc = unitizeFine(loc);
  } else {
    loc = unitize(loc);
  }
  loc = pointOnLandNatural(loc);
  if (loc.z < beachLine) loc.z = beachLine;
  loc += vec3(0, 0, c(CZTileSize) * zOffset + c(CRoadRise)*2.f);

  if (vecDistance(loc, blueprintCursorLoc) < 0.1 && !wasModified) return;
  wasModified = false;

  setBPHighlights(false);
  clearBufferBlueprint();
  graphElemsSelected.clear();
  lotsSelected.clear();
  discardAllUnsetPlans();

  blueprintCursorLoc = loc;

  if (isSelectingBP) {
    // Select BP
    bpLine = clampBPLine(line(selectionStart, loc));
    graphElemsSelected = getBPElems(bpLine);
    Box b = alignedBox(bpLine);
    lotsSelected = setBufferBlueprint(graphElemsSelected, collidingLots(b));
    setBPHighlights(true);

  } else if (!isBPSelectionMode) {
    // Apply BP
    loc.z = c(CZTileSize)*zOffset + c(CRoadRise)*2;
    applyDraftBlueprint(loc);
  }

  renderBlueprintCursor();
}

void blueprint_select() {
  wasModified = true;
  setAutomaticBridges(false);
  lastBPUpdateTime = 0;
  resetVirtualLots();
  clearBufferBlueprint();
  if (bpCursorEntity == 0) {
    bpCursorEntity = addEntity(PaletteShader);
    bpTextEntity = addEntity(WSUITextShader);
  }

  setPlansActive(true);
  if (isPlansEnabled()) {
    toolBlueprint.flags |= _toolForceInstructions;
  } else {
    toolBlueprint.flags &= ~_toolForceInstructions;
  }
}

void blueprint_reset() {
  wasModified = true;
  lastBPUpdateTime = 0;
  setPlansActive(false);
  setAutomaticBridges(true);
  setBPHighlights(false);
  resetVirtualLots();
  // Reset sidebar
  closeBlueprintSidebar();
  graphElemsSelected.clear();
  lotsSelected.clear();
  discardAllUnsetPlans();
  clearBufferBlueprint();
  isSelectingBP = false;
  if (bpCursorEntity != 0) {
    removeEntityAndMesh(bpCursorEntity);
    removeEntityAndMesh(bpTextEntity);
    bpCursorEntity = 0;
    bpTextEntity = 0;
  }
}

bool toggleBPSelectionMode(Part* part, InputEvent event) {
  isBPSelectionMode = !isBPSelectionMode;
  wasModified = true;
  clearBufferBlueprint();
  return true;
}

void endBPSelectionMode() {
  isBPSelectionMode = false;
  wasModified = true;
}

bool blueprintElevationCallback(Part* part, InputEvent event) {
  item ndx = part->itemData;
  if (ndx > 0) {
    zOffset ++;
    if(zOffset > maxZOffset) {
      zOffset = maxZOffset;
    }
    wasModified = true;
    return true;

  } else if (ndx < 0) {
    zOffset --;
    if(zOffset < minZOffset) {
      zOffset = minZOffset;
    }
    wasModified = true;
    return true;
  }
  return false;
}

bool rotateBlueprint(Part* part, InputEvent event) {
  rotateDraftBlueprint();
  wasModified = true;
  return true;
}

bool flipBlueprint(Part* part, InputEvent event) {
  flipDraftBlueprint();
  wasModified = true;
  return true;
}

bool saveBlueprint(Part* part, InputEvent event) {
  saveDraftBlueprint();
  writeBlueprints();
  reportTutorialUpdate(TutorialUpdateCode::SaveBlueprint);
  return true;
}

void importBlueprint(const char* str) {
  int prevNumBp = numBlueprints();
  SPDLOG_INFO("importBlueprint {}", str);
  readBlueprints(str);
  writeBlueprints();
  if (numBlueprints() > prevNumBp) {
    selectBlueprint(numBlueprints()-1);
    wasModified = true;
  }
}

bool importBlueprint(Part* part, InputEvent event) {
  getClipboard(importBlueprint);
  return true;
}

bool exportBlueprint(Part* part, InputEvent event) {
  sendToClipboard(blueprintToString(getDraftBlueprint()));
  return true;
}

bool exportBlueprintLibrary(Part* part, InputEvent event) {
  sendToClipboard(blueprintLibraryToString());
  return true;
}

void setBlueprintInfo() {
  if (isPlansEnabled()) {
    toolBlueprint.flags |= _toolForceInstructions;
  } else {
    toolBlueprint.flags &= ~_toolForceInstructions;
  }
}

bool togglePlanPanelBP(Part* part, InputEvent event) {
  setPlansEnabled(!isPlansEnabled());
  return true;
}

/*
bool hoverHelpText(Part* part, InputEvent event) {
  if(part == 0) {
    return false;
  }

  int val = part->itemData;
  helpTxt = getTooltipText(TooltipType::BlueprintTool, val);

  return true;
}
*/

Part* blueprint_render(Line dim) {
  Part* result = panel(dim);
  r(result, label(vec2(0,0), 1, strdup_s("Blueprint")));
  //r(result, hr(vec2(0,1), dim.end.x-.2));
  r(result, label(vec2(0,5), 0.85f, strdup_s(helpTxt)));

  Blueprint* activeBP = isBPSelectionMode ? getBufferBlueprint() :
    getDraftBlueprint();
  if (!isBPSelectionMode && (activeBP->flags & _blueprintExists)) {
    Part* buttRot = button(vec2(0.f,3.75f), iconBlueprintRotate,
        rotateBlueprint);
    buttRot->inputAction = ActBPRotate;
    setPartTooltipValues(buttRot,
      TooltipType::BluRotate);
    r(result, buttRot);

    Part* buttFlip = button(vec2(1.f,3.75f), iconBlueprintFlip,
        flipBlueprint);
    buttFlip->inputAction = ActBPFlip;
    setPartTooltipValues(buttFlip,
      TooltipType::BluFlip);
    r(result, buttFlip);

    Part* buttUp = button(vec2(2.f,3.75f), iconUp,
        blueprintElevationCallback);
    buttUp->inputAction = ActBPRaiseElevation;
    setPartTooltipValues(buttUp,
      TooltipType::BluRaiseEle);
    buttUp->itemData = 1;
    if (zOffset > 0) {
      buttUp->flags |= _partHighlight;
    }
    r(result, buttUp);

    Part* buttDown = button(vec2(3.f,3.75f), iconDown,
        blueprintElevationCallback);
    buttDown->inputAction = ActBPLowerElevation;
    setPartTooltipValues(buttDown,
      TooltipType::BluLowerEle);
    buttDown->itemData = -1;
    if (zOffset < 0) {
      buttDown->flags |= _partHighlight;
    }
    r(result, buttDown);
  }

  if (activeBP->flags & _blueprintExists) {
    Part* bpPanel = blueprintPanel(vec2(0.f,1.25f), vec2(8.f,2.25f),
        activeBP, &draftTB, -1);
    r(result, bpPanel);
  } else {
    r(result, icon(vec2(3.f,2.5f), vec2(1,1), iconBlueprintNew));
  }

  Part* tabPanel = panel(vec2(0,6), vec2(10,1));
  tabPanel->flags |= _partLowered;
  r(result, tabPanel);

  Part* buttNew = button(vec2(0.f, 0.f), iconBlueprintNew,
      toggleBPSelectionMode);
  buttNew->inputAction = ActBPToggleCapture;
  setPartTooltipValues(buttNew,
    TooltipType::BluNew);
  if (isBPSelectionMode) {
    buttNew->flags |= _partHighlight;
  }
  r(tabPanel, buttNew);

  Part* buttImport = button(vec2(1.f, 0.f), iconBlueprintImport,
      importBlueprint);
  buttImport->inputAction = ActBPImport;
  setPartTooltipValues(buttImport,
    TooltipType::BluImport);
  r(tabPanel, buttImport);

  if (numBlueprints() > 0) {
    Part* buttExportLib = button(vec2(3.f, 0.f), iconBlueprintExportLibrary,
        exportBlueprintLibrary);
    r(tabPanel, buttExportLib);
  }

  if (!isBPSelectionMode && (activeBP->flags & _blueprintExists)) {
    Part* buttExport = button(vec2(2.f, 0.f), iconBlueprintExport,
        exportBlueprint);
    buttExport->inputAction = ActBPExport;
    setPartTooltipValues(buttExport,
      TooltipType::BluExport);
    r(tabPanel, buttExport);

    Part* buttSave = button(vec2(4.f, 0.f), iconSave, saveBlueprint);
    buttSave->inputAction = ActBPSave;
    setPartTooltipValues(buttSave,
      TooltipType::BluSave);
    r(tabPanel, buttSave);
  }

  Part* buttPlans = button(vec2(9.f, 0.f), iconCheck, togglePlanPanelBP);
  setPartTooltipValues(buttPlans,
    TooltipType::BluPlan);
  if (isPlansEnabled()) {
    buttPlans->flags |= _partHighlight;
  }
  r(tabPanel, buttPlans);

  return result;
}

void blueprintInstructionPanel(Part* panel) {
  if (isPlansEnabled()) {
    plansPanel(panel);
  } else {
    r(panel, label(vec2(0,2), .85, isBPSelectionMode ?
      strdup_s(bpInstructionMessage) : strdup_s(bpApplyInstructionMessage)));
  }
}

bool blueprint_visible() {
  return true;
}

Tool* getToolBlueprint() {
  return &toolBlueprint;
}

