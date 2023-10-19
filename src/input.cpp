#include "input.hpp"

#include "color.hpp"
#include "cup.hpp"
#include "console/conDisplay.hpp"
#include "draw/camera.hpp"
#include "game/game.hpp"
#include "land.hpp"
#include "util.hpp"
#include "parts/toolbar.hpp"
#include "parts/tooltip.hpp"
#include "platform/binaryfilebuilder.hpp"
#include "platform/binaryfilereader.hpp"
#include "pillar.hpp"
#include "plan.hpp"
#include "thread.hpp"

#include "draw/entity.hpp"
#include "draw/texture.hpp"
#include "draw/mesh.hpp"
#include "renderUtils.hpp"

#include <atomic>
#include "spdlog/spdlog.h"


static mutex inputMutex;
InputEvent lastEvent;
Cup<InputEvent>* inputQueue = Cup<InputEvent>::newCup(100);
std::atomic<bool> grabClipboard(false);
char* sendToClipboardFront = 0;
char* sendToClipboardBack = 0;
char* clipboardContents = 0;
vector<ClipboardCallback> clipboardCallbacks;

static bool resetPos = false;   // Should we reset the mouse cursor position in collectInput()?
static bool lockMouse = false;  // Should the mouse be locked to the window?

// Key binding
static KeyBind keymap[InputAction::NumActions];
static bool waitingForKey = false;
static InputAction actionToMap = InputAction::ActNone;
static bool allowCollisions = true;

