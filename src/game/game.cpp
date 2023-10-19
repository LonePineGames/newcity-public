#include "game.hpp"

#include "achievement.hpp"
#include "feature.hpp"
#include "task.hpp"
#include "update.hpp"
#include "version.hpp"

#include "../amenity.hpp"
#include "../blueprint.hpp"
#include "../board.hpp"
#include "../building/building.hpp"
#include "../building/buildingTexture.hpp"
#include "../building/designOrganizer.hpp"
#include "../building/designPackage.hpp"
#include "../building/renderBuilding.hpp"
#include "../building/design.hpp"
#include "../business.hpp"
#include "../city.hpp"
#include "../collisionTable.hpp"
#include "../compass.hpp"
#include "../economy.hpp"
#include "../error.hpp"
#include "../heatmap.hpp"
#include "../input.hpp"
#include "../intersection.hpp"
#include "../graph.hpp"
#include "../graph/transit.hpp"
#include "../graph/stop.hpp"
#include "../label.hpp"
#include "../land.hpp"
#include "../lot.hpp"
#include "../money.hpp"
#include "../name.hpp"
#include "../newspaper/newspaper.hpp"
#include "../option.hpp"
#include "../person.hpp"
#include "../pillar.hpp"
#include "../plan.hpp"
#include "../renderLand.hpp"
#include "../route/broker.hpp"
#include "../route/router.hpp"
#include "../selection.hpp"
#include "../sound.hpp"
#include "../string_proxy.hpp"
#include "../thread.hpp"
#include "../time.hpp"
#include "../vehicle/pedestrian.hpp"
#include "../vehicle/travelGroup.hpp"
#include "../vehicle/update.hpp"
#include "../vehicle/wanderer.hpp"
#include "../weather.hpp"
#include "../tutorial.hpp"
#include "../util.hpp"

#include "../console/conDisplay.hpp"
#include "../console/conInput.hpp"

#include "../draw/buffer.hpp"
#include "../draw/camera.hpp"
#include "../draw/entity.hpp"
#include "../draw/framebuffer.hpp"

#include "../import/mesh-import.hpp"

#include "../parts/budgetPanel.hpp"
#include "../parts/economyPanel.hpp"
#include "../parts/leftPanel.hpp"
#include "../parts/mainMenu.hpp"
#include "../parts/messageBoard.hpp"
#include "../parts/part.hpp"
#include "../parts/renderParts.hpp"
#include "../parts/root.hpp"
#include "../parts/toolbar.hpp"

#include "../platform/event.hpp"
#include "../platform/file.hpp"
#include "../platform/lookup.hpp"
#include "../platform/mod.hpp"

#include "spdlog/spdlog.h"
#include <errno.h>

//#ifdef __linux__
  //#include <valgrind/callgrind.h>
//#endif

static GameMode gameMode = ModeGame;
static bool gameAccelerated = true;
static bool nextGameAccelerated = true;
static bool nextLeftHandTraffic = false;
static GameDifficulty gameDifficulty = DifficultyMedium;
static GameDifficulty nextGameDifficulty = DifficultyMedium;
static item gameSpeed = 2;
static item lastGameSpeed = 2;
static const char* loaderText;
static char* filenameTarget;
static atomic<LoadMode> loadMode(LoadTargetAutosave);
static GameMode gameModeTarget;
static bool loaderThreadStarted = true;
static bool gameLoaded = false;
static bool loading = true;
static bool waitForRender = false;
static bool waitForSwap = false;
static bool menuPause = true;
static FileBuffer* asyncBuffer;
static char* asyncFilename = 0;
static char* asyncAutosaveFilename = 0;
static atomic<bool> isSaving(false);
static atomic<bool> isSavingDataCopy(false);
static char* cityName = 0;
static char* nextCityName = 0;
static bool startedInstrumentation = false;
static bool presim_g = false;
vector<item> versionHistory;

char* moveCorruptAutosave();
FileBuffer* writeDataToFileBuffer(const char* filename, bool saveTestAsCity);
bool saveGameInner(FileBuffer* file, const char* name);
void loaderAutosave();

#include "autosave.cpp"

void winGame(item building) {
  if (building == 0) return;
  setLeftPanel(NoPanel);
  setSelection(SelectionBuilding, building);
  //setCameraTarget(getBuildingTop(building));
  setCameraDistance(200);
  addWinWanderers_g(building);
  setGameSpeed(1);
  setTool(0);
  setHeatMap(Pollution, false);
  playSound(_soundWin);
}

