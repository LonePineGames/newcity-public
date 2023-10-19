#include "conCallbacks.hpp"

#include "conCmd.hpp"
#include "conDisplay.hpp"
#include "conInput.hpp"
#include "conMod.hpp"

#include "../building/design.hpp"
#include "../business.hpp"
#include "../draw/framebuffer.hpp"
#include "../game/achievement.hpp"
#include "../game/feature.hpp"
#include "../game/game.hpp"
#include "../graph.hpp"
#include "../import/obj.hpp"
#include "../input.hpp"
#include "../main.hpp"
#include "../newspaper/article.hpp"
#include "../option.hpp"
#include "../person.hpp"
#include "../parts/tutorialPanel.hpp"
#include "../platform/binaryfilereader.hpp"
#include "../platform/binaryfilebuilder.hpp"
#include "../platform/lua.hpp"
#include "../platform/lookup.hpp"
#include "../platform/mod.hpp"
#include "../route/location.hpp"
#include "../selection.hpp"
#include "../sound.hpp"
#include "../steam/steamwrapper.hpp"
#include "../string.hpp"
#include "../tools/building.hpp"
#include "../tutorial.hpp"
#include "../weather.hpp"
#include "../vehicle/travelGroup.hpp"
#include "../zone.hpp"


bool conCallbackAnyBuildingPlop(std::string data) {
  setAnyBuildingPlop(true);
  setFeatureEnabled(FBuildingTool, true);

  consolePrintLine("The question isn't who is going to let me;\n"
    "it's who is going to stop me.");
  consolePrintLine("Check the Amenity Tool (4)");
  return true;
}

bool conCallbackCameraInfo(std::string data) {
  Camera main = getMainCamera();

  consolePrintLine(sprintf_o("Main camera up: (%s,%s,%s)",
    std::to_string(main.up.x).c_str(),
    std::to_string(main.up.y).c_str(),
    std::to_string(main.up.z).c_str()));

  consolePrintLine(sprintf_o("Main camera right: (%s,%s,%s)",
    std::to_string(main.right.x).c_str(),
    std::to_string(main.right.y).c_str(),
    std::to_string(main.right.z).c_str()));

  return true;
}

bool conCallbackCapture(std::string data) {
  item ndx = 0;
  char* fbStr = strdup_s(data.c_str());
  char* filenameStr = fbStr;
  for (; filenameStr[0] != '\0' && filenameStr[0] != ' '; filenameStr++);
  if (filenameStr[0] == ' ') filenameStr++;
  if (filenameStr[0] == '\0') filenameStr = "capture.png";
  item fbNdx = 0;
  sscanf(fbStr, "%d", &fbNdx);

  consolePrintLine(sprintf_o("Capturing framebuffer %d to image %s...",
      fbNdx, filenameStr));
  captureFramebuffer((Framebuffer)fbNdx, filenameStr);
  free(fbStr);

  return true;
}

bool conCallbackClear(std::string data) {
  consoleClear();
  return true;
}

bool conCallbackCrash(std::string data) {
  consolePrintDebug("Forcing crash...", true);
  Part* aPart = 0;
  aPart->itemData = 1;
  return true;
}

bool conCallbackCursorPos(std::string data) {
  double x = 0.0, y = 0.0;
  glfwGetCursorPos(getWindow(), &x, &y);
  SPDLOG_INFO("Cursor pos: {}/{}", x, y);
  return true;
}

bool conCallbackDebug(std::string data) {
  bool currentDebug = debugMode();
  
  if (data.length() == 0 || data[0] == ' ' || data[0] == '?') {
    consolePrintLine(sprintf_o("Debug: %s",
      currentDebug ? "True" : "False"));
    return true;
  }

  if (data[0] == 'f' || data[0] == 'F' || data[0] == '0')
    setDebugMode(false);
  else if (data[0] == 't' || data[0] == 'T' || data[0] == '1')
    setDebugMode(true);
  else
    consolePrintLine(sprintf_o("Invalid value for debug flag %s", data));

  if (debugMode() != currentDebug) {
    consolePrintLine(sprintf_o("Debug is now: %s",
      debugMode() ? "True" : "False"));
  }

  return true;
}