// Can use InputAction cast to int as index
static ActData actions[] = {
  ActData(ActNone, "None", "none", GroupNone),
  
  // Global Group
  // Camera actions
  ActData(ActMoveUp, "Move Fwd", "movef", GroupGlobal),
  ActData(ActMoveDown, "Move Back", "moveb", GroupGlobal),
  ActData(ActMoveLeft, "Move Left", "movel", GroupGlobal),
  ActData(ActMoveRight, "Move Right", "mover", GroupGlobal),
  ActData(ActRotateUp, "Rotate Up", "rotateu", GroupGlobal),
  ActData(ActRotateDown, "Rotate Down", "rotated", GroupGlobal),
  ActData(ActRotateLeft, "Rotate Left", "rotatel", GroupGlobal),
  ActData(ActRotateRight, "Rotate Right", "rotater", GroupGlobal),
  ActData(ActZoomIn, "Zoom In", "zoomin", GroupGlobal),
  ActData(ActZoomOut, "Zoom Out", "zoomout", GroupGlobal),
  ActData(ActHideUI, "Hide UI", "hideui", GroupGlobal),

  // Panels
  ActData(ActChartsPanel, "Charts Panel", "chartspanel", GroupGlobal),
  ActData(ActBudgetPanel, "Budget Panel", "budgetpanel", GroupGlobal),
  ActData(ActNewspaperPanel, "Newspaper Panel", "newspanel", GroupGlobal),

  // Game Speed
  ActData(ActPauseGame, "Pause Game", "pausegame", GroupGlobal),
  ActData(ActSpeedRealTime, "Game Speed 1 (Real Time)", "gamespeed1", GroupGlobal),
  ActData(ActSpeedNormal, "Game Speed 2 (Normal)", "gamespeed2", GroupGlobal),
  ActData(ActSpeedFast, "Game Speed 3 (Fast)", "gamespeed3", GroupGlobal),
  ActData(ActSpeedTimeLapse, "Game Speed 4 (Time Lapse)", "gamespeed4", GroupGlobal),

  ActData(ActPlansComplete, "Complete All Plans", "completeplans", GroupGlobal),
  ActData(ActPlansDiscard, "Discard All Plans", "discardplans", GroupGlobal),
  ActData(ActPlansShowWidgets, "Show/Hide Plan Widgets", "showwidgets", GroupGlobal),

  // Misc NewCity Actions
  ActData(ActToggleConsole, "Open/Close the Console", "console", GroupGlobal),
  ActData(ActRootEscape, "Escape/Back", "esc", GroupGlobal),

  // Tool selection
  ActData(ActTool1, "Tool Slot 1", "tool1", GroupGlobal),
  ActData(ActTool2, "Tool Slot 2", "tool2", GroupGlobal),
  ActData(ActTool3, "Tool Slot 3", "tool3", GroupGlobal),
  ActData(ActTool4, "Tool Slot 4", "tool4", GroupGlobal),
  ActData(ActTool5, "Tool Slot 5", "tool5", GroupGlobal),
  ActData(ActBlueprintTool, "Blueprint Tool", "blueprint", GroupGlobal),

  ActData(ActHeatmapPollution, "Pollution HM", "hmpollution", GroupGlobal),
  ActData(ActHeatmapCrime, "Crime HM", "hmcrime", GroupGlobal),
  ActData(ActHeatmapEducation, "Education HM", "hmeducation", GroupGlobal),
  ActData(ActHeatmapProsperity, "Prosperity HM", "hmprosperity", GroupGlobal),
  ActData(ActHeatmapValue, "Value HM", "hmvalue", GroupGlobal),
  ActData(ActHeatmapDensity, "Density HM", "hmdensity", GroupGlobal),
  ActData(ActHeatmapCommunity, "Community HM", "hmcommunity", GroupGlobal),
  ActData(ActHeatmapHealth, "Health HM", "hmhealth", GroupGlobal),
  ActData(ActHeatmapTraffic, "Traffic HM", "hmtraffic", GroupGlobal),
  ActData(ActHeatmapTransit, "Transit HM", "hmtransit", GroupGlobal),
  ActData(ActHeatmapZone, "Zone HM", "hmzone", GroupGlobal),
  ActData(ActHeatmapRoad, "Roadmap HM", "hmroad", GroupGlobal),
  // End Global Group

  // Query Tool
  ActData(ActQuerySubtool, "Query Subtool in Query Tool", "queryst", GroupQueryTool),
  ActData(ActHeatmapSubtool, "Heatmap Subtool in Query Tool", "heatmapst", GroupQueryTool),
  ActData(ActRouteInspectorSubtool, "Route Inspector Subtool in Query Tool", "routest", GroupQueryTool),
  ActData(ActLabelSubtool, "Label Subtool in Query Tool", "labelst", GroupQueryTool),

  // Transportation Tool
  ActData(ActRoadSubtool, "Build Roads in Transportation Tool", "roadst", GroupRoadTool),
  ActData(ActExpresswaySubtool, "Build Expressways in Transportation Tool", "expwyst", GroupRoadTool),
  ActData(ActBusSubtool, "Build Buses in Transportation Tool", "busst", GroupRoadTool),
  ActData(ActTrainSubtool, "Build Trains in Transportation Tool", "trainst", GroupRoadTool),
  ActData(ActTTBuildMode, "Build Infrastructure in Transportation Tool", "buildmode", GroupRoadTool),
  ActData(ActTTTransitRouteMode, "Plan Transit Routes in Transportation Tool", "transitmode", GroupRoadTool),
  ActData(ActTTCutMode, "Insert Joints in Transportation Tool", "cutmode", GroupRoadTool),
  ActData(ActTTPillarMode, "Build Bridge Pillars in Transportation Tool", "pillarmode", GroupRoadTool),
  ActData(ActTTRaiseElevation, "Raise Elevation in Transportation Tool", "ttraise", GroupRoadTool),
  ActData(ActTTLowerElevation, "Lower Elevation in Transportation Tool", "ttlower", GroupRoadTool),
  ActData(ActTTTunnelMode, "Tunnel Mode in Transportation Tool", "tunnelmode", GroupRoadTool),
  ActData(ActTTGridMode, "Snap to Grid in Transportation Tool", "ttgridmode", GroupRoadTool),
  ActData(ActTTLinkMode, "Link Mode in Transportation Tool", "ttlinkmode", GroupRoadTool),
  ActData(ActTTPlannerMode, "Planner Mode in Transportation Tool", "ttplannermode", GroupRoadTool),
  ActData(ActTTPreviousType, "Previous Road or Rail Type in Transportation Tool", "ttprevtype", GroupRoadTool),
  ActData(ActTTNextType, "Next Road or Rail Type in Transportation Tool", "ttnexttype", GroupRoadTool),
  ActData(ActTTAddTransitLine, "Add Transit Line in Transportation Tool", "ttaddline", GroupRoadTool),
  ActData(ActTTAddStopsToLine, "Add Stops to Line in Transportation Tool", "ttaddstops", GroupRoadTool),
  ActData(ActTTTrussPillar, "Build Truss Pillar in Transportation Tool", "tttrusspillar", GroupRoadTool),
  ActData(ActTTSuspensionPillar, "Build Suspension Pillar in Transportation Tool", "ttsuspensionpillar", GroupRoadTool),

  // Zone Tool
  ActData(ActZTOverzoneMode, "Overzone Mode in Zone Tool", "ztoverzone", GroupZoneTool),
  ActData(ActZTBrushLine, "Line Brush in Zone Tool", "ztlinebrush", GroupZoneTool),
  ActData(ActZTBrushLine, "Small Brush in Zone Tool", "ztsmallbrush", GroupZoneTool),
  ActData(ActZTBrushSmall, "Medium Brush in Zone Tool", "ztmedbrush", GroupZoneTool),
  ActData(ActZTBrushLarge, "Large Brush in Zone Tool", "ztlargebrush", GroupZoneTool),
  ActData(ActZTBrushPointer, "Pointer Brush in Zone Tool", "ztpointerbrush", GroupZoneTool),
  ActData(ActZTNoZone, "Remove Zone in Zone Tool", "ztremovezone", GroupZoneTool),
  ActData(ActZTResidentialZone, "Residential in Zone Tool", "ztresidential", GroupZoneTool),
  ActData(ActZTRetailZone, "Retail in Zone Tool", "ztretail", GroupZoneTool),
  ActData(ActZTFarmZone, "Farm in Zone Tool", "ztfarm", GroupZoneTool),
  ActData(ActZTGovernmentZone, "Government in Zone Tool", "ztgov", GroupZoneTool),
  ActData(ActZTMixedUseZone, "Mixed Use in Zone Tool", "ztmixeduse", GroupZoneTool),
  ActData(ActZTOfficeZone, "Offices in Zone Tool", "ztoffice", GroupZoneTool),
  ActData(ActZTIndustrialZone, "Industry in Zone Tool", "ztindustrial", GroupZoneTool),
  ActData(ActZTParkZone, "Parks in Zone Tool", "ztpark", GroupZoneTool),

  // Amenity Tool
  ActData(ActATEducation, "Education in Amenity Tool", "ateducation", GroupAmenityTool),
  ActData(ActATRecreation, "Recreation in Amenity Tool", "atrecreation", GroupAmenityTool),
  ActData(ActATServices, "Services in Amenity Tool", "atservices", GroupAmenityTool),
  ActData(ActATUniversity, "University in Amenity Tool", "atuniversity", GroupAmenityTool),
  ActData(ActATPlop, "Plop Building in Amenity Tool", "atplop", GroupAmenityTool),

  // Bulldozer Tool
  ActData(ActBTEarthworks, "Earthworks in Bulldozer Tool", "btearthworks", GroupBulldozerTool),
  ActData(ActBTDestroy, "Destroy in Bulldozer Tool", "btdestroy", GroupBulldozerTool),
  ActData(ActBTPlaceTrees, "Place Trees in Bulldozer Tool", "bttrees", GroupBulldozerTool),
  ActData(ActBTRaiseElevation, "Raise Elevation in Bulldozer Tool", "btraise", GroupBulldozerTool),
  ActData(ActBTLowerElevation, "Lower Elevation in Bulldozer Tool", "btlower", GroupBulldozerTool),
  ActData(ActBTBrushXSmall, "Extra Small Brush in Bulldozer Tool", "btbrushxsmall", GroupBulldozerTool),
  ActData(ActBTBrushSmall, "Small Brush in Bulldozer Tool", "btbrushsmall", GroupBulldozerTool),
  ActData(ActBTBrushLarge, "Large Brush in Bulldozer Tool", "btbrushlarge", GroupBulldozerTool),
  ActData(ActBTBrushXLarge, "Extra Large Brush in Bulldozer Tool", "btbrushxlarge", GroupBulldozerTool),
  ActData(ActBTSmooth, "Smooth Earthworks in Bulldozer Tool", "btsmooth", GroupBulldozerTool),

  ActData(ActBPRaiseElevation, "Raise Elevation in Blueprint Tool", "bpraise", GroupBlueprintTool),
  ActData(ActBPLowerElevation, "Lower Elevation in Blueprint Tool", "bplower", GroupBlueprintTool),
  ActData(ActBPToggleCapture, "Toggle Blueprint Capture in Blueprint Tool", "bpcapture", GroupBlueprintTool),
  ActData(ActBPImport, "Import Blueprint in Blueprint Tool", "bpimport", GroupBlueprintTool),
  ActData(ActBPExport, "Export Blueprint in Blueprint Tool", "bpexport", GroupBlueprintTool),
  ActData(ActBPSave, "Save Blueprint in Blueprint Tool", "bpsave", GroupBlueprintTool),
  ActData(ActBPRotate, "Rotate Blueprint in Blueprint Tool", "bprotate", GroupBlueprintTool),
  ActData(ActBPFlip, "Flip Blueprint in Blueprint Tool", "bpflip", GroupBlueprintTool),

  // Building Designer
  ActData(ActDesignDuplicate, "Duplicate in Building Designer", "designduplicate", GroupDesignerTool),
  ActData(ActDesignDelete, "Delete in Building Designer", "designdelete", GroupDesignerTool),
  ActData(ActDesignGridMode, "Snap to Grid in Building Designer", "designgridmode", GroupDesignerTool),
  ActData(ActDesignHideStructures, "Hide Structures in Building Designer", "designhidestructures", GroupDesignerTool),
  ActData(ActDesignGrab, "Grab Decoration in Building Designer", "designgrab", GroupDesignerTool),
  ActData(ActDesignSearch, "Search Decorations in Building Designer", "designsearch", GroupDesignerTool),
  ActData(ActDesignRoofOnly, "Roof Only in Building Designer", "designroofonly", GroupDesignerTool),
  ActData(ActDesignUndo, "Undo in Building Designer", "designundo", GroupDesignerTool),
  ActData(ActDesignRedo, "Redo in Building Designer", "designredo", GroupDesignerTool),
  ActData(ActDesignGable, "Gamble Roof in Building Designer", "designgamble", GroupDesignerTool),
  ActData(ActDesignHip, "Hip Roof in Building Designer", "designhip", GroupDesignerTool),
  ActData(ActDesignFlat, "Flat Roof in Building Designer", "designflat", GroupDesignerTool),
  ActData(ActDesignBarrel, "Barrel Roof in Building Designer", "designbarrel", GroupDesignerTool),
  ActData(ActDesignSlant, "Slant Roof in Building Designer", "designslant", GroupDesignerTool),
  ActData(ActDesignGambrel, "Gambrel Roof in Building Designer", "designgambrel", GroupDesignerTool),

  // Added after hotkeys update
  ActData(ActTutorialOK, "Advance Tutorial", "tutorialok", GroupGlobal),
  ActData(ActBTContinuous, "Earthworks: Toggle Between Raise/Lower and Level", "btcontinuous", GroupBulldozerTool),

  // Keep this as last entry for enum length hack
  ActData(NumActions, "Unk Action", errAct, GroupNone)
};

