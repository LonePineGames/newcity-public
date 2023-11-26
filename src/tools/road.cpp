#include "road.hpp"

#include "../building/building.hpp"
#include "../city.hpp"
#include "../color.hpp"
#include "../configuration.hpp"
#include "../draw/camera.hpp"
#include "../draw/entity.hpp"
#include "../draw/shader.hpp"
#include "../draw/texture.hpp"
#include "../game/feature.hpp"
#include "../graph.hpp"
#include "../icons.hpp"
#include "../land.hpp"
#include "../option.hpp"
#include "../pillar.hpp"
#include "../plan.hpp"
#include "../renderGraph.hpp"
#include "../renderUtils.hpp"
#include "../selection.hpp"
#include "../sound.hpp"
#include "../string.hpp"
#include "../string_proxy.hpp"
#include "../tutorial.hpp"
#include "../util.hpp"

#include "../parts/button.hpp"
#include "../parts/hr.hpp"
#include "../parts/icon.hpp"
#include "../parts/label.hpp"
#include "../parts/panel.hpp"
#include "../parts/toolbar.hpp"
#include "../parts/tooltip.hpp"

#include "elevation.hpp"
#include "plansPanel.hpp"

#include "spdlog/spdlog.h"

const int roadSystem = 0;
const int expresswaySystem = 1;
const int transitSystems = 2;

const int buildTab = 0;
const int repairTab = 1;
const int transitTab = 2;
const int pillarTab = 3;
const int cutTab = 4;

static item currentRoadTab = 0;
static item currentSystem = 0;
static item currentConfigNdx[4] = {0,2,1,0};
static bool isPlacingEdge = false;
static bool isPlansMade = false;
static bool raiseCursor = false;
static bool suspension = false;
static bool gridMode = true;
static bool linkMode = true;
static vec3 roadEnd[2];
static item roadPillar[2];
static item cityNode[2];

//static item graphPlan = 0;
static item pillarPlan = 0;
static item highlightElement = 0;
static item builderCursorEntity = 0;
static item builderTextEntity = 0;
static Elevation elevation = Elevation {true, 0};
static double startClickTime = 0;
static vec3 roadCursorLoc;
static double lastPillarTime = -100;

void road_mouse_button_callback(InputEvent event);
void road_mouse_move_callback(InputEvent event);
void road_select();
void road_reset();
bool road_visible();
Part* road_render(Line dim);
void roadInstructionPanel(Part* panel);
bool isBuilderMode();
void setRoadTab(item tab);
void setRoadSystem(item system);

std::string road_name() {
  return "Transportation";
}

const int numTransitKeys = 6;
const char* transitKeyNames[] = {
  "R", "T", "Y", "U", "O", "P"
};

int transitKeyCodes[] = {
  GLFW_KEY_R, GLFW_KEY_T, GLFW_KEY_Y, GLFW_KEY_U, GLFW_KEY_O, GLFW_KEY_P,
};

#include "transit.cpp"

int tabKeyCodes[] = {
  GLFW_KEY_B, GLFW_KEY_L, GLFW_KEY_V, GLFW_KEY_N, GLFW_KEY_C
};

void resetRoadTool() {
  currentRoadTab = 0;
  currentSystem = 0;
  elevation.zOffset = 0;
  elevation.moveEarth = true;
  suspension = false;
  gridMode = true;
  linkMode = true;
}

Tool toolRoad = {
  road_mouse_button_callback,
  road_mouse_move_callback,
  road_select,
  road_reset,
  road_visible,
  iconRoad,
  road_render,
  roadInstructionPanel,
  road_name,
};

const char* roadTabNames[6] = {
  "Road",
  "Expressway",
  "Repair",
  "Transit",
  "Bridge Pillar",
  "Cut"
};

const int numConfigs[4] = {5, 5, 3, 2};
const char* roadName[4][5] = {
  {
    "Street",
    "Avenue",
    "Boulevard",
    "One Way (Two Lane)",
    "One Way (Four Lane)",

  }, {
    "One Lane Expwy",
    "Two Lane Expwy",
    "Three Lane Expwy",
    "Four Lane Expwy",
    "Five Lane Expwy",

  }, {
    "Single Rail",
    "Double Rail",
    //"Quadruple Rail",
    "Station Platform",
    //"Station Platform (Bypass)",

  }, {
    "Pedestrian Path",
    "Turnstiles",
  }
};

const char* instructionMessages[6] = {
  // Build
  "Left click to start a road. "
  "Left click again to complete it, "
  "or right click to cancel.",

  // Repair
  "Cars cannot drive as "
  "fast on roads with wear. "
  "Worn out roads look dirty. "
  "Click on road to fix wear.",

  // Transit
  "Add a Transit Line, then add a chain "
  "of stops. For a bidirectional line, "
  "also add a reverse chain of stops. "
  "(There and Back)",

  // Pillar
  "Pillars allow bridges. "
  "Circle shows max bridge length. "
  "Click on top of pillar with "
  "Build Tool (B) to make the bridge.\n \n"
  "Note: You do not need pillars to make elevated roads"
  " (or viaducts) over land. Read about road elevation"
  " in the Build Tool (B)",

  // Cut
  "Left click on a road "
  "to cut it in two.",
};

