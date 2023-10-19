#pragma once

#include "../item.hpp"

void resetGameUpdate_g();
void logStage(const char* stage);
item suggestEffectiveGameSpeed();
item getEffectiveGameSpeed();
void oneGameUpdate_g(double duration);
void updateGameInner_g();
void updateRender(double duration);
void runPreSimulation_g();