void setGameLoaded(bool val) {
  gameLoaded = val;
}

void resetRender() {
  SPDLOG_INFO("Reset Render");
  //stopRenderLoop();
  resetSkybox();
  resetParts();
  resetPartsEntities();
  resetEntities();
  resetMeshes();
  resetMeshImports();
  renderSkybox();
  SPDLOG_INFO("Reset Render Done");
}

void resetAll() {
  SPDLOG_INFO("Resetting everything\n");

  if (gameLoaded) {
    fireEvent(EventUnload);
  }

  if (cityName != 0) {
    free(cityName);
    cityName = 0;
  }
  cityName = strdup_s("NewCity");
  if (nextCityName == 0) {
    nextCityName = strdup_s("NewCity");
  }

  resetTools();
  resetGameUpdate_g();
  resetRouting();
  resetRouteBroker_g();
  resetCollisionTables();
  resetFeatures();
  resetAchievements();
  resetMessages();
  resetLand();
  resetHeatMaps();
  resetBuildings();
  resetBusinesses();
  resetGovernmentBuildings();
  resetDesigns();
  resetLots();
  resetGraph();
  resetMoney();
  resetStatistics();
  resetPillars();
  resetLanes();
  resetVehicles();
  resetTravelGroups_g();
  resetPedestrians_g();
  resetWanderers_g();
  resetPeople();
  resetWeather();
  resetTime();
  resetBoards();
  resetSelection();
  resetCities();
  resetLabels();
  resetCompass();
  resetTransit();
  resetPlans();
  resetBudgetPanel();
  resetNewspaper();
  resetStringInternTable();
  resetDesignPackages();
  softResetBuildingTextures();

  // Commented out until we push the new Tutorial system live
  // TutorialState* tState = getTutorialStatePtr();
  // tState->resetState();

  //resetConstants();
  //resetEntities();
  SPDLOG_INFO("Done resetting everything");

  gameSpeed = 2; 
  lastGameSpeed = 2;
  presim_g = false;
  versionHistory.clear();

  // Refresh Lua and constants
  refreshLua();
  initTasks_g();
}

void renderAll() {
  SPDLOG_INFO("Rendering everything");
  renderLand();
  renderLandQueue();
  rerenderGraph();
  renderPillars();
  renderVehicles();
  renderStops();
  renderCities();
  renderLabels();
  renderLots();
  loadDesignPackageTextures();
  renderBuildings();
  renderWeather();

  if (getGameMode() == ModeBuildingDesigner ||
      getGameMode() == ModeDesignOrganizer) {
    designRender();
  }

  //setUndergroundView(false);
  //renderLandQueue();
  //startRenderLoop();
  SPDLOG_INFO("Done rendering everything");
}

void initEntities() {
  SPDLOG_INFO("Initing entities");
  renderSkybox();
  initLandEntities();
  initBuildingsEntities();
  initGraphEntities();
  initStopEntities();
  initPillarsEntities();
  initVehiclesEntities();
  initCitiesEntities();
  initLabelsEntities();
  initLotsEntities();
  initWeatherEntities();
  SPDLOG_INFO("Done initing entities");
}

const char* designDirectory() {
  return "designs/";
}

const char* designExtension() {
  return ".design";
}

const char* saveDirectory(item gm) {
  if (gm == ModeBuildingDesigner ||
      gm == ModeDesignOrganizer) {
    return designDirectory();
  } else {
    return "saves/";
  }
}

const char* saveDirectory() {
  return saveDirectory(getGameMode());
}

const char* fileExtension(item gm) {
  if (gm == ModeBuildingDesigner ||
      gm == ModeDesignOrganizer) {
    return ".design";
  } else if (gm == ModeTest) {
    return ".test";
  } else {
    return ".city";
  }
}

const char* fileExtension() {
  return fileExtension(getGameMode());
}

const char* dataDirectory() {
  return "data/";
}

const char* gameRootDirectory() {
  return "./";
}

const char* imagesDirectory() {
  return "images/";
}

const char* modelsDirectory() {
  return "models/";
}

const char* modpacksDirectory() {
  return "modpacks/";
}

const char* soundDirectory() {
  return "sound/";
}

const char* texturesDirectory() {
  return "textures/";
}

void setIsPresimulating(bool val) {
  presim_g = val;
}

bool isPresimulating() {
  return presim_g;
}

GameMode getGameMode() {
  return gameMode;
}

bool getGameAccelerated() {
  return true; //gameAccelerated;
}