const Configuration configs[4][5] = {
  { // Roads
    {1, 0, ConfigTypeRoad, StopSignStrategy, 0},
    {2, 1, ConfigTypeRoad, TrafficLightStrategy, 0},
    {3, 2, ConfigTypeRoad, TrafficLightStrategy, _configMedian},
    {2, 1, ConfigTypeRoad, StopSignStrategy, _configOneWay},
    {4, 1, ConfigTypeRoad, TrafficLightStrategy, _configOneWay},

  }, { // Expressway
    {1, 2, ConfigTypeExpressway, JointStrategy, _configOneWay},
    {2, 3, ConfigTypeExpressway, JointStrategy, _configOneWay},
    {3, 3, ConfigTypeExpressway, JointStrategy, _configOneWay},
    {4, 3, ConfigTypeExpressway, JointStrategy, _configOneWay},
    {5, 3, ConfigTypeExpressway, JointStrategy, _configOneWay},

  }, { // Rail
    {1, 4, ConfigTypeHeavyRail, TrafficLightStrategy, _configOneWay},
    {1, 4, ConfigTypeHeavyRail, TrafficLightStrategy, 0},
    //{2, 4, ConfigTypeHeavyRail, JunctionStrategy, _configDisabled},
    {1, 2, ConfigTypeHeavyRail, TrafficLightStrategy, _configPlatform},
    //{2, 2, ConfigTypeHeavyRail, JunctionStrategy,
      //_configPlatform | _configDisabled},

  }, { // Pedestrian
    {1, 0, ConfigTypePedestrian, UnregulatedStrategy, 0},
    {4, 0, ConfigTypePedestrian, UnregulatedStrategy, _configToll},
  }
};

const vec3 configIcons[4][5][2] = {
  {
    {iconRoad2, iconIntersectionStrategy[0]},
    {iconRoad2x2, iconIntersectionStrategy[1]},
    {iconRoad3x3m, iconTree},
    {iconRoad2ow, iconOneWaySignRight},
    {iconRoad4ow, iconOneWaySignRight}

  }, {
    {iconRoad1, iconExpressway},
    {iconRoad2, iconExpressway},
    {iconRoad3, iconExpressway},
    {iconRoad4, iconExpressway},
    {iconRoad5, iconExpressway}

  }, {
    {iconRail1, iconOneWaySignRight},
    {iconRail2, iconTrain},
    //{iconRoad2x2, iconNull},
    {iconRailPlatform, iconTrainSign},
    //{iconRoad2x2, iconBusSign}

  }, {
    {iconPersonMan, iconNull},
    {iconPersonMan, iconCash},
  }
};

item getConfigNdx() {
  if (currentSystem >= 2) {
    if (getCurrentTransitSystem() == 0) {
      return 0;
    } else {
      item configType = getSystemGraphType(getCurrentTransitSystem());
      if (configType == ConfigTypeHeavyRail) {
        return 2;
      } else if (configType == ConfigTypeRoad) {
        return 0;
      } else {
        return 0;
      }
    }
  } else {
    return currentSystem;
  }
}

Configuration getCurrentConfig() {
  Configuration result;
  item configNdx = getConfigNdx();
  item currentNdx = currentConfigNdx[configNdx];
  return elevate(elevation, configs[configNdx][currentNdx]);
}

bool isBuilderMode() {
  if (currentRoadTab != buildTab) return false;
  if (currentSystem != transitSystems) {
    return true;
  } else if (currentSystem == transitSystems) {
    if (isAddingStop) return false;
    item currentSys = getCurrentTransitSystem();
    if (currentSys <= 0 || currentSys > sizeTransitSystems()) {
      return false;
    }
    TransitSystem* system = getTransitSystem(currentSys);
    if (!(system->flags & _transitComplete)) return false;
    return true;
  }
  return false;
}

void setRoadTab(item tab) {
  if (currentSystem == transitSystems && tab != transitTab) {
    transit_reset();
  }

  currentRoadTab = tab;
  setWaterVisible(tab != pillarTab);
  if (tab != pillarTab) {
    stopGuide();
  }
  if (tab != pillarTab && pillarPlan) {
    discardPlan(pillarPlan);
    pillarPlan = 0;
  //} else if (tab >= repairTab && graphPlan) {
    //discardPlan(graphPlan);
    //graphPlan = 0;
  }
  discardAllUnsetPlans();

  if (currentRoadTab == transitTab) {
    transit_select();
  } else {
    transit_reset();
    elevationSelect(true, &elevation);
  }

  if (tab == pillarTab) {
    reportTutorialUpdate(SelectedPillarsModeInTT);
    stopBlinkingFeature(FRoadPillars);
  } else if (tab == cutTab) {
    reportTutorialUpdate(SelectedCutModeInTT);
    stopBlinkingFeature(FRoadCut);
  } else if (tab == repairTab) {
    stopBlinkingFeature(FRoadRepair);
  } else if (tab == transitTab) {
    reportTutorialUpdate(SelectedLinesModeInTT);
    stopBlinkingFeature(FBus);
  } else if (tab == buildTab) {
    reportTutorialUpdate(SelectedBuildModeInTT);
  }
}

void setRoadSystem(item system) {
  currentSystem = system;
  if (system == transitSystems) {
    setRoadTab(transitTab);
  } else {
    setRoadTab(buildTab);
  }
  if (system == expresswaySystem) {
    stopBlinkingFeature(FRoadExpressways);
  }
}

