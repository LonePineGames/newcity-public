#include "../blueprint.hpp"
#include "../collisionTable.hpp"
#include "../draw/camera.hpp"
#include "../draw/entity.hpp"
#include "../draw/texture.hpp"
#include "../game/game.hpp"
#include "../main.hpp"
#include "../selection.hpp"
#include "../sound.hpp"
#include "../string.hpp"
#include "../zone.hpp"

#include "spdlog/spdlog.h"

#include <set>
using namespace std;
typedef set<item> itemset;
typedef itemset::iterator itemsetIter;

static itemset demBuildings;
static vector<item> demGraphElems;
static vector<item> demPillars;
static vector<item> demStops;
static float totalCost = 0;
static float emDomCost = 0;
static float assetSales = 0;
static bool isNodeSimplify = false;
static bool mouseDown = false;
static bool bullBoxActive = false;
static item bulldozerText = 0;
static vec3 bulldozerCursorLoc = vec3(0,0,0);
static vec3 bullSelectionStart = vec3(0,0,0);
static Line bulldozerLine;
static item bullBoxCursorEntity = 0;
static float treesToAdd = 0;
static float moneySpentOnTrees = 0;
static item dozerBrush = 1;
static item numDozerBrushes = 4;
static float earthworksHeight = -10;
static bool earthworksContinuous = true;
static bool smoothEarthworks = true;
static bool bulldozer_wasMod = true;

enum DozerTools {
  earthworksTool, destroyTool, placeTreeTool,
  numDozers,
};

static item currentDozer = DozerTools::destroyTool; // Default to destroy

static const char* dozerLabel[] = {
  "Earthworks", "Destroy", "Place Trees",
};

const vec3 iconDozer[] = {
  iconEarthworks, iconBulldozer, iconTree,
};

const float dozerBrushRadius[] = {
  50.0f,   // Micro
  200.0f,  // Small
  725.0f,  // Med
  3125.0f, // Large
};

const vec3 iconDozerBrush[] = {
  iconDotTiny,
  iconDotSmall,
  iconDotMedium,
  iconDotLarge,
};

void renderBulldozerText() {
  if (bulldozerText == 0) {
    bulldozerText = addEntity(WSUITextShader);
  }

  char* text = 0;
  isNodeSimplify = false;
  totalCost = 0;
  emDomCost = 0;
  assetSales = 0;

  for (int i = 0; i < demGraphElems.size(); i ++) {
    item elemNdx = demGraphElems[i];
    if (elemNdx > 0) {
      bool complete = getEdge(elemNdx)->flags & _graphComplete;
      if (complete) totalCost += c(CDestroyRoadCost)*getInflation();
    } else if (demGraphElems.size() == 1) {
      isNodeSimplify = true;
      bool complete = getNode(elemNdx)->flags & _graphComplete;
      if (complete) totalCost += c(CDestroyRoadCost)*getInflation();
      bool nodeSimplifyPossible = canSimplifyNode(elemNdx, true);

      if (nodeSimplifyPossible) {
        if (totalCost != 0) {
          char* costStr = printMoneyString(totalCost);
          text = sprintf_o("Remove Joint for %s", costStr);
          free(costStr);
        } else {
          text = strdup_s("Remove Joint");
        }
        setEntityRedHighlight(bulldozerText, false);

      } else {
        text = strdup_s("Cannot Remove");
        setEntityRedHighlight(bulldozerText, true);
      }
    }
  }

  for (itemsetIter it = demBuildings.begin(); it != demBuildings.end(); it++) {
    item buildingNdx = *it;
    Building* b = getBuilding(buildingNdx);
    if (b->flags & _buildingComplete) {
      if (b->zone == GovernmentZone) {
        float amount = c(CBuildingSalesFactor) *
          getBuildingValue(buildingNdx);
        totalCost -= amount;
        assetSales -= amount;
      } else {
        float amount = c(CEminentDomainFactor) *
          getBuildingValue(buildingNdx);
        totalCost += amount;
        emDomCost += amount;
      }
    }
  }

  totalCost += c(CDestroyPillarCost)*getInflation()*demPillars.size();
  totalCost += moneySpentOnTrees;

  if (totalCost == 0) {
    setEntityVisible(bulldozerText, false);
    return;
  }

  Entity* textEntity = getEntity(bulldozerText);
  textEntity->texture = textTexture;
  setEntityBringToFront(bulldozerText, true);
  setEntityHighlight(bulldozerText, true);
  setEntityVisible(bulldozerText, true);
  createMeshForEntity(bulldozerText);
  Mesh* textMesh = getMeshForEntity(bulldozerText);

  float dist = getCameraDistance();
  float fontSize = dist/15;
  float angle = getHorizontalCameraAngle() - pi_o/2;
  placeEntity(bulldozerText, bulldozerCursorLoc, angle, 0);

  if (text == 0) {
    if (totalCost < 0) {
      char* costStr = printMoneyString(abs(totalCost));
      text = sprintf_o("Sell for %s", costStr);
      free(costStr);
      setEntityRedHighlight(bulldozerText, false);

    } else {
      text = printMoneyString(abs(totalCost));
      bool affordable = currentDozer != destroyTool ||
        canBuy(RoadBuildExpenses, totalCost);
      setEntityRedHighlight(bulldozerText, !affordable);
    }
  }

  renderString(textMesh, text, vec3(0,-fontSize,0), fontSize);
  free(text);

  bufferMesh(textEntity->mesh);
}

