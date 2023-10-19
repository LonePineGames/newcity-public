//-----------------------------------------------------------------------------
// tutorial - Contains the data structure and API for
// accessing and manipulating Tutorial specific state data
//-----------------------------------------------------------------------------

#include "tutorial.hpp"

#include "city.hpp"
#include "economy.hpp"
#include "game/feature.hpp"
#include "game/game.hpp"
#include "heatmap.hpp"
#include "parts/leftPanel.hpp"
#include "parts/tutorialPanel.hpp"
#include "option.hpp"
#include "string_proxy.hpp"

static const char* tutorialCodes[TutorialCategory::Num_Categories] = {
  #define TUT(N) "Tutorial::" #N,
  #include "tutorialsEnum.hpp"
  #undef TUT
};

static std::string tutorialFiles[TutorialCategory::Num_Categories] = {
  #define TUT(N) "tutorial/" #N,
  #include "tutorialsEnum.hpp"
  #undef TUT
};


static TutorialState tState;

//-----------------------------------------------------------------------------
// TutorialState member functions
//-----------------------------------------------------------------------------

bool TutorialState::setCategory(TutorialCategory cat) {
  if(cat >= TutorialCategory::Num_Categories || cat < 0) return false;
  _category = cat;
  _tutorialOKWait = false;
  setTimerTarget(tutorialDefaultTimer);
  stopBlinkingFeatures();

  if (!setActionsForCategory(_category))
    SPDLOG_ERROR("Error setting tutorial actions");
  return true;
}