void renderRoadCursor() {
  Entity* entity = getEntity(builderCursorEntity);
  entity->texture = paletteTexture;
  setEntityBringToFront(builderCursorEntity, true);
  setEntityTransparent(builderCursorEntity, true);
  setEntityHighlight(builderCursorEntity, true);
  createMeshForEntity(builderCursorEntity);
  float scal = getCameraDistance() / 1000;
  if (scal < 1) scal = 1;
  placeEntity(builderCursorEntity, roadCursorLoc, 0, 0, scal);

  Entity* textEntity = getEntity(builderTextEntity);
  textEntity->texture = textTexture;
  textEntity->flags |= _entityNoHeatmap;
  setEntityBringToFront(builderTextEntity, true);
  createMeshForEntity(builderTextEntity);

  Mesh* mesh = getMeshForEntity(builderCursorEntity);
  Mesh* textMesh = getMeshForEntity(builderTextEntity);
  float angle = getHorizontalCameraAngle() - pi_o/2;
  float pitch = -getVerticalCameraAngle();
  float dist = getCameraDistance();
  float fontSize = dist/15;
  float tx = fontSize;
  float ty = 0;
  float tz = 0; //fontSize*2;
  vec3 tup = vec3(0,0,fontSize*2);

  // Are we in planner mode?
  bool plannerMode = isPlansEnabled();

  if (isBuilderMode() || currentRoadTab == cutTab) {
    const float cursorZ = 10;
    const float cursorX = 20;
    const float cursorBaseX = 12;
    const float cursorBaseInner = 8;

    vec3 tex = isPlacingEdge ? colorBrightGreen : colorWhite;
    float height = elevation.zOffset*c(CZTileSize);
    float baseZ = -height;

    if (height < 0) {
      baseZ += height;
    }

    vec3 baseLoc = vec3(0,0,baseZ);
    makePipe(mesh, baseLoc, vec3(0,0,abs(height)+2), cursorBaseX,
      cursorBaseInner, 20, tex);

    bool isModify = !isPlansMade && highlightElement != 0 &&
      isBuilderMode();
    if (highlightElement < 0 &&
        getNode(highlightElement)->flags & _graphCity) {
      isModify = false;
    }

    money cost = 0;
    char* resString = 0;
    Configuration config = getCurrentConfig();

    if (currentRoadTab == cutTab && highlightElement > 0) {
      config = getElementConfiguration(highlightElement);
      cost = getInflation()*strategyCost(config.strategy)*2;
      char* costStr = printMoneyString(cost);
      resString = sprintf_o("%s to Insert Intersection", costStr);
      free(costStr);

    } else if (isModify) {
      Configuration original = getElementConfiguration(highlightElement);
      bool equal = highlightElement < 0 ? configsEqualNode(config, original) :
        configsEqualEdge(config, original);
      bool switchDir = equal &&
        (original.flags & _configOneWay) && highlightElement > 0;
      if (!switchDir && (equal || config.type != original.type)) {
        isModify = false;
        cost = 0;
      } else {
        cost = switchDir ? minReconfigureCost(config.type)*getInflation() :
          reconfigureCost(highlightElement, config);
        if (cost > 1) {
          char* costStr = printMoneyString(cost);
          resString = switchDir ?
            sprintf_o("%s to Switch Direction", costStr) :
            sprintf_o("%s to Modify", printMoneyString(cost));
          free(costStr);
        } else {
          resString = switchDir ? strdup_s("Switch Direction") :
            strdup_s("Modify");
        }
      }

    } else if (isPlansMade) {
      cost = getTotalUnsetPlansCost();
      resString = printMoneyString(cost);
    }

    bool affordable = canBuy(config.type ?
        ExpwyBuildExpenses : RoadBuildExpenses, cost);

    if (resString != 0) {
      renderString(textMesh, resString, vec3(tx,ty,tz), fontSize);
      free(resString);
      if (isModify && affordable) {
        renderString(textMesh, "(Double Click)", vec3(tx,ty+fontSize,tz),
            fontSize*.75);
      }
    }

    placeEntity(builderTextEntity, roadCursorLoc+tup, angle, pitch);
    setEntityRedHighlight(builderTextEntity, !affordable && !plannerMode);
    setEntityHighlight(builderTextEntity, true);
    setEntityVisible(builderTextEntity, true);

  } else if (currentRoadTab == repairTab) {
    if (highlightElement > 0) {
      Edge* edge = getEdge(highlightElement);
      money cost = getRepairCost(highlightElement);
      float wear = edge->wear;
      float maxSpeed = getLaneBlock(edge->laneBlocks[0])->speedLimit;
      float postedSpeed = speedLimits[edge->config.speedLimit];
      bool affordable = canBuy(RepairExpenses, cost);

      placeEntity(builderTextEntity, roadCursorLoc+tup, angle, pitch);
      setEntityRedHighlight(builderTextEntity, !affordable);
      setEntityHighlight(builderTextEntity, true);
      setEntityVisible(builderTextEntity, true);

      char* resString;
      char* moneyString = "";
      char* speedString = "";
      if (cost > 1000) {
        char* mStr = printMoneyString(cost);
        moneyString = sprintf_o(" %s to Repair", mStr);
        free(mStr);
      }
      if (maxSpeed < postedSpeed) {
        speedString = printSpeedString("", maxSpeed, " Maximum Speed\n");
      }
      resString = sprintf_o(" %d%% Wear\n%s%s",
        int(wear*100), speedString, moneyString);

      renderString(textMesh, resString,
          vec3(tx-fontSize,ty-fontSize*.5,tz), fontSize);
      if (cost > 1000) {
        free(moneyString);
      }
      if (maxSpeed < postedSpeed) {
        free(speedString);
      }
      free(resString);
    }

  } else if (currentRoadTab == pillarTab) {
    money cost = getTotalUnsetPlansCost();
    if (cost > 0) {

      char* resString = printMoneyString(cost);
      bool affordable = canBuy(PillarBuildExpenses, cost);
      renderString(textMesh, resString, vec3(tx,ty,tz), fontSize);
      free(resString);
      setEntityRedHighlight(builderTextEntity, !affordable && !plannerMode);
    }

    placeEntity(builderTextEntity, roadCursorLoc+tup, angle, pitch);
    setEntityHighlight(builderTextEntity, true);
    setEntityVisible(builderTextEntity, true);

  } else if (currentRoadTab == transitTab) {
    money cost = getTotalUnsetPlansCost();
    if (cost > 0) {
      char* resString = printMoneyString(cost);
      bool affordable = canBuy(TransitBuildExpenses, cost);
      renderString(textMesh, resString, vec3(tx,ty,tz), fontSize);
      free(resString);
      setEntityRedHighlight(builderTextEntity, !affordable && !plannerMode);

    } else if (highlightStop != 0) {
      if (lastStopAdded < 0) {
        // no-op
      } else {
        renderString(textMesh, "Add Stop To Line",
            vec3(tx,ty,tz), fontSize);
        setEntityRedHighlight(builderTextEntity, false);
      }
    }

    placeEntity(builderTextEntity, roadCursorLoc+tup, angle, pitch);
    setEntityHighlight(builderTextEntity, true);
    setEntityVisible(builderTextEntity, true);

  } else {
    setEntityVisible(builderTextEntity, false);
  }

  bufferMesh(textEntity->mesh);
  bufferMesh(entity->mesh);
}