void renderBulldozerBox() {
  if (currentDozer == placeTreeTool ||
      (currentDozer == destroyTool && !bullBoxActive)) {
    if (bullBoxCursorEntity != 0) setEntityVisible(bullBoxCursorEntity, false);
    return;
  }

  if (bullBoxCursorEntity == 0) {
    bullBoxCursorEntity = addEntity(PaletteShader);
  }

  Entity* entity = getEntity(bullBoxCursorEntity);
  setEntityVisible(bullBoxCursorEntity, true);
  entity->texture = paletteTexture;
  setEntityBringToFront(bullBoxCursorEntity, true);
  setEntityTransparent(bullBoxCursorEntity, true);
  setEntityHighlight(bullBoxCursorEntity, getLightLevel() < 0.5);
  createMeshForEntity(bullBoxCursorEntity);
  vec3 offset = bulldozerCursorLoc;
  placeEntity(bullBoxCursorEntity, offset, 0, 0);

  Mesh* mesh = getMeshForEntity(bullBoxCursorEntity);

  if (currentDozer == destroyTool) {
    vec3 center = .5f*(bulldozerLine.start + bulldozerLine.end);
    vec3 size = vec3(
        abs(bulldozerLine.start.x-center.x)*2.f,
        abs(bulldozerLine.start.y-center.y)*2.f,
        5);
    makeCube(mesh, center-offset, size, colorRed, true, false);

  } else if (currentDozer == earthworksTool) {
    float z = !earthworksContinuous ? earthworksHeight :
      bulldozerCursorLoc.z + (bulldozer_wasMod ? -10 : 10);
    makeCylinder(mesh, vec3(0,0,-bulldozerCursorLoc.z), z,
        dozerBrushRadius[dozerBrush], 48, colorTransparentWhite);
  }

  bufferMesh(entity->mesh);
}

void bulldozer_mouse_button_callback(InputEvent event) {
  int button = event.button;
  int action = event.action;
  mouseDown = false;

  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    bool redrawSatMap = false; // Should we redraw the sat map afterward?

    if (action == GLFW_PRESS) {
      mouseDown = true;
      treesToAdd = 0;
      moneySpentOnTrees = 0;
      bullSelectionStart = landIntersect(event.mouseLine);

    } else if (currentDozer == destroyTool &&
        canBuy(RoadBuildExpenses, totalCost)) {
      transaction(EminentDomainExpenses, -emDomCost);
      transaction(AssetSalesIncome, -assetSales);
      playSound(_soundClickComplete);
      redrawSatMap = true; // Triggered a change 

      for (itemsetIter it = demBuildings.begin();
          it != demBuildings.end(); it++) {
        removeBuilding(*it);
      }

      for (int i = 0; i < demGraphElems.size(); i++) {
        item elemNdx = demGraphElems[i];
        if (elemNdx > 0) {
          Edge* edge = getEdge(elemNdx);
          if (edge->flags & _graphComplete) {
            item configType = edge->config.type;
            BudgetLine l =
              configType == ConfigTypeExpressway ? ExpwyBuildExpenses :
              configType == ConfigTypeHeavyRail ? TransitBuildExpenses :
              RoadBuildExpenses;
            transaction(l, c(CDestroyRoadCost)*getInflation());
          }
          if (bullBoxActive) {
            trimEdge(elemNdx, alignedBox(bulldozerLine));
          } else {
            removeEdge(elemNdx, true);
          }

        } else if (isNodeSimplify) {
          simplifyNode(elemNdx, true);
        }
      }

      for (int i = 0; i < demStops.size(); i++) {
        removeStop(demStops[i]);
      }

      for (int i = 0; i < demPillars.size(); i++) {
        transaction(PillarBuildExpenses,
            -c(CDestroyPillarCost)*getInflation());
        removePillar(demPillars[i]);
      }
    }

    // We triggered a redraw of the sat map due to a change
    if (redrawSatMap) {
      renderMap_g();
    }
  }
}

