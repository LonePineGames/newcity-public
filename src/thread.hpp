#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

typedef void (*ThreadCallback)();

void startThread(const char* name, ThreadCallback loop);
void nameThread(const char* name);
void sleepMilliseconds(int sleepTime);
void sleepMicroseconds(int sleepTime);

