#include "task.hpp"

#include "achievement.hpp"
#include "game.hpp"
#include "update.hpp"

#include "../pool.hpp"
#include "../thread.hpp"

//#include "../amenity.hpp"
#include "../blueprint.hpp"
//#include "../board.hpp"
#include "../building/building.hpp"
#include "../building/design.hpp"
#include "../business.hpp"
#include "../city.hpp"
//#include "../collisionTable.hpp"
//#include "../compass.hpp"
#include "../economy.hpp"
#include "../error.hpp"
#include "../heatmap.hpp"
#include "../input.hpp"
#include "../intersection.hpp"
#include "../graph.hpp"
#include "../graph/transit.hpp"
#include "../graph/stop.hpp"
//#include "../label.hpp"
//#include "../land.hpp"
#include "../lot.hpp"
#include "../money.hpp"
//#include "../name.hpp"
#include "../newspaper/newspaper.hpp"
#include "../option.hpp"
#include "../person.hpp"
//#include "../pillar.hpp"
#include "../plan.hpp"
#include "../renderLand.hpp"
//#include "../route/broker.hpp"
#include "../route/router.hpp"
#include "../selection.hpp"
#include "../sound.hpp"
#include "../string_proxy.hpp"
//#include "../thread.hpp"
#include "../time.hpp"
#include "../vehicle/pedestrian.hpp"
#include "../vehicle/travelGroup.hpp"
#include "../vehicle/update.hpp"
#include "../weather.hpp"
#include "../tutorial.hpp"
#include "../util.hpp"

#include "../draw/buffer.hpp"
#include "../draw/camera.hpp"
#include "../platform/event.hpp"

#include "taskRunners.cpp"
#include "spdlog/spdlog.h"

Pool<Task> tasks_g;
Cup<TaskTypeData> typeData_g;

item frameTasksPending = 0;
item updateTasksPending = 0;
double frameDeadline = 0;
double maxPendingTime = 0;
double lastTaskTime = 0;
double lastFrameDur = 0;
double targetFrameDur = 0.05;
const double meanUpdateRate = 0.02;
const double maxTardiness = 0.01;

void initType_g(item type, uint32_t flags, TaskRunner runner,
    TaskTester tester) {
  TaskTypeData* data = typeData_g.get(type);
  data->flags = flags;
  data->runner = runner;
  data->tester = tester;
  data->totalTime = 0;
  data->slowestTime = 0;
  data->movingSlowestTime = 0;
  data->lastTime = 0;
  data->averageTime = 0;
  data->numCompleted = 0;
  data->numPending = 0;
}

void initType_g(item type, uint32_t flags, TaskRunner runner) {
  initType_g(type, flags, runner, 0);
}