bool getNextGameAccelerated() {
  return nextGameAccelerated;
}

void setGameAccelerated(bool accel) {
  gameAccelerated = accel;
}

void setNextGameAccelerated(bool accel) {
  nextGameAccelerated = accel;
}

bool getNextLeftHandTraffic() {
  return nextLeftHandTraffic;
}

void setNextLeftHandTraffic(bool val) {
  nextLeftHandTraffic = val;
}

void suggestGameMode(item mode) {
  if (!gameLoaded) {
    gameMode = (GameMode)mode;
    if (gameMode < 0 || gameMode >= numGameModes) {
      gameMode = ModeGame;
    }
  }
}

const char* getCityName() {
  return cityName;
}

char** getCityNamePtr() {
  return &cityName;
}

char** getNextCityNamePtr() {
  return &nextCityName;
}

GameDifficulty getGameDifficulty() {
  return gameDifficulty;
}

GameDifficulty getNextGameDifficulty() {
  return nextGameDifficulty;
}

void setGameDifficulty(GameDifficulty diff) {
  if(diff < 0) {
    gameDifficulty = DifficultyEasy;
  } else if(diff >= numDifficulties) {
    gameDifficulty = DifficultyVeryHard;
  } else {
    gameDifficulty = diff;
  }
  SPDLOG_INFO("Game difficulty set to {} ({})",
      gameDifficulty, difficultyName[gameDifficulty]);
}

void setNextGameDifficulty(GameDifficulty diff) {
  if(diff < 0) {
    nextGameDifficulty = DifficultyEasy;
  } else if(diff >= numDifficulties) {
    nextGameDifficulty = DifficultyVeryHard;
  } else {
    nextGameDifficulty = diff;
  }
}

bool readHeader(FileBuffer* file) {
  char header[4];
  fread(&header, sizeof(char), 4, file);
  int version = fread_int(file);
  if (version > saveVersion) return false;
  if (version < 0) return false;
  if (version >= 46) decompressBuffer(file);

  file->version = version;
  if (version >= 51) {
    file->patchVersion = fread_int(file);
  }

  //SPDLOG_INFO("version {} patch {}", file->version, file->patchVersion);

  int flags = defaultFileFlags;
  if (version >= 50) {
    flags = fread_int(file);
    char* modpack = fread_string(file);
    //SPDLOG_INFO("File was saved under modpack {}", modpack);
    free(modpack);
  }

  if (version >= 45) {
    unsigned char* extraData;
    int num = fread_data(file, &extraData);
    free(extraData);
  }

  if (version >= 13) {
    item gameMode = fread_item(file, version);
    //SPDLOG_INFO("gamemode {}", gameMode);
  }

  if (version >= 14) {
    char* saveFilename = fread_string(file);
    //SPDLOG_INFO("saveFilename {}", saveFilename);
    free(saveFilename);
  }

  return true;
}

void postLoad(bool isNew) {
  resetCamera();
  initTools();
  postInitLua();
  rebuildStats();
  updateMoney(0);
}

