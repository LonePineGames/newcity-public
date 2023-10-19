#include "renderPlan.hpp"

#include "building/renderBuilding.hpp"
#include "draw/camera.hpp"
#include "draw/entity.hpp"
#include "draw/texture.hpp"
#include "graph.hpp"
#include "icons.hpp"
#include "money.hpp"
#include "parts/toolbar.hpp"
#include "pillar.hpp"
#include "plan.hpp"
#include "renderPillar.hpp"
#include "renderUtils.hpp"
#include "string.hpp"
#include "string_proxy.hpp"
#include "util.hpp"

#include <stdio.h>
#include "spdlog/spdlog.h"

void renderPlan(item ndx) {
  if (!isRenderEnabled()) { return; }

  Plan* plan = getPlan(ndx);
  if (!plan->flags & _planExists) return;
  bool affordable = plan->flags & _planAffordable;
  const char* message = plan->legalMessage;
  bool legal = message == 0;
  bool set = plan->flags & _planSet;
  bool completable = isPlanCompletable(ndx);

  if (plan->planType == PillarPlan) {
    renderPillar(plan->element);
  } else if (plan->planType == BuildingPlan) {
    renderBuilding(plan->element);
  }

  if (plan->textEntity == 0) {
    for (int i = 0; i < numPlanIconEntities; i++) {
      plan->iconEntity[i] = addEntity(
          i == planMarkerIconEntity ? SignShader : WSUIShader);
    }
    plan->textEntity = addEntity(WSUITextShader);
  }

  Entity* iconEntity[numPlanIconEntities];
  Mesh* iconMesh[numPlanIconEntities];
  for (int i = 0; i < numPlanIconEntities; i++) {
    iconEntity[i] = getEntity(plan->iconEntity[i]);
    iconEntity[i]->texture = iconTexture;
    setCull(plan->iconEntity[i], 50, planCull);
    setEntityBringToFront(plan->iconEntity[i], true);
    setEntityHighlight(plan->iconEntity[i], false);
    createMeshForEntity(plan->iconEntity[i]);
  }

  for (int i = 0; i < numPlanIconEntities; i++) {
    iconMesh[i] = getMeshForEntity(plan->iconEntity[i]);
  }

  Entity* textEntity = getEntity(plan->textEntity);
  textEntity->texture = textTexture;
  setEntityBringToFront(plan->textEntity, true);
  setEntityHighlight(plan->textEntity, true);
  setCull(plan->textEntity, 50, set ? planCull : 100000);
  createMeshForEntity(plan->textEntity);
  Mesh* textMesh = getMeshForEntity(plan->textEntity);

  char* str = 0;
  if (!legal) {
    str = strdup_s(message);

  } else if (set || plan->planType == BuildingPlan) {
    money cost = getPlanFullCost(ndx);
    affordable = canBuy(getPlanBudgetLine(ndx), cost);
    str = printMoneyString(cost);
  }

  if (str) {
    float fontSize = set ? 25*boxScale : getCameraDistance()/25;
    float offset = stringWidth(str)*fontSize;
    vec3 textLoc = planStringLoc - vec3(offset,fontSize*.5f,0);
    renderString(textMesh, str, textLoc, fontSize);

    bool showHoldShift = !legal && !set;
    if (plan->planType != BuildingPlan && plan->planType != GraphElementPlan) {
      showHoldShift = false;
    }

    if (showHoldShift &&
        !streql("Road in the Way", str) &&
        !streql("Building in the Way", str) &&
        !streql("Another Building is in the Way", str)) {
      showHoldShift = false;
    }

    if (!isRoadTool() && !isAmenityTool()) {
      showHoldShift = false;
    }

    if (showHoldShift) {
      renderString(textMesh, "(Hold Shift to Demolish)", textLoc + vec3(0, fontSize, 0), fontSize*.75f);
    }

    free(str);
  }

  if (set) {
    Line l = planDiscardBox;
    Line iconL = iconToSpritesheet(iconNo);
    makeQuad(iconMesh[planNoIconEntity],
      l.start, vec3(l.end.x, l.start.y, l.start.z),
      vec3(l.start.x, l.end.y, l.end.z), l.end,
      iconL.start, iconL.end);

    if (affordable && completable) {
      l = planCompleteBox;
      Line iconL = iconToSpritesheet(iconYes);
      makeQuad(iconMesh[planYesIconEntity],
        l.start, vec3(l.end.x, l.start.y, l.start.z),
        vec3(l.start.x, l.end.y, l.end.z), l.end,
        iconL.start, iconL.end);
    }
  }

  if (plan->planType == GraphElementPlan && plan->element > 0) {
    Edge* edge = getEdge(plan->element);
    bool isDupe = edge->flags & _graphIsDuplicate;
    bool anyIcons = false;
    for (int i = 0; i < 2; i++) {
      Node* node = getNode(edge->ends[i]);
      bool colliding = node->flags & _graphIsColliding;
      bool complete = node->flags & _graphComplete;
      if (colliding || (!isDupe && (complete || node->edges.size() > 2))) {
        anyIcons = true;
        vec3 loc = i == 0 ? edge->line.start : edge->line.end;
        vec3 otherEnd = i != 0 ? edge->line.start : edge->line.end;
        float hiconSize = 20;
        Line iconLine = iconToSpritesheet(iconT, 0.f);
        vec3 hdown = otherEnd-loc;
        vec3 halong = hiconSize*uzNormal(hdown);
        hdown = hiconSize*normalize(hdown);
        loc -= plan->location;
        loc += hdown;
        loc.z += 5;
        makeQuad(iconMesh[planMarkerIconEntity],
            loc-hdown-halong, loc-hdown+halong,
            loc+hdown-halong, loc+hdown+halong,
            iconLine.start, iconLine.end);
      }
    }

    if (anyIcons) {
      item markerEntity = plan->iconEntity[planMarkerIconEntity];
      setCull(markerEntity, length(edge->line), planCull);
      setEntityBringToFront(markerEntity, true);
      //setEntityRedHighlight(markerEntity, true);
    }

    if (isDupe) {
      Line l = planWSUIIconBox;
      Line iconL = iconToSpritesheet(iconUpgrade);
      makeQuad(iconMesh[planMarkerWSUIIconEntity],
        l.start, vec3(l.end.x, l.start.y, l.start.z),
        vec3(l.start.x, l.end.y, l.end.z), l.end,
        iconL.start, iconL.end);
    }
  }

  for (int i = 0; i < numPlanIconEntities; i++) {
    bufferMesh(iconEntity[i]->mesh);
  }
  bufferMesh(textEntity->mesh);
  setEntityRedHighlight(plan->textEntity, !legal || !affordable);

  placePlan(ndx);
}

void placePlan(item ndx) {
  Plan* plan = getPlan(ndx);
  bool set = plan->flags & _planSet;
  bool show = (isPlansVisible() && set) ||
    plan->legalMessage != 0 || plan->planType == BuildingPlan;
  float angle = 0; //getHorizontalCameraAngle() - pi_o/2;
  float pitch = 0; //-getVerticalCameraAngle();
  float scal = set ? 1 : getCameraDistance()/20;
  placeEntity(plan->textEntity, plan->location, angle, pitch, scal);
  setEntityVisible(plan->textEntity, show);

  for (int i = 0; i < 2; i++) {
    placeEntity(plan->iconEntity[i], plan->location, angle, pitch, scal);
    setEntityVisible(plan->iconEntity[i], show && set);
  }

  placeEntity(plan->iconEntity[planMarkerIconEntity],
      plan->location, angle, pitch, 1);
  setEntityVisible(plan->iconEntity[planMarkerIconEntity],
      !set || isPlansVisible());

  placeEntity(plan->iconEntity[planMarkerWSUIIconEntity],
      plan->location, angle, pitch, 1);
  setEntityVisible(plan->iconEntity[planMarkerWSUIIconEntity],
      !set || isPlansVisible());
}