void initTasks_g() {
  frameTasksPending = 0;
  updateTasksPending = 0;
  lastTaskTime = glfwGetTime();

  tasks_g.clear();
  typeData_g.clear();
  typeData_g.resize(numTaskTypes);

  initType_g(TaskNull, 0, 0);
  initType_g(TaskInput,
      _taskFrame | _taskMergeable | _taskFrameWait, handleInputTask);
  //initType_g(TaskCamera,
      //_taskFrame | _taskMergeable | _taskFrameWait, 0);
  initType_g(TaskUpdateRender,
      _taskFrame | _taskAtMid, updateRender);
  initType_g(TaskCollectEntities,
      _taskFrame | _taskAtEnd, collectEntities, testCollectEntities);

  //initType_g(TaskFinishGraph,
      //_taskFrame | _taskFrameWait, 0);
  initType_g(TaskFinishLanes,
      _taskFrame, finishLanes);
  //initType_g(TaskFinishLots,
      //_taskFrame, finishLots);
  initType_g(TaskApplyBlueprintZones,
      _taskFrame, applyBlueprintZones);
  initType_g(TaskFinishTransit,
      _taskFrame | _taskFrameWait, finishTransit);
  //initType_g(TaskUpdatePlans,
      //_taskFrame | _taskFrameWait, 0);
  initType_g(TaskUpdateSelection,
      _taskFrame | _taskFrameWait, updateSelection);
  initType_g(TaskAssignHeights,
      _taskFrame, assignHeights, testAssignHeights);
  initType_g(TaskFireEventFrame,
      _taskFrame | _taskRepeatable | _taskFrameWait, fireEventFrame);
  initType_g(TaskRenderLandStep,
      _taskFrame, renderLandStep, testRenderLandStep);
  //initType_g(TaskRenderGraph,
      //_taskFrame | _taskFrameWait, 0, testRenderGraph);
  initType_g(TaskUpdateSound,
      _taskFrame | _taskRepeatable, updateSoundEnvironment);
  initType_g(TaskUpdateAutosave,
      _taskFrame | _taskMergeable, updateAutosave);
  initType_g(TaskDoAutosave,
      _taskFrame | _taskMergeable, doAutosave, testDoAutosave);
  initType_g(TaskUpdateAchievements,
      _taskFrame | _taskMergeable, updateAchievements);
  initType_g(TaskUpdateGraphVisuals, _taskFrame, updateGraphVisuals);

  initType_g(TaskSendRouteBatches,
      _taskFrameUpdate | _taskMergeable, sendRouteBatches);
  initType_g(TaskFinishRouting,
      _taskFrameUpdate | _taskMergeable, finishRouting);
  initType_g(TaskFinish1000Routes,
      _taskRepeatable, finish1000Routes);
  initType_g(TaskUpdateVehicles,
      _taskFrameUpdate | _taskFrameWait | _taskRepeatable, updateVehicles);
  initType_g(TaskPartialVehicleSwap, _taskFrameWait, partialVehicleSwap);
  initType_g(TaskFullVehicleSwap, _taskFrameWait, fullVehicleSwap);
  initType_g(TaskUpdateIntersections,
      _taskFrameUpdate | _taskMergeable, updateIntersections);
  initType_g(TaskUpdateAnimation,
      _taskFrameUpdate | _taskMergeable | _taskFrameWait, updateAnimation);
  initType_g(TaskUpdateWeather,
      _taskFrameUpdate | _taskMergeable | _taskFrameWait, updateWeather);
  initType_g(TaskUpdateTime,
      _taskFrameUpdate | _taskMergeable | _taskFrameWait, updateTime);
  initType_g(TaskUpdateTutorial,
      _taskFrame | _taskMergeable, updateTutorial);

  initType_g(TaskUpdateGraph,
      _taskRepeatable | _taskUpdate, updateGraph);
  initType_g(TaskUpdateBuildings,
      _taskRepeatable | _taskUpdate, updateBuildings);
  initType_g(TaskUpdate1000Buildings,
      _taskRepeatable, update1000Buildings);
  initType_g(TaskAddBuilding,
      _taskRepeatable, addOneBuilding);
  initType_g(TaskUpdateBusinesses,
      _taskRepeatable | _taskUpdate, updateBusinesses);
  initType_g(TaskUpdatePeople,
      _taskRepeatable | _taskUpdate, updatePeople);
  initType_g(TaskUpdate1000People,
      _taskRepeatable, update1000People);
  initType_g(TaskAddFamily,
      _taskRepeatable, addFamily);
  initType_g(TaskAddTouristFamily,
      _taskRepeatable, addTouristFamily);
  initType_g(TaskUpdateMoney,
      _taskRepeatable | _taskUpdate, updateMoney);
  initType_g(TaskUpdateLots,
      _taskRepeatable | _taskUpdate | _taskMergeable, updateLots);
  initType_g(TaskUpdateTransit,
      _taskRepeatable | _taskUpdate, updateTransit);
  initType_g(TaskUpdateHeatMaps,
      _taskRepeatable | _taskUpdate, updateHeatMaps);
  initType_g(TaskUpdateEconomy,
      _taskRepeatable | _taskUpdate, updateStatistics);
  initType_g(TaskUpdateCities,
      _taskRepeatable | _taskUpdate, updateCities);
  initType_g(TaskFireEventGameUpdate,
      _taskRepeatable | _taskUpdate, fireEventGameUpdate);

  #define TASK(N) typeData_g.get(Task##N)->name = #N;
  #include "tasksEnum.hpp"
  #undef TASK
}

double getTimeEstimate(TaskTypeData* type) {
  //return type->movingArithMean;
  //return pow(10, type->movingGeoMean);
  return mix(type->movingArithMean, type->movingSlowestTime, 0.8);
}

void queueTask_g(item typeNdx, double duration) {
  TaskTypeData* type = typeData_g.get(typeNdx);

  if ((type->flags & _taskMergeable) && type->pendingTask != 0) {
    Task* task = tasks_g.get(type->pendingTask);
    task->duration += duration;
    return;
  }

  if (!(type->flags & _taskRepeatable) && type->numPending > 0) return;
  if (type->tester != 0 && !type->tester(duration)) return;

  item taskNdx = tasks_g.create();
  Task* task = tasks_g.get(taskNdx);
  task->flags = _taskEnabled;
  task->type = typeNdx;
  task->duration = duration;
  task->timeQueued = glfwGetTime();

  type->numPending ++;
  if (type->pendingTask == 0) type->pendingTask = taskNdx;
  if (type->flags & _taskFrameWait) frameTasksPending ++;
  if (type->flags & _taskUpdate) updateTasksPending ++;
}