bool conCallbackDifficulty(std::string data) {
  if(data.length() == 0 || data[0] == ' ') {
    consolePrintLine(sprintf_o("Difficulty: %d (%s)",
          (int)getGameDifficulty(), difficultyName[getGameDifficulty()]));
    return true;
  }

  int val = -1;
  sscanf(data.c_str(), "%d", &val);

  if(val < DifficultyEasy || val >= numDifficulties) {
    consolePrintLine(sprintf_o("Invalid value for Difficulty: %d", val));
    return false;
  }

  setGameDifficulty((GameDifficulty)val);
  consolePrintLine(sprintf_o("Difficulty set to %d (%s)",
        (int)getGameDifficulty(), difficultyName[getGameDifficulty()]));
  consolePrintLine(strdup_s("Restart required to change game difficulty."));

  return true;
}

bool conCallbackDir(std::string data) {
  consolePrintLine("Main dir:");
  consolePrintLine(modDirectoryNonNull());
  consolePrintLine("Save dir:");
  consolePrintLine(lookupSave(saveDirectory()));
  return true;
}

bool conCallbackEmergencyPowers(std::string data) {
  consolePrintLine("There is no power but me!");
  unlockTaxes();
  setFeatureEnabled(FPropertyTax, true);
  setFeatureEnabled(FSalesTax, true);
  setFeatureEnabled(FFinesAndFees, true);
  return true;
}

bool conCallbackUnachiever(std::string data) {
  consolePrintLine("Stop being lazy!");
  resetAchievements();
  return true;
}

bool conCallbackEnableAmenity(std::string data) {
  return unlockGovDesignsByName(data.c_str());
}

bool conCallbackEnableFeature(std::string data) {
  item fCodeVal = getFeatureCodeByStr(data);

  if(fCodeVal < 0 || fCodeVal >= numFeatures) {
    SPDLOG_INFO("Could not find feature code {}", data);
    return false;
  }

  std::string output = "Enabling feature code " + data + "... ";

  setFeatureEnabled(fCodeVal, true);
  output += isFeatureEnabled(fCodeVal) ? "success!" :
    "Failed to enable feature.";

  SPDLOG_INFO("{}", output);
  return true;
}

bool conCallbackFinalDollar(std::string data) {
  Budget* currentBudget = getCurrentBudget();
  float cashAvailable = currentBudget->line[BudgetLine::BudgetBalance];

  transaction(AssetSalesIncome, -(cashAvailable)+1);

  consolePrintLine("Mayor seen checking phone booths for loose change");
  return true;
}

bool conCallbackFov(std::string data) {
  if(data.length() == 0 || data[0] == ' ') {
    consolePrintLine(sprintf_o("Current FOV: %f", getFOV() * 180.0f));
    return true;
  }

  float val = -1;
  sscanf(data.c_str(), "%f", &val);

  if(val < 0) {
    consolePrintLine(sprintf_o("Invalid value for FOV: %f", val));
    return false;
  }

  float maxFov = getFOVMaxDegrees();

  if(val > maxFov) {
    consolePrintLine(sprintf_o("New FOV value exceeds max (%f), setting to max", maxFov));
    val = maxFov;
  }

  float modVal = val / 180.0f;

  consolePrintLine(sprintf_o("Setting FOV to %f (%f)", val, modVal));
  setFOV(modVal);
  return true;
}

bool conCallbackHello(std::string data) {
  std::string hello = "Hello World!";
  if(data.length() > 0) {
    hello += " Oh, and " + data + ".";
  }
  consolePrintLine(hello);
  return true;
}

bool conCallbackHelp(std::string data) {
  ConCmd cmd;
  if(data.length() > 0) {
    if(getCmd(data,cmd)) {
      consolePrintLine(cmd.help);
      return true;
    }
    // Data was passed, but was not a recognized command
    consolePrintError(consoleGetError(ConStatus::INVALID_ARG),data);
  }

  consolePrintLine("\nCommands:");
  for(int c = 0; c < conCmds.size(); c++) {
    if(conCmds[c].hidden) {
      continue;
    }
    std::string txt = conCmds[c].name;
    txt += " - ";
    txt.append(conCmds[c].help);
    consolePrintLine(txt);
  }

  /*
  consolePrintLine("\nMods:");
  for(int m = 0; m < conMods.size(); m++) {
    std::string txt = "";
    txt += conMods[m].value + " - ";
    txt.append(conMods[m].help);
    consolePrintLine(txt);
  }
  */

  // Add a buffer blank line after the command list
  consolePrintLine("");
  return true;
}