// Represents the necessary actions to advance to the specified TutorialCategory, 
// not the actions for that particular Tutorial Category.
// Also sets timer targets for WaitForTimer actions.
bool TutorialState::setActionsForCategory(TutorialCategory cat) {
  switch(cat) {
    case TutorialCategory::Welcome00:
      setFeatureBlink(FPlay, true);
      setActions(TutorialUpdateCode::ClosedMainMenu);
      break;

    case TutorialCategory::Camera00:
      setActions(TutorialUpdateCode::CameraZoomIn, TutorialUpdateCode::CameraZoomOut);
      break;
    case TutorialCategory::Camera01:
      setActions(TutorialUpdateCode::CameraMoveUp, TutorialUpdateCode::CameraMoveDown,
        TutorialUpdateCode::CameraMoveLeft, TutorialUpdateCode::CameraMoveRight);
      break;
    case TutorialCategory::Camera02:
      setActions(TutorialUpdateCode::WaitForTimer);
      setTimerTarget(tutorialDefaultTimer);
      break;
    case TutorialCategory::Camera03:
      setActions(TutorialUpdateCode::CameraRotateLeft, TutorialUpdateCode::CameraRotateRight);
      break;

    case TutorialCategory::Road00:
      setFeatureBlink(FRoadTool, true);
      setActions(TutorialUpdateCode::SelectedRoadTool);
      break;
    case TutorialCategory::Road01:
      setActions(TutorialUpdateCode::BuildRoad);
      break;
    case TutorialCategory::Road02:
      setFeatureBlink(FRoadAve, true);
      setActions(TutorialUpdateCode::BuildRoad, TutorialUpdateCode::BuildRoad, TutorialUpdateCode::BuildRoad);
      break;
    case TutorialCategory::Road03:
      setActions(TutorialUpdateCode::BuildRoad, TutorialUpdateCode::BuildRoad);
      break;

    case TutorialCategory::Zone00:
      setFeatureBlink(FZoneTool, true);
      setActions(TutorialUpdateCode::SelectedZoneTool);
      break;
    case TutorialCategory::Zone01:
      setActions(TutorialUpdateCode::ZonedResidential, TutorialUpdateCode::ZonedResidential, TutorialUpdateCode::ZonedResidential, TutorialUpdateCode::ZonedResidential);
      break;
    case TutorialCategory::Zone02:
      setFeatureBlink(FZoneFarm, true);
      setActions(TutorialUpdateCode::ZonedAgriculture, TutorialUpdateCode::ZonedAgriculture, TutorialUpdateCode::ZonedAgriculture, TutorialUpdateCode::ZonedAgriculture);
      break;
    case TutorialCategory::Zone03:
      setActions(TutorialUpdateCode::Population500);
      break;
    case TutorialCategory::Zone04:
      setFeatureBlink(FZoneRetail, true);
      setActions(TutorialUpdateCode::ZonedRetail);
      break;

    case TutorialCategory::Amenity00:
      setFeatureBlink(FBuildingTool, true);
      setActions(TutorialUpdateCode::SelectedAmenityTool);
      break;
    case TutorialCategory::Amenity01:
      setActions(TutorialUpdateCode::SelectedSchoolInAmenityTool);
      break;
    case TutorialCategory::Amenity02:
      setActions(TutorialUpdateCode::BuildAmenity, TutorialUpdateCode::ClosedTool);
      break;
    case TutorialCategory::Amenity03:
      setGameSpeed(3);
      setFeatureBlink(FObserveEducation, true);
      setFeatureBlink(FObserveValue, true);
      setActions(TutorialUpdateCode::SelectedHeatmapValue,
          TutorialUpdateCode::SelectedHeatmapEducation);
      break;
    case TutorialCategory::Amenity04:
      setGameSpeed(2);
      setActions(TutorialUpdateCode::ClosedTool);
      break;

    case TutorialCategory::Budget00:
      setFeatureBlink(FBudget, true);
      setActions(TutorialUpdateCode::SelectedBudgetPanel);
      break;
    case TutorialCategory::Budget01:
      setFeatureBlink(FPropertyTax, true);
      setFeatureBlink(FLoanTerm, true);
      setActions(TutorialUpdateCode::UpdatedBudgetTax,
        TutorialUpdateCode::UpdatedBudgetLoan);
      break;
    case TutorialCategory::Budget02:
      setHasCompletedTutorial(true);
      writeOptions();
      setActions(TutorialUpdateCode::ClosedBudgetPanel, TutorialUpdateCode::WaitForTimer);
      setTimerTarget(tutorialDefaultTimer);
      break;

    case TutorialCategory::TutorialMenu00:
      setActions(TutorialUpdateCode::WaitForTimer);
      setTimerTarget(tutorialDefaultTimer);
      break;

    case TutorialCategory::Blueprints00:
      setActions(TutorialUpdateCode::OpenScenarioEditor);
      break;
    case TutorialCategory::Blueprints01:
      setActions(TutorialUpdateCode::BuildRoad, TutorialUpdateCode::BuildRoad, TutorialUpdateCode::BuildRoad);
      break;
    case TutorialCategory::Blueprints02:
      setActions(TutorialUpdateCode::ZonedResidential, TutorialUpdateCode::ZonedResidential, TutorialUpdateCode::ZonedResidential, TutorialUpdateCode::ZonedResidential);
      break;
    case TutorialCategory::Blueprints03:
      setActions(TutorialUpdateCode::SelectedBlueprintTool);
      break;
    case TutorialCategory::Blueprints04:
      setActions(TutorialUpdateCode::StartBlueprintCapture, TutorialUpdateCode::FinishBlueprintCapture);
      break;
    case TutorialCategory::Blueprints05:
      setActions(TutorialUpdateCode::PlaceBlueprint);
      break;
    case TutorialCategory::Blueprints06:
      setActions(TutorialUpdateCode::SaveBlueprint);
      break;
    case TutorialCategory::Blueprints07:
      setActions(TutorialUpdateCode::WaitForTimer);
      setTimerTarget(tutorialDefaultTimer);
      break;

    case TutorialCategory::Economy00:
      setActions(TutorialUpdateCode::WaitForTimer);
      setTimerTarget(tutorialDefaultTimer);
      break;
    case TutorialCategory::Economy01:
      setActions(TutorialUpdateCode::WaitForTimer);
      setTimerTarget(tutorialDefaultTimer);
      break;
    case TutorialCategory::Economy02:
      setActions(TutorialUpdateCode::WaitForTimer);
      setTimerTarget(tutorialDefaultTimer);
      break;
    case TutorialCategory::Economy03:
      setActions(TutorialUpdateCode::SelectedQueryTool);
      break;
    case TutorialCategory::Economy04:
      setActions(TutorialUpdateCode::QueryBuilding);
      break;
    case TutorialCategory::Economy05:
      setActions(TutorialUpdateCode::QueryPerson);
      break;
    case TutorialCategory::Economy06:
      setActions(TutorialUpdateCode::QueryBuilding);
      break;
    case TutorialCategory::Economy07:
      setActions(TutorialUpdateCode::QueryBusiness);
      break;
    case TutorialCategory::Economy08:
      setFeatureBlink(FEconomyPanel, true);
      setActions(TutorialUpdateCode::SelectedChartsPanel);
      break;
    case TutorialCategory::Economy09:
      setActions(TutorialUpdateCode::SelectedChart);
      break;
    case TutorialCategory::Economy10:
      setActions(TutorialUpdateCode::SelectedChart);
      break;
    case TutorialCategory::Economy11:
      setActions(TutorialUpdateCode::SelectedAmenityScoresPanel, WaitForTimer);
      setTimerTarget(tutorialDefaultTimer);
      break;

    case TutorialCategory::Transit00:
      setActions(SelectedRoadTool, SelectedBusesInTT);
      break;
    case TutorialCategory::Transit01:
      setActions(AddedTransitLineInTT);
      break;
    case TutorialCategory::Transit02:
      setActions(BuildTransitStop);
      break;
    case TutorialCategory::Transit03:
      setActions(BuildTransitStop, BuildTransitStop, BuildTransitStop, BuildTransitStop);
      break;
    case TutorialCategory::Transit04:
      setActions(BuildTransitStop, BuildTransitStop, BuildTransitStop, BuildTransitStop);
      break;
    case TutorialCategory::Transit05:
      setActions(StopAddingStopsInTT, SelectedTrainsInTT);
      break;
    case TutorialCategory::Transit06:
      setActions(SelectedBuildModeInTT);
      break;
    case TutorialCategory::Transit07:
      setActions(BuildRail, BuildRail, BuildRail, BuildRail);
      break;
    case TutorialCategory::Transit08:
      setActions(ChangeElevationInTT);
      setFeatureBlink(FRoadElevation, true);
      break;
    case TutorialCategory::Transit09:
      setActions(SelectedTunnelViaductInTT);
      break;
    case TutorialCategory::Transit10:
      setActions(BuildRail, BuildRail, BuildRail, BuildRail);
      break;
    case TutorialCategory::Transit11:
      setActions(SelectedLinesModeInTT, AddedTransitLineInTT, BuildTransitStop, BuildTransitStop);
      break;
    case TutorialCategory::Transit12:
      setActions(SelectedRouteInspector, InspectedRoute);
      break;
    case TutorialCategory::Transit13:
      setActions(ChangedHeadway, ChangedTransitPrice);
      break;
    case TutorialCategory::Transit14:
      setActions(TutorialUpdateCode::WaitForTimer);
      setTimerTarget(tutorialDefaultTimer);
      break;

    case TutorialCategory::Expressways00:
      setActions(SelectedRoadTool, SelectedExpresswaysInTT);
      setFeatureBlink(FRoadExpressways, true);
      break;
    case TutorialCategory::Expressways01:
      setActions(BuildExpressway, BuildExpressway, BuildExpressway, BuildExpressway);
      break;
    case TutorialCategory::Expressways02:
      setActions(ChangeElevationInTT, BuildExpressway, BuildExpressway, BuildExpressway);
      setFeatureBlink(FRoadElevation, true);
      break;
    case TutorialCategory::Expressways03:
      setActions(SelectedPlannerModeInTT, BuildExpressway, BuildExpressway, CompletedPlansInTT);
      setFeatureBlink(FRoadPlanner, true);
      break;
    case TutorialCategory::Expressways04:
      setActions(BuildExpressway, BuildExpressway, BuildExpressway, BuildExpressway);
      break;
    case TutorialCategory::Expressways05:
      setActions(SelectedPillarsModeInTT, BuildPillar, BuildExpressway, BuildExpressway);
      break;
    case TutorialCategory::Expressways06:
      setActions(SelectedGridlessModeInTT, BuildExpressway, BuildExpressway, BuildExpressway);
      break;
    case TutorialCategory::Expressways07:
      setActions(BuildExpressway, BuildExpressway, BuildExpressway, BuildExpressway);
      break;
    case TutorialCategory::Expressways08:
      setFeatureBlink(FObserveTraffic, true);
      setActions(SelectedHeatmapTraffic);
      break;
    case TutorialCategory::Expressways09:
      setActions(TutorialUpdateCode::WaitForTimer);
      setTimerTarget(tutorialDefaultTimer);
      break;

    case TutorialCategory::Heatmaps00:
      setActions(TutorialUpdateCode::SelectedHeatmapPollution);
      setFeatureBlink(FObservePollution, true);
      break;
    case TutorialCategory::Heatmaps01:
      setActions(TutorialUpdateCode::SelectedHeatmapCrime);
      break;
    case TutorialCategory::Heatmaps02:
      setActions(TutorialUpdateCode::SelectedHeatmapEducation);
      break;
    case TutorialCategory::Heatmaps03:
      setActions(TutorialUpdateCode::SelectedHeatmapProsperity);
      break;
    case TutorialCategory::Heatmaps04:
      setActions(TutorialUpdateCode::SelectedHeatmapValue);
      break;
    case TutorialCategory::Heatmaps05:
      setActions(TutorialUpdateCode::SelectedHeatmapDensity);
      break;
    case TutorialCategory::Heatmaps06:
      setActions(TutorialUpdateCode::SelectedHeatmapCommunity);
      break;
    case TutorialCategory::Heatmaps07:
      setActions(TutorialUpdateCode::SelectedHeatmapHealth);
      break;
    case TutorialCategory::Heatmaps08:
      setActions(TutorialUpdateCode::WaitForTimer);
      setTimerTarget(tutorialDefaultTimer);
      break;
    case TutorialCategory::Heatmaps09:
      setActions(TutorialUpdateCode::SelectedHeatmapZone,
          TutorialUpdateCode::SelectedHeatmapRoad,
          TutorialUpdateCode::SelectedHeatmapTraffic,
          TutorialUpdateCode::SelectedHeatmapTransit);
      break;
    case TutorialCategory::Heatmaps10:
      setActions(TutorialUpdateCode::WaitForTimer);
      setTimerTarget(tutorialDefaultTimer);
      break;

    case TutorialCategory::Disabled:
      setCategory(TutorialCategory::TutorialMenu00);
      break;
    default:
      setActions(TutorialUpdateCode::NoAction);
      return false;
  }
  return true;
}