item mouseLineEntity;
item mouseLineMesh;

static const int numValidKeys = 120;
static const int validKeys[numValidKeys] = {
  GLFW_KEY_SPACE,
  GLFW_KEY_APOSTROPHE,
  GLFW_KEY_COMMA,
  GLFW_KEY_MINUS,
  GLFW_KEY_PERIOD,
  GLFW_KEY_SLASH,
  GLFW_KEY_0,
  GLFW_KEY_1,
  GLFW_KEY_2,
  GLFW_KEY_3,
  GLFW_KEY_4,
  GLFW_KEY_5,
  GLFW_KEY_6,
  GLFW_KEY_7,
  GLFW_KEY_8,
  GLFW_KEY_9,
  GLFW_KEY_SEMICOLON,
  GLFW_KEY_EQUAL,
  GLFW_KEY_A,
  GLFW_KEY_B,
  GLFW_KEY_C,
  GLFW_KEY_D,
  GLFW_KEY_E,
  GLFW_KEY_F,
  GLFW_KEY_G,
  GLFW_KEY_H,
  GLFW_KEY_I,
  GLFW_KEY_J,
  GLFW_KEY_K,
  GLFW_KEY_L,
  GLFW_KEY_M,
  GLFW_KEY_N,
  GLFW_KEY_O,
  GLFW_KEY_P,
  GLFW_KEY_Q,
  GLFW_KEY_R,
  GLFW_KEY_S,
  GLFW_KEY_T,
  GLFW_KEY_U,
  GLFW_KEY_V,
  GLFW_KEY_W,
  GLFW_KEY_X,
  GLFW_KEY_Y,
  GLFW_KEY_Z,
  GLFW_KEY_LEFT_BRACKET,
  GLFW_KEY_BACKSLASH,
  GLFW_KEY_RIGHT_BRACKET,
  GLFW_KEY_GRAVE_ACCENT,
  GLFW_KEY_WORLD_1,
  GLFW_KEY_WORLD_2,
  GLFW_KEY_ESCAPE,
  GLFW_KEY_ENTER,
  GLFW_KEY_TAB,
  GLFW_KEY_BACKSPACE,
  GLFW_KEY_INSERT,
  GLFW_KEY_DELETE,
  GLFW_KEY_RIGHT,
  GLFW_KEY_LEFT,
  GLFW_KEY_DOWN,
  GLFW_KEY_UP,
  GLFW_KEY_PAGE_UP,
  GLFW_KEY_PAGE_DOWN,
  GLFW_KEY_HOME,
  GLFW_KEY_END,
  GLFW_KEY_CAPS_LOCK,
  GLFW_KEY_SCROLL_LOCK,
  GLFW_KEY_NUM_LOCK,
  GLFW_KEY_PRINT_SCREEN,
  GLFW_KEY_PAUSE,
  GLFW_KEY_F1,
  GLFW_KEY_F2,
  GLFW_KEY_F3,
  GLFW_KEY_F4,
  GLFW_KEY_F5,
  GLFW_KEY_F6,
  GLFW_KEY_F7,
  GLFW_KEY_F8,
  GLFW_KEY_F9,
  GLFW_KEY_F10,
  GLFW_KEY_F11,
  GLFW_KEY_F12,
  GLFW_KEY_F13,
  GLFW_KEY_F14,
  GLFW_KEY_F15,
  GLFW_KEY_F16,
  GLFW_KEY_F17,
  GLFW_KEY_F18,
  GLFW_KEY_F19,
  GLFW_KEY_F20,
  GLFW_KEY_F21,
  GLFW_KEY_F22,
  GLFW_KEY_F23,
  GLFW_KEY_F24,
  GLFW_KEY_F25,
  GLFW_KEY_KP_0,
  GLFW_KEY_KP_1,
  GLFW_KEY_KP_2,
  GLFW_KEY_KP_3,
  GLFW_KEY_KP_4,
  GLFW_KEY_KP_5,
  GLFW_KEY_KP_6,
  GLFW_KEY_KP_7,
  GLFW_KEY_KP_8,
  GLFW_KEY_KP_9,
  GLFW_KEY_KP_DECIMAL,
  GLFW_KEY_KP_DIVIDE,
  GLFW_KEY_KP_MULTIPLY,
  GLFW_KEY_KP_SUBTRACT,
  GLFW_KEY_KP_ADD,
  GLFW_KEY_KP_ENTER,
  GLFW_KEY_KP_EQUAL,
  GLFW_KEY_LEFT_SHIFT,
  GLFW_KEY_LEFT_CONTROL,
  GLFW_KEY_LEFT_ALT,
  GLFW_KEY_LEFT_SUPER,
  GLFW_KEY_RIGHT_SHIFT,
  GLFW_KEY_RIGHT_CONTROL,
  GLFW_KEY_RIGHT_ALT,
  GLFW_KEY_RIGHT_SUPER,
  GLFW_KEY_MENU
};

int32_t getFirstKeyPressed() {
  for (int i = 0; i < numValidKeys; i++) {
    int key = validKeys[i];
    if (lastEvent.isKeyDown[key])
      return key;
  }

  return GLFW_KEY_UNKNOWN;
}

bool getAllowCollisions() {
  return allowCollisions;
}

void setAllowCollisions(bool col) {
  allowCollisions = col;
  detectKeyCollisionsAll(); // Refresh key collisions when changing
}

void clearCollidedAll() {
  // Start at 1, because index 0 is ActNone
  for (int i = 1; i < (int)InputAction::NumActions; i++) {
    keymap[i].collided = false;
  }
}

bool runLuaInputHandlers(InputEvent event);

void handleEvent(InputEvent event) {
  if (runLuaInputHandlers(event)) {
    // no-op

  } else if (acceptInput(event)) {
    tooltip_onInput(event);

  } else if (event.action == GLFW_PRESS && handlePlanClick(event.mouseLine)) {
    // no-op

  } else if (event.action == SCROLL) {
    scrollCamera(lastEvent);

  } else if (event.action == MOUSE_MOVED_INPUT) {
    if (handlePlanMouseMove(lastEvent.mouseLine)) {
      // no-op

    } else {
      tool_mouse_move_callback(lastEvent);
    }

  } else if (event.isMouse) {
    tool_mouse_button_callback(event);
  }

}

void characterEvent(GLFWwindow* window, unsigned int unicode) {
  lastEvent.unicode = unicode;
  lastEvent.action = UNICODE_INPUT;
  lastEvent.isMouse = false;

  inputQueue->push_back(lastEvent);
}

void keyEvent(GLFWwindow* window, int key, int scancode, int action,
  int mods) {

  lastEvent.key = key;
  lastEvent.scancode = scancode;
  lastEvent.action = action;
  lastEvent.mods = mods;
  lastEvent.isMouse = false;

  inputQueue->push_back(lastEvent);
}

void mouseButtonEvent(GLFWwindow* window, int button, int action,
  int mods) {

  lastEvent.button = button;
  lastEvent.action = action;
  lastEvent.mods = mods;
  lastEvent.isMouse = true;

  for (int i=0; i < 3; i++) {
    lastEvent.isButtonDown[i] =
      glfwGetMouseButton(window, mouseButtonToGLFW[i]) == GLFW_PRESS;
  }

  inputQueue->push_back(lastEvent);
}