bool conCallbackKeyBind(std::string data) {
  if (data.length() == 0 || data[0] == ' ') {
    SPDLOG_INFO("Specify an key and an action, separated by a space");
    return false;
  }

  // Convert to lower if possible for consistency
  for (int i = 0; i < data.length(); i++) {
    data[i] = std::tolower(data[i]);
  }

  // Try to get action and key strings
  std::string keyStr = "";
  std::string actStr = "";
  size_t split = data.find(" ");

  // Didn't find a space; not necessarily a bad thing
  if (split == std::string::npos) {
    // User could be checking current bind
    for (int i = 0; i < InputAction::NumActions; i++) {
      KeyBind bind = getKeyBind(i);
      if (data == bind.keyStr() || data == bind.actCmdStr()) {
        SPDLOG_INFO("Key {} currently bound to action {}", bind.keyStr(), bind.actCmdStr());
        return true; // We found it, return;
      }
    }

    // No valid key or action found in the string
    SPDLOG_WARN("Couldn't find key or action {}", data);
  } else {
    keyStr = data.substr(0, split);
    actStr = data.substr(split+1, std::string::npos);

    InputAction act = InputAction::ActNone;
    int32_t key = GLFW_KEY_UNKNOWN;

    for (int i = 0; i < InputAction::NumActions; i++) {
      KeyBind bind = getKeyBind(i);
      if (actStr == bind.actCmdStr()) {
        act = (InputAction)i;
        break;
      }
    }

    // Couldn't find a matching action; early return
    if (act == InputAction::ActNone) {
      SPDLOG_WARN("Couldn't find action {}", actStr);
      return true;
    }

    // Key string was invalid; early return
    if (!getKeyForStr(keyStr, key)) {
      SPDLOG_WARN("Couldn't find key {}", keyStr);
      return true;
    }

    // Assign key to action
    bindKeyToAction(act, key);
    // Sweep all keybinds for collisions; this should reset flags if we are no longer colliding
    detectKeyCollisionsAll();
    // Write changes to input.data
    writeInputFile();
  }

  return true;
}

bool conCallbackKeyUnbind(std::string data) {
  if (data.length() == 0 || data[0] == ' ') {
    SPDLOG_INFO("Specify an action to be unbound");
    return false;
  }

  // Convert to lower for consistency
  for (int i = 0; i < data.length(); i++) {
    data[i] = std::tolower(data[i]);
  }

  for (int i = 1; i < InputAction::NumActions; i++) {
    KeyBind bind = getKeyBind(i);
    if (data == bind.actCmdStr()) {
      unbindAction((InputAction)i);
      // Write changes to input.data
      writeInputFile();
      return true; // We found it, return
    }
  }

  // Couldn't find a matching action; early return
  SPDLOG_WARN("Couldn't find action {}", data);

  return true;
}

bool conCallbackKeyUnbindAll(std::string data) {
  for (int i = 0; i < (int)InputAction::NumActions; i++) {
    unbindAction((InputAction)i);
  }

  // Write changes to input.data
  writeInputFile();

  SPDLOG_INFO("Unbound all actions");

  return true;
}

bool conCallbackLoadObj(std::string data) {
  return loadObjFile(data);
}

bool conCallbackLua(std::string data) {
  if(data.length() == 0) {
    return false;
  }

  if(streql(data.c_str(),"reload")) {
    // reload values
    consolePrintLine("Reload Lua Values Placeholder!!");
    return true;
  }

  int status = consoleHandleLua(data);
  if(status != ConStatus::SUCCESS) {
    consolePrintError(consoleGetError((ConStatus)status),data);
    return false;
  }

  return true;
}

bool conCallbackMod(std::string data) {
  consolePrintLine("Current mod:");
  consolePrintLine(getMod());
  consolePrintLine("Mod dir:");
  consolePrintLine(modDirectoryNonNull());
  return true;
}

bool conCallbackPartyTime(std::string data) {
  for(int i = 0; i < numFeatures; i++) {
    setFeatureEnabled(i,true);
  }

  setAllGovernmentBuildingsAvailable();
  consolePrintLine("Everybody's going to the party");
  consolePrintLine("Have a real good time");
  return true;
}

bool conCallbackPropaganda(std::string data) {
  forceNewspaperArticle(data);
  consolePrintLine("Trying to send a message?");
  return true;
}

bool conCallbackQuit(std::string data) {
  endGame();
  return true;
}