void setBulldozerHighlight(bool highlight) {
  for (itemsetIter it = demBuildings.begin(); it != demBuildings.end(); it++) {
    setRedHighlight(highlight, SelectionBuilding, *it);
  }

  for (int i = 0; i < demGraphElems.size(); i++) {
    setRedHighlight(highlight, SelectionGraphElement, demGraphElems[i]);
  }

  for (int i = 0; i < demPillars.size(); i++) {
    setRedHighlight(highlight, SelectionPillar, demPillars[i]);
  }

  for (int i = 0; i < demStops.size(); i++) {
    setRedHighlight(highlight, SelectionStop, demStops[i]);
  }
}

void bulldozer_mouse_move_callback(InputEvent event) {
  setBulldozerHighlight(false);
  demBuildings.clear();
  demGraphElems.clear();
  demStops.clear();
  demPillars.clear();
  vec3 landPoint = landIntersect(event.mouseLine);
  bulldozerCursorLoc = landPoint;
  bool isMod = event.mods & GLFW_MOD_ALT;
  bulldozer_wasMod = isMod;
  bullBoxActive = false;

  if (currentDozer != placeTreeTool) {
    stopGuide();
  }

  if (currentDozer == earthworksTool) {
    if (mouseDown) {
      if (!earthworksContinuous && isMod) {
        // Alt + Earthworks should enable LMB/RMB Raise/Lower relative 
        // to current surface height functionality - supersoup

        // This was the former Alt + Earthworks behavior
        earthworksHeight = round(bulldozerCursorLoc.z);
      } else if (canBuy(BuildingBuildExpenses, 1)) {
        vec3 target = bulldozerCursorLoc;
        float bypassHeight = 0.0f;

        if (!earthworksContinuous) {
          target.z = earthworksHeight;
        } else {
          float mult = isMod ? -1 : 1;
          target.z = round(landPoint.z + mult*10.00f);
        }

        // SPDLOG_INFO("Target: ({},  {},  {}),   landPoint.z: {},   bypassHeight: {}", target.x, target.y, target.z, landPoint.z, bypassHeight);

        float change = setNaturalHeight(target, dozerBrushRadius[dozerBrush],
            smoothEarthworks);
        if (getGameMode() == ModeGame) { // Earthworks should only cost money when in a normal game
          float cost = change * c(CEarthworksCost);
          forceTransaction(BuildingBuildExpenses, -cost);
          moneySpentOnTrees += cost;
        }
      }
    }
  } else if (currentDozer == placeTreeTool) {
    float throwDist = dozerBrushRadius[dozerBrush];
    setGuide(bulldozerCursorLoc, throwDist);

    if (mouseDown) {
      float time = getFrameDuration();
      float treeCost = getInflation() * c(CTreesAddCost);
      float effectiveRate = c(CTreesAddRate) *
        throwDist / dozerBrushRadius[0];
      if (isMod) effectiveRate *= 2;
      treesToAdd += time * effectiveRate;

      while (treesToAdd > 1) {
        treesToAdd --;

        if (!isMod) {
          if (!canBuy(BuildingBuildExpenses, treeCost)) continue;
        }

        float angle = randFloat(0, pi_o*2);
        float mag = sqrt(randFloat(0, 1)) * throwDist;
        vec3 thrw = vec3(sin(angle), cos(angle), 0.f) * mag;
        vec3 loc = bulldozerCursorLoc + thrw;

        if (isMod) {
          clearTrees(loc);
        } else {
          if (addTrees(loc)) {
            transaction(BuildingBuildExpenses, -treeCost);
            moneySpentOnTrees += treeCost;
          }
        }
      }
    }

  } else if (mouseDown &&
      vecDistance(bulldozerCursorLoc,bullSelectionStart) > c(CSnapDistance)) {
    bulldozerLine = clampBPLine(line(bullSelectionStart, bulldozerCursorLoc));
    Box b = alignedBox(bulldozerLine);
    bullBoxActive = true;

    demGraphElems = getCollisions(GraphCollisions, b, 0);

    vector<item> buildings = getCollisions(BuildingCollisions, b, 0);
    std::copy(buildings.begin(),
        buildings.end(),
        std::inserter(demBuildings, demBuildings.end()));

    for (int i = 1; i <= sizePillars(); i++) {
      Pillar* p = getPillar(i);
      if (!(p->flags & _pillarExists)) continue;
      if (boxDistance(b, p->location) > 0) continue;
      demPillars.push_back(i);
    }

    for (int i = 1; i <= sizeStops(); i++) {
      Stop* stop = getStop(i);
      if (!(stop->flags & _stopExists)) continue;
      if (boxDistance(b, stop->location) > 0) continue;
      demStops.push_back(i);
    }

  } else {
    float minDist = c(CBulldozerSnapDistance);
    SelectionTypes bulldozerSelectionType = NoSelection;
    item bulldozerSelection = 0;
    Line mouseLine = event.mouseLine;

    item nearbyPillar = intersectPillar(mouseLine);
    if (nearbyPillar) {
      Pillar* pillar = getPillar(nearbyPillar);
      float dist = pointLineDistance(pillar->location, mouseLine);
      bulldozerCursorLoc = pillar->location;
      if (dist < minDist) {
        bulldozerSelection = nearbyPillar;
        bulldozerSelectionType = SelectionPillar;
        bulldozerCursorLoc = pillar->location;
        minDist = dist;
      }
    }

    item elem = nearestElement(mouseLine, true);
    if (elem > 0) {
      Line l = getLine(elem);
      float dist = lineDistance(mouseLine, l);
      if (dist < minDist) {
        bulldozerSelection = elem;
        bulldozerSelectionType = SelectionGraphElement;
        minDist = dist;
        bulldozerCursorLoc = (l.start+l.end)/2.f;
        Edge* e = getEdge(elem);
      }

    } else if (elem < 0) {
      Node* n = getNode(elem);
      float dist = pointLineDistance(n->center, mouseLine);
      if (dist < minDist) {
        bulldozerSelection = elem;
        bulldozerSelectionType = SelectionGraphElement;
        minDist = dist;
        bulldozerCursorLoc = n->center;
      }
    }

    item buildingNdx = nearestBuilding(mouseLine);
    if (buildingNdx > 0) {
      Building* b = getBuilding(buildingNdx);
      float dist = boxDistance(getBuildingBox(buildingNdx), landPoint);
      if (dist < minDist) {
        bulldozerSelection = buildingNdx;
        bulldozerSelectionType = SelectionBuilding;
        bulldozerCursorLoc = b->location;
        minDist = dist;
      }
    }

    item stopNdx = nearestStop(mouseLine);
    if (stopNdx > 0) {
      Stop* s = getStop(stopNdx);
      float dist = pointLineDistance(s->location, mouseLine);
      if (dist < minDist) {
        bulldozerSelection = stopNdx;
        bulldozerSelectionType = SelectionStop;
        bulldozerCursorLoc = s->location;
        minDist = dist;
      }
    }

    if (bulldozerSelectionType == SelectionGraphElement) {
      demGraphElems.push_back(bulldozerSelection);
    } else if (bulldozerSelectionType == SelectionPillar) {
      demPillars.push_back(bulldozerSelection);
    } else if (bulldozerSelectionType == SelectionBuilding) {
      demBuildings.insert(bulldozerSelection);
    } else if (bulldozerSelectionType == SelectionStop) {
      demStops.push_back(bulldozerSelection);
    }
  }

  for (int i = 1; i <= numBuildings(); i++) {
    Building* b = getBuilding(i);
    if (!(b->flags & _buildingExists)) continue;
    if (!(b->flags & _buildingComplete)) continue;
    if (b->graphLoc.lane <= 0) continue;
    LaneBlock* block = getLaneBlock(b->graphLoc);
    item elem = block->graphElements[1];
    for (int j = 0; j < demGraphElems.size(); j++) {
      if (demGraphElems[j] == elem) {
        demBuildings.insert(i);
        break;
      }
    }
  }

  setBulldozerHighlight(true);
  renderBulldozerText();
  renderBulldozerBox();
}