void scrollEvent(GLFWwindow* window, double xoffset, double yoffset) {
  lastEvent.scroll = vec2(xoffset, yoffset);
  lastEvent.action = SCROLL;
  lastEvent.isMouse = true;

  inputQueue->push_back(lastEvent);
}

void mouseMoved(vec2 mouseLoc, Line mouseL) {
  lastEvent.cameraSpaceMouseLoc = vec2(mouseLoc);
  lastEvent.mouseLoc = transformMouseLoc(vec2(mouseLoc));
  lastEvent.action = MOUSE_MOVED_INPUT;
  lastEvent.mouseLine = mouseL;
  lastEvent.isMouse = true;

  inputQueue->push_back(lastEvent);
}

void getClipboard(ClipboardCallback c) {
  clipboardCallbacks.push_back(c);
  grabClipboard = true;
}

void sendToClipboard(char* str) {
  if (sendToClipboardFront != 0) {
    free(sendToClipboardFront);
  }
  sendToClipboardFront = str;
}

KeyBind* getKeymapPtr() {
  return keymap;
}

bool writeInputFile() {
  std::string filename = getInputFileName();

  if (filename.length() == 0) {
    SPDLOG_ERROR("Null length filename in writeInputFile()");
    return false;
  }

  uint32_t numActions = InputAction::NumActions;

  SPDLOG_INFO("Writing input data to file {}...", filename);

  // Populate FileSector data structure with keybinds
  FileSector<KeyBind> kbSector(true, FileDataType::DataKeyBind, (ubyte)InputAction::NumActions);
  int i = 0;
  for (i; i < numActions; i++) {
    KeyBind bind = getKeyBind(i);
    if (!kbSector.push(bind)) {
      SPDLOG_ERROR("Error pushing data to sector - i:{}", i);
      break;
    }
  }

  // Store allowCollisions in the Header as FreeByteA
  if (BinaryFileBuilder::getVerbose()) {
    SPDLOG_INFO("Writing KeyBind collisions allowed: {}", (ubyte)allowCollisions);
  }
  kbSector.getHeader()->operator[]((int32_t)HeaderByte::FreeByteA) = (ubyte)allowCollisions;

  if (!BinaryFileBuilder::startWrite(filename)) {
    SPDLOG_ERROR("Error starting write process for file {}", filename);
    BinaryFileBuilder::reset();
    return false;
  }

  if (!BinaryFileBuilder::writeSectorKeyBind(&kbSector)) {
    SPDLOG_ERROR("Error writing data to file {}", filename);
    BinaryFileBuilder::reset();
    return false;
  }

  if (!BinaryFileBuilder::finishWrite()) {
    SPDLOG_ERROR("Error finishing write process for file {}", filename);
    BinaryFileBuilder::reset();
    return false;
  }

  SPDLOG_INFO("Input data successfully written to {}!", filename);
  BinaryFileBuilder::reset();
  return true;
}

// Reads the input file and sets state accordingly
// Has a boolean arg if the file can't be found and should be created
bool readInputFile(bool create) {
  std::string filename = getInputFileName();

  if (filename.length() == 0) {
    SPDLOG_ERROR("Null length filename in readInputFile()");
    return false;
  }

  // If the file doesn't exist
  if (!fileExists(filename)) {
    if (!create) {
      // If it shouldn't be created
      SPDLOG_ERROR("Input data file does not exist");
      return false;
    }

    SPDLOG_INFO("Input data file does not exist; creating...");
    if (!writeInputFile()) {
      SPDLOG_ERROR("Could not create input data file");
      return false;
    }
    return true; // Would be redundant to read it in here, so early return as success
  }

  SPDLOG_INFO("Reading input data from file {}", filename);

  if (!BinaryFileReader::startRead(filename)) {
    SPDLOG_ERROR("Error starting read process for file {}", filename);
    BinaryFileReader::reset();
    return false;
  }

  // Read keybinds into a sector data structure
  FileSector<KeyBind> sector;
  if (!BinaryFileReader::readSectorKeyBind(sector)) {
    SPDLOG_ERROR("Error reading data from file {}", filename);
    BinaryFileReader::reset();
    return false;
  }

  if (sector.size() <= 0) {
    SPDLOG_ERROR("Error parsing input data; sector size is invalid");
    BinaryFileReader::reset();
    return false;
  }

  if (sector.count() <= 0) {
    SPDLOG_ERROR("Error parsing input data; no data was found");
    BinaryFileReader::reset();
    return false;
  }

  // Retrieve allowCollisions from the Header as FreeByteA
  if (BinaryFileReader::getVerbose()) {
    SPDLOG_INFO("Read KeyBind collisions allowed: {}", sector.getHeader()->operator[]((int32_t)HeaderByte::FreeByteA));
  }
  allowCollisions = sector.getHeader()->operator[]((int32_t)HeaderByte::FreeByteA) > 0;

  bool report = false; // Default to not reporting to cut down on log noise when initializing
  if (sector.size() != InputAction::NumActions) {
    SPDLOG_WARN("Input data read from file has different length than NumActions in code");
    report = true;
  }

  // Apply input data to keymap
  for (int i = 0; i < sector.size(); i++) {
    KeyBind* bind = sector.pop();
    if (bind == NULL) {
      SPDLOG_WARN("Null keybind in input data at index {}", i);
      continue;
    }
    bindKeyToAction(bind->action, bind->key, report);
  }

  // Check for any key collisions
  detectKeyCollisionsAll();

  SPDLOG_INFO("Input data successfully read from {}!", filename);
  BinaryFileReader::reset();
  return true;
}

KeyBind getKeyBind(int index) {
  if (index < 0 || index >= (int)InputAction::NumActions) {
    return KeyBind();
  }

  if (keymap == 0) {
    return KeyBind();
  }

  return keymap[index];
}