bool TutorialState::setActions(TutorialAction act0, TutorialAction act1, TutorialAction act2, TutorialAction act3) {
  if(_actions.capacity() < tutorialMaxActions)
    _actions.resize(tutorialMaxActions);
  
  _actions[0] = act0;
  _actions[1] = act1;
  _actions[2] = act2;
  _actions[3] = act3;

  return true;
}

bool TutorialState::setActions(TutorialUpdateCode code0, TutorialUpdateCode code1, TutorialUpdateCode code2, TutorialUpdateCode code3) {
  if(_actions.capacity() < tutorialMaxActions)
    _actions.resize(tutorialMaxActions);

  _actions[0] = TutorialAction(code0);
  _actions[1] = TutorialAction(code1);
  _actions[2] = TutorialAction(code2);
  _actions[3] = TutorialAction(code3);

  return true;
}

bool TutorialState::setLastGameSpeed(item spd) {
  _lastGameSpeed = spd;
  return true;
}

bool TutorialState::setTimerTarget(double tar) {
  //logStacktrace();
  _timerTarget = getCameraTime() + tar;
  return true;
}

void TutorialState::setTutorialActive(bool act) {
  _tutorialActive = true;
}

//void TutorialState::setTutorialCooldown(bool cool) {
  //_tutorialCooldown = cool;
