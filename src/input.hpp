#pragma once

#include "main.hpp"
#include "line.hpp"
#include "inputEvent.hpp"

#include <string>

const std::string errKey = "errorkey";
const std::string errAct = "erroract";
const std::string errActCmd = "erroractcmd";
const std::string errActUI = "erroractui";

const std::string inputFileName = "input";
const std::string inputFileExt = ".data";

static std::string getInputFileName() {
  return inputFileName + inputFileExt;
}

// Needs an ActData defined in input.cpp,
// and a default key in input.cpp->getDefaultKeyForAction()
enum InputAction : uint32_t {
  ActNone = 0,

  // Global Group
  // Camera actions
  ActMoveUp,
  ActMoveDown,
  ActMoveLeft,
  ActMoveRight,
  ActRotateUp,
  ActRotateDown,
  ActRotateLeft,
  ActRotateRight,
  ActZoomIn,
  ActZoomOut,
  ActHideUI,

  // Panels
  ActChartsPanel,
  ActBudgetPanel,
  ActNewspaperPanel,

  // Game Speed
  ActPauseGame,
  ActSpeedRealTime,
  ActSpeedNormal,
  ActSpeedFast,
  ActSpeedTimeLapse,

  // Planner Mode
  ActPlansComplete,
  ActPlansDiscard,
  ActPlansShowWidgets,

  // Misc NewCity Actions
  ActToggleConsole,
  ActRootEscape,

  // Tool selection
  ActTool1,
  ActTool2,
  ActTool3,
  ActTool4,
  ActTool5,
  ActBlueprintTool,

  // Heatmap selection
  ActHeatmapPollution,
  ActHeatmapCrime,
  ActHeatmapEducation,
  ActHeatmapProsperity,
  ActHeatmapValue,
  ActHeatmapDensity,
  ActHeatmapCommunity,
  ActHeatmapHealth,
  ActHeatmapTraffic,
  ActHeatmapTransit,
  ActHeatmapZone,
  ActHeatmapRoad,
  // End Global Group

  // Query Tool
  ActQuerySubtool,
  ActHeatmapSubtool,
  ActRouteInspectorSubtool,
  ActLabelSubtool,

  // Transportation Tool
  ActRoadSubtool,
  ActExpresswaySubtool,
  ActBusSubtool,
  ActTrainSubtool,
  ActTTBuildMode,
  ActTTTransitRouteMode,
  ActTTCutMode,
  ActTTPillarMode,
  ActTTRaiseElevation,
  ActTTLowerElevation,
  ActTTTunnelMode,
  ActTTGridMode,
  ActTTLinkMode,
  ActTTPlannerMode,
  ActTTPreviousType,
  ActTTNextType,
  ActTTAddTransitLine,
  ActTTAddStopsToLine,
  ActTTTrussPillar,
  ActTTSuspensionPillar,

  // Zone Tool
  ActZTOverzoneMode,
  ActZTBrushLine,
  ActZTBrushSmall,
  ActZTBrushMed,
  ActZTBrushLarge,
  ActZTBrushPointer,
  ActZTNoZone,
  ActZTResidentialZone,
  ActZTRetailZone,
  ActZTFarmZone,
  ActZTGovernmentZone,
  ActZTMixedUseZone,
  ActZTOfficeZone,
  ActZTIndustrialZone,
  ActZTParkZone,

  // Amenity Tool
  ActATEducation,
  ActATRecreation,
  ActATServices,
  ActATUniversity,
  ActATPlop,

  // Bulldozer Tool
  ActBTEarthworks,
  ActBTDestroy,
  ActBTPlaceTrees,
  ActBTRaiseElevation,
  ActBTLowerElevation,
  ActBTBrushXSmall,
  ActBTBrushSmall,
  ActBTBrushLarge,
  ActBTBrushXLarge,
  ActBTSmooth,

  // Blueprint Tool
  ActBPRaiseElevation,
  ActBPLowerElevation,
  ActBPToggleCapture,
  ActBPImport,
  ActBPExport,
  ActBPSave,
  ActBPRotate,
  ActBPFlip,

  // Designer
  ActDesignDuplicate,
  ActDesignDelete,
  ActDesignGridMode,
  ActDesignHideStructures,
  ActDesignGrab,
  ActDesignSearch,
  ActDesignRoofOnly,
  ActDesignUndo,
  ActDesignRedo,
  ActDesignGable,
  ActDesignHip,
  ActDesignFlat,
  ActDesignBarrel,
  ActDesignSlant,
  ActDesignGambrel,

  // Added after hotkeys update
  ActTutorialOK,
  ActBTContinuous,

  NumActions
};

// Enum for groups of input actions
enum InputGroup : uint32_t {
  GroupNone = 0,
  GroupGlobal,
  GroupQueryTool,
  GroupRoadTool,
  GroupZoneTool,
  GroupAmenityTool,
  GroupBulldozerTool,
  GroupBlueprintTool,
  GroupDesignerTool,

  NumInputGroups
};

std::string getKeyStr(int key);
std::string getActionStr(InputAction act, bool ui);
std::string getInputGroupStr(InputGroup group);
InputGroup getInputGroupForAction(InputAction act);

struct KeyBind {
  InputAction action;
  int32_t key;
  bool active;
  bool collided;
  
  KeyBind() {
    action = InputAction::ActNone;
    key = GLFW_KEY_UNKNOWN;
    active = false;
    collided = false;
  }

  KeyBind(InputAction a, int32_t k) {
    action = a;
    key = k;
    active = false;
    collided = false;
  }

  std::string actCmdStr() {
    return getActionStr(action, false);
  }

  std::string actUIStr() {
    return getActionStr(action, true);
  }

  std::string keyStr() {
    return getKeyStr(key);
  }
};

struct ActData {
  InputAction act;
  std::string uiStr;
  std::string cmdStr;
  InputGroup group;
  bool hidden;

  ActData() {
    act = InputAction::ActNone;
    uiStr = errActUI;
    cmdStr = errActCmd;
    group = InputGroup::GroupGlobal;
    hidden = false;
  }

  ActData(InputAction a, std::string u, std::string c, InputGroup g, bool h = false) {
    act = a;
    uiStr = u;
    cmdStr = c;
    group = g;
    hidden = h;
  }
};

int32_t getFirstKeyPressed();
KeyBind* getKeymapPtr();
bool getAllowCollisions();
void setAllowCollisions(bool col);
void clearCollidedAll();
bool writeInputFile();
bool readInputFile(bool create);
KeyBind getKeyBind(int index);
std::string getKeyStr(int index);
bool isKeyValid(int32_t key);
bool isKeyStrValid(std::string str);
bool getKeyForStr(std::string str, int32_t &key);
void detectKeyCollisionsAll(bool report = true);
bool unbindAction(InputAction act, bool report = true);
bool bindKeyToAction(InputAction act, int32_t key, bool report = true);
int32_t getDefaultKeyForAction(InputAction a);
bool getWaitingForKey();
void setWaitingForKey(bool val);
void initKeymap();
void initInput();
void updateInput(double duration);
void setLockMouse(bool val);
bool getLockMouse();
void collectInput();
void initLuaInputCallbacks();
bool inputActionValid(InputAction a, InputEvent event);
bool inputActionPressed(InputAction a);
bool doesSearchStringMatch(KeyBind a, const char* str);

typedef void (*ClipboardCallback)(const char* str);
void getClipboard(ClipboardCallback c);
void sendToClipboard(char* str);