void bulldozer_select() {
  mouseDown = false;
  treesToAdd = 0;
  moneySpentOnTrees = 0;
  totalCost = 0;
  emDomCost = 0;
  assetSales = 0;
  stopGuide();
  demBuildings.clear();
  demGraphElems.clear();
  demStops.clear();
  demPillars.clear();
}

void bulldozer_reset() {
  currentDozer = DozerTools::destroyTool; // Reset the bulldozer to the Destroy tool
  mouseDown = false;
  treesToAdd = 0;
  moneySpentOnTrees = 0;
  totalCost = 0;
  emDomCost = 0;
  assetSales = 0;
  stopGuide();
  setBulldozerHighlight(false);
  demBuildings.clear();
  demGraphElems.clear();
  demStops.clear();
  demPillars.clear();

  if (bulldozerText != 0) {
    removeEntity(bulldozerText);
    bulldozerText = 0;
  }

  if (bullBoxCursorEntity != 0) {
    removeEntity(bullBoxCursorEntity);
    bullBoxCursorEntity = 0;
  }
}

bool setDozer(Part* part, InputEvent event) {
  currentDozer = part->itemData;
  treesToAdd = 0;
  moneySpentOnTrees = 0;

  /*
  // If we're switching to the Earthworks tool, and it's in continuous mode
  // we don't want a negative height value
  // -- Actually, earthworksHeight should not matter in continuous mode - LP
  if (currentDozer == earthworksTool && earthworksContinuous && earthworksHeight < 0.0f) {
    earthworksHeight = 0.0f;
  }
  */

  return true;
}