bool conCallbackReadObj(std::string data) {
  return tryPrintObjData(data);
}

bool conCallbackRenderObj(std::string data) {
  return renderObj(data);
}

bool conCallbackRoute(std::string data) {
  if(data.length() <= 1) {
    consolePrintLine("Insufficient data for fetching route");
    return false;
  }

  std::string type = data.substr(0, 1);
  std::string index = data.substr(1, std::string::npos);

  switch(type[0]) {
    case 'g':
      return conCallbackRouteTravelGroup(index);
    case 'p':
      return conCallbackRoutePerson(index);
    case 'v':
      return conCallbackRouteVehicle(index);
    default:
      consolePrintLine(sprintf_o("Unrecognized type for route (%s)", type.c_str()));
      return false;
  }
}

bool conCallbackRoutePerson(std::string data) {
  if(!conCallbackSelectPerson(data)) {
    consolePrintLine("Could not select person to fetch route");
    return false;
  }

  Person* person = getPerson(getSelection());
  char* routeStr = 0;

  if(person->flags & _personTraveling) {
    TravelGroup* tg = getTravelGroup_g(locationNdx(person->location));
    routeStr = routeString(&tg->route);
  } else {
    routeStr = strdup_s("<no route>");
  }

  consolePrintLine(sprintf_o("Route for person %s: %s", data.c_str(),
        routeStr));
  free(routeStr);
  return true;
}

bool conCallbackRouteTravelGroup(std::string data) {
  if(!conCallbackSelectTravelGroup(data)) {
    consolePrintLine("Could not select travel group to fetch route");
    return false;
  }

  TravelGroup* tg = getTravelGroup_g(getSelection());

  if(tg == 0) {
    consolePrintLine("No route for null travel group 0");
    return false;
  }

  char* routeStr = routeString(&tg->route);
  consolePrintLine(sprintf_o("Route for travel group %s: %s", data.c_str(), routeStr));
  free(routeStr);
  return true;
}

bool conCallbackRouteVehicle(std::string data) {
  if(!conCallbackSelectVehicle(data)) {
    consolePrintLine("Could not select vehicle to fetch route");
    return false;
  }

  Vehicle* vehicle = getVehicle(getSelection());

  if(vehicle == 0) {
    consolePrintLine("No route for null vehicle 0");
    return false;
  }

  char* routeStr = routeString(&vehicle->route);
  consolePrintLine(sprintf_o("Route for vehicle %s: %s", data.c_str(),
        routeStr));
  free(routeStr);
  return true;
}

bool conCallbackSelect(std::string data) {
  if(data.length() <= 1) {
    consolePrintLine("Insufficient data for selection");
    return false;
  }

  std::string type = data.substr(0, 1);
  std::string index = data.substr(1, std::string::npos);

  switch(type[0]) {
    case 'b':
      return conCallbackSelectBuilding(index);
    case 'e':
      return conCallbackSelectElement(index);
    case 'f':
      return conCallbackSelectFamily(index);
    case 'g':
      return conCallbackSelectTravelGroup(index);
    case 'p':
      return conCallbackSelectPerson(index);
    case 'v':
      return conCallbackSelectVehicle(index);
    default:
      consolePrintLine(sprintf_o("Unrecognized type for selection (%s)", type.c_str()));
      return false;
  }
}

bool conCallbackSelectBuilding(std::string data) {
  int ndx = -1;
  sscanf(data.c_str(), "%d", &ndx);

  if (ndx == 0) {
    consolePrintLine("Deselect");
    clearSelection();
    return true;
  } else if (ndx < 0) {
    consolePrintLine("Invalid index for building");
    return false;
  } else if(ndx > sizeBuildings()) {
    consolePrintLine("Index out of bounds for building");
    return false;
  }

  Building* building = getBuilding(ndx);

  if(building == 0) {
    consolePrintLine("Null ptr fetch from building index");
    return false;
  }

  if(building->flags & _buildingExists) {
    consolePrintLine("Selecting building " + std::to_string(ndx));
    setSelection(SelectionTypes::SelectionBuilding, ndx);
  } else {
    consolePrintLine("Building to select does not exist");
    return false;
  }

  return true;
}