//}

/*
bool TutorialState::setCurrentActionCompleted() {
  if(_actionIndex >= _actions.size()) {
    SPDLOG_WARN("Tried to set current action completed when actionIndex was out of bounds");
    return false;
  }
  
  _actions[_actionIndex].completed = true;
  _actionIndex++;
  return true;
}
*/

bool TutorialState::setActionCompleted(int index) {
  if(index < 0 || index >= _actions.size()) {
    SPDLOG_WARN("Tried to set a tutorial action complete with out-of-bounds index");
    return false;
  }

  _actions[index].completed = true;
  return true;
}

void TutorialState::setAllActionsCompleted() {
  for(int i = 0; i < _actions.size(); i++) {
    _actions[i].completed = true;
  }
}

void TutorialState::resetTimer() {
  _timerTarget = getCameraTime();
}

void TutorialState::handleActionsCompleted(float deltaTime) {
  //SPDLOG_INFO("handleActionsCompleted {} {} {} {} {}",
      //deltaTime, _tutorialOKWait, _tutorialCooldown, getCameraTime(), _timerTarget);
  // Showing the OK button
  if (_tutorialOKWait) {
    return;
  }

  if(!_tutorialCooldown) {
    _tutorialCooldown = true;
    setTimerTarget(tutorialCooldownTimer);
    return;
  }

  if (getCameraTime() < _timerTarget) {
    return;
  }

  resetTimer();
  if (_category != TutorialCategory::TutorialMenu00) {
    setLastGameSpeed(getGameSpeed());
    //setGameSpeed(0);
  }
  _tutorialCooldown = false;

  if (!c(CFastAdvanceTutorial)) {
    _tutorialOKWait = true;
    return;
  }

  for(int i = 0; i < _actions.size(); i++) {
    if (_actions[i].code == WaitForTimer) {
      _tutorialOKWait = true;
      return;
    }
  }

  stepTutorial();
}

void TutorialState::stepTutorial() {
  if(!setCategory(getNextCategoryForCategory(_category)))
    SPDLOG_ERROR("Error setting tutorial category");

  if (!setActionsForCategory(_category))
    SPDLOG_ERROR("Error setting tutorial actions");

  _tutorialOKWait = false;

  if (_category == TutorialCategory::Disabled) {
    setTutorialActive(false);
  }
}

void TutorialState::tutorialBack() {
  if(!setCategory(getPrevCategoryForCategory(_category)))
    SPDLOG_ERROR("Error setting tutorial category");

  _tutorialOKWait = true;
}