bool loadGame(char* filename, bool isAutosave) {
  FILE *fileHandle;
  string filePath = saveDirectory();
  filePath = filePath + filename + fileExtension();

  bool designer = getGameMode() == ModeBuildingDesigner;
  if (designer) {
    filePath = fixDesignerPath(filePath);
  }

  resetAll();
  FileBuffer buf = readFromFile(filePath.c_str());
  FileBuffer* file = &buf;

  if (file->length <= 0) {
    if (!isAutosave) {
      handleError(sprintf_o("Failed to load file %s", filePath.c_str()));
    }
    freeBuffer(file);
    return false;
  }

  startSegment(file, "Header");
  char header[4];
  fread(&header, sizeof(char), 4, file);
  int version = fread_int(file);
  if (version > saveVersion) {
    char* corrupt = moveCorruptAutosave();
    if (corrupt == 0) {
      handleError("This save file is from a newer version"
          " (0.0.%d.x) of NewCity.\nFile %s will be overwritten.",
          version, filePath.c_str());
    } else {
      handleError("This save file is from a newer version"
          " (0.0.%d.x) of NewCity.\nFile %s moved to %s.",
          version, filePath.c_str(), corrupt);
      free(corrupt);
    }
    freeBuffer(file);
    return false;
  }

  SPDLOG_INFO("Reading file {} version {}", filePath, version);

  if (version < 0) {
    char* corrupt = moveCorruptAutosave();
    if (corrupt == 0) {
      handleError("Corrupt file %s\nwill be overwritten.", filePath.c_str());
    } else {
      handleError("Corrupt file %s\nmoved to %s", filePath.c_str(), corrupt);
      free(corrupt);
    }
    freeBuffer(file);
    return false;
  }
  if (version >= 46) {
    decompressBuffer(file);
  }

  file->version = version;
  if (version >= 51) {
    file->patchVersion = fread_int(file);
  }

  int flags = defaultFileFlags;

  if (version >= 50) {
    flags = fread_int(file);
    char* modpack = fread_string(file);
    SPDLOG_INFO("File was saved under modpack {}", modpack);
    free(modpack);
  }

  // Check flags for whether the loading save is Accelerated Mode
  // Default is "false"
  bool accel = false;
  if(version >= 54) {
    accel = (flags & _fileAcceleratedStart);
  }
  nextGameAccelerated = gameAccelerated = accel;

  nextLeftHandTraffic = (flags & _fileLeftHandTraffic);
  setLeftHandTraffic(nextLeftHandTraffic);

  // Setting default difficulty of medium for old saves, or pulling the 
  // save difficulty bit from the flags for saves v51 or newer
  int difficulty;
  if(version <= 50) {
    difficulty = GameDifficulty::DifficultyMedium;
  } else {
    difficulty = (flags & _fileDifficultyMask) >> _fileDifficultyShift;
    if(difficulty < 0 || difficulty >= numDifficulties) {
      std::string err = "Invalid difficulty fetched from file flags, defaulting to medium";
      SPDLOG_INFO(err);
      consolePrintError(consoleGetError(ConStatus::FILE_READ_ERROR), err);
      difficulty = GameDifficulty::DifficultyMedium;
    }
  }
  gameDifficulty = (GameDifficulty)difficulty;
  nextGameDifficulty = gameDifficulty;
  SPDLOG_INFO("flags {} {}", flags, gameDifficulty);
  refreshLua();

  if (version >= 45) {
    unsigned char* extraData;
    int num = fread_data(file, &extraData);
    free(extraData);
  }

  if (designer) {
    if (version >= 13) {
      fread_item(file, version); // Game Mode
    }
    if (version >= 14) {
      char* saveFilename = fread_string(file);
      setSaveFilename(saveFilename);
    }
    readDesign(file, version, 1, filename);

  } else {
    gameMode = (GameMode)fread_item(file, version);

    if (version >= 13) {
      char* saveFilename = fread_string(file);
      setSaveFilename(saveFilename);
    }

    if (version >= 58) {
      fread_item_vector(file, &versionHistory, version);
    }

    if (cityName != 0) {
      free(cityName);
      cityName = 0;
    }
    if (version >= 52) {
      cityName = fread_string(file);
    } else {
      cityName = strdup_s(getSaveFilename());
    }

    startSegment(file, "Time, Weather, Money, Achievements");
    readTime(file, version);
    if (version >= 9) {
      readWeather(file, version);
    }
    readMoney(file, version);
    readAchievements(file, version);

    startSegment(file, "Land");
    readLand(file, version);
    if (version >= 15) {
      startSegment(file, "Heatmaps");
      readHeatMaps(file, version);
    } else {
      initHeatMaps();
    }
    initCollisionTables();
    initRouteBroker_g();

    if (version >= 52) {
      startSegment(file, "Statistics");
      readStatistics(file, version);
      startSegment(file, "Labels");
      readLabels(file);
    } else {
      initEcons();
    }

    if (version >= 3) {
      startSegment(file, "Cities");
      readCities(file, version);
    }
    startSegment(file, "Designs");
    readDesigns(file, version);
    startSegment(file, "Buildings");
    readBuildings(file, version);
    if (version >= 32) {
      readGovernmentBuildings(file, version);
    }

    startSegment(file, "Graph");
    readPillars(file, version);
    readGraph(file, version);
    readLaneBlocks(file, version);
    startSegment(file, "Vehicles");
    readVehicles(file, version);
    readTravelGroups_g(file);
    startSegment(file, "Transit");
    readTransit_g(file);
    readStops_g(file);
    readPlans(file, version);

    startSegment(file, "People");
    readPeople(file, version);
    startSegment(file, "Businesses");
    readBusinesses(file, version);
    startSegment(file, "Boards");
    readBoards(file, version);
    startSegment(file, "Lots");
    readLots(file, version);

    startSegment(file, "UI");
    readMessages(file, version);
    if (version < 52) {
      readStatistics(file, version);
    }
    readEconomyPanel(file, version);
    startSegment(file, "Routes");
    readRouter(file, version);
    startSegment(file, "Newspaper");
    readNewspaper(file);

    startSegment(file, "Terminus");
    if (version < 51) vehiclePassengersToTravelGroups51_g();
    initAchievements();
    swapVehiclesBack();
    if (version <= 40) repairElevators();
    if (version < 52) nameBuildings();

    if (version < 52 || (version == 52 && file->patchVersion <= 1)) {
      removeAllTravelGroups_g();
      removeAllVehicles_g();
    }

    //if (version < 52) {
      //generateCities();
    //}
  }

  postLoad(false);

  SPDLOG_INFO("Reading {} successful", filePath);
  char* dateTime = printDateTimeString(getCurrentDateTime());
  SPDLOG_INFO("Population {} Date {} LOC {}",
      numPeople(ourCityEconNdx()), dateTime, getCredit());
  free(dateTime);
  freeBuffer(file);
  return true;
}