vec3 smartUnitize(vec3 loc) {
  if (gridMode) {
    if (currentSystem == roadSystem) {
      loc = unitize(loc);
    } else {
      loc = unitizeFine(loc);
    }
  }
  return loc;
}

void findRoadLocation(Line mouseLine, item ndx, bool shift) {
  item nearbyPillar = intersectPillar(mouseLine);
  item nearbyCity = intersectCity(mouseLine);

  if (nearbyPillar) {
    Pillar* p = getPillar(nearbyPillar);
    roadEnd[ndx] = p->location;
    roadPillar[ndx] = nearbyPillar;
    cityNode[ndx] = 0;
    highlightElement = p->node;
    roadCursorLoc = p->location;
    raiseCursor = false;

  } else if (nearbyCity) {
    City* city = getCity(nearbyCity);
    cityNode[ndx] = city->node;
    roadEnd[ndx] = city->visualCenter;
    roadPillar[ndx] = 0;
    highlightElement = city->node;
    roadCursorLoc = getNode(city->node)->center;
    raiseCursor = false;

  } else {
    roadPillar[ndx] = 0;
    cityNode[ndx] = 0;
    raiseCursor = true;
    item nearbyElem = nearestElement(mouseLine, true);

    if (nearbyElem == 0) {
      roadEnd[ndx] = roadCursorLoc;
      return;
    }

    Line l = getLine(nearbyElem);
    vec3 elemLoc = pointOfIntersection(l, mouseLine);
    elemLoc = smartUnitize(elemLoc);
    elemLoc = nearestPointOnLine(elemLoc, l);
    item newElementToSnapTo = 0;

    if (nearbyElem > 0) {
      // First, let's check if we're near the ends of the edge.
      Line endsPosition = getEdge(nearbyElem)->line;
    
      if (vecDistance(roadCursorLoc, endsPosition.start) < c(CSnapDistance)) {
        newElementToSnapTo = getEdge(nearbyElem)->ends[0];
      } else if (vecDistance(roadCursorLoc, endsPosition.end) < c(CSnapDistance)) {
        newElementToSnapTo = getEdge(nearbyElem)->ends[1];
      }
    } else if (pointLineDistance(elemLoc, mouseLine) < c(CSnapDistance)) {
      // If we're close to a node, say in the middle of a road,
      // then let's snap to that instead.
      newElementToSnapTo = nearbyElem;
    }

    if (newElementToSnapTo) {
      // We will always hit a node here because at this point:
      // 1) we snap to a nearby node.
      // 2) we are on an edge and snapped to either the start node or end node.
      elemLoc = getNode(newElementToSnapTo)->center;

      highlightElement = newElementToSnapTo;
      roadCursorLoc = elemLoc;

      if (!shift) {
        roadCursorLoc.z = pointOnLandNatural(roadCursorLoc).z + c(CZTileSize) * elevation.zOffset;
        if (abs(roadCursorLoc.z - elemLoc.z) > 4) {
          highlightElement = 0;
        }
      }
      raiseCursor = false;
    }

    roadEnd[ndx] = roadCursorLoc;
  }
}

item getRoadNode(item end) {
  Configuration config = getCurrentConfig();
  if (cityNode[end] != 0) {
    return cityNode[end];
  } else if (roadPillar[end] != 0) {
    return getOrCreatePillarNodeAt(roadPillar[end], config);
  } else {
    return getOrCreateNodeAt(roadEnd[end], config);
  }
}

void swapEndToStart() {
  roadEnd[0] = roadEnd[1];
  roadPillar[0] = roadPillar[1];
  cityNode[0] = cityNode[1];
}