void TutorialState::tick(float deltaTime) {
  //SPDLOG_INFO("tick {} {} {} {}",
      //deltaTime, tutorialActive(), _tutorialOKWait, actionsCompleted());
  if (!tutorialActive()) {
    // Tutorial disabled, shouldn't show
    return;
  }

  if (_category == TutorialMenu00) {
    _tutorialOKWait = false;
    return;
  }

  // Already showing tutorial/handled actions completed, quick return
  if(_tutorialOKWait)
    return;

  if(actionsCompleted()) {
    handleActionsCompleted(deltaTime);
    return;
  }

  for(int i = 0; i < _actions.size(); i++) {
    TutorialUpdateCode code = _actions[i].code;

    if(code == TutorialUpdateCode::WaitForTimer) {
      // Handle wait for timer
      if (_timerTarget < getCameraTime()) {
        setActionCompleted(i);
      }

    } else if (code == TutorialUpdateCode::Population500) {
      if (getStatistic(ourCityEconNdx(), Population) >= 500) {
        setActionCompleted(i);
      }

    } else if (code == TutorialUpdateCode::ClosedBudgetPanel) {
      if (getLeftPanel() != BudgetPanel) {
        setActionCompleted(i);
      }

    } else if (code == TutorialUpdateCode::OpenScenarioEditor) {
      if (getGameMode() == ModeTest) setActionCompleted(i);
    } else if (code == TutorialUpdateCode::OpenBuildingDesigner) {
      if (getGameMode() == ModeBuildingDesigner) setActionCompleted(i);
    }
  }
}

uint8_t TutorialState::category() {
  return _category;
}

std::vector<TutorialAction> TutorialState::actions() {
  return _actions;
}

/*
TutorialAction TutorialState::currentAction() {
  if(_actionIndex >= _actions.size())
    return _actions[_actions.size()-1];
  
  return _actions[_actionIndex];
}
*/

bool TutorialState::actionsCompleted() {
  for(int i = 0; i < _actions.size(); i++)
  {
    if(_actions[i].code != TutorialUpdateCode::NoAction && !_actions[i].completed)
      return false;
  }

  return true;
}

item TutorialState::lastGameSpeed() {
  return _lastGameSpeed;
}