// Gets a string for a GLFW Key
std::string getKeyStr(int key) {
  switch (key) {
    case GLFW_KEY_UNKNOWN:
      return "???";
    case GLFW_KEY_SPACE:
      return "Space";
    case GLFW_KEY_APOSTROPHE:
      return "\'";
    case GLFW_KEY_COMMA:
      return ",";
    case GLFW_KEY_MINUS:
      return "-";
    case GLFW_KEY_PERIOD:
      return ".";
    case GLFW_KEY_SLASH:
      return "/";
    case GLFW_KEY_0:
      return "0";
    case GLFW_KEY_1:
      return "1";
    case GLFW_KEY_2:
      return "2";
    case GLFW_KEY_3:
      return "3";
    case GLFW_KEY_4:
      return "4";
    case GLFW_KEY_5:
      return "5";
    case GLFW_KEY_6:
      return "6";
    case GLFW_KEY_7:
      return "7";
    case GLFW_KEY_8:
      return "8";
    case GLFW_KEY_9:
      return "9";
    case GLFW_KEY_SEMICOLON:
      return ";";
    case GLFW_KEY_EQUAL:
      return "=";
    case GLFW_KEY_A:
      return "A";
    case GLFW_KEY_B:
      return "B";
    case GLFW_KEY_C:
      return "C";
    case GLFW_KEY_D:
      return "D";
    case GLFW_KEY_E:
      return "E";
    case GLFW_KEY_F:
      return "F";
    case GLFW_KEY_G:
      return "G";
    case GLFW_KEY_H:
      return "H";
    case GLFW_KEY_I:
      return "I";
    case GLFW_KEY_J:
      return "J";
    case GLFW_KEY_K:
      return "K";
    case GLFW_KEY_L:
      return "L";
    case GLFW_KEY_M:
      return "M";
    case GLFW_KEY_N:
      return "N";
    case GLFW_KEY_O:
      return "O";
    case GLFW_KEY_P:
      return "P";
    case GLFW_KEY_Q:
      return "Q";
    case GLFW_KEY_R:
      return "R";
    case GLFW_KEY_S:
      return "S";
    case GLFW_KEY_T:
      return "T";
    case GLFW_KEY_U:
      return "U";
    case GLFW_KEY_V:
      return "V";
    case GLFW_KEY_W:
      return "W";
    case GLFW_KEY_X:
      return "X";
    case GLFW_KEY_Y:
      return "Y";
    case GLFW_KEY_Z:
      return "Z";
    case GLFW_KEY_LEFT_BRACKET:
      return "[";
    case GLFW_KEY_BACKSLASH:
      return "\\";
    case GLFW_KEY_RIGHT_BRACKET:
      return "]";
    case GLFW_KEY_GRAVE_ACCENT:
      return "`";
    case GLFW_KEY_WORLD_1:
      return "#1";
    case GLFW_KEY_WORLD_2:
      return "#2";
    case GLFW_KEY_ESCAPE:
      return "Esc";
    case GLFW_KEY_ENTER:
      return "Enter";
    case GLFW_KEY_TAB:
      return "Tab";
    case GLFW_KEY_BACKSPACE:
      return "Back";
    case GLFW_KEY_INSERT:
      return "Ins";
    case GLFW_KEY_DELETE:
      return "Del";
    case GLFW_KEY_RIGHT:
      return "Right";
    case GLFW_KEY_LEFT:
      return "Left";
    case GLFW_KEY_DOWN:
      return "Down";
    case GLFW_KEY_UP:
      return "Up";
    case GLFW_KEY_PAGE_UP:
      return "PGUP";
    case GLFW_KEY_PAGE_DOWN:
      return "PGDN";
    case GLFW_KEY_HOME:
      return "Home";
    case GLFW_KEY_END:
      return "End";
    case GLFW_KEY_CAPS_LOCK:
      return "Capslock";
    case GLFW_KEY_SCROLL_LOCK:
      return "Scrllock";
    case GLFW_KEY_NUM_LOCK:
      return "Numlock";
    case GLFW_KEY_PRINT_SCREEN:
      return "Prtscn";
    case GLFW_KEY_PAUSE:
      return "Pause";
    case GLFW_KEY_F1:
      return "F1";
    case GLFW_KEY_F2:
      return "F2";
    case GLFW_KEY_F3:
      return "F3";
    case GLFW_KEY_F4:
      return "F4";
    case GLFW_KEY_F5:
      return "F5";
    case GLFW_KEY_F6:
      return "F6";
    case GLFW_KEY_F7:
      return "F7";
    case GLFW_KEY_F8:
      return "F8";
    case GLFW_KEY_F9:
      return "F9";
    case GLFW_KEY_F10:
      return "F10";
    case GLFW_KEY_F11:
      return "F11";
    case GLFW_KEY_F12:
      return "F12";
    case GLFW_KEY_F13:
      return "F13";
    case GLFW_KEY_F14:
      return "F14";
    case GLFW_KEY_F15:
      return "F15";
    case GLFW_KEY_F16:
      return "F16";
    case GLFW_KEY_F17:
      return "F17";
    case GLFW_KEY_F18:
      return "F18";
    case GLFW_KEY_F19:
      return "F19";
    case GLFW_KEY_F20:
      return "F20";
    case GLFW_KEY_F21:
      return "F21";
    case GLFW_KEY_F22:
      return "F22";
    case GLFW_KEY_F23:
      return "F23";
    case GLFW_KEY_F24:
      return "F24";
    case GLFW_KEY_F25:
      return "F25";
    case GLFW_KEY_KP_0:
      return "KP0";
    case GLFW_KEY_KP_1:
      return "KP1";
    case GLFW_KEY_KP_2:
      return "KP2";
    case GLFW_KEY_KP_3:
      return "KP3";
    case GLFW_KEY_KP_4:
      return "KP4";
    case GLFW_KEY_KP_5:
      return "KP5";
    case GLFW_KEY_KP_6:
      return "KP6";
    case GLFW_KEY_KP_7:
      return "KP7";
    case GLFW_KEY_KP_8:
      return "KP8";
    case GLFW_KEY_KP_9:
      return "KP9";
    case GLFW_KEY_KP_DECIMAL:
      return "KP.";
    case GLFW_KEY_KP_DIVIDE:
      return "KP/";
    case GLFW_KEY_KP_MULTIPLY:
      return "KP*";
    case GLFW_KEY_KP_SUBTRACT:
      return "KP-";
    case GLFW_KEY_KP_ADD:
      return "KP+";
    case GLFW_KEY_KP_ENTER:
      return "KPEnter";
    case GLFW_KEY_KP_EQUAL:
      return "KP=";
    case GLFW_KEY_LEFT_SHIFT:
      return "LShift";
    case GLFW_KEY_LEFT_CONTROL:
      return "LCtrl";
    case GLFW_KEY_LEFT_ALT:
      return "LAlt";
    case GLFW_KEY_LEFT_SUPER:
      return "LSys";
    case GLFW_KEY_RIGHT_SHIFT:
      return "RShift";
    case GLFW_KEY_RIGHT_CONTROL:
      return "RCtrl";
    case GLFW_KEY_RIGHT_ALT:
      return "RAlt";
    case GLFW_KEY_RIGHT_SUPER:
      return "RSys";
    case GLFW_KEY_MENU:
      return "RMenu";
    default:
      return errKey;
  }
}

std::string getInputGroupStr(InputGroup group) {
  switch (group) {
    case InputGroup::GroupNone:
      return "None";
    case InputGroup::GroupGlobal:
      return "Global";
    case InputGroup::GroupQueryTool:
      return "Query Tool";
    case InputGroup::GroupRoadTool:
      return "Road Tool";
    case InputGroup::GroupZoneTool:
      return "Zone Tool";
    case InputGroup::GroupAmenityTool: 
      return "Amenity Tool";
    case InputGroup::GroupBulldozerTool:
      return "Bulldozer Tool";
    case InputGroup::GroupDesignerTool:
      return "Building Designer";
    default:
      return "Unknown Group";
  }
}

bool isKeyValid(int32_t key) {
  // Kinda sorta a valid key
  if (key == GLFW_KEY_UNKNOWN) {
    return true;
  }

  for (int i = 0; i < numValidKeys; i++) {
    if (validKeys[i] == key) return true;
  }

  return false;
}

bool isKeyStrValid(std::string str) {
  for (int i = 0; i < numValidKeys; i++) {
    if (iequals(getKeyStr(validKeys[i]), str)) return true;
  }

  return false;
}

bool getKeyForStr(std::string str, int32_t &key) {
  for (int i = 0; i < numValidKeys; i++) {
    if (iequals(getKeyStr(validKeys[i]), str)) {
      key = validKeys[i];
      return true;
    }
  }

  key = GLFW_KEY_UNKNOWN;
  return false;
}

bool doesSearchStringMatch(KeyBind a, const char* str) {
  ActData data = actions[a.action];
  if (stringContainsCaseInsensitive(data.uiStr, str)) return true;
  if (stringContainsCaseInsensitive(data.cmdStr, str)) return true;
  if (stringContainsCaseInsensitive(getKeyStr(a.key), str)) return true;
  return false;
}

