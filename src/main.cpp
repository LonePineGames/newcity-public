#include "main.hpp"

#include "building/buildingTexture.hpp"
#include "building/renderBuilding.hpp"
#include "draw/buffer.hpp"
#include "draw/camera.hpp"
#include "draw/culler.hpp"
#include "draw/framebuffer.hpp"
#include "draw/texture.hpp"
#include "error.hpp"
#include "game/game.hpp"
#include "heatmap.hpp"
#include "import/mesh-import.hpp"
#include "icons.hpp"
#include "input.hpp"
#include "land.hpp"
#include "log.hpp"
#include "name.hpp"
#include "option.hpp"
#include "platform/lua.hpp"
#include "platform/mod.hpp"
#include "serialize.hpp"
#include "sound.hpp"
#include "string.hpp"
#include "string_proxy.hpp"
#include "route/router.hpp"
#include "test.hpp"
#include "thread.hpp"
#include "vehicle/update.hpp"
#include "weather.hpp"

#include "parts/part.hpp"
#include "parts/root.hpp"

#include "spdlog/spdlog.h"

#ifdef INCLUDE_STEAM
  #include "steam/steamwrapper.hpp"
  #include "steam/steamws_core.hpp"
#endif

#ifdef WIN32
  #include <windows.h>
  extern "C"
  {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
  }
#endif

//#ifdef __linux__
  //#include <valgrind/callgrind.h>
//#endif

static bool shouldContinue = true;
static bool shouldRestart = true;
static double lastDuration = 1;
static double lastTime = 0;
static double fps = 60;
static double frameRateCap = -1;
static double targetDuration = 0; //1./frameRateCap;
static double frameDurationAccum = 0;

static bool multithreading = true;
static mutex syncMutex;
static mutex drawMutex;
static condition_variable syncCondition;
static condition_variable drawCondition;
static bool gameUpdateReady = false;
static bool gameUpdateDone = false;
static std::atomic<bool> isDrawing(false);
std::atomic<bool> renderLagging(false);
std::atomic<bool> gameLagging(false);
std::atomic<float> lastDurationError(0);

void endGame() {
  shouldContinue = false;
  shouldRestart = false;
}

void restartGame() {
  shouldContinue = false;
  shouldRestart = true;
}

int getFPSCap() {
  return frameRateCap;
}

bool isRenderLagging() {
  return renderLagging;
}

bool isGameLagging() {
  return gameLagging;
}

float getLastDurationError() {
  return lastDurationError;
}

void setFPSCap(int cap) {
  frameRateCap = cap;
  if (cap < 0) {
    targetDuration = 1./400.f; // max 400 fps
  } else {
    targetDuration = 1./frameRateCap;
  }
}

double getTargetFrameDuration_g() {
  return targetDuration;
}

void swap() {
  swapMeshImports();
  swapBuildingTextures();
  swapTextures();
  swapSound();
  swapCameras();
  swapHeatMaps();
  swapDrawBuffers();
  swapMeshCommands();
  swapFramebuffers();
  runMeshCommands();
}

void updateDuration() {
  bool blameRender = gameUpdateDone;
  double currentTime = glfwGetTime();
  double duration = double(currentTime - lastTime);
  lastTime = currentTime;
  lastDuration = duration;

  renderLagging = false;
  gameLagging = false;
  if (duration > targetDuration * .5) {
    const char* bounding = blameRender ? "render" : "game";
    renderLagging = blameRender;
    gameLagging = !blameRender;
    lastDurationError = (duration-targetDuration)/targetDuration;
    if (duration > 0.1) {
      lastDuration = 0.1;
      SPDLOG_WARN("{}ms frame - {} bound - {} fps",
          int(duration*1000), bounding, int(fps));
    }
  }

  /*
  if (c(CEnableFPSCap)) {
    while (duration < targetDuration) {
      int sleepTime = 1000000.*(targetDuration-duration);
      std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));

      currentTime = glfwGetTime();
      duration = double(currentTime - lastTime);
      lastTime = currentTime;
      lastDuration = duration;
    }
  }
  */

  double fpsRaw = 1./duration;
  fps = mix(fps, fpsRaw, 0.1);
}

void updateGameSync() {
  unique_lock<mutex> syncLock(syncMutex);
  syncCondition.wait(syncLock, []{
    return gameUpdateReady && shouldContinue;
  });
  if (!shouldContinue) {
    return;
  }

  gameUpdateReady = false;

  updateGame();

  gameUpdateDone = true;
  syncLock.unlock();
  syncCondition.notify_one();
}

#ifdef INCLUDE_STEAM
void steamLoop() {
  while(shouldContinue && steam_isActive()) {
    steam_tick();
    steamws_tick();
    sleepMilliseconds(steam_tickrate*1000); // 1000 milliseconds in a second
  }
}
#endif

void gameLoop() {
  initGame();
  //#ifdef __linux__
    //CALLGRIND_START_INSTRUMENTATION;
    //CALLGRIND_TOGGLE_COLLECT;
  //#endif
  while (shouldContinue) {
    updateGameSync();
  }
  //#ifdef __linux__
    //CALLGRIND_STOP_INSTRUMENTATION;
  //#endif
}