bool setTreeBrush(Part* part, InputEvent event) {
  int brush = part->itemData;
  if(brush < 0 || brush >= numDozerBrushes) return false;

  dozerBrush = brush;
  return true;
}

bool earthworksHeightCallback(Part* part, InputEvent event) {
  /*
  // We don't want a negative height value in continuous mode
  // -- Actually, earthworksHeight should not matter in continuous mode - LP
  if (currentDozer == earthworksTool && earthworksContinuous && ((earthworksHeight + part->itemData * 4) < 0.0f)) {
    earthworksHeight = 0.0f;
    return true;
  }
  */

  earthworksHeight += part->itemData * 4;
  return true;
}

bool toggleSmooth(Part* part, InputEvent event) {
  smoothEarthworks = !smoothEarthworks;
  return true;
}

bool toggleContinuous(Part* part, InputEvent event) {
  if (event.isMouse) {
    earthworksContinuous = part->itemData;
  } else {
    earthworksContinuous = !earthworksContinuous;
  }

  /*
  // We don't want a negative height value in continuous mode
  // -- Actually, earthworksHeight should not matter in continuous mode - LP
  if (currentDozer == earthworksTool && earthworksContinuous && ((earthworksHeight + part->itemData * 4) < 0.0f)) {
    earthworksHeight = 0.0f;
  }
  */

  return true;
}