void writeInner(char* filename, FileBuffer* buffer) {
  makeDirectoryForFilename(filename);

  FILE *fileHandle = fopen(filename, "wb");

  if (fileHandle != 0) {
    writeToFile(buffer, fileHandle);
    fclose(fileHandle);
    SPDLOG_INFO("Writing {} successful", filename);
  } else {
    SPDLOG_ERROR(sprintf_o("Failed to save file %s, %d", filename, errno));
  }
}

void compressAndWrite() {
  isSaving = true;
  FileBuffer* buffer = asyncBuffer;
  char* filename = asyncFilename;
  asyncFilename = 0;
  if(!(buffer->flags & _fileCompressed)) {
    compressBuffer(buffer);
  }

  writeInner(filename, buffer);
  free(filename);

  if (asyncAutosaveFilename != 0) {
    filename = asyncAutosaveFilename;
    asyncAutosaveFilename = 0;
    writeInner(filename, buffer);
    free(filename);
  }

  freeBuffer(buffer);
  isSaving = false;
  isSavingDataCopy = false;
}

void asyncCompressAndWrite(FileBuffer* buffer, char* filename,
    char* autosaveFilename) {
  while (isSavingDataCopy) {
    sleepMilliseconds(10);
  }
  isSavingDataCopy = true;
  asyncBuffer = buffer;
  asyncFilename = filename;
  asyncAutosaveFilename = autosaveFilename;
  startThread("SAVE THREAD", compressAndWrite);
}

FileBuffer* writeDataToFileBuffer(const char* filename, bool saveTestAsCity) {
  if (gameMode == ModeBuildingDesigner) {
    return writeDesignToFileBuffer(filename);
  }

  FileBuffer* file = makeFileBufferPtr();
  file->version = saveVersion;
  file->patchVersion = patchVersion;
  fputs("CITY", file);
  fwrite_int(file, saveVersion);
  fwrite_int(file, 0); // For uncompressed size
  fwrite_int(file, patchVersion);

  int flags = defaultFileFlags;
  flags |= ((int)gameDifficulty) << _fileDifficultyShift;
  if (gameAccelerated) {
    flags |= _fileAcceleratedStart;
  }

  if (isLeftHandTraffic()) {
    flags |= _fileLeftHandTraffic;
  }

  fwrite_int(file, flags);
  fwrite_string(file, getMod());

  unsigned char* extraData =
    (unsigned char*) calloc(1024, sizeof(unsigned char));
  fwrite_data(file, extraData, 1024);
  free(extraData);

  item gameModeToSave = gameMode;
  if (saveTestAsCity && gameMode == ModeTest) gameModeToSave = ModeGame;
  fwrite_item(file, gameModeToSave);
  fwrite_string(file, getSaveFilename());

  if(gameMode != ModeBuildingDesigner) {

    item histSize = versionHistory.size();
    if (histSize > 0 &&
        versionHistory[histSize-1] != combinedVersionNumber()) {
      versionHistory.push_back(combinedVersionNumber());
    }
    fwrite_item_vector(file, &versionHistory);

    fwrite_string(file, cityName);
    writeTime(file);
    writeWeather(file);
    writeMoney(file);
    writeAchievements(file);
    writeLand(file);
    writeHeatMaps(file);
    writeStatistics(file);
    writeLabels(file);
    writeCities(file);
    writeDesigns(file);
    writeBuildings(file);
    writeGovernmentBuildings(file);
    writePillars(file);
    writeGraph(file);
    writeLaneBlocks(file);
    writeVehicles(file);
    writeTravelGroups_g(file);
    writeTransit_g(file);
    writeStops_g(file);
    writePlans(file);
    writePeople(file);
    writeBusinesses(file);
    writeBoards(file);
    writeLots(file);
    writeMessages(file);
    writeEconomyPanel(file);
    writeRouter(file);
    writeNewspaper(file);
  }

  return file;
}