std::string TutorialState::waitString() {
  if (_category == TutorialMenu00) {
    return "";
  } else if (actionsCompleted()) {
    return "Step complete!";
  } else {

    for (int i = 0; i < _actions.size(); i ++) {
      TutorialAction action = _actions[i];
      if (action.completed) continue;

      switch (action.code) {
        case WaitForTimer: {
          std::string result = ".";
          float timerRemaining = _timerTarget - getCameraTime();
          for (int k = 1; k < timerRemaining*2 && k < 3; k++) {
            result += ".";
          }
          return result;
        }
        case BuildRoad: 
          if (i > 0) {
            if (i+1 >= _actions.size()) {
              return "Last Road...";
            } else {
              return "Build Another Road";
            }
          } else {
            return "Build a Road";
          }
        case BuildAvenue: return "Build an Avenue";
        case BuildBoulevard: return "Build a Boulevard";
        case BuildExpressway:
          if (i > 0) {
            if (i+1 >= _actions.size()) {
              return "Last Expressway...";
            } else {
              return "Build Another Expressway";
            }
          } else {
            return "Build an Expressway";
          }
        case BuildRail:
          if (i > 0) {
            if (i+1 >= _actions.size()) {
              return "Last Rail...";
            } else {
              return "Build Another Rail";
            }
          } else {
            return "Build a Rail";
          }
        case BuildAmenity: return "Build a School";
        case BuildTransitStop:
          if (i > 0) {
            if (i+1 >= _actions.size()) {
              return "Last Stop...";
            } else {
              return "Place Another Stop";
            }
          } else {
            return "Place a Transit Stop";
          }
        case BuildPillar: return "Build a Pillar";

  // Camera actions
        case CameraMoveUp: return "Move Camera Up [W]";
        case CameraMoveDown: return "Move Camera Down [S]";
        case CameraMoveLeft: return "Move Camera Left [A]";
        case CameraMoveRight: return "Move Camera Right [D]";

        case CameraRotateUp: return "Rotate Camera Up [Middle Click and Drag]";
        case CameraRotateDown: return "Rotate Camera Down [Middle Click and Drag]";
        case CameraRotateLeft: return "Rotate Camera Left [Middle Click and Drag]";
        case CameraRotateRight: return "Rotate Camera Right [Middle Click and Drag]";

        case CameraZoomIn: return "Zoom In [+]";
        case CameraZoomOut: return "Zoom Out [-]";

  // Query Tool
        case QueryBuilding: return "Select a Building";
        case QueryPerson: return "Select a Person";
        case QueryBusiness: return "Select a Business";
        case SelectedRouteInspector: return "Select the Route Inspector";
        case InspectedRoute: return "Inspect a Route";

  // Transportation Tool
        case SelectedRoadsInTT: return "Select Roads [E]";
        case SelectedExpresswaysInTT: return "Select Expressways [X]";
        case SelectedBusesInTT: return "Select Buses [R]";
        case SelectedTrainsInTT: return "Select NewTrack [T]";
        case SelectedBuildModeInTT: return "Select Build Mode [B]";
        case SelectedLinesModeInTT: return "Select Transit Mode [V]";
        case SelectedCutModeInTT: return "Select Cut Mode [C]";
        case SelectedPillarsModeInTT: return "Select Pillar Mode [N]";
        case SelectedTunnelViaductInTT: return "Toggle Viaduct/Tunnel [H]";
        case SelectedGridlessModeInTT: return "Toggle Gridless Mode [Tab]";
        case SelectedLinkModeInTT: return "Toggle Link Mode [U]";
        case SelectedPlannerModeInTT: return "Select Planner Mode [J]";
        case CompletedPlansInTT: return "Complete Plans [Right Bracket]";
        case ChangeElevationInTT: return "Change Elevation [Q] or [Z]";
        case AddedTransitLineInTT: return "Add Transit Line [Key Pad 4]";
        case StopAddingStopsInTT: return "Stop Adding Stops [Key Pad 5]";

  // Blueprint Tool
        case StartBlueprintCapture: return "Start Capturing a Blueprint [Left Click]";
        case FinishBlueprintCapture: return "Finish Capturing a Blueprint [Left Click]";
        case PlaceBlueprint: return "Place the Blueprint [Left Click]";
        case SaveBlueprint: return "Save the Blueprint [T]";

  // Tool selection
        case SelectedQueryTool: return "Select Query Tool [1]";
        case SelectedRoadTool: return "Select Road Tool [2]";
        case SelectedZoneTool: return "Select Zone Tool [3]";
        case SelectedAmenityTool: return "Select Amenity Tool [4]";
        case SelectedBulldozerTool: return "Select Bulldozer Tool [5]";
        case SelectedBlueprintTool: return "Select Blueprint Tool [P]";
        case ClosedTool: return "Close Tool [ESC]";

  // Heatmap selection
        case SelectedHeatmapPollution: return "Select Pollution Heatmap [F1]";
        case SelectedHeatmapCrime: return "Select Crime Heatmap [F2]";
        case SelectedHeatmapEducation: return "Select Education Heatmap [F3]";
        case SelectedHeatmapProsperity: return "Select Prosperity Heatmap [F4]";
        case SelectedHeatmapValue: return "Select Value Heatmap [F5]";
        case SelectedHeatmapDensity: return "Select Density Heatmap [F6]";
        case SelectedHeatmapCommunity: return "Select Community Heatmap [F7]";
        case SelectedHeatmapHealth: return "Select Health Heatmap [F8]";
        case SelectedHeatmapTraffic: return "Select Traffic Infoview [F9]";
        case SelectedHeatmapTransit: return "Select Transit Infoview [F10]";
        case SelectedHeatmapZone: return "Select Zone Infoview [F11]";
        case SelectedHeatmapRoad: return "Select Road Infoview [F12]";

  // Other Selection
        case SelectedChartsPanel: return "Open the Charts Panel [O]";
        case SelectedChart: return "Select a Chart";
        case SelectedAmenityScoresPanel: return "Open the Amenity Scores Panel";
        case SelectedBudgetPanel: return "Open the Budget Panel [L]";
        case SelectedSchoolInAmenityTool: return "Select School in the Amenity Tool";
        case ClosedMainMenu: return "Click Play to Continue";
        case ClosedBudgetPanel: return "Close Budget Panel [ESC]";

  // Budget
        case UpdatedBudgetTax: return "Change Property Tax by Dragging the Slider";
        case UpdatedBudgetControl: return "Change Budget Control by Dragging the Slider";
        case UpdatedBudgetLoan: return "Change Loan Term by Dragging the Slider";

  // Zoning actions
        case ZonedNoZone: return "Remove a Zone";
        case ZonedParkZone: return "Zone a Park";
        case ZonedResidential: return "Zone Some Residential";
        case ZonedMixedUse: return "Zone Some Mixed Use";
        case ZonedRetail: return "Zone Some Retail";
        case ZonedOffice: return "Zone Some Offices";
        case ZonedAgriculture: return "Zone Some Agricultural";
        case ZonedFactory: return "Zone Some Industrial";
        case ZonedGovernment: return "Invalid";

  // Other
        case Population500: return "Get a Population of 500";
        case OpenScenarioEditor: return "Open the Scenario Editor";
        case OpenBuildingDesigner: return "Open the Building Designer";
        case ChangedHeadway: return "Shorten the Daytime Headway";
        case ChangedTransitPrice: return "Reduce the Ticket Price";

        default: break;
      }
    }
  }

  return "Invalid State";
}

double TutorialState::timerTarget() {
  return _timerTarget;
}

bool TutorialState::tutorialActive() {
  return _tutorialActive;
}

bool TutorialState::tutorialCooldown() {
  return _tutorialCooldown;
}

bool TutorialState::tutorialOKWait() {
  return _tutorialOKWait;
}

void TutorialState::resetState() {
  resetTimer();
  if (!hasCompletedTutorial()) {
    setCategory(TutorialCategory::Welcome00);
  } else {
    setCategory(TutorialCategory::TutorialMenu00);
  }
  setActionsForCategory(_category);
}


