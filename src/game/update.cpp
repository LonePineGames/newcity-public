#include "update.hpp"

#include "achievement.hpp"
#include "game.hpp"
#include "task.hpp"

#include "../building/building.hpp"
#include "../business.hpp"
#include "../city.hpp"
#include "../draw/buffer.hpp"
#include "../draw/camera.hpp"
#include "../economy.hpp"
#include "../heatmap.hpp"
#include "../input.hpp"
#include "../money.hpp"
#include "../option.hpp"
#include "../parts/part.hpp"
#include "../person.hpp"
#include "../platform/event.hpp"
#include "../route/router.hpp"
#include "../thread.hpp"
#include "../tutorial.hpp"
#include "../vehicle/travelGroup.hpp"
#include "../vehicle/update.hpp"
#include "../weather.hpp"

#include "spdlog/spdlog.h"

static double gameUpdateToGo = 0;
static double smoothedGameSpeedup = 0;

const static float speedupBySpeed[] = {
  0, .1, 1, 5, 30, 120, 600, 1200
};

float lastTime = 0;
float lastFrameTime = 0;
const char* lastStage = "";
float timeSinceMap = 0;

void resetGameUpdate_g() {
  gameUpdateToGo = 0;
  smoothedGameSpeedup = 0;
  timeSinceMap = c(CMapRenderInterval) - 5;
}

void logStage(const char* stage) {
  if (!c(CLogUpdateTime)) return;
  float time = glfwGetTime();
  float timeDiff = time - lastTime;
  int timeMS = timeDiff * 10000;
  if (timeMS > 0) {
    SPDLOG_INFO("{}ms {}", timeMS/10.f, lastStage);
  }
  lastStage = stage;
  lastTime = time;
}

item suggestEffectiveGameSpeed() {
  item gameSpeed = getGameSpeed();
  item effectiveGameSpeed = gameSpeed;
  item routerQueueSize = getStatistic(ourCityEconNdx(), RouterQueueSize);
  item vehicleUpdatesPending = getVehicleUpdatesPending();
  item result = 1;
  for (; result < gameSpeed; result++) {
    if (vehicleUpdatesPending > 5*(8-result)) break;
    item limit = c((IntConstant)(CRouterSlowGameThreshold1+result-1));
    if (limit < routerQueueSize) break;
    if (countUpdateTasksPending_g() > (8-result)*10) break;
    //if (gameUpdateToGo > c(CMaxPendingGameUpdate)*(8-result)*0.2) break;
  }
  return result;
}

item getEffectiveGameSpeed() {
  float bestDiff = FLT_MAX;
  item bestVal = getGameSpeed();
  for (int i = 0; i < 8; i++) {
    float diff = abs(smoothedGameSpeedup - speedupBySpeed[i]);
    if (diff < bestDiff) {
      bestDiff = diff;
      bestVal = i;
    }
  }
  return bestVal;
}

void updateRender(double duration) {
  renderUI();

  timeSinceMap += duration;
  if (timeSinceMap > c(CMapRenderInterval)) {
    //logStage("renderMap_g");
    timeSinceMap = 0;
    renderMap_g();
  } else {
    //logStage("collectDrawBuffer");
    //collectDrawBuffer();
    startEntityCulling_g();
  }
}

void updateGameInner_g() {
  double duration = getFrameDuration();
  float nextFrameTime = glfwGetTime();
  float frameDur = nextFrameTime - lastFrameTime;
  lastFrameTime = nextFrameTime;

  queueMatchingTasks_g(_taskFrame, duration);

  if (!isGamePaused()) {
    adjustStat(ourCityEconNdx(), CPUTime, duration);
    item effectiveGameSpeed = suggestEffectiveGameSpeed();
    double targetSpeedup = speedupBySpeed[effectiveGameSpeed];
    if (c(CGameSpeedSmoothing) == 0) {
      smoothedGameSpeedup = targetSpeedup;
    } else if (smoothedGameSpeedup < 0.1) {
      smoothedGameSpeedup = 0.1;

    } else {
      float d = duration / c(CGameSpeedSmoothing);

      // Harmonic mean
      //smoothedGameSpeedup = (1+d)/(1/smoothedGameSpeedup + d/targetSpeedup);

      // Geometric mean
      smoothedGameSpeedup = exp(
          (log(smoothedGameSpeedup) + d*log(targetSpeedup))/(1+d));

      // Aritmetic mean
      //smoothedGameSpeedup = mix(smoothedGameSpeedup, targetSpeedup,
          //duration / c(CGameSpeedSmoothing));
    }

    double gameDur = duration * smoothedGameSpeedup;
    gameUpdateToGo += gameDur;
    float utime = c(CGameUpdateTime);

    if (getGameSpeed() <= 3 && gameUpdateToGo > c(CMaxPendingGameUpdate)) {
      gameUpdateToGo = c(CMaxPendingGameUpdate);
    }

    while (gameUpdateToGo > utime && countUpdateTasksPending_g() < 80) {
      gameUpdateToGo -= utime;
      queueMatchingTasks_g(_taskUpdate, utime);
    }

    queueMatchingTasks_g(_taskFrameUpdate, gameDur);
  }

  runTaskQueue_g();
}

void onePreSimGameUpdate_g(double duration) {
  //updateInput(duration);
  updateCamera(duration);

  updateTime(duration);
  SPDLOG_INFO("Simulating {}", printDateTimeString(getCurrentDateTime()));
  updateBuildings(duration);
  updateBusinesses(duration);
  updatePeople(duration);
  //updateHeatMaps(duration);
  updateCities(duration);
  updateWeather(duration);
  setTravelGroupStats();
  sendRouteBatches(0);
  finishRouting(0);
  updateStatistics(duration);
  fireEvent(EventGameUpdate);
  runAllTaskQueue_g();
}

void runPreSimulation_g() {
  setIsPresimulating(true);
  setTime(-5*oneYear);
  while (getCurrentDateTime() < startTime) {
    item routerQueueSize = getStatistic(ourCityEconNdx(), RouterQueueSize);
    if (routerQueueSize > c(CRouterQueueCapacity)*.5f) {
      finishRouting(0);
      sleepMilliseconds(1);
    } else {
      onePreSimGameUpdate_g(.25f*oneHour*gameDayInRealSeconds);
    }
  }
  setIsPresimulating(false);
  resetMoney();
  updateMoney(0);
}