void road_mouse_button_callback(InputEvent event) {
  if (event.action != GLFW_PRESS || event.button == GLFW_MOUSE_BUTTON_MIDDLE) {
    return;
  }

  /*
  // Don't allow for building if Tutorial is waiting for input
  TutorialState* ptr = getTutorialStatePtr();
  if (ptr != 0 && ptr->showTutorial()) {
    // Reset road tool state to prevent accidents
    discardAllUnsetPlans();
    isPlacingEdge = false;
    isPlansMade = false;
    return;
  }
  */

  bool isLMB = event.button == GLFW_MOUSE_BUTTON_LEFT;
  bool isRMB = event.button == GLFW_MOUSE_BUTTON_RIGHT;
  bool isRMBMod = isRMB && (event.mods & GLFW_MOD_CONTROL);

  if (isBuilderMode()) {
    if (isRMB) {
      if (isPlacingEdge) {
        discardAllUnsetPlans();
        isPlacingEdge = false;
        isPlansMade = false;

      } else if (highlightElement != 0 && isRMBMod) {
        Configuration selectConfig = getElementConfiguration(highlightElement);
        item type = selectConfig.type;
        //setRoadTab(type == ConfigTypeRoad ? 0 :
          //type == ConfigTypeExpressway ? 1 : transitTab);
        //if (type == ConfigTypePedestrian) setTransitTab(pedTab);
        //if (type == ConfigTypeHeavyRail) setTransitTab(railTab);

        if (type == ConfigTypePedestrian) {
          playSound(_soundClickCancel);
        } else {
          for (int j = 0; j < 3; j++) {
            item numC = numConfigs[j];
            for (int i = 0; i < numC; i++) {
              Configuration compare = configs[j][i];
              if (configsEqual(compare, selectConfig)) {
                playSound(_soundClickCancel);
                currentConfigNdx[j] = i;

                if (type == ConfigTypeExpressway) {
                  setRoadSystem(expresswaySystem);
                } else if (type == ConfigTypeHeavyRail) {
                  setRoadSystem(transitSystems);
                } else if (type == ConfigTypeRoad &&
                    currentSystem == expresswaySystem) {
                  setRoadSystem(roadSystem);
                }

                break;
              }
            }
          }
        }
      }

    } else if (isPlansMade) {
      if (linkMode) {
        isPlacingEdge = true;
        swapEndToStart();

      } else {
        isPlacingEdge = false;
      }

      playSound(_soundClickComplete);
      isPlansMade = false;
      if (isPlansEnabled()) {
        setAllPlans();
      } else {
        buyAllUnsetPlans();
      }

      item type = getCurrentConfig().type;
      reportTutorialUpdate(
          type == ConfigTypeRoad ? BuildRoad :
          type == ConfigTypeExpressway ? BuildExpressway :
          type == ConfigTypeHeavyRail ? BuildRail :
          NoAction);

    } else if (isPlacingEdge && highlightElement != 0 &&
        glfwGetTime()-startClickTime < 0.75) {
      isPlacingEdge = false;
      isPlansMade = false;
      Configuration config = getCurrentConfig();
      Configuration original = getElementConfiguration(highlightElement);
      bool equal = highlightElement < 0 ?
        configsEqualNode(config, original) :
        configsEqualEdge(config, original);
      BudgetLine line = config.type ?
        ExpwyBuildExpenses : RoadBuildExpenses;

      if (equal && (config.flags & _configOneWay) && highlightElement > 0) {
        money cost = minReconfigureCost(config.type) * getInflation();
        if (transaction(line, -cost)) {
          playSound(_soundClickComplete);
          switchDirection(highlightElement);
        }

      } else if (!equal &&
          transaction(line, -reconfigureCost(highlightElement, config))) {
        playSound(_soundClickComplete);
        reconfigure(highlightElement, config);
      }
      discardAllUnsetPlans();

    } else if (!isPlacingEdge) {
      playSound(_soundClickPrimary);
      startClickTime = glfwGetTime();
      isPlacingEdge = true;
      isPlansMade = false;
    }

  } else if (currentRoadTab == repairTab) {
    if(isLMB || isRMBMod) {
      if(highlightElement > 0) {
        if(transaction(RepairExpenses, -getRepairCost(highlightElement))) {
          playSound(_soundClickComplete);
          repairEdge(highlightElement);
        }
      }
    }
  } else if (currentRoadTab == cutTab) {
    if(isLMB || isRMBMod) {
      if(highlightElement > 0) {
        Configuration config = getElementConfiguration(highlightElement);
        item node = getOrCreateNodeAt(roadCursorLoc, config);
        Node* n = getNode(node);
        money cost = graphCostSimple(node);
        BudgetLine costType = config.type ?
          ExpwyBuildExpenses : RoadBuildExpenses;
        if(canBuy(costType, cost)) {
          complete(node, config);
        } else {
          removeNode(node);
        }
        if(n->edges.size() == 0) {
          removeNode(node);
        }
        if(n->flags & _graphExists) {
          playSound(_soundClickComplete);
        } else {
          playSound(_soundClickCancel);
        }
      }
    }
  } else if (currentRoadTab == pillarTab) {
    if(isLMB || isRMBMod) {
      if(pillarPlan != 0) {
        playSound(_soundClickComplete);
        setPlan(pillarPlan);
        reportTutorialUpdate(BuildPillar);
        if(!isPlansEnabled()) {
          if(!buyPlan(pillarPlan)) {
            discardPlan(pillarPlan);
          }
        }
        pillarPlan = 0;
      }
    }
  } else if (currentRoadTab == transitTab) {
    if(isLMB || isRMBMod) {
      transit_click(event);
    }
  }
}

void road_mouse_move_callback(InputEvent event) {
  //for (int i = 0; i < emDomBuildingsR.size(); i++) {
    //setRedHighlight(false, SelectionBuilding, emDomBuildingsR[i]);
  //}
  //emDomBuildingsR.clear();

  vec3 loc = landIntersect(event.mouseLine);
  loc = smartUnitize(loc);
  roadCursorLoc = pointOnLandNatural(loc) +
    vec3(0, 0, c(CZTileSize) * elevation.zOffset + c(CRoadRise)*2.f);

  if (highlightElement != 0) {
    setElementHighlight(highlightElement, false);
    highlightElement = 0;
  }
  discardAllUnsetPlans();
  highlightElement = nearestElement(event.mouseLine, true);
  if (highlightElement != 0) {
    Line highLine = getLine(highlightElement);
    if (highlightElement != 0 &&
        lineDistance(event.mouseLine, highLine) > c(CSnapDistance)) {
      highlightElement = 0;
    } else {
      roadCursorLoc = nearestPointOnLine(roadCursorLoc, highLine);
    }
  }

  bool shift = event.mods & GLFW_MOD_SHIFT;
  //setAutomaticBridges(!shift);
  //SPDLOG_INFO("shift {}", shift);

  if (isBuilderMode()) {
    Configuration config = getCurrentConfig();
    //if (graphPlan) {
      //discardPlan(graphPlan);
      //graphPlan = 0;
    //}
    isPlansMade = false;

    if (isPlacingEdge) {
      findRoadLocation(event.mouseLine, 1, shift);
      item node0 = getRoadNode(0);
      item node1 = getRoadNode(1);
      if (node0 != node1) {
        item edge = addEdge(node0, node1, config);
        if (edge != 0) {
          addPlan(GraphElementPlan, edge, config);
          //if (!shift) 
          split(edge);
          isPlansMade = true;
          //graphPlan = addPlan(GraphElementPlan, edge, config);
        }
      }

    } else {
      findRoadLocation(event.mouseLine, 0, shift);
    }

    if (numUnsetPillarPlans() > 0) {
      lastPillarTime = getCameraTime();
    }
    bool waterViz = ((config.flags & _configDontMoveEarth) &&
        elevation.zOffset < 0) ||
      getCameraTime() - lastPillarTime > .5;
    setWaterVisible(waterViz);

  } else if (currentRoadTab == pillarTab) {
    if (pillarPlan != 0) {
      discardPlan(pillarPlan);
      pillarPlan = 0;
    }
    pillarPlan = addPlan(PillarPlan, addPillar(roadCursorLoc, suspension));
    setGuide(roadCursorLoc, suspension ?
        c(CMaxSuspensionBridgeLength) : c(CMaxBridgeLength));
    setWaterVisible(false);

  } else if (currentRoadTab == transitTab) {
    transit_move(event);

  } else {
    setWaterVisible(true);
  }

  if (highlightElement != 0) {
    setElementHighlight(highlightElement, true);
  }
  renderRoadCursor();
}