void startGameUpdate() {
  {
    lock_guard<mutex> syncLock(syncMutex);
    gameUpdateReady = true;
  }
  syncCondition.notify_one();
}

void waitForGameUpdate() {
  unique_lock<mutex> syncLock(syncMutex);
  syncCondition.wait(syncLock, []{
    return gameUpdateDone;
  });
  gameUpdateDone = false;
}

void drawSync() {
  unique_lock<mutex> drawLock(drawMutex);
  drawCondition.wait(drawLock, []{
    return !isDrawing;
  });
  isDrawing = true;

  draw();

  isDrawing = false;
  drawLock.unlock();
  drawCondition.notify_one();
}

void renderMap_g() {
  bool wasUnderground = isUndergroundView();
  if (wasUnderground) setUndergroundView(false);
  setNextPerspective_g(SatMapPerspective);
  collectDrawBuffer();
  if (wasUnderground) setUndergroundView(true);
}

void renderCapture_g() {
  bool wasUnderground = isUndergroundView();
  if (wasUnderground) setUndergroundView(false);
  if (getGameMode() == ModeBuildingDesigner) {
    setDesignRender(false);
  }
  setNextPerspective_g(CapturePerspective);
  collectDrawBuffer();
  if (wasUnderground) setUndergroundView(true);
  if (getGameMode() == ModeBuildingDesigner) {
    //setDesignRender(true);
  }
}

void quickDraw() {
  /*
  collectDrawBuffer();
  swap();
  draw();
  */
  //drawSync();
}

int mainLoop() {
  initLua();
  loadConstantsFromLua();
  // Load autosave values after Lua load
  initAutosave();
  readOptions();
  
  initKeymap(); // Need to init keymap before reading input file
  if (!readInputFile(true)) {
    SPDLOG_ERROR("Error reading input file on startup!");
  }

  if (initGraphics() < 0) {
    return -1;
  }

  setTime(0.5);
  Weather w = getWeather();
  w.clouds = 0.1;
  w.percipitation = 0;
  setWeather(w);

  loadTextures();
  initIconsMap();
  parseFont();
  initDrawBuffers();
  positionCamera();
  renderUI();
  // quickDraw(); supersoup - Function is all commented out, so commenting out here
  setDefaultLandConfig();
  loadBuildingTextures();
  #ifdef INCLUDE_STEAM
    steamws_initWorkshop(); // Initializes Workshop folders and files
  #endif

  nameThread("MAIN THREAD");

  if (multithreading) {
    startThread("GAME THREAD", gameLoop);
    if (c(CCullingAlgo) >= 2) {
      startThread("CULLER THREAD", cullerLoop_c);
    }
    startThread("SOUND THREAD", soundLoop);
    startThread("HEATMAP THREAD", heatMapLoop);
    startThread("VEHICLE THREAD", vehicleLoop);
    #ifdef INCLUDE_STEAM
      startThread("STEAM THREAD", steamLoop);
    #endif
    //startThread("LOG UPLOAD THREAD", sendLogFile);

    initRoutingThreads();
  } else {
    initGame();
  }

  initInput();
  lastTime = glfwGetTime();

  while(shouldContinue) {
    if (multithreading) {
      swap();
      startGameUpdate();
      drawSync();
      updateDuration();
      collectInput();
      waitForGameUpdate();

    } else {
      updateGame();
      swap();
      draw();
      //updateSound();
      collectInput();
      updateDuration();
    }
  }

  syncCondition.notify_one();
  killHeatMaps();
  resetRender();
  swapMeshCommands();
  runMeshCommands();
  resetGraphics();

  if (getErrorMessage() == 0) {
    autosave();
  }

  killVehicles();
  killRouting();
  killCuller();
  resetNames();
  resetSound();
  resetBuildingTextures();
  resetDrawBuffers();
  resetLua();
  resetConstants();
  resetLand();
  setGameLoaded(false);
  writeOptions();
  //resetAll();

  #ifdef INCLUDE_STEAM
    steamws_shutdown();
    steam_shutdown();
  #endif

  sleepMilliseconds(2000);

  while(isGameSaving()) {
    sleepMilliseconds(10);
  }
  while(!isRoutingDone()) {
    sleepMilliseconds(10);
  }

  return 0;
}

float getFrameDuration() {
  return lastDuration;
}

float getCameraTime() {
  return lastTime;
}

float getFPS() {
  return fps;
}

bool isMultithreading() {
  return multithreading;
}

int start() {
  #ifdef _WIN32
    _setmaxstdio(1024);
  #endif

  initLogging();
  initErrorHandling();
  readOptions();
  sendLogFile();

  #ifdef INCLUDE_STEAM
    steam_init();
    steam_spdlogInfo();
  #endif

  //try {

    do {
      shouldContinue = true;
      mainLoop();
      setModNext();
    } while (shouldRestart);

    removeSentinelFile();
    return 0;

  //} catch(const char* e) {
    //handleError(e);
    //throw e;
  //}

  return -1;
}

#ifdef _WIN32
int CALLBACK WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nShowCmd
) {
  return start();
}

#endif

int main(int argc, char** argv) {
  //return luaInterpreter();
  return start();
}