bool runTask_g(item ndx, bool force) {
  Task* task = tasks_g.get(ndx);
  if (!(task->flags & _taskEnabled)) return false;
  TaskTypeData* type = typeData_g.get(task->type);

  if (!force && ((type->flags & _taskAtMid) || (type->flags & _taskAtEnd))) {
    return false;
  }

  double startTime = glfwGetTime();
  double timeEstimate = getTimeEstimate(type);

  if (!force && !(type->flags & _taskFrameWait)) {
    double estimateEndTime = startTime + timeEstimate;
    double timePending = lastTaskTime - task->timeQueued;
    bool willBeLate = estimateEndTime > frameDeadline;
    bool willBeSuperLate = estimateEndTime > frameDeadline+maxTardiness;
    bool isDelayed = timePending > maxPendingTime;
    bool isSuperDelayed = timePending > maxPendingTime*2;

    if ((willBeSuperLate && !isSuperDelayed) ||
        (willBeLate && !isDelayed)) {
      return false;
    }
  }

  if (type->runner != 0) type->runner(task->duration);
  double endTime = glfwGetTime();
  double timeTaken = endTime - startTime;

  // if runner creates new task, task* might be invalid.
  task = tasks_g.get(ndx);

  type->numPending --;
  type->numCompleted ++;
  type->lastTime = timeTaken;
  type->totalTime += timeTaken;
  type->averageTime = type->totalTime / type->numCompleted;

  double timeTakenForGeo = timeTaken < 0.00001 ? 0.00001 : timeTaken;

  if (type->numCompleted <= 1) {
    type->movingArithMean = timeTaken;
    type->movingGeoMean = log10(timeTakenForGeo);
  } else {
    type->movingArithMean = mix(type->movingArithMean, timeTaken,
        meanUpdateRate);
    type->movingGeoMean = mix(type->movingGeoMean, log10(timeTakenForGeo),
        meanUpdateRate);
  }

  type->movingSlowestTime *= 1-meanUpdateRate;
  if (timeTaken > type->movingSlowestTime) type->movingSlowestTime = timeTaken;
  if (timeTaken > type->slowestTime) type->slowestTime = timeTaken;
  if (type->pendingTask == ndx) type->pendingTask = 0;
  if (type->flags & _taskFrameWait) frameTasksPending --;
  if (type->flags & _taskUpdate) updateTasksPending --;

  if (c(CLogUpdateTime)) {
    if (timeTaken - timeEstimate > 0.001 || timeTaken > 0.01) {
      SPDLOG_WARN("ran {} -- time:{:.2f}ms est:{:.2f}ms",
          type->name, timeTaken*1000, timeEstimate*1000);
    } else if (timeTaken > 0.001) {
      SPDLOG_INFO("ran {} -- time:{:.2f}ms est:{:.2f}ms",
          type->name, timeTaken*1000, timeEstimate*1000);
    }
    if (timeTaken > 0.001) {
      SPDLOG_INFO("frame time {:.2f}ms", (glfwGetTime() - lastTaskTime)*1000);
    }
  }

  task->flags = 0;
  tasks_g.free(ndx);
  return true;
}

void queueMatchingTasks_g(uint32_t flags, double duration) {
  for (int i = 0; i < numTaskTypes; i++) {
    if (typeData_g[i].flags & flags) {
      queueTask_g(i, duration);
    }
  }
}

void runAllTaskQueue_g() {
  item numRun = 0;

  while (tasks_g.count() > 0) {
    for (int i = 1; i <= tasks_g.size(); i++) {
      if (runTask_g(i, true)) numRun ++;
    }
  }

  if (c(CLogUpdateTime)) {
    SPDLOG_INFO("ran {} tasks", numRun);
  }
}