bool conCallbackSelectElement(std::string data) {
  int ndx = -1;
  sscanf(data.c_str(), "%d", &ndx);

  Edge* edge = 0;
  Node* node = 0;

  if (ndx == 0) {
    consolePrintLine("Deselect");
    clearSelection();
    return true;

  } else if(ndx > 0) {
    if(ndx > sizeEdges()) {
      consolePrintLine("Index out of bounds for edge");
      return false;
    }

    edge = getEdge(ndx);

    if(edge == 0) {
      consolePrintLine("Null ptr fetch from edge index");
      return false;
    }

    consolePrintLine("Selecting edge " + std::to_string(ndx));
    setSelection(SelectionTypes::SelectionGraphElement, ndx);
  } else {
    if(-ndx > numNodes()) {
      consolePrintLine("Index out of bounds for node");
      return false;
    }

    node = getNode(ndx);

    if(node == 0) {
      consolePrintLine("Null ptr fetch from node index");
      return false;
    }

    consolePrintLine("Selecting node " + std::to_string(ndx));
    setSelection(SelectionTypes::SelectionGraphElement, ndx);
  }

  return true;
}

bool conCallbackSelectFamily(std::string data) {
  int ndx = -1;
  sscanf(data.c_str(), "%d", &ndx);

  if (ndx == 0) {
    consolePrintLine("Deselect");
    clearSelection();
    return true;

  } else if (ndx < 0) {
    consolePrintLine("Invalid index for family");
    return false;

  } else if (ndx > sizeFamilies()) {
    consolePrintLine("Index out of bounds for family");
    return false;
  }

  Family* family = getFamily(ndx);

  if(family == 0) {
    consolePrintLine("Null ptr fetch from family index");
    return false;
  }

  if(family->flags & _familyExists) {
    consolePrintLine("Selecting family " + std::to_string(ndx));
    setSelection(SelectionTypes::SelectionFamily, ndx);
  } else {
    consolePrintLine("Family to select does not exist");
    return false;
  }

  return true;
}

bool conCallbackSelectPerson(std::string data) {
  int ndx = -1;
  sscanf(data.c_str(), "%d", &ndx);

  if (ndx == 0) {
    consolePrintLine("Deselect");
    clearSelection();
    return true;

  } else if (ndx < 0) {
    consolePrintLine("Invalid index for person");
    return false;

  } else if (ndx > sizePeople()) {
    consolePrintLine("Index out of bounds for person");
    return false;
  }

  Person* person = getPerson(ndx);

  if(person == 0) {
    consolePrintLine("Null ptr fetch from person index");
    return false;
  }

  if(person->flags & _personExists) {
    consolePrintLine("Selecting person " + std::to_string(ndx));
    setSelection(SelectionTypes::SelectionPerson, ndx);
  } else {
    consolePrintLine("Person to select does not exist");
    return false;
  }

  return true;
}

bool conCallbackSelectTravelGroup(std::string data) {
  int ndx = -1;
  sscanf(data.c_str(), "%d", &ndx);

  if (ndx == 0) {
    consolePrintLine("Deselect");
    clearSelection();
    return true;

  } else if (ndx < 0) {
    consolePrintLine("Invalid index for travel group");
    return false;

  } else if (ndx > sizeTravelGroups()) {
    consolePrintLine("Index out of bounds for travel group");
    return false;
  }

  TravelGroup* group = getTravelGroup_g(ndx);

  if(group == 0) {
    consolePrintLine("Null ptr fetch from travel group index");
    return false;
  }

  if(group->flags & _groupExists) {
    consolePrintLine("Selecting lead member of travel group " + std::to_string(ndx));
    // Just grab the first member, since we can't select the group explicitly
    setSelection(SelectionTypes::SelectionPerson, group->members[0]);
  } else {
    consolePrintLine("Travel group to select does not exist");
    return false;
  }

  return true;
}

bool conCallbackSelectVehicle(std::string data) {
  int ndx = -1;
  sscanf(data.c_str(), "%d", &ndx);

  if (ndx == 0) {
    consolePrintLine("Deselect");
    clearSelection();
    return true;

  } else if(ndx < 0) {
    consolePrintLine("Invalid index for vehicle");
    return false;

  } else if(ndx > sizeVehicles()) {
    consolePrintLine("Index out of bounds for vehicle");
    return false;
  }

  Vehicle* vehicle = getVehicle(ndx);

  if(vehicle == 0) {
    consolePrintLine("Null ptr fetch from vehicle index");
    return false;
  }

  if(vehicle->flags & _vehicleExists) {
    consolePrintLine("Selecting vehicle " + std::to_string(ndx));
    setSelection(SelectionTypes::SelectionVehicle, ndx);
  } else {
    consolePrintLine("Vehicle to select does not exist");
    return false;
  }

  return true;
}