//-----------------------------------------------------------------------------
// Tutorial API functions
//-----------------------------------------------------------------------------

TutorialState* getTutorialStatePtr() {
  return &tState;
}

std::string getTutorialArticleForCurrentState() {
  uint8_t cat = tState.category();
  if (cat < 0 || cat >= Num_Categories) {
    return "404";
  } else {
    return tutorialFiles[cat];
  }

  /*
  switch(cat) {
    case TutorialCategory::Welcome00:
      return "tutorial/welcome00";
    case TutorialCategory::Camera00:
      return "tutorial/camera00";
    case TutorialCategory::Camera01:
      return "tutorial/camera01";
    case TutorialCategory::Camera02:
      return "tutorial/camera02";
    case TutorialCategory::Camera03:
      return "tutorial/camera03";
    case TutorialCategory::Road00:
      return "tutorial/road00";
    case TutorialCategory::Road01:
      return "tutorial/road01";
    case TutorialCategory::Road02:
      return "tutorial/road02";
    case TutorialCategory::Road03:
      return "tutorial/road03";
    case TutorialCategory::Zone00:
      return "tutorial/zone00";
    case TutorialCategory::Zone01:
      return "tutorial/zone01";
    case TutorialCategory::Zone02:
      return "tutorial/zone02";
    case TutorialCategory::Zone03:
      return "tutorial/zone03";
    case TutorialCategory::Zone04:
      return "tutorial/zone04";
    case TutorialCategory::Amenity00:
      return "tutorial/amenity00";
    case TutorialCategory::Amenity01:
      return "tutorial/amenity01";
    case TutorialCategory::Amenity02:
      return "tutorial/amenity02";
    case TutorialCategory::Amenity03:
      return "tutorial/amenity03";
    case TutorialCategory::Amenity04:
      return "tutorial/amenity04";
    case TutorialCategory::Budget00:
      return "tutorial/budget00";
    case TutorialCategory::Budget01:
      return "tutorial/budget01";
    case TutorialCategory::Budget02:
      return "tutorial/budget02";
    case TutorialCategory::Budget02:
      return "tutorial/budget02";

      /*
    case TutorialCategory::Welcome01:
      return "tutorial/camera00";
    case TutorialCategory::Welcome02:
      return "tutorial/camera01";
    case TutorialCategory::Welcome03:
      return "tutorial/welcome03";
    case TutorialCategory::Welcome04:
      return "tutorial/welcome04";
    case TutorialCategory::Welcome05:
      return "tutorial/welcome05";
    case TutorialCategory::Welcome06:
      return "tutorial/welcome06";
    case TutorialCategory::Query00:
      return "tutorial/query00";
    case TutorialCategory::Query01:
      return "tutorial/query01";
    case TutorialCategory::Road00:
      return "tutorial/road00";
    case TutorialCategory::Road01:
      return "tutorial/road01";
    case TutorialCategory::Zone00:
      return "tutorial/zone00";
    case TutorialCategory::Zone01:
      return "tutorial/zone01";
    case TutorialCategory::Zone02:
      return "tutorial/zone02";
    case TutorialCategory::Amenity00:
      return "tutorial/amenity00";
    case TutorialCategory::Heatmaps00:
      return "tutorial/heatmaps00";
    case TutorialCategory::Budget00:
      return "tutorial/budget00";
    case TutorialCategory::BuildingDesigner00:
      return "tutorial/building-designer00";
      */
  /*
    case TutorialCategory::TutorialMenu00:
      return "tutorial/menu";
    case TutorialCategory::Disabled:
      return "tutorial/disabled";
    default:
      return "404";
  }
  */
}