void runTaskQueue_g() {
  double startTime = lastTaskTime;
  double endTimeEst = 0;
  maxPendingTime = getFrameDuration() * 10;
  item numRun = 0;

  //SPDLOG_INFO("start {:.2f}ms", (glfwGetTime() - startTime)*1000);
  if (c(CLogUpdateTime)) {
    double renderTime = (glfwGetTime() - startTime)*1000;
    if (renderTime > 0.001) {
      SPDLOG_INFO("Spent {:.2f}ms waiting for render", renderTime);
    }
  }

  // Run high priority tasks
  frameDeadline = startTime;
  for (int i = 1; i <= tasks_g.size(); i++) {
    if (runTask_g(i, false)) numRun ++;
  }

  //double endTasksEstimate = 0;
  for (int i = 1; i <= tasks_g.size(); i++) {
    Task* task = tasks_g.get(i);
    if (!(task->flags & _taskEnabled)) continue;
    TaskTypeData* type = typeData_g.get(task->type);
    if (type->flags & _taskAtMid) {
      if (runTask_g(i, true)) numRun ++;
   //   endTasksEstimate += type->averageTime;
    }
    if (type->flags & _taskAtEnd) {
      endTimeEst += getTimeEstimate(type);
    }
  }

  //SPDLOG_INFO("mid {:.2f}ms endTimeEst {:.2f}ms",
      //(glfwGetTime() - startTime)*1000,
      //endTimeEst*1000);

  // Run other tasks
  frameDeadline = startTime + targetFrameDur - endTimeEst;
  if (glfwGetTime() < frameDeadline) {
    for (int i = 1; i <= tasks_g.size(); i++) {
      if (runTask_g(i, false)) numRun ++;
    }
    if (glfwGetTime() < frameDeadline) {
      tasks_g.defragment("tasks");
    }
  }

  //SPDLOG_INFO("end {:.2f}ms", (glfwGetTime() - startTime)*1000);

  for (int i = 1; i <= tasks_g.size(); i++) {
    Task* task = tasks_g.get(i);
    if (!(task->flags & _taskEnabled)) continue;
    TaskTypeData* type = typeData_g.get(task->type);
    if (type->flags & _taskAtEnd) {
      if (runTask_g(i, true)) numRun ++;
   //   endTasksEstimate += type->averageTime;
    }
  }

  //SPDLOG_INFO("end {:.2f}ms", (glfwGetTime() - startTime)*1000);

  frameDeadline = startTime + targetFrameDur;
  double endTime = glfwGetTime();
  while (endTime < frameDeadline) {
    double timeToSleep = frameDeadline-endTime;
    if (c(CLogUpdateTime)) {
      SPDLOG_INFO("sleeping for {}ms", timeToSleep*1000.);
    }
    sleepMicroseconds(timeToSleep*1000000.);
    endTime = glfwGetTime();
  }

  double timeTaken = endTime - startTime;
  if (c(CLogUpdateTime)) {
    SPDLOG_INFO("ran {} tasks; pending: {}", numRun, tasks_g.count());
    SPDLOG_INFO("frame took {:.2f}ms; planned {:.2f}ms; {:.2f}ms {}\n",
        timeTaken*1000, targetFrameDur*1000,
        abs(timeTaken-targetFrameDur)*1000,
        timeTaken > targetFrameDur ? "late" : "early");

    item numB = typeData_g.size();
    item* indices = (item*) alloca(sizeof(item)*numB);

    for (int stat = 0; stat < 3; stat ++) {
      double* totalTimes = (double*) alloca(sizeof(double)*numB);
      for (int i = 0; i < numB; i++) {
        indices[i] = i;
        if (stat == 0) {
          totalTimes[i] = typeData_g.get(i)->totalTime;
        } else if (stat == 1) {
          totalTimes[i] = typeData_g.get(i)->slowestTime;
        } else {
          totalTimes[i] = typeData_g.get(i)->movingSlowestTime;
        }
      }

      // Insertion sort
      for (int i = 0; i < numB; i++) {
        item x = indices[i];
        double xCost = totalTimes[x];
        int j = i-1;
        for (; j >= 0 && totalTimes[indices[j]] < xCost; j--) {
          indices[j+1] = indices[j];
        }
        indices[j+1] = x;
      }

      if (stat == 0) {
        SPDLOG_INFO("Worst total CPU time:");
      } else if (stat == 1) {
        SPDLOG_INFO("Slowest time (single run, ever):");
      } else {
        SPDLOG_INFO("Slowest time (single run, moving est.):");
      }

      for (int i = 0; i < 5; i++) {
        item x = indices[i];
        /*
        SPDLOG_INFO("i:{} x:{}", i, x);
        if (x <= 0 || x > typeData_g.size()) {
          SPDLOG_WARN("i:{} x:{}", i, x);
          continue;
        }
        */

        double time = totalTimes[x];
        TaskTypeData* data = typeData_g.get(x);
        const char* name = data->name;
        if (name == 0) name = "[NULL]";
        if (time > 1) {
          SPDLOG_INFO("#{} {:6.2f}s  {}{}",
              i+1, time, name, i == 4 ? "\n" : "");
        } else {
          SPDLOG_INFO("#{} {:6.2f}ms {}{}",
              i+1, time*1000, name, i == 4 ? "\n" : "");
        }
      }
    }
  }

  lastFrameDur = endTime - lastTaskTime;
  lastTaskTime = endTime;
  targetFrameDur = mix(targetFrameDur, lastFrameDur, meanUpdateRate);
  double fpsCap = getTargetFrameDuration_g();
  if (targetFrameDur < fpsCap) targetFrameDur = fpsCap;
  if (targetFrameDur*(1-meanUpdateRate) > fpsCap) {
    targetFrameDur *= (1-meanUpdateRate);
  }
  if (targetFrameDur > 0.05) targetFrameDur = 0.05;
}

item countUpdateTasksPending_g() {
  return updateTasksPending;
}

bool isFrameTaskPending_g() {
  return frameTasksPending > 0;
}