bool conCallbackSongEnable(std::string data) {
  bool currentSongsEnabled = getSongsEnabled();

  if (data.length() == 0 || data[0] == ' ' || data[0] == '?') {
    consolePrintLine(sprintf_o("Music: %s",
      currentSongsEnabled ? "Enabled" : "Disabled"));
    return true;
  }

  if (data[0] == 'f' || data[0] == 'F' || data[0] == '0')
    setSongsEnabled(false);
  else if (data[0] == 't' || data[0] == 'T' || data[0] == '1')
    setSongsEnabled(true);
  else
    consolePrintLine(sprintf_o("Invalid value for songs enabled flag %s", data));

  if (getSongsEnabled() != currentSongsEnabled) {
    consolePrintLine(sprintf_o("Music is now: %s",
      getSongsEnabled() ? "Enabled" : "Disabled"));
  }

  return true;
}

bool conCallbackSongInfo(std::string data) {
  displaySongDebug();
  return true;
}

bool conCallbackSongPlay(std::string data) {
  int ndx = -1;
  sscanf(data.c_str(), "%d", &ndx);

  tryPlaySong(ndx, true);
  return true;
}

bool conCallbackSongShuffle(std::string data) {
  tryPlayRandomSong(true);
  return true;
}

bool conCallbackSteamInfo(std::string data) {
  #ifdef INCLUDE_STEAM
    consolePrintLine(steam_getSteamInfo());
  #else
    consolePrintLine("Steam not included");
  #endif
  return true;
}

bool conCallbackSetAutosave(std::string data) {
  int val = -1;
  sscanf(data.c_str(), "%d", &val);
  setAutosaveInterval(val);
  writeOptions();
  SPDLOG_INFO("Autosave Interval is now {}", val);
  return true;
}

bool conCallbackTestFileWrite(std::string data) {
  std::string filename = "test.data";
  uint32_t numActions = InputAction::NumActions;
  FileSector<KeyBind> test(true, FileDataType::DataKeyBind, (ubyte)InputAction::NumActions);

  int i = 0;
  for (i; i < numActions; i++) {
    KeyBind bind = getKeyBind(i);
    if (!test.push(bind)) {
      SPDLOG_ERROR("Error pushing data to test sector - i:{}", i);
      break;
    }
    SPDLOG_INFO("Pushed ({},{})", bind.action, bind.key);
  }

  SPDLOG_INFO("Pushed {} elements into file sector; test.count() {}", i, test.count());

  SPDLOG_INFO("Writing binary data to {}", filename);

  // Set verbose
  bool wasVerbose = BinaryFileBuilder::getVerbose();
  BinaryFileBuilder::setVerbose(true);

  bool success = true;
  if (!BinaryFileBuilder::startWrite(filename)) {
    SPDLOG_ERROR("Error starting write process for file {}", filename);
    success = false;
  }

  if (success) {
    SPDLOG_INFO("Start write successful for file {}", filename);
  }

  if (success && !BinaryFileBuilder::writeSectorKeyBind(&test)) {
    SPDLOG_ERROR("Error writing sector for file {}", filename);
    success = false;
  }

  if (success) {
    SPDLOG_INFO("Write sector successful for file {}", filename);
  }

  if (success && !BinaryFileBuilder::finishWrite()) {
    SPDLOG_ERROR("Error finishing write process for file {}", filename);
    success = false;
  }

  if (success) {
    SPDLOG_INFO("Successfully wrote data to test file");
  }
  
  // Reset verbose
  BinaryFileBuilder::setVerbose(wasVerbose);

  return success;
}

bool conCallbackTestFileRead(std::string data) {
  std::string filename = "test.data";

  SPDLOG_INFO("Reading binary data from {}", filename);

  // Set verbose
  bool wasVerbose = BinaryFileReader::getVerbose();
  BinaryFileReader::setVerbose(true);

  bool success = true;
  if (!BinaryFileReader::startRead(filename)) {
    SPDLOG_ERROR("Error reading file {}", filename);
    success = false;
  }

  if (success) {
    FileSector<KeyBind> sector;
    if (!BinaryFileReader::readSectorKeyBind(sector)) {
      SPDLOG_ERROR("Error reading binary data from file {}", filename);
      success = false;
    }
  }

  if (success) {
    SPDLOG_INFO("Successfully read binary data from file {}", filename);
  }

  // Reset verbose
  BinaryFileReader::setVerbose(wasVerbose);

  return success;
}