TutorialCategory getNextCategoryForCategory(TutorialCategory cat) {
  //
  // Are we at the end of any of the advanced tutorials? Go back to the menu.
  if (cat == Blueprints07 || cat == Economy11 || cat == Heatmaps10 ||
      cat == Expressways09 || cat == Transit14) {
    return TutorialCategory::TutorialMenu00;

  // Are we on the menu or disabled? Don't go anywhere.
  } else if (cat == TutorialMenu00 || cat == TutorialCategory::Disabled) {
    return TutorialCategory::Disabled;

  // Special case: if there are no neighboring cities, skip the neighboring city step
  } else if (cat == Road02 && countNeighbors() == 0) {
    return Zone00;

  // Otherwise, go to the next tutorial in order.
  } else {
    return (TutorialCategory)(cat+1);
  }

  /*
  switch(cat) {
    case TutorialCategory::Welcome00:
      return TutorialCategory::Camera00;
    case TutorialCategory::Camera00:
      return TutorialCategory::Camera01;
    case TutorialCategory::Camera01:
      return TutorialCategory::Camera02;
    case TutorialCategory::Camera02:
      return TutorialCategory::Camera03;
    case TutorialCategory::Camera03:
      return TutorialCategory::Road00;
    case TutorialCategory::Road00:
      return TutorialCategory::Road01;
    case TutorialCategory::Road01:
      return TutorialCategory::Road02;
    case TutorialCategory::Road02:
      return TutorialCategory::Road03;
    case TutorialCategory::Road03:
      return TutorialCategory::Zone00;
    case TutorialCategory::Zone00:
      return TutorialCategory::Zone01;
    case TutorialCategory::Zone01:
      return TutorialCategory::Zone02;
    case TutorialCategory::Zone02:
      return TutorialCategory::Zone03;
    case TutorialCategory::Zone03:
      return TutorialCategory::Zone04;
    case TutorialCategory::Zone04:
      return TutorialCategory::Amenity00;
    case TutorialCategory::Amenity00:
      return TutorialCategory::Amenity01;
    case TutorialCategory::Amenity01:
      return TutorialCategory::Amenity02;
    case TutorialCategory::Amenity02:
      return TutorialCategory::Amenity03;
    case TutorialCategory::Amenity03:
      return TutorialCategory::Amenity04;
    case TutorialCategory::Amenity04:
      return TutorialCategory::Budget00;
    case TutorialCategory::Budget00:
      return TutorialCategory::Budget01;
    case TutorialCategory::Budget01:
      return TutorialCategory::Budget02;
    case TutorialCategory::Budget02:
      return TutorialCategory::TutorialMenu00;
    case TutorialCategory::TutorialMenu00:
      return TutorialCategory::Disabled;
    case TutorialCategory::Disabled:
      return TutorialCategory::Disabled; // Making sure we stay here
    default:
      return TutorialCategory::Welcome00;
  }
  */
}

TutorialCategory getPrevCategoryForCategory(TutorialCategory cat) {
  // Are we at the start of any of the advanced tutorials? Go back to the menu.
  if (cat == Blueprints00 || cat == Economy00 || cat == Heatmaps00 ||
      cat == Expressways00 || cat == Transit00) {
    return TutorialCategory::TutorialMenu00;

  // Are we on the menu, welcome page or disabled? Don't go anywhere.
  } else if (cat == TutorialMenu00 || cat == Welcome00 || cat == TutorialCategory::Disabled) {
    return cat;

  // Otherwise, go to the previous tutorial.
  } else {
    return (TutorialCategory)(cat-1);
  }
}

void reportTutorialUpdate(uint32_t code) {
  if(!tState.tutorialActive())
    return;

  std::vector<TutorialAction> actions = tState.actions();

  for(int i = 0; i < actions.size(); i++)
  {
    // Only complete one action at a time, in case of multiple
    // of the same action
    if(code == actions[i].code && !actions[i].completed)
    {
      tState.setActionCompleted(i);
      break;
    }
  }
}

TutorialUpdateCode getUpdateCodeForHeatmap(item heatmapNdx) {
  switch (heatmapNdx) {
    case Pollution: return SelectedHeatmapPollution;
    case Value: return SelectedHeatmapValue;
    case Density: return SelectedHeatmapDensity;
    case Crime: return SelectedHeatmapCrime;
    case Education: return SelectedHeatmapEducation;
    case Prosperity: return SelectedHeatmapProsperity;
    case CommunityHM: return SelectedHeatmapCommunity;
    case HealthHM: return SelectedHeatmapHealth;
    case TrafficHeatMap: return SelectedHeatmapTraffic;
    case TransitHeatMap: return SelectedHeatmapTransit;
    case ZoneHeatMap: return SelectedHeatmapZone;
    case RoadHeatMap: return SelectedHeatmapRoad;
    default: return NoAction;
  }
}

void reportTutorialHeatmapSelected(item ndx) {
  TutorialUpdateCode code = getUpdateCodeForHeatmap(ndx);
  reportTutorialUpdate(code);
}

bool acknowledgeTutorialInfo(Part* part, InputEvent input) {
  TutorialState* ptr = getTutorialStatePtr();
  ptr->stepTutorial();
  return true;
}

TutorialCategory getTutorialCategoryForCode(char* code) {
  for (int i = 0; i < TutorialCategory::Num_Categories; i++) {
    if (strcmpi_s(code, tutorialCodes[i]) == 0) {
      return (TutorialCategory) i;
    }
  }
  return (TutorialCategory) -1;
}

void selectTutorial(char* code) {
  if (code == 0) return;
  TutorialCategory category = getTutorialCategoryForCode(code);
  if (category == -1) {
    SPDLOG_WARN("Invalid tutorial: {}", code);
  }
  selectTutorial(category);
}

void selectTutorial(TutorialCategory category) {
  if (category < 0) return;
  TutorialState* ptr = getTutorialStatePtr();
  setTutorialPanelOpen(true);
  ptr->setTutorialActive(true);
  ptr->resetTimer();
  ptr->setCategory(category);
}

