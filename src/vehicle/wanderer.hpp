#pragma once

#include "../item.hpp"

void updateOneWanderer_g(item ndx, float duration);
void updateWanderers_g(float duration);
void addSpawnPoints(item buildingNdx);
void addWinWanderers_g(item buildingNdx);
void removeSpawnPoints(item buildingNdx);
void readdSpawnPoints(item buildingNdx);
void resetWanderers_g();