bool conCallbackTutorialEnable(std::string data)
{
  if(data.length() == 0)
  {
    SPDLOG_INFO("Tutorial is {}; Player {} completed it.",
        isTutorialPanelOpen() ? "enabled" : "disabled",
        hasCompletedTutorial() ? "has" : "has not");
    return true;
  }

  int val = -1;
  sscanf(data.c_str(), "%d", &val);

  // Failed to parse as decimal integer; check string
  if(val < 0) {
    if(data[0] == 't' || data[0] == 'T')
    {
      val = 1;
    }
    else if(data[0] == 'f' || data[0] == 'F')
    {
      val = 0;
    }
  }

  if(val < 0)
  {
    SPDLOG_INFO("Invalid value {} for tutorialenable command", val);
    return false;
  }

  setTutorialPanelOpen(val > 0);

  SPDLOG_INFO("Tutorial is {}; Player {} completed it.",
      isTutorialPanelOpen() ? "enabled" : "disabled",
      hasCompletedTutorial() ? "has" : "has not");
  return true;
}

bool conCallbackTutorialInfo(std::string data) {
  TutorialState* ptr = getTutorialStatePtr();
  std::vector<TutorialAction> actions = ptr->actions();
  SPDLOG_INFO("Tutorial State Info: tutorialActive - {}, showTutorial - {}, timer - {}, camera time - {}", ptr->tutorialActive(), isTutorialPanelOpen(), ptr->timerTarget(), getCameraTime());
  SPDLOG_INFO("Tutorial State Actions: {} {} {} {}", actions[0].code, actions[1].code, actions[2].code, actions[3].code);
  return true;
}

bool conCallbackUnloadObj(std::string data) {
  return tryUnloadObj(data);
}

bool conCallbackVoodooEconomics(std::string data) {
  transaction(AssetSalesIncome, 1000000000);
  consolePrintLine("Mr Bernanke, your helicopter has arrived.");
  return true;
}

bool conCallbackTheSnap(std::string data) {
  consolePrintLine("Reality can be whatever I want.");
  for (int i = 1; i <= sizePeople(); i++) {
    if (randFloat() < 0.5) killPerson(i);
  }
  for (int i = 1; i <= sizeBusinesses(); i++) {
    if (randFloat() < 0.5) removeBusiness(i);
  }
  return true;
}

bool conCallbackWeather(std::string data) {
  int ndx = -1;
  sscanf(data.c_str(), "%d", &ndx);

  if(ndx < 0 || ndx >= WeatherType::NUM_WEATHER_TYPES) {
    SPDLOG_INFO("Invalid index {} for weather", ndx);
    return false;
  }

  if (ndx == WeatherType::STROBE) {
    SPDLOG_INFO("Setting weather to Strobe");
    setStrobeClouds(true);
    return true;
  } else {
    setStrobeClouds(false);
  }

  Weather w = getWeather();

  switch((WeatherType)ndx) {
    default:
      SPDLOG_INFO("Unhandled WeatherType {}", ndx);
    case WeatherType::NORMAL:
      SPDLOG_INFO("Setting weather to Normal");
      w.clouds = 0;
      w.drought = 0;
      w.percipitation = 0;
      w.pressure = 1;
      w.snow = 0;
      w.temp = 20;
      w.wind = vec3(10, 0, 0);
      break;
    case WeatherType::RAIN:
      SPDLOG_INFO("Setting weather to Rain");
      w.clouds = 1;
      w.drought = 0;
      w.percipitation = 100;
      w.pressure = -1;
      w.snow = 0;
      w.temp = 25;
      w.wind = vec3(10, 0, 0);
      break;
    case WeatherType::SNOW:
      SPDLOG_INFO("Setting weather to Snow");
      w.clouds = 1;
      w.drought = 0;
      w.percipitation = 100;
      w.pressure = -1;
      w.snow = 1;
      w.temp = -5;
      w.wind = vec3(10, 0, 0);
      break;
  }

  setWeather(w);
  return true;
}

bool conCallbackMap(std::string data) {
  consolePrintLine("Sending command to redraw map.");
  renderMap_g();
  return true;
}