char* getSaveFilename(const char* name, const char* ext) {
  if (getGameMode() == ModeBuildingDesigner ||
      getGameMode() == ModeDesignOrganizer) {
    string filePath = "designs/";
    filePath = filePath + name + "/design" + ext;
    return strdup_s(lookupSave(filePath).c_str());
  } else {
    return sprintf_o("%s%s%s",
      saveDirectory(), name, ext);
  }
}

char* getSaveFilename(const char* name, bool saveTestAsCity) {
  const char* ext = gameMode == ModeTest && saveTestAsCity ?
    ".city" : fileExtension();
  return getSaveFilename(name, ext);
}

char* getSaveFilename(const char* name) {
  return getSaveFilename(name, false);
}

bool saveGameInner(FileBuffer* file, const char* name, bool saveTestAsCity) {
  char* filename = getSaveFilename(name, saveTestAsCity);
  char* captureFilename = sprintf_o("%s.png", filename);
  char* autosaveFn = 0;
  bool autosave = streql(name, autosaveFilename());
  if (!autosave) {
    autosaveFn = getSaveFilename(autosaveFilename());
    SPDLOG_INFO("autosaveFn {}", autosaveFn);
    playSound(_soundSaveLoad);
  }
  fireEvent(EventSave);
  asyncCompressAndWrite(file, filename, autosaveFn);

  // Capture an image
  if (!autosave) {
    renderCapture_g();
    captureFramebuffer(CaptureFramebuffer, captureFilename);
  }
  free(captureFilename);

  return true;
}

bool saveGameInner(FileBuffer* file, const char* name) {
  return saveGameInner(file, name, false);
}

bool saveGame(char* filename, bool saveTestAsCity) {
  if (nextCityName != 0) free(nextCityName);
  nextCityName = strdup_s("NewCity");

  bool designResaved = false;
  if (getGameMode() == ModeBuildingDesigner) {
    Design* d = getSelectedDesign();
    SPDLOG_INFO("startsWith({}, {}) == {}", filename, d->name, startsWith(filename, d->name));
    if (!startsWith(filename, d->name)) {
      designResaved = true;
    }
  }

  FileBuffer* file = writeDataToFileBuffer(filename, saveTestAsCity);
  SPDLOG_INFO("saveGame {}", saveTestAsCity);

  if (designResaved) {
    designPackageResaved();
  }

  return saveGameInner(file, filename, saveTestAsCity);
}

bool saveGame(char* filename) {
  return saveGame(filename, false);
}

void newBuildingDesigner() {
  SPDLOG_INFO("New Design");
  int landSize = 5;
  gameMode = ModeBuildingDesigner;
  generateFlatLand(landSize);
  initHeatMaps();
  initCollisionTables();
  initRouteBroker_g();
  initEcons();
  resetCamera();

  Configuration config;
  config.numLanes = 3;
  config.speedLimit = 3;
  config.type = ConfigTypeRoad;
  config.strategy = StopSignStrategy;
  config.flags = _configMedian;

  int chunkSize = getChunkSize();
  float r = tileSize*(chunkSize*landSize)/2;
  int z = c(CSuperflatHeight)+1;
  item in0 = addNode(vec3(tileSize*5, 0, z), config);
  item in1 = addNode(vec3(tileSize*5, r*2, z), config);
  item edge = addEdge(in0, in1, config);
  complete(edge, config);

  vec3 bLoc = pointOnLand(vec3(tileSize*6, r+tileSize*.5f, 0));
  Line l = getLine(edge);
  vec3 intersection = nearestPointOnLine(bLoc, l);
  vec3 normal = bLoc - intersection;
  normal.z = 0;
  vec3 unorm = normalize(normal);
  normal = unorm * c(CBuildDistance);
  bLoc = intersection + normal;
  bLoc = pointOnLand(bLoc);

  if (sizeDesigns() < 1) addDesign();
  Design* d = getDesign(1);
  vec3 prevSize = d->size;
  d->size = vec3(tileSize*24*2, tileSize*24*2, 0);
  addBuilding(bLoc, vec3(1, 0, 0), 1, d->zone);
  d->size = prevSize;
  setSpawnProbGlobal(0.1f);

  initTools();
  postInitLua();
  pushDesignerUndoHistory();
}

