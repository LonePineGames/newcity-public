#ifndef PLAN_H
#define PLAN_H

#include "serialize.hpp"
#include "configuration.hpp"

enum PlanType {
  GraphElementPlan,
  PillarPlan,
  BuildingPlan,
  StopPlan
};


enum PlanIconEntities {
  planYesIconEntity,
  planNoIconEntity,
  planMarkerIconEntity,
  planMarkerWSUIIconEntity,
  numPlanIconEntities
};

struct Plan {
  int flags;
  item planType;
  item element;
  item textEntity;
  item iconEntity[numPlanIconEntities];
  money cost;
  Configuration config;
  vec3 location;
  const char* legalMessage;
};

const int _planExists = 1 << 0;
const int _planSet = 1 << 1;
const int _planAffordable = 1 << 3;
const int _planForceDemolish = 1 << 4;

const float planCull = 10000;
const float boxScale = 1.5;
const vec3 planCompleteBoxCenter = vec3(40,-30,0) * boxScale;
const vec3 planDiscardBoxCenter = vec3(10,-30,0) *boxScale;
const vec3 planBoxSize = vec3(10,10,0) * boxScale;
const Line planCompleteBox = line(planCompleteBoxCenter - planBoxSize,
      planCompleteBoxCenter + planBoxSize);
const Line planDiscardBox = line(planDiscardBoxCenter - planBoxSize,
      planDiscardBoxCenter + planBoxSize);
const vec3 planStringLoc = vec3(-10, -30, 0)*boxScale;
const vec3 planWSUIIconCenter = vec3(-10,0,0) *boxScale;
const Line planWSUIIconBox = line(planWSUIIconCenter - planBoxSize*2.f,
      planWSUIIconCenter + planBoxSize*2.f);
  /*
const Line bigPlanCompleteBox =
  line(vec3(-40, -10, -1) * boxScale, vec3(0, 30, -1) * boxScale);
const Line bigPlanDiscardBox =
  line(vec3(0, -10, -2) * boxScale, vec3(40, 30, -2) * boxScale);
  */

item addPlan(item planType, item element);
item addPlan(item planType, item element, Configuration config);
Plan* getPlan(item plan);
BudgetLine getPlanBudgetLine(item ndx);
void updatePlanCost(item plan);
void updatePlan(item ndx);
money getPlanFullCost(item planNdx);
bool isPlanCompletable(item ndx);
const char* planLegalMessage(item ndx);
void setPlan(item plan);
void setPlan(item plan, bool deintersect);
bool buyPlan(item ndx);
void rerenderPlan(item ndx);
void completePlan(item plan);
void discardPlan(item ndx);
void removePlan(item ndx);

void setShowPlans(bool value);
bool isShowPlans();
void setPlansEnabled(bool enabled);
void setPlansActive(bool active);
bool isPlansEnabled();
bool isPlansVisible();
bool buyAllPlans();
void discardAllPlans();
bool buyAllUnsetPlans();
bool buyAllPlans(bool isSet, bool discardAfter);
void completeAllPlans();
void setAllPlans();
void splitAllUnsetPlans();
void discardAllUnsetPlans();
money getTotalPlansCost();
money getTotalUnsetPlansCost();
bool anyIllegalPlans();
item numUnsetPillarPlans();
item getNumGraphPlans();
void updatePlans();
void updatePlanCosts();
void cleanPlans();
void resetPlans();
bool handlePlanClick(Line mouseLine);
bool handlePlanMouseMove(Line mouseLine);
void writePlans(FileBuffer* file);
void readPlans(FileBuffer* file, int version);

#endif
