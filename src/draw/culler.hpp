#pragma once

#include "cull.hpp"
#include "buffer.hpp"

#include "../item.hpp"

enum CullerState {
  CullerWaiting, CullerReady, CullerRunning, CullerReset,
  numCullerStates
};

void resetCuller_c();
void resizeCuller_c();
void markEntityDirty_g(item ndx);
void updateAllEntitiesInCuller_c();
void selectMeshes_c(Cull cull);
bool cullEntity_c(item ndx, UngroupedDrawCommand* command);
void waitForCullerState_g(CullerState state);
void setCullerState_g(CullerState state);
void nextCullerCull_g(Cull cull);
void updateEntityCuller_g();
Cull getCull_g();
void cullerLoop_c();
void killCuller();

