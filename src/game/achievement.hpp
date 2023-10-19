#pragma once

#include "../serialize.hpp"
#include <string>

struct Achievement {
  char* code;
  char* name;
  char* text;
  char* effect;
  char* hint;
  char* effectText;
  int conditionFunc;
  char* conditionText;
};

void updateAchievements(double duration);
Achievement getAchievement(item ndx);
bool isAchievementAcquired(item ndx);
item getNumAchievements();
bool isFeatureEnabled(item ndx);
item getAmenityForAchievement(item achNdx);
item numAchievementsUnlocked();

void writeAchievements(FileBuffer* file);
void readAchievements(FileBuffer* file, int version);
void resetAchievements();
void initAchievements();
void initAchievementsLua();
void renderAchievements();