void setRoadInfo() {
  if (isPlansEnabled()) {
    toolRoad.flags |= _toolForceInstructions;
  } else {
    toolRoad.flags &= ~_toolForceInstructions;
  }
}

void road_select() {
  if (builderCursorEntity == 0) {
    builderCursorEntity = addEntity(PaletteShader);
    builderTextEntity = addEntity(WSUITextShader);
  }
  setRoadTab(currentRoadTab);
  setRoadInfo();
  if (currentRoadTab == transitTab) {
    transit_select();
  }
  elevationSelect(true, &elevation);
  setPlansActive(true);
}

void road_reset() {
  //if (graphPlan != 0) {
    //discardPlan(graphPlan);
    //graphPlan = 0;
  //}
  discardAllUnsetPlans();
  if (currentRoadTab == transitTab) {
    transit_reset();
  }
  if (pillarPlan != 0) {
    discardPlan(pillarPlan);
    pillarPlan = 0;
  }
  isPlacingEdge = false;
  isPlansMade = false;
  if (highlightElement != 0) {
    setElementHighlight(highlightElement, false);
    highlightElement = 0;
  }
  if (builderCursorEntity != 0) {
    removeEntityAndMesh(builderCursorEntity);
    builderCursorEntity = 0;
  }
  if (builderTextEntity != 0) {
    removeEntityAndMesh(builderTextEntity);
    builderTextEntity = 0;
  }
  elevationSelect(false, &elevation);
  stopGuide();
  setWaterVisible(true);
  setPlansActive(false);
}

static bool setConfig(Part* part, InputEvent event) {
  item ndx = part->itemData;
  item type = getConfigNdx();
  item currentNdx = currentConfigNdx[type];
  item numC = numConfigs[type];
  stopBlinkingFeature(FRoadStreet+ndx);
  //stopAddingStop();
  currentConfigNdx[type] = ndx;
  return true;
}

static bool setRoadTab(Part* part, InputEvent event) {
  item ndx = part->itemData;
  setRoadTab(ndx);
  return true;
}

static bool setRoadSystem(Part* part, InputEvent event) {
  item ndx = part->itemData;
  setRoadSystem(ndx);
  if (ndx == 0) {
    reportTutorialUpdate(SelectedRoadsInTT);
  } else if (ndx == 1) {
    reportTutorialUpdate(SelectedExpresswaysInTT);
  }
  return true;
}

static bool toggleSuspension(Part* part, InputEvent event) {
  item ndx = part->itemData;
  suspension = ndx;
  return true;
}

static bool toggleGridMode(Part* part, InputEvent event) {
  item ndx = part->itemData;
  gridMode = !gridMode;
  reportTutorialUpdate(SelectedGridlessModeInTT);
  return true;
}

static bool toggleLinkMode(Part* part, InputEvent event) {
  item ndx = part->itemData;
  linkMode = !linkMode;
  reportTutorialUpdate(SelectedLinkModeInTT);
  return true;
}

