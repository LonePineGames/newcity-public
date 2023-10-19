//-----------------------------------------------------------------------------
// tutorial - Contains the data structure and API for
// accessing and manipulating Tutorial specific state data
//-----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>

#include "item.hpp"
#include "parts/leftPanel.hpp"
#include "spdlog/spdlog.h"

const double tutorialMaxTimer = 1000000;
const double tutorialDefaultTimer = 2.0;
const double tutorialCooldownTimer = 1.0;
const int tutorialMaxActions = 4;
const int tutorialMaxStrings = 3;

// Code for a Tutorial Subject to inform an Observer of a change
enum TutorialUpdateCode : uint32_t {
  NoAction = 0,
  WaitForTimer,

  // Build actions
  BuildRoad,
  BuildAvenue,
  BuildBoulevard,
  BuildExpressway,
  BuildRail,
  BuildAmenity,
  BuildTransitStop,
  BuildPillar,

  // Camera actions
  CameraMoveUp,
  CameraMoveDown,
  CameraMoveLeft,
  CameraMoveRight,

  CameraRotateUp,
  CameraRotateDown,
  CameraRotateLeft,
  CameraRotateRight,

  CameraZoomIn,
  CameraZoomOut,

  // Tool selection
  SelectedQueryTool,
  SelectedRoadTool,
  SelectedZoneTool,
  SelectedAmenityTool,
  SelectedBulldozerTool,
  SelectedBlueprintTool,
  ClosedTool,

  // Query Tool
  QueryBuilding,
  QueryPerson,
  QueryBusiness,
  SelectedRouteInspector,
  InspectedRoute,

  // Transportation Tool
  SelectedRoadsInTT,
  SelectedExpresswaysInTT,
  SelectedBusesInTT,
  SelectedTrainsInTT,
  SelectedBuildModeInTT,
  SelectedLinesModeInTT,
  SelectedCutModeInTT,
  SelectedPillarsModeInTT,
  SelectedTunnelViaductInTT,
  SelectedGridlessModeInTT,
  SelectedLinkModeInTT,
  SelectedPlannerModeInTT,
  CompletedPlansInTT,
  ChangeElevationInTT,
  AddedTransitLineInTT,
  StopAddingStopsInTT,

  // Blueprint Tool
  StartBlueprintCapture,
  FinishBlueprintCapture,
  PlaceBlueprint,
  SaveBlueprint,

  // Heatmap and Infoview selection
  SelectedHeatmapPollution,
  SelectedHeatmapCrime,
  SelectedHeatmapEducation,
  SelectedHeatmapProsperity,
  SelectedHeatmapValue,
  SelectedHeatmapDensity,
  SelectedHeatmapCommunity,
  SelectedHeatmapHealth,
  SelectedHeatmapTraffic,
  SelectedHeatmapTransit,
  SelectedHeatmapZone,
  SelectedHeatmapRoad,

  // Other Selection
  SelectedChartsPanel,
  SelectedChart,
  SelectedAmenityScoresPanel,
  SelectedBudgetPanel,
  SelectedSchoolInAmenityTool,
  ClosedMainMenu,
  ClosedBudgetPanel,

  // Budget
  UpdatedBudgetTax,
  UpdatedBudgetControl,
  UpdatedBudgetLoan,

  // Zoning actions
  ZonedNoZone,
  ZonedParkZone,
  ZonedResidential,
  ZonedMixedUse,
  ZonedRetail,
  ZonedOffice,
  ZonedAgriculture,
  ZonedFactory,
  ZonedGovernment,

  // Other
  Population500,
  OpenScenarioEditor,
  OpenBuildingDesigner,
  ChangedHeadway,
  ChangedTransitPrice,

  NumUpdateCodes
};

enum TutorialCategory : uint8_t {

  #define TUT(N) N,
  #include "tutorialsEnum.hpp"
  #undef TUT

  Num_Categories,
  // Max_Categories = UINT8_MAX
};

/*
enum TutorialAction : uint8_t {
  NoAction = 0,
  WaitForTimer,
  
  MoveCameraUp,
  MoveCameraDown,
  MoveCameraLeft,
  MoveCameraRight,
  
  RotateCameraUp,
  RotateCameraDown,
  RotateCameraLeft,
  RotateCameraRight,
  
  ZoomCameraIn,
  ZoomCameraOut,

  ClickUIElement,
  ZoneSomething,
  BuildSomething,
  ChangeValue,

  Num_Actions,
};
*/

struct TutorialAction
{
  TutorialUpdateCode code;
  bool completed;

  TutorialAction()
  {
    code = TutorialUpdateCode::NoAction;
    completed = false;
  }

  TutorialAction(TutorialUpdateCode c)
  {
    code = c;
    completed = false;
  }

  bool operator==(const TutorialUpdateCode& c)
  {
    return code == c;
  }

  bool operator==(const TutorialAction& act)
  {
    return code = act.code;
  }
};

struct TutorialState {
private:
  TutorialCategory _category = TutorialCategory::Welcome00;
  std::vector<TutorialAction> _actions = {TutorialAction(), TutorialAction(), TutorialAction(), TutorialAction()};
  // uint8_t _actionIndex;
  item _lastGameSpeed = 0;
  double _timerTarget = 5;
  bool _tutorialActive = false;
  bool _tutorialCooldown = false;
  bool _showTutorial = false;
  bool _tutorialOKWait = false;

  // Private setters and methods
  bool setActionsForCategory(TutorialCategory cat);
  // For setting actions with actions
  bool setActions(TutorialAction act0 = TutorialAction(), TutorialAction act1 = TutorialAction(), 
    TutorialAction act2 = TutorialAction(), TutorialAction act3 = TutorialAction());
  // For setting actions by TutorialUpdateCode
  bool setActions(TutorialUpdateCode code0 = TutorialUpdateCode::NoAction, TutorialUpdateCode code1 = TutorialUpdateCode::NoAction,
    TutorialUpdateCode code2 = TutorialUpdateCode::NoAction, TutorialUpdateCode code3 = TutorialUpdateCode::NoAction);

public:
  // Setters and methods
  bool setCategory(TutorialCategory cat);
  bool setLastGameSpeed(item spd);
  bool setTimerTarget(double tar);
  void setTutorialActive(bool act);
  void setTutorialCooldown(bool cool);
  void setShowTutorial(bool show);
  // bool setCurrentActionCompleted();
  bool setActionCompleted(int index);
  void setAllActionsCompleted();
  void resetTimer();
  void handleActionsCompleted(float deltaTime);
  void handleShowTutorial();
  void resetState();
  void stepTutorial();
  void tutorialBack();
  void tick(float deltaTime);

  uint8_t category();
  std::vector<TutorialAction> actions();
  // TutorialAction currentAction();
  bool actionsCompleted();
  item lastGameSpeed();
  std::string waitString();
  // double timer();
  double timerTarget();
  bool tutorialActive();
  bool tutorialCooldown();
  bool showTutorial();
  bool tutorialOKWait();
};


//-----------------------------------------------------------------------------
// Tutorial API functions
//-----------------------------------------------------------------------------

TutorialState* getTutorialStatePtr();
// void clearTutorialStringArr();
std::string getTutorialArticleForCurrentState();
TutorialCategory getNextCategoryForCategory(TutorialCategory cat);
TutorialCategory getPrevCategoryForCategory(TutorialCategory cat);
void reportTutorialUpdate(uint32_t code);
void reportTutorialHeatmapSelected(item ndx);
bool acknowledgeTutorialInfo(Part* part, InputEvent input);
void selectTutorial(char* code);
void selectTutorial(TutorialCategory category);

