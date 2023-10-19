#pragma once

#include "../item.hpp"

#include <cstdint>
#include <string>

enum TaskTypes {
  #define TASK(N) Task##N,
  #include "tasksEnum.hpp"
  #undef TASK
  numTaskTypes
};

typedef void (*TaskRunner)(double duration);
typedef bool (*TaskTester)(double duration);

const uint32_t _taskEnabled     = 1 << 0;
const uint32_t _taskFrame       = 1 << 1;
const uint32_t _taskUpdate      = 1 << 2;
const uint32_t _taskFrameUpdate = 1 << 3;
const uint32_t _taskFrameWait   = 1 << 4;
const uint32_t _taskAtMid       = 1 << 5;
const uint32_t _taskAtEnd       = 1 << 6;
const uint32_t _taskRepeatable  = 1 << 7;
const uint32_t _taskMergeable   = 1 << 8;

struct TaskTypeData {
  double totalTime = 0;
  double slowestTime = 0;
  double movingSlowestTime = 0;
  double lastTime = 0;
  double averageTime = 0;
  double movingArithMean = 0;
  double movingGeoMean = 0;
  item numCompleted = 0;
  item numPending = 0;
  item pendingTask = 0;
  uint32_t flags = 0;
  TaskRunner runner = 0;
  TaskTester tester = 0;
  const char* name;
};

struct Task {
  uint32_t flags;
  item type;
  double timeQueued;
  double duration;
};

void queueTask_g(item type, double duration);
void queueMatchingTasks_g(uint32_t flags, double duration);
void initTasks_g();
void runTaskQueue_g();
void runAllTaskQueue_g();
bool isUpdateTaskPending_g();
bool isFrameTaskPending_g();
item countUpdateTasksPending_g();