Part* road_render(Line dim) {
  Part* result = panel(dim);

  const char* systemName;
  vec3 sysIco;
  if (currentSystem == roadSystem) {
    systemName = "Roads";
    sysIco = iconRoad;
  } else if (currentSystem == expresswaySystem) {
    systemName = "Expressways";
    sysIco = iconExpressway;
  } else if (getCurrentTransitSystem() <= 0) {
    systemName = "Transit";
    sysIco = iconBus;
  } else {
    TransitSystem* system = getTransitSystem(getCurrentTransitSystem());
    systemName = system->name;
    sysIco = iconTransitLogos[system->logo];
  }

  r(result, icon(vec2(0,0), sysIco));
  r(result, label(vec2(1,0), 1, strdup_s(systemName)));
  //r(result, hr(vec2(0,1), dim.end.x-.2));

  //bool builder = currentRoadTab < 2 ||
    //(currentRoadTab == transitTab && getCurrentTransitSystem() > 0);
  bool builder = currentRoadTab == buildTab;

  if (currentSystem == transitSystems) {
    item currentSys = getCurrentTransitSystem();
    if (currentSys <= 0 || currentSys > sizeTransitSystems()) {
      builder = false;
    } else {
      TransitSystem* system = getTransitSystem(currentSys);
      if (!(system->flags & _transitComplete)) {
        builder = false;
      }
    }
  }

  item graphType = currentSystem == roadSystem ? ConfigTypeRoad :
    currentSystem == expresswaySystem ? ConfigTypeExpressway :
    currentSystem == transitSystems && getCurrentTransitSystem() > 0 ?
    getSystemGraphType(getCurrentTransitSystem()) : ConfigTypeRoad;

  // Set appropriate tooltips for the current system
  TooltipType ttType = TooltipType::RoadButtRoad;
  switch(currentSystem) {
    case roadSystem:
      ttType = TooltipType::RoadButtRoad;
      break;
    case expresswaySystem:
      ttType = TooltipType::RoadButtExpress;
      break;
    case transitSystems:
      if(graphType == ConfigTypeHeavyRail) {
        ttType = TooltipType::RoadButtRail;
      } else {
        ttType = TooltipType::RoadButtRoad;
      }
    default: // Default to Road tooltips
      break;
  }

  if (builder) {
    item configNdx = getConfigNdx();
    item currentNdx = currentConfigNdx[configNdx];
    item numC = numConfigs[configNdx];
    for (int i=0; i < numC; i++) {
      if (configNdx == 0 && !isFeatureEnabled(FRoadStreet+i)) continue;
      Part* configContain = panel(vec2(i*1.25+1.25, 1.25f), vec2(1, 2));
      configContain->renderMode = RenderTransparent;
      configContain->itemData = i;
      configContain->onClick = setConfig;
      configContain->flags |= _partHover;
      setPartTooltipValues(configContain,
        ttType+1+i);
      if (i > 0 && blinkFeature(FRoadStreet+i)) {
        configContain->flags |= _partBlink;
      }
      r(result, configContain);

      Part* roadButt = icon(vec2(0,0), configIcons[configNdx][i][0]);
      roadButt->onClick = setConfig;
      roadButt->itemData = i;
      r(configContain, roadButt);
      r(configContain, icon(vec2(0,1), configIcons[configNdx][i][1]));

      if (i == currentNdx) {
        configContain->flags |= _partHighlight;
      } else if (i == (currentNdx+numC-1)%numC) {
        configContain->inputAction = ActTTPreviousType;
        roadButt->inputAction = ActTTPreviousType;
      } else if (i == (currentNdx+1)%numC) {
        configContain->inputAction = ActTTNextType;
        roadButt->inputAction = ActTTNextType;
      }
    }

    if (isFeatureEnabled(FRoadAve) || isFeatureEnabled(FRoadBlvd)) {
      r(result, labelCenter(vec2(1.25f, 3.5f),
            vec2(6.5f, .8f), strdup_s(roadName[configNdx][currentNdx])));
    }

    r(result, elevationWidget(vec2(7.5f, 1.25f), &elevation));

    if (isFeatureEnabled(FGrid)) {
      Part* gridButt = r(result, button(vec2(1.25f, 4.75f),
            iconGrid, vec2(1,1),
            toggleGridMode, 0));
      gridButt->inputAction = ActTTGridMode;
      setPartTooltipValues(gridButt, TooltipType::RoadGridMode);
      if (gridMode) gridButt->flags |= _partHighlight;

      Part* linkButt = r(result, button(vec2(2.5f, 4.75f),
            iconLink, vec2(1,1),
            toggleLinkMode, 0));
      linkButt->inputAction = ActTTLinkMode;
      setPartTooltipValues(linkButt, TooltipType::RoadLinkMode);
      if (linkMode) linkButt->flags |= _partHighlight;
    }

  } else if (currentRoadTab == transitTab) {
    transit_render(result);

  } else if (currentRoadTab == repairTab) {
    r(result, icon(vec2(4.5f,2.5f), vec2(1,1), iconWrench));

  } else if (currentRoadTab == cutTab) {
    r(result, icon(vec2(4.5f,2.5f), vec2(1,1), iconCut));

  } else if (currentRoadTab == transitTab) {
    transit_render(result);

  } else if (currentRoadTab == pillarTab) {
    for (int i = 0; i < 2; i++) {
      if (i && !isFeatureEnabled(FRoadSuspensionBridge)) continue;
      Part* pillarButt = r(result, button(vec2(3.5f+i*2,2.5f),
            iconBridgePillar[i], vec2(1,1),
            toggleSuspension, i));
      pillarButt->inputAction = i ? ActTTSuspensionPillar : ActTTTrussPillar;
      if (i == suspension) pillarButt->flags |= _partLowered;
    }
  }

  Part* systemsPanel = panel(vec2(0,6), vec2(10,1));
  systemsPanel->flags |= _partLowered;
  r(result, systemsPanel);

  for (int i = 0; i < 2 ; i++) {
    if (i == 1 && !isFeatureEnabled(FRoadExpressways)) continue;
    Part* roadButt = button(vec2(i, 0.f),
        i == 0 ? iconRoad : iconExpressway, setRoadSystem, i);
    roadButt->inputAction = i ? ActExpresswaySubtool : ActRoadSubtool;
    setPartTooltipValues(roadButt,
      i == 0 ? TooltipType::RoadButtRoad : TooltipType::RoadButtExpress);
    if (currentSystem == i) roadButt->flags |= _partHighlight;
    if (i == 0 ? blinkFeature(FRoadStreet) : blinkFeature(FRoadExpressways)) {
      roadButt->flags |= _partBlink;
    }

    r(systemsPanel, roadButt);
  }

  float x = 2;
  int k = 0;
  for (int i = 1; i <= sizeTransitSystems(); i++) {
    TransitSystem* system = getTransitSystem(i);
    if (!(system->flags & _transitExists)) continue;
    if (!(system->flags & _transitComplete)) continue;

    Part* butt = r(systemsPanel, button(vec2(x, 0.f),
          iconTransitLogos[system->logo], setTransitSystem, i));
    if (currentSystem == transitSystems && getCurrentTransitSystem() == i) {
      butt->flags |= _partHighlight;
    }
    butt->inputAction = (i==1 ? ActBusSubtool : 
        i==2 ? ActTrainSubtool : ActNone);
    x ++;
    k ++;
  }

  if (isFeatureEnabled(FTransitSystems)) {
    Part* butt = r(systemsPanel, button(vec2(x, 0.f), iconPlus,
          newTransitSystemWithKey, 0));
    setPartTooltipValues(butt,
      TooltipType::RoadButtTransit);
  }

  if (isFeatureEnabled(FRoadPlanner)) {
    Part* buttPlans = button(vec2(9.f, 0.f), iconCheck, togglePlanPanel);
    buttPlans->inputAction = ActTTPlannerMode;
    setPartTooltipValues(buttPlans,
      TooltipType::RoadButtPlanner);
    if (blinkFeature(FRoadPlanner)) {
      buttPlans->flags |= _partBlink;
    }
    if (isPlansEnabled()) {
      buttPlans->flags |= _partHighlight;
    }
    r(systemsPanel, buttPlans);
  }

  if (!isFeatureEnabled(FRoadPillars) &&
    !isFeatureEnabled(FRoadCut) &&
    !isFeatureEnabled(FBus) &&
    !isFeatureEnabled(FRail) &&
    !isFeatureEnabled(FTransitSystems)) return result;

  Part* tabPanel = panel(vec2(0,1.25), vec2(1,4.5));
  tabPanel->flags |= _partLowered;
  r(result, tabPanel);
  float ty = 0;
  float tySpacing = 1.083333;

  vec3 buildIcon = graphType == ConfigTypeHeavyRail ? iconRail2 :
    graphType == ConfigTypeExpressway ? iconExpressway : iconRoad;

  Part* buildButt = button(vec2(0.f, ty), buildIcon,
      setRoadTab, buildTab);
  buildButt->inputAction = ActTTBuildMode;
  setPartTooltipValues(buildButt,
    ttType);
  if (currentRoadTab == buildTab) {
    buildButt->flags |= _partHighlight;
  }
  r(tabPanel, buildButt);
  ty += tySpacing;

  if (currentSystem == transitSystems) {
    Part* transitButt = button(vec2(0.f, ty), iconRoute,
        setRoadTab, transitTab);
    transitButt->inputAction = ActTTTransitRouteMode;
    setPartTooltipValues(transitButt,
      TooltipType::RoadButtTransit);
    if (blinkFeature(FBus) || blinkFeature(FRail)
        || blinkFeature(FTransitSystems)) {
      transitButt->flags |= _partBlink;
    }
    if (currentRoadTab == transitTab) {
      transitButt->flags |= _partHighlight;
    }
    r(tabPanel, transitButt);
  }
  ty += tySpacing;

  /*
  if (isFeatureEnabled(FRoadRepair)) {
    Part* repairButt = button(vec2(0.f, ty), iconWrench,
        setRoadTab, repairTab);
    repairButt->onKeyDown = setRoadTab;
    repairButt->text = strdup_s(tabKeyNames[repairTab]);
    setPartTooltipValues(repairButt,
      TooltipType::RoadButtRepair);
    if (blinkFeature(FRoadRepair)) {
      repairButt->flags |= _partBlink;
    }
    if (currentRoadTab == repairTab) {
      repairButt->flags |= _partHighlight;
    }
    r(tabPanel, repairButt);
    ty += tySpacing;
  }
  */

  if (c(CEnableCutTool) && isFeatureEnabled(FRoadCut)) {
    Part* cutButt = button(vec2(0, ty), iconCut, setRoadTab, cutTab);
    cutButt->inputAction = ActTTCutMode;
    setPartTooltipValues(cutButt,
      TooltipType::RoadButtCut);
    if (blinkFeature(FRoadCut)) {
      cutButt->flags |= _partBlink;
    }
    if (currentRoadTab == cutTab) {
      cutButt->flags |= _partHighlight;
    }
    r(tabPanel, cutButt);
    ty += tySpacing;
  }

  if (isFeatureEnabled(FRoadPillars)) {
    Part* pillarButt = button(vec2(0.f, ty), iconBridgePillar[0],
        setRoadTab, pillarTab);
    pillarButt->inputAction = ActTTPillarMode;
    setPartTooltipValues(pillarButt,
      TooltipType::RoadButtPillar);
    if (blinkFeature(FRoadPillars)) {
      pillarButt->flags |= _partBlink;
    }
    if (currentRoadTab == pillarTab) {
      pillarButt->flags |= _partHighlight;
    }
    r(tabPanel, pillarButt);
    ty += tySpacing;
  }

  return result;
}