void newGame() {
  resetAll();

  if (gameMode == ModeBuildingDesigner) {
    setSaveFilename(strdup_s("New Design"));
    newBuildingDesigner();
    return;
  }

  SPDLOG_INFO("Generating New City");
  gameDifficulty = nextGameDifficulty;
  gameAccelerated = nextGameAccelerated;
  setLeftHandTraffic(nextLeftHandTraffic);
  setSaveFilename(strdup_s(nextCityName));
  if (cityName != 0) {
    free(cityName);
    cityName = 0;
  }
  cityName = strdup_s(nextCityName);
  refreshLua();
  initDesigns();

  //if (gameMode == ModeTest) {
    //generateFlatLand(10);
  //} else {
    generateLand();
  //}

  initEcons();
  initCollisionTables();
  initRouteBroker_g();

  initHeatMaps();
  initLandValue();

  if (getLandConfig().flags & _landGenerateNeighbors) {
    generateCities();
  }

  initTools();
  initAchievements();

  postLoad(true);
  fireEvent(EventNewGame);
}

void loaderSystem() {
  if (loadMode == Autosaving) {
    SPDLOG_INFO("Loader autosave starting");
    loaderText = "Autosaving";
    FileBuffer* file = writeDataToFileBuffer(0, false);
    autosave(file);
    loadMode = GameReady;
    SPDLOG_INFO("Loader autosave done");
    return;
  }

  if (gameLoaded) {
    FileBuffer* file = writeDataToFileBuffer(0, false);
    autosave(file);
  }

  LoadMode lm = loadMode;
  loadMode = GameLoading;
  gameMode = gameModeTarget;
  bool wasNewGame = false;

  if (gameMode == ModeDesignOrganizer) {
    resetAll();
    startDesignOrganizer();

  } else if (lm == LoadTargetNew) {
    newGame();
    wasNewGame = true;

  } else {

    if (lm == LoadTargetAutosave) {
      if (filenameTarget) free(filenameTarget);
      if (gameMode == ModeBuildingDesigner) {
        filenameTarget = strdup_s("autosave/design");
      } else {
        filenameTarget = strdup_s(autosaveFilename());
      }
    }

    bool loaded = false;
    if (gameMode == ModeBuildingDesigner) {
      loaded = loadDesign(filenameTarget, lm == LoadTargetAutosave);
      newBuildingDesigner();
    } else {
      loaded = loadGame(filenameTarget, lm == LoadTargetAutosave);
    }
    timeSinceAutosave = 0;
    free(filenameTarget);
    if (!loaded) {
      loaderText = "Generating";
      if (gameMode != ModeGame) {
        getNextLandConfig()->flags = _landSuperflat;
        getNextLandConfig()->landSize = 10;
      }
      newGame();
      wasNewGame = true;
    }
  }

  /*
  if (!hasSeenLogUploadMessage()) {
    addMessage(InfoMessage, LogUploadNotification);
    setSeenLogUploadMessage();
  }
  */

  gameLoaded = true;
  nextGameDifficulty = gameDifficulty;
  loadMode = GameUnrendered;
  loaderText = "Drawing";
  waitForRender = true;
  while (waitForRender) {
    std::this_thread::sleep_for(std::chrono::microseconds(5));
  }

  renderAll();
  finishLanes();
  setTravelGroupStats();

  if (getGameMode() != ModeGame) {
    setAllGovernmentBuildingsAvailable();
  }

  if (wasNewGame && countNeighbors() > 0) {
    loadMode = GameSimulating;
    loaderText = "Simulating";
    runPreSimulation_g();
  }

  if (getGameMode() == ModeBuildingDesigner) {
    completeBuilding(1);
  }

  fireEvent(EventLoad);

  // Set game speed
  item newGameSpeed = 2;
  if(gameMode == ModeBuildingDesigner || gameMode == ModeDesignOrganizer) {
    newGameSpeed = 1;
  } else if(getOptionsStartPaused()) {
    newGameSpeed = 0;
  } else {
    newGameSpeed = 2;
  }
  setGameSpeed(newGameSpeed);

  loadMode = GameReady;
}

const char* getLoaderText() {
  return loaderText;
}

LoadMode getLoadMode() {
  return loadMode;
}

