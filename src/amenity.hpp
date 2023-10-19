#pragma once
#ifndef GOV_BUILDING_H
#define GOV_BUILDING_H

#include "money.hpp"
#include "heatmap.hpp"

#include <vector>

enum Effect {

  Nature, ValueEffect, DensityEffect, Law, EducationEffect, ProsperityEffect,
  Community, Technology, BusinessEffect, Tourism, Prestige,
  Environmentalism, Order, Health,

  numEffects
};

enum BuildingCategory {
  EducationBuildings, RecreationalBuildings, ServicesBuildings,
  UniversityBuildings,
  numBuildingCategories
};

float getEffectValue(item effect);
float getEffectMultiplier(item effect);
float getAdjustedDensity(item zone, vec3 loc);
float getAdjustedLandValue(item zone, vec3 loc);
float getHeatMapAdjustment(HeatMapIndex ndx);
const char* getEffectString(item effect);
const vec3 getEffectIcon(item effect);
const char* getBuildingCategoryString(item category);
char* getEffectDescriptor(item effect);
char* getEffectDescriptor(item effect, int value, int flags);
char* getMacroEffectDescriptor(item effect);
float getEduLevelLimit(item edu);

item getCategoryCount(item category);
bool getGovBuildingAvailable(item ndx);
void setGovBuildingAvailable(item ndx, bool val);
void setAllGovernmentBuildingsAvailable();
void rebuildAmenityStats();

item getRandomGovernmentBuilding();
item getRandomHealthcare();
money getMaintenance(item buildingNdx);
BudgetLine getBudgetLineFromBuildingCategory(char category);
void updateGovernmentBuilding(item ndx, float duration);
void addGovernmentBuilding(item ndx);
void removeGovernmentBuilding(item ndx);
const char* governmentBuildingLegalMessage(item ndx);
void addBuildingEffect(item designNdx);
void subtractBuildingEffect(item designNdx);
void makeGovernmentBuildingPayments(Budget* budget, double duration);
void sellGovBuilding(item buildingNdx);
void setGovBuildingEnabled(item buildingNdx, bool enabled);
int getGovBuildingsPlaced(item design);
void setAmenityHighlights();
void setAmenityHighlight(item ndx);
float getAmenityThrow(item ndx);

void resetGovernmentBuildings();
void recalculateEffectsAllGovernmentBuildings();
void writeGovernmentBuildings(FileBuffer* file);
void readGovernmentBuildings(FileBuffer* file, int version);

#endif