void roadInstructionPanel(Part* panel) {
  if (isPlansEnabled()) {
    plansPanel(panel);

  } else {
    float multilineWidth = 13.5f;
    float multilineTextSize = 0.8f;
    std::string helpText = instructionMessages[currentRoadTab];

    if(currentRoadTab == buildTab && isFeatureEnabled(FRoadElevation)) {
      multilineTextSize = 0.75f;
      helpText += "\n \nRoads can be raised and lowered by pressing the "
        " up and down arrows (Q and Z). By default, engineers will"
        " build a trench or embankment under your roads, but if you select the"
        " tunnel/viaduct option (H), they will build a tunnel or viaduct"
        " (bridge structure) instead.";
    }

    float y = 0;
    Part* info = multiline(vec2(0,0),
        vec2(multilineWidth,multilineTextSize),
        strdup_s(helpText.c_str()), &y);
    info->dim.start.y = 3.5 - y*.5;
    r(panel, info);
  }
}

bool isTransitTool() {
  return getCurrentTool() == 2 && currentRoadTab == transitTab;
}

bool isCutTool() {
  return getCurrentTool() == 2 && currentRoadTab == cutTab;
}

bool road_visible() {
  return true;
}

Tool* getToolRoad() {
  return &toolRoad;
}

bool togglePlanPanel(Part* part, InputEvent event) {
  setPlansEnabled(!isPlansEnabled());
  reportTutorialUpdate(SelectedPlannerModeInTT);
  return true;
}