bool detectKeyCollisions(InputAction act, int32_t key, InputGroup g, bool report) {
  if (key == GLFW_KEY_UNKNOWN) {
    return false;
  }

  if (g == GroupNone) {
    return false;
  }

  bool collision = false;
  for (int i = 1; i < (int)InputAction::NumActions; i++) {
    if ((int)act == i) continue;
    if (g != GroupGlobal && actions[i].group != GroupGlobal && actions[i].group != g) continue;

    // Use InputAction as index for both keymap and actions
    if (keymap[i].key != GLFW_KEY_UNKNOWN && keymap[i].key == key) {
      collision = true;
      keymap[i].collided = true;

      // Report if desired
      if (report) {
        SPDLOG_INFO("Collisions detected for key {} between actions {} and {}", getKeyStr(key), keymap[(int)act].actUIStr(), keymap[i].actUIStr());
      }

      // If we aren't allowing collisions, unbind the key(s)
      if (!allowCollisions) {
        unbindAction(keymap[i].action, report);
      }
    }
  }

  return collision;
}

void detectKeyCollisionsAll(bool report) {
  for (int i = 0; i < (int)InputAction::NumActions; i++) {
    keymap[i].collided = detectKeyCollisions(keymap[i].action, keymap[i].key, actions[i].group, report);
  }
}

bool unbindAction(InputAction act, bool report) {
  if (act < 0 || act >= InputAction::NumActions) {
    SPDLOG_ERROR("Attempted to unbind out-of-bounds action {}", act);
    return false;
  }

  // Silently ignore if a null action
  if (act == InputAction::ActNone) {
    return false;
  }

  int32_t prev = keymap[(int)act].key;
  keymap[(int)act].key = GLFW_KEY_UNKNOWN;

  if (report) {
    SPDLOG_INFO("Unbound action {} from key {}", keymap[(int)act].actCmdStr(), getKeyStr(prev));
  }

  return true;
}

bool bindKeyToAction(InputAction act, int32_t key, bool report) {
  if (act < 0 || act >= InputAction::NumActions) {
    SPDLOG_ERROR("Attempted to bind key {} to out-of-bounds action {}", getKeyStr(key), act);
    return false;
  }

  // Silently ignore if a null action
  if (act == InputAction::ActNone) {
    return false;
  }

  if (!isKeyValid(key)) {
    SPDLOG_ERROR("Attempted to bind invalid key {} to action {}", key, act);
    return false;
  }

  /*
  if (keymap[(int)act].key == key) {
    return true; // Key already assigned to action
  }
  */

  keymap[(int)act].key = key;
  keymap[(int)act].collided = detectKeyCollisions(act, key, actions[(int)act].group, report);

  if (report) {
    SPDLOG_INFO("Bound key {} to action {}", keymap[(int)act].keyStr(), keymap[(int)act].actCmdStr());
  }
  
  return true;
}

// Get a string for an action 
// ui arg specifies whether UI readable (true) or the command form (false)
std::string getActionStr(InputAction act, bool ui) {
  if (act < 0 || act >= InputAction::NumActions) {
    SPDLOG_ERROR("Index out of bounds in input::getActionStr() ({})", (int32_t)act);
    return ui ? actions[(int)InputAction::NumActions].uiStr : actions[(int)InputAction::NumActions].cmdStr;
  }

  return ui ? actions[act].uiStr : actions[act].cmdStr;
}

bool inputActionValid(InputAction a, InputEvent event) {
  if (event.isMouse) return false;
  if (event.action != GLFW_PRESS) return false;
  return event.key == getKeyBind(a).key;
}

bool inputActionPressed(InputAction a) {
  return a != ActNone && !consoleIsOpen() && keymap[a].key != GLFW_KEY_UNKNOWN && keymap[a].active;
}

InputGroup getInputGroupForAction(InputAction act) {
  if (act < 0 || act >= InputAction::NumActions) {
    SPDLOG_ERROR("Index out of bounds in input::getInputGroupForAction() ({})", (int32_t)act);
    return InputGroup::GroupNone;
  }

  return actions[(int)act].group;
}

