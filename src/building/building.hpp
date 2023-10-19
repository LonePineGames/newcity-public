#pragma once

#include "../main.hpp"
#include "../box.hpp"
#include "../money.hpp"
#include "../serialize.hpp"

const int _buildingExists = 1 << 0;
const int _buildingLights = 1 << 1;
const int _buildingComplete = 1 << 2;
const int _buildingCity = 1 << 3;
const int _buildingEnabled = 1 << 4;
const int _buildingAbandoned = 1 << 5;
const int _buildingHistorical = 1 << 6;
const int _buildingWasPlopped = 1 << 7;

const item numBuildingColors = 15;

struct Building {
  int flags;
  vec3 location;
  vec3 normal;
  item design;
  item color;
  item econ;
  money value;
  float lastUpdateTime;
  float builtTime;
  GraphLocation graphLoc;
  item entity;
  item decoEntity;
  item iconEntity;
  item zone;
  item plan;
  vector<item> businesses;
  vector<item> families;
  vector<item> peopleInside;
  vector<item> lots;
  char* name;
};

enum IssueIcon {
  NoIssue,
  Homeless,
  Jobless,
  Hungry,
  NeedsWorkers,
  NoCustomers,
  NeedsFreight,
  SickIssue,
  numIssueIcons
};

int numBuildings();
int sizeBuildings();
item addBuilding(vec3 bLoc, vec3 normal, item design, item zone);
item addGovernmentBuilding(item lot, item design);
item addCityBuilding(item cityNdx, vec3 loc, vec3 normal, item zone,
    float density, float landValue, bool render);
void updateBuildings(double duration);
void update1000Buildings(double duration);
void addOneBuilding(double duration);
Building* getBuilding(item ndx);
void completeBuilding(item ndx);
void removeBuilding(item ndx);
void resetBuildings();
item nearestBuilding(Line ml);
item nearestBuilding(vec3 loc);
void initBuildingsEntities();
void renderBuildings();
IssueIcon getBuildingIssue(item ndx);
money getBuildingTaxes(item ndx);
money getBuildingValue(item buildingNdx);
void updateBuildingValue(item buildingNdx);
vec3 getBuildingCenter(item ndx);
vec3 getBuildingTop(item ndx);
const char* buildingLegalMessage(item ndx);
Box getBuildingBox(item designNdx, vec3 loc, vec3 normal);
Box getBuildingBox(item buildingNdx);
vector<item> collideBuilding(Box box0, item self);
vector<item> collideBuilding(item buildingNdx);
void removeCollidingBuildings(Box b);
bool canPlaceBuilding(item design, vec3 loc, vec3 unorm);
bool canRemoveBuilding(item ndx);

void makeBuildingPayments(Budget* budget, double duration);
void nameBuildings();
void rebuildBuildingStats();
money getCityAssetValue();
void setMaxLandValues(item building);
float getMaxLandValues(item zone, item xSize);
float getMaxDensity(item zone, item xSize);
float getMaxFloors(item zone, item xSize);
item getMaxLandValuesBuilding(item zone, item xSize);
item getMaxDensityBuilding(item zone, item xSize);
item getMaxFloorsBuilding(item zone, item xSize);
item targetEmptyTenancies(item board);
bool issuesIconsVisible();
void setIssuesIconsVisible();
bool buildingsVisible();
void setBuildingsVisible();
item getTallestBuilding();
float getTargetIllumLevel(item buildingNdx);
item getNewestSkyscraper();
bool buildingIsSkyscraper(item ndx);

void writeBuildings(FileBuffer* file);
void readBuildings(FileBuffer* file, int version);

void removeBusinessFromTenancy(item buildingNdx, item businessNdx);
void removeFamilyFromHome(item buildingNdx, item familyNdx);