void setGameSpeed(item speed) {
  gameSpeed = speed;
  if (speed != 0) {
    lastGameSpeed = speed;
  }
}

item getGameSpeed() {
  return gameSpeed;
}

item getUnpausedSpeed() {
  return lastGameSpeed;
}

bool isGamePaused() {
  return menuPause || gameSpeed == 0;
}

bool isGameLoaded() {
  return gameLoaded;
}

bool isGameLoading() {
  return loading;
}

bool isGameWaitingForSwap() {
  return waitForSwap;
}

bool isGameSaving() {
  return isSaving;
}

void toggleGamePause() {
  if (!menuPause) {
    if (gameSpeed == 0) {
      gameSpeed = lastGameSpeed;
    } else {
      lastGameSpeed = gameSpeed;
      gameSpeed = 0;
    }
  }
}

void setGameMenuPause(bool pause) {
  menuPause = pause;
}

void updateGame() {
  if (waitForSwap) {
    waitForRender = false;
    waitForSwap = false;
  } else if (waitForRender) {
    initEntities();
    resetCamera();
    waitForSwap = true;
  }

  //SPDLOG_INFO("updateGame: {}", loadMode);
  if (loadMode == GameReady) {

    loading = false;
    updateGameInner_g();

    /*
    #ifdef __linux__
      if (!startedInstrumentation && getCameraTime() > 90) {
        CALLGRIND_START_INSTRUMENTATION;
        //CALLGRIND_TOGGLE_COLLECT;
        startedInstrumentation = true;
      }
    #endif
    */

  } else if (loadMode == ErrorLoad) {
    loading = true;
    updateInput(0);
    renderErrorLoadPanel();
    collectDrawBuffer();

  } else {
    loading = true;
    if (!loaderThreadStarted) {
      if (loadMode != Autosaving) resetRender();
      startThread("LOADER THREAD", loaderSystem);
      loaderThreadStarted = true;
    }

    //positionCamera();
    renderUI();
    collectDrawBuffer();
  }
}

void initGame() {
  initNames();
  renderSkybox();
  resetCamera();
  readBlueprints();

  // Set TutorialState according to the hasCompletedTutorial() flag
  TutorialState* ptr = getTutorialStatePtr();
  ptr->resetState();
  ptr->setTutorialActive(hasCompletedTutorial());

  if (loadMode != ErrorLoad) {
    gameLoad(LoadTargetAutosave, gameMode, 0);
  }
}

void gameLoad(LoadMode lm, GameMode gm, char* filename) {
  // Write the last game mode to the options file
  if (gameMode != ModeBuildingDesigner) setWasTestMode(gameMode == ModeTest);
  writeOptions();

  bool isDO = gm == ModeDesignOrganizer;
  loaderThreadStarted = false;
  loadMode = lm;
  gameModeTarget = gm;
  setSaveLoadModeTab(gm);
  filenameTarget = filename;
  loaderText = lm == LoadTargetNew || isDO ? "Generating" : "Loading";
}

void loaderAutosave() {
  loaderThreadStarted = false;
  loadMode = Autosaving;
  loaderText = "Autosaving";
}

void doErrorLoad() {
  SPDLOG_WARN("Doing Error Load");
  loadMode = ErrorLoad;
}

void clearErrorLoad() {
  SPDLOG_WARN("Error Load: Continuing");
  gameLoad(LoadTargetAutosave, gameMode, 0);
}

char* moveCorruptAutosave() {
  //char* autosaveFile = sprintf_o("%s%s%s",
      //saveDirectory(), autosaveFilename(), fileExtension());
  char* autosaveFile = getSaveFilename(autosaveFilename());
  for (int i = 0; i < c(CMaxCorruptSaves); i ++) {
    char* fn = sprintf_o("corrupt.%d", i);
    char* filename = getSaveFilename(fn);
    free(fn);
    //char* filename = sprintf_o("%scorrupt.%d%s",
        //saveDirectory(), i, fileExtension());

    if (fileExists(filename)) {
      free(filename);

    } else {
      SPDLOG_INFO("Moving {} to {}", autosaveFile, filename);
      rename(autosaveFile, filename);
      return filename;
    }
  }

  return 0;
}

void doErrorLoadNewGame() {
  char* val = moveCorruptAutosave();
  if (val != 0) free(val);
  SPDLOG_WARN("Error Load: Starting New Game");
  gameLoad(LoadTargetNew,
      c(CStartBuildingDesigner) ? ModeBuildingDesigner : ModeGame, 0);
}