int32_t getDefaultKeyForAction(InputAction a) {
  switch (a) {

    // Global Group
    // Camera actions
    case InputAction::ActMoveUp:
      return GLFW_KEY_W;
    case InputAction::ActMoveDown:
      return GLFW_KEY_S;
    case InputAction::ActMoveLeft:
      return GLFW_KEY_A;
    case InputAction::ActMoveRight:
      return GLFW_KEY_D;
    case InputAction::ActRotateUp:
      return GLFW_KEY_UP;
    case InputAction::ActRotateDown:
      return GLFW_KEY_DOWN;
    case InputAction::ActRotateLeft:
      return GLFW_KEY_COMMA;
    case InputAction::ActRotateRight:
      return GLFW_KEY_PERIOD;
    case InputAction::ActZoomIn:
      return GLFW_KEY_EQUAL;
    case InputAction::ActZoomOut:
      return GLFW_KEY_MINUS;
    case InputAction::ActHideUI:
      return GLFW_KEY_I;

    // Panels
    case InputAction::ActChartsPanel:
      return GLFW_KEY_O;
    case InputAction::ActBudgetPanel:
      return GLFW_KEY_L;
    case InputAction::ActNewspaperPanel:
      return GLFW_KEY_LEFT_BRACKET;

    // Game Speed
    case InputAction::ActPauseGame:
      return GLFW_KEY_SPACE;
    case InputAction::ActSpeedRealTime:
      return GLFW_KEY_KP_0;
    case InputAction::ActSpeedNormal:
      return GLFW_KEY_KP_1;
    case InputAction::ActSpeedFast:
      return GLFW_KEY_KP_2;
    case InputAction::ActSpeedTimeLapse:
      return GLFW_KEY_KP_3;

    // Planner Mode
    case InputAction::ActPlansComplete:
      return GLFW_KEY_RIGHT_BRACKET;
    case InputAction::ActPlansDiscard:
      return GLFW_KEY_SEMICOLON;
    case InputAction::ActPlansShowWidgets:
      return GLFW_KEY_APOSTROPHE;

    // Misc NewCity Actions
    case InputAction::ActToggleConsole:
      return GLFW_KEY_GRAVE_ACCENT;
    case InputAction::ActRootEscape:
      return GLFW_KEY_ESCAPE;

    // Tool Action
    case InputAction::ActTool1:
      return GLFW_KEY_1;
    case InputAction::ActTool2:
      return GLFW_KEY_2;
    case InputAction::ActTool3:
      return GLFW_KEY_3;
    case InputAction::ActTool4:
      return GLFW_KEY_4;
    case InputAction::ActTool5:
      return GLFW_KEY_5;
    case InputAction::ActBlueprintTool:
      return GLFW_KEY_P;

    // Heatmap Action
    case InputAction::ActHeatmapPollution:
      return GLFW_KEY_F1;
    case InputAction::ActHeatmapCrime:
      return GLFW_KEY_F2;
    case InputAction::ActHeatmapEducation:
      return GLFW_KEY_F3;
    case InputAction::ActHeatmapProsperity:
      return GLFW_KEY_F4;
    case InputAction::ActHeatmapValue:
      return GLFW_KEY_F5;
    case InputAction::ActHeatmapDensity:
      return GLFW_KEY_F6;
    case InputAction::ActHeatmapCommunity:
      return GLFW_KEY_F7;
    case InputAction::ActHeatmapHealth:
      return GLFW_KEY_F8;
    case InputAction::ActHeatmapTraffic:
      return GLFW_KEY_F9;
    case InputAction::ActHeatmapTransit:
      return GLFW_KEY_F10;
    case InputAction::ActHeatmapZone:
      return GLFW_KEY_F11;
    case InputAction::ActHeatmapRoad:
      return GLFW_KEY_F12;
    // End Global Group

    // Query Tool
    case InputAction::ActQuerySubtool:
      return GLFW_KEY_Q;
    case InputAction::ActHeatmapSubtool:
      return GLFW_KEY_E;
    case InputAction::ActRouteInspectorSubtool:
      return GLFW_KEY_R;
    case InputAction::ActLabelSubtool:
      return GLFW_KEY_T;

    // Transportation Tool
    case InputAction::ActRoadSubtool:
      return GLFW_KEY_E;
    case InputAction::ActExpresswaySubtool:
      return GLFW_KEY_X;
    case InputAction::ActBusSubtool:
      return GLFW_KEY_R;
    case InputAction::ActTrainSubtool:
      return GLFW_KEY_T;
    case InputAction::ActTTBuildMode:
      return GLFW_KEY_B;
    case InputAction::ActTTTransitRouteMode:
      return GLFW_KEY_V;
    case InputAction::ActTTCutMode:
      return GLFW_KEY_C;
    case InputAction::ActTTPillarMode:
      return GLFW_KEY_N;
    case InputAction::ActTTRaiseElevation:
      return GLFW_KEY_Q;
    case InputAction::ActTTLowerElevation:
      return GLFW_KEY_Z;
    case InputAction::ActTTTunnelMode:
      return GLFW_KEY_H;
    case InputAction::ActTTGridMode:
      return GLFW_KEY_TAB;
    case InputAction::ActTTLinkMode:
      return GLFW_KEY_U;
    case InputAction::ActTTPlannerMode:
      return GLFW_KEY_J;
    case InputAction::ActTTPreviousType:
      return GLFW_KEY_F;
    case InputAction::ActTTNextType:
      return GLFW_KEY_G;
    case InputAction::ActTTAddTransitLine:
      return GLFW_KEY_KP_4;
    case InputAction::ActTTAddStopsToLine:
      return GLFW_KEY_KP_5;
    case InputAction::ActTTTrussPillar:
      return GLFW_KEY_KP_7;
    case InputAction::ActTTSuspensionPillar:
      return GLFW_KEY_KP_8;

    // Zone Tool
    case InputAction::ActZTOverzoneMode:
      return GLFW_KEY_TAB;
    case InputAction::ActZTBrushLine:
      return GLFW_KEY_Z;
    case InputAction::ActZTBrushSmall:
      return GLFW_KEY_X;
    case InputAction::ActZTBrushMed:
      return GLFW_KEY_C;
    case InputAction::ActZTBrushLarge:
      return GLFW_KEY_V;
    case InputAction::ActZTBrushPointer:
      return GLFW_KEY_Y;
    case InputAction::ActZTNoZone:
      return GLFW_KEY_Q;
    case InputAction::ActZTResidentialZone:
      return GLFW_KEY_E;
    case InputAction::ActZTRetailZone:
      return GLFW_KEY_F;
    case InputAction::ActZTFarmZone:
      return GLFW_KEY_G;
    case InputAction::ActZTGovernmentZone:
      return GLFW_KEY_U;
    case InputAction::ActZTMixedUseZone:
      return GLFW_KEY_R;
    case InputAction::ActZTOfficeZone:
      return GLFW_KEY_T;
    case InputAction::ActZTIndustrialZone:
      return GLFW_KEY_H;
    case InputAction::ActZTParkZone:
      return GLFW_KEY_B;

    // Amenity Tool
    case InputAction::ActATEducation:
      return GLFW_KEY_E;
    case InputAction::ActATRecreation:
      return GLFW_KEY_R;
    case InputAction::ActATServices:
      return GLFW_KEY_T;
    case InputAction::ActATUniversity:
      return GLFW_KEY_Y;
    case InputAction::ActATPlop:
      return GLFW_KEY_H;

    // Bulldozer Tool
    case InputAction::ActBTEarthworks:
      return GLFW_KEY_E;
    case InputAction::ActBTDestroy:
      return GLFW_KEY_R;
    case InputAction::ActBTPlaceTrees:
      return GLFW_KEY_T;
    case InputAction::ActBTRaiseElevation:
      return GLFW_KEY_Q;
    case InputAction::ActBTLowerElevation:
      return GLFW_KEY_Z;
    case InputAction::ActBTBrushXSmall:
      return GLFW_KEY_F;
    case InputAction::ActBTBrushSmall:
      return GLFW_KEY_G;
    case InputAction::ActBTBrushLarge:
      return GLFW_KEY_C;
    case InputAction::ActBTBrushXLarge:
      return GLFW_KEY_V;
    case InputAction::ActBTSmooth:
      return GLFW_KEY_X;

    case InputAction::ActBPRaiseElevation:
      return GLFW_KEY_Q;
    case InputAction::ActBPLowerElevation:
      return GLFW_KEY_Z;
    case InputAction::ActBPToggleCapture:
      return GLFW_KEY_E;
    case InputAction::ActBPImport:
      return GLFW_KEY_G;
    case InputAction::ActBPExport:
      return GLFW_KEY_H;
    case InputAction::ActBPSave:
      return GLFW_KEY_T;
    case InputAction::ActBPRotate:
      return GLFW_KEY_R;
    case InputAction::ActBPFlip:
      return GLFW_KEY_F;

    // Designer
    case InputAction::ActDesignDuplicate:
      return GLFW_KEY_C;
    case InputAction::ActDesignDelete:
      return GLFW_KEY_DELETE;
    case InputAction::ActDesignGridMode:
      return GLFW_KEY_TAB;
    case InputAction::ActDesignHideStructures:
      return GLFW_KEY_Q;
    case InputAction::ActDesignGrab:
      return GLFW_KEY_KP_7;
    case InputAction::ActDesignSearch:
      return GLFW_KEY_KP_8;
    case InputAction::ActDesignRoofOnly:
      return GLFW_KEY_E;
    case InputAction::ActDesignUndo:
      return GLFW_KEY_Z;
    case InputAction::ActDesignRedo:
      return GLFW_KEY_Y;
    case InputAction::ActDesignGable:
      return GLFW_KEY_G;
    case InputAction::ActDesignHip:
      return GLFW_KEY_H;
    case InputAction::ActDesignFlat:
      return GLFW_KEY_F;
    case InputAction::ActDesignBarrel:
      return GLFW_KEY_B;
    case InputAction::ActDesignSlant:
      return GLFW_KEY_T;
    case InputAction::ActDesignGambrel:
      return GLFW_KEY_R;

    // Added after hotkeys update
    case InputAction::ActTutorialOK:
      return GLFW_KEY_ENTER;
    case InputAction::ActBTContinuous:
      return GLFW_KEY_BACKSLASH;

    default:
      return GLFW_KEY_UNKNOWN;
  }
}

bool getWaitingForKey() {
  return waitingForKey;
}

void setWaitingForKey(bool val) {
  waitingForKey = val;
}

void initKeymap() {
  // Set up keymap
  for (int i = 0; i < (int)InputAction::NumActions; i++) {
    InputAction act = (InputAction)i;
    keymap[i].action = act;
    keymap[i].key = getDefaultKeyForAction(act);
  }
  // Check for any key collisions, just in case
  detectKeyCollisionsAll();
}