Part* bulldozer_render(Line dim) {
  Part* result = panel(dim);
  r(result, label(vec2(0,0), 1, strdup_s(dozerLabel[currentDozer])));
  //r(result, hr(vec2(0,1), dim.end.x-toolPad*2));
  r(result, icon(vec2(4.5f,0.0f), iconDozer[currentDozer]));

  if (currentDozer != destroyTool) {
    for(int b = 0; b < numDozerBrushes; b++) {
      bool tree = currentDozer == placeTreeTool;
      float brushOffset = (tree ? 2.f : 3.5f) + b*1.5f;
      Part* butt = button(vec2(brushOffset, tree ? 3. : 1.5),
          iconDozerBrush[b], setTreeBrush);
      butt->itemData = b;
      butt->inputAction = (InputAction)(ActBTBrushXSmall+b);
      if(dozerBrush == b) butt->flags |= _partHighlight;
      //setPartTooltipValues(butt, TooltipType::ZoneBrushEdge+b);

      r(result, butt);
    }
  }

  if (currentDozer == earthworksTool) {
    if (!earthworksContinuous) {
      float z = earthworksHeight;
      Part* buttUp = button(vec2(1,2), iconUp, earthworksHeightCallback);
      buttUp->inputAction = ActBTRaiseElevation;
      buttUp->itemData = 1;
      setPartTooltipValues(buttUp, TooltipType::RoadRaiseEle);
      //if (z > 0) buttUp->flags |= _partHighlight;
      r(result, buttUp);

      r(result, labelCenter(vec2(.5f,2.85f), vec2(2.f,.85f),
             sprintf_o("%s%dm", z == 0 ? " " : z < 0 ? "" : "+", int(z))));
      r(result, labelCenter(vec2(.5f,3.5f), vec2(2.f,.6f), strdup_s(z < 0 ? "Below Sea Lvl" : "Above Sea Lvl")));

      Part* buttDown = button(vec2(1,4), iconDown, earthworksHeightCallback);
      buttDown->inputAction = ActBTLowerElevation;
      buttDown->itemData = -1;
      setPartTooltipValues(buttDown, TooltipType::RoadLowerEle);
      //if (z < 0) buttDown->flags |= _partHighlight;
      r(result, buttDown);
    }

    Part* buttSmoooth = buttonCenter(vec2(3.5,2.75), vec2(6,0.85),
        strdup_s("Smooth"), toggleSmooth);
    buttSmoooth->inputAction = ActBTSmooth;
    //setPartTooltipValues(buttDown, TooltipType::RoadLowerEle);
    if (smoothEarthworks) buttSmoooth->flags |= _partHighlight;
    r(result, buttSmoooth);

    /*
    Revisit this; add a callback? -supersoup
    KeyBind bind = getKeyBind(buttSmoooth->inputAction);
    if (bind.key != GLFW_KEY_UNKNOWN) {
      Part* buttSmooothKey = button(vec2(6.5, 3.5), iconNull, 0);
      buttSmooothKey->text = strdup_s(getKeyStr(bind.key).c_str());
      r(result, buttSmooothKey);
    }
    */

Part* superButton(vec2 start, vec2 size, vec3 ico, char* text,
  InputCallback callback, item itemData, InputAction action, bool highlight);

    r(result, superButton(vec2(3.5, 3.75), vec2(6, 0.85), iconEarthworks, strdup_s("Raise/Lower"), toggleContinuous, 1, ActBTContinuous, earthworksContinuous));
    r(result, superButton(vec2(3.5, 4.75), vec2(6, 0.85), iconEarthworksLevel, strdup_s("Level"), toggleContinuous, 0, ActBTContinuous, !earthworksContinuous));
  }

  Part* tabPanel = panel(vec2(0,6), vec2(10,1));
  tabPanel->flags |= _partLowered;
  r(result, tabPanel);

  item numDozersEnabled = 1 + isFeatureEnabled(FPlaceTrees) +
    isFeatureEnabled(FEarthworks);
  float tabOffset = (10-numDozersEnabled*2+1)*.5f;

  for (int i=0; i < numDozers; i++) {
    if (i == placeTreeTool && !isFeatureEnabled(FPlaceTrees)) continue;
    if (i == earthworksTool && !isFeatureEnabled(FEarthworks)) continue;

    Part* butt = button(vec2(i*2+tabOffset,0), iconDozer[i], setDozer, i);
    if (i == currentDozer) {
      butt->flags |= _partHighlight;
    }
    //setPartTooltipValues(butt, TooltipType::AmeEdu+i);
    butt->inputAction = (InputAction)(ActBTEarthworks+i);
    r(tabPanel, butt);
  }

  return result;
}

void bulldozerInstructionPanel(Part* panel) {
  if (currentDozer == destroyTool) {
    r(panel, label(vec2(1,1.5), .85, strdup_s(
      "Use left click to destroy\n"
      "roads or buildings. Click\n"
      "and drag to destroy multiple\n"
      "things at once. Be careful where\n"
      "you click!")));
  } else if (currentDozer == placeTreeTool) {
    r(panel, label(vec2(1,2), .85, strdup_s(
      "Use left click to place trees.\n"
      "Hold down Alt and left click\n"
      "to remove trees.")));
  } else if (currentDozer == earthworksTool) {
    if (earthworksContinuous) {
      r(panel, label(vec2(1,2), .85, strdup_s(
        "Use left click to raise terrain.\n"
        "Hold down Alt and left click to\n"
        "lower terrain.")));
    } else {
      r(panel, label(vec2(1,1.5), .85, strdup_s(
        "Use left click to level\n"
        "terrain to the set height.\n"
        "Hold down Alt and left\n"
        "click on target land to\n"
        "set the level height.")));
    }
  }
}

std::string bulldozer_name() {
  return "Bulldozer";
}

bool bulldozer_visible() {
  return true;
  //return getGameMode() == ModeTest || numCompleteEdges() >= 5;
}

Tool toolBulldozer = {
  bulldozer_mouse_button_callback,
  bulldozer_mouse_move_callback,
  bulldozer_select,
  bulldozer_reset,
  bulldozer_visible,
  iconBulldozer,
  bulldozer_render,
  bulldozerInstructionPanel,
  bulldozer_name,
};

Tool* getToolBulldozer() {
  return &toolBulldozer;
}