void initInput() {
  GLFWwindow* window = getWindow();
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  //glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
  glfwSetScrollCallback(window, scrollEvent);
  glfwSetMouseButtonCallback(window, mouseButtonEvent);
  glfwSetKeyCallback(window, keyEvent);
  glfwSetCharCallback(window, characterEvent);
}

void renderMouseLine(Line ml) {
  if (!c(CRenderMouseLine)) return;
  if (mouseLineEntity == 0) {
    mouseLineEntity = addEntity(PaletteShader);
    Entity* e = getEntity(mouseLineEntity);
    e->texture = paletteTexture;
    createMeshForEntity(mouseLineEntity);
    mouseLineMesh = e->mesh;
  }

  Mesh* mesh = getMesh(mouseLineMesh);
  mesh->flags |= _meshStreamDraw;
  makeCylinder(mesh, ml.start, ml.end - ml.start, 12, 12, colorRed);
  makeCube(mesh, ml.start, vec3(100, 100, 100), colorRed, true, true);
  makeCube(mesh, ml.end, vec3(100, 100, 100), colorBlue, true, true);
  bufferMesh(mouseLineMesh);
}

void updateInput(double duration) {
  inputMutex.lock();

  sendToClipboardBack = sendToClipboardFront;
  sendToClipboardFront = 0;
  char* clipboardContentsCopy = clipboardContents;
  clipboardContents = 0;

  InputEvent lastEventCopy = lastEvent;
  vector<InputEvent> events;
  for (int i = 0; i < inputQueue->size(); i++) {
    events.push_back(inputQueue->operator[](i));
  }
  inputQueue->empty();

  inputMutex.unlock();

  // Iterate through actions here
  // Set a keybind active based on if key is pressed
  for (int i = 0; i < InputAction::NumActions; i++) {
      keymap[i].active = lastEventCopy.isKeyDown[keymap[i].key];
  }

  moveCamera(duration, lastEventCopy);
  for (int i = 0; i < events.size(); i++) {
    handleEvent(events[i]);
  }

  renderMouseLine(lastEventCopy.mouseLine);

  if (clipboardContentsCopy) {
    for (int i = 0; i < clipboardCallbacks.size(); i++) {
      clipboardCallbacks[i](clipboardContentsCopy);
    }
    clipboardCallbacks.clear();
    free(clipboardContentsCopy);
  }
}

void setLockMouse(bool val) {
  lockMouse = val;
}

bool getLockMouse() {
  return lockMouse;
}

void collectInput() {
  GLFWwindow* window = getWindow();

  if (glfwWindowShouldClose(window) != 0) {
    endGame();
  }

  Camera camera = getMainCameraBack();
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);

  if (lockMouse) {
    resetPos = false;
    if (xpos < 0.0 || ypos < 0.0) {
      resetPos = true;
      xpos = xpos < 0.0 ? 0.0 : xpos;
      ypos = ypos < 0.0 ? 0.0 : ypos;
    }

    if (xpos > camera.window.x || ypos > camera.window.y) {
      resetPos = true;
      xpos = xpos > camera.window.x ? camera.window.x : xpos;
      ypos = ypos > camera.window.y ? camera.window.y : ypos;
    }

    if (resetPos)
      glfwSetCursorPos(window, xpos, ypos);
  }

  vec2 cursor = vec2(xpos, ypos);
  Line mouseLine;
  vec2 mouseLoc = vec2(
      (2.0f * xpos) / camera.window.x - 1.0f,
      -((2.0f * ypos) / camera.window.y - 1.0f));

  if (getFOV() == 0) {
    vec4 cameraSpaceMouseRaySource = vec4(mouseLoc, 0, 1);

    vec3 mouseRaySource = vec3(
      inverse(camera.viewProjection) *
      cameraSpaceMouseRaySource
    );

    float mapSize = getMapSize();
    mouseLine = line(
      mouseRaySource - vec3(camera.direction) * mapSize * 2.f,
      mouseRaySource + vec3(camera.direction) * mapSize * 2.f);

  } else {
    vec4 cameraSpaceMouseRaySource = vec4(mouseLoc, -1, 1);

    /*
    vec3 mouseRaySource = vec3(
      inverse(camera.viewProjection) *
      cameraSpaceMouseRaySource
    );
    */

    vec4 mouseRaySource4 = inverse(camera.viewProjection) *
      cameraSpaceMouseRaySource;
    vec3 mouseRaySource = vec3(mouseRaySource4)/mouseRaySource4.w;

    /*
    vec4 cameraSpaceMouseRayDest = vec4(mouseLoc, 1, 1);
    vec3 mouseRayDest = vec3(
      inverse(camera.viewProjection) *
      cameraSpaceMouseRayDest
    );
    */

    float mapSize = getMapSize();
    //mouseRaySource *= getMapSize();
    //mouseRayDest *= getMapSize();
    //mouseRaySource *= camera.distance;
    //mouseRayDest *= camera.distance;
    //mouseRaySource *= 10000;
    //mouseRayDest *= 10000;
    //mouseRaySource *= camera.distance / (15 * getFOV());

    vec3 mouseRayAlong = mouseRaySource - vec3(camera.position);
    mouseRayAlong = normalize(mouseRayAlong);
    mouseRaySource -= mouseRayAlong * 20.f;
    vec3 mouseRayDest = mouseRayAlong * (mapSize*2) + vec3(camera.position);
    mouseLine = line(mouseRaySource, mouseRayDest);

    /*
    SPDLOG_INFO("mouseRaySource {},{},{}",
        mouseRaySource.x,
        mouseRaySource.y,
        mouseRaySource.z);

    SPDLOG_INFO("mouseRayDest {},{},{}",
        mouseRayDest.x,
        mouseRayDest.y,
        mouseRayDest.z);
        */
  }

  inputMutex.lock();
  lastEvent.mouseLine = mouseLine;
  glfwPollEvents();

  for (int i = 0; i <= GLFW_KEY_LAST; i++) {
    lastEvent.isKeyDown[i] = false;
  }

  for (int i = 0; i < numValidKeys; i++) {
    int key = validKeys[i];
    lastEvent.isKeyDown[key] = glfwGetKey(window, key) == GLFW_PRESS;
  }

  // fix to modifier keys
  int mods = 0;
  if (lastEvent.isKeyDown[GLFW_KEY_LEFT_CONTROL]) mods |= GLFW_MOD_CONTROL;
  if (lastEvent.isKeyDown[GLFW_KEY_RIGHT_CONTROL]) mods |= GLFW_MOD_CONTROL;
  if (lastEvent.isKeyDown[GLFW_KEY_LEFT_SHIFT]) mods |= GLFW_MOD_SHIFT;
  if (lastEvent.isKeyDown[GLFW_KEY_RIGHT_SHIFT]) mods |= GLFW_MOD_SHIFT;
  if (lastEvent.isKeyDown[GLFW_KEY_LEFT_ALT]) mods |= GLFW_MOD_ALT;
  if (lastEvent.isKeyDown[GLFW_KEY_RIGHT_ALT]) mods |= GLFW_MOD_ALT;
  lastEvent.mods = mods;

  mouseMoved(mouseLoc, mouseLine);

  if (sendToClipboardBack != 0) {
    glfwSetClipboardString(window, sendToClipboardBack);
    free(sendToClipboardBack);
    sendToClipboardBack = 0;
  }

  if (grabClipboard) {
    clipboardContents = strdup_s(glfwGetClipboardString(window));
    grabClipboard = false;
  }

  inputMutex.unlock();
}

#include "inputCallbacks.cpp"
