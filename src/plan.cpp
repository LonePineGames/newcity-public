#include "plan.hpp"

#include "building/building.hpp"
#include "building/design.hpp"
#include "draw/camera.hpp"
#include "draw/entity.hpp"
#include "error.hpp"
#include "graph.hpp"
#include "graph/stop.hpp"
#include "money.hpp"
#include "pillar.hpp"
#include "pool.hpp"
#include "renderPlan.hpp"
#include "tools/road.hpp"
#include "tools/blueprint.hpp"
#include "tutorial.hpp"
#include "util.hpp"
#include "zone.hpp"

#include <set>
#include <glm/gtx/rotate_vector.hpp>

vec3 planHeight = vec3(0, 0, 20);
const float boxClickSize = 8;

Pool<Plan>* plans = Pool<Plan>::newPool(2000);
set<item> unsetPlans;
static set<item> toUpdateCosts;
static set<item> toRender;
static vector<item> toFree;
static bool plansEnabled = false;
static bool showPlans = true;
static bool plansVisible = false;
static bool plansActive = true;
static item numGraphPlans = 0;

bool isShowPlans() {
  return showPlans;
}

void placePlans() {
  bool vis = plansActive && showPlans && plansEnabled;
  if (vis == plansVisible) return;
  plansVisible = vis;

  for (int i=1; i <= plans->size(); i++) {
    Plan* plan = getPlan(i);
    if (!plan->flags & _planExists) continue;
    placePlan(i);
  }
}

bool isPlansVisible() {
  return plansVisible;
}

void setShowPlans(bool value) {
  showPlans = value;
  placePlans();
}

void setPlansEnabled(bool enabled) {
  plansEnabled = enabled;
  setRoadInfo();
  setBlueprintInfo();
  placePlans();
}

bool isPlansEnabled() {
  return plansEnabled;
}

void setPlansActive(bool active) {
  plansActive = active;
  placePlans();
}

item getNumGraphPlans() {
  return numGraphPlans;
}

item numUnsetPillarPlans() {
  int result = 0;
  vector<item> s(unsetPlans.begin(), unsetPlans.end());
  for (int i = 0; i < s.size(); i++) {
    item pNdx = s[i];
    Plan* p = getPlan(pNdx);
    if ((p->flags & _planExists) && !(p->flags & _planSet) &&
      p->planType == PillarPlan) {
      result ++;
    }
  }
  return result;
}

BudgetLine getPlanBudgetLine(item ndx) {
  Plan* plan = getPlan(ndx);
  item type = plan->planType;
  if (type == GraphElementPlan) {
    Configuration config = getElementConfiguration(plan->element);
    if (config.type == ConfigTypeRoad) {
      return RoadBuildExpenses;
    } else if (config.type == ConfigTypeExpressway) {
      return ExpwyBuildExpenses;
    } else {
      return TransitBuildExpenses;
    }
  } else if (type == PillarPlan) {
    return PillarBuildExpenses;
  } else if (type == StopPlan) {
    return TransitBuildExpenses;
  } else if (type == BuildingPlan) {
    if (plan->cost < 0) {
      return AmenityIncome;
    } else {
      return BuildingBuildExpenses;
    }
  } else {
    return MiscDiscExpenses;
  }
}

void discardAllPlans() {
  for (int i = 1; i <= plans->size(); i++) {
    Plan* p = getPlan(i);
    if (p->flags & _planSet) {
      discardPlan(i);
    }
  }
}

void setAllPlans() {
  vector<item> s(unsetPlans.begin(), unsetPlans.end());
  for (int i = 0; i < s.size(); i++) {
    item pNdx = s[i];
    setPlan(pNdx);
  }
}

void splitAllUnsetPlans() {
  vector<item> s(unsetPlans.begin(), unsetPlans.end());
  for (int i = 0; i < s.size(); i++) {
    item pNdx = s[i];
    Plan* plan = getPlan(pNdx);
    if (plan->flags & _planSet) return;
    if (plan->planType == GraphElementPlan) {
      if (split(plan->element)) continue;
      dedupe(plan->element);
      queueCollide(plan->element);
    }
  }
}

void discardAllUnsetPlans() {
  vector<item> s(unsetPlans.begin(), unsetPlans.end());
  for (int i = 0; i < s.size(); i++) {
    item pNdx = s[i];
    discardPlan(pNdx);
  }
}

unordered_map<BudgetLine, money> getTotalPlansCostTable(bool isSet) {
  updatePlanCosts();
  unordered_map<BudgetLine, money> result;
  set<item> newNodes;
  unordered_map<item, Configuration> reconfiguredNodes;
  for (int i = 1; i <= plans->size(); i++) {
    Plan* p = getPlan(i);
    if ((p->flags & _planExists) && p->legalMessage == 0 &&
        (bool(p->flags & _planSet) == isSet)) {
      BudgetLine l = getPlanBudgetLine(i);
      result[l] += p->cost;

      if (p->planType == GraphElementPlan) {
        Edge* e = getEdge(p->element);
        item dupe = detectSemiDuplicate(p->element);
        for (int j = 0; j < 2; j++) {
          item nodeNdx = e->ends[j];
          Node* node = getNode(nodeNdx);
          if (!(node->flags & _graphComplete)) {
            newNodes.insert(nodeNdx);
          } else if (dupe != 0) {
            reconfiguredNodes[nodeNdx] = e->config;
          }
        }
      }
    }
  }

  for (std::pair<item, Configuration> p : reconfiguredNodes) {
    bool isExpwy = p.second.type == ConfigTypeExpressway;
    BudgetLine l = isExpwy ? ExpwyBuildExpenses : RoadBuildExpenses;
    result[l] += reconfigureCost(p.first, p.second, isSet, !isSet);
  }

  for (item nodeNdx : newNodes) {
    bool isExpwy = getElementConfiguration(nodeNdx).type ==
      ConfigTypeExpressway;
    BudgetLine l = isExpwy ? ExpwyBuildExpenses : RoadBuildExpenses;
    result[l] += graphCostFull(nodeNdx);
  }

  money total = 0;
  for (std::pair<BudgetLine, money> m : result) {
    total += m.second;
  }
  result[NullBudget] = total;
  return result;
}

money getTotalPlansCost(bool isSet) {
  return getTotalPlansCostTable(isSet)[NullBudget];
}

money getPlanFullCost(item planNdx) {
  Plan* p = getPlan(planNdx);
  money result = p->cost;
  if (p->planType != GraphElementPlan) {
    return result;
  }

  Edge* edge = getEdge(p->element);
  item dupe = detectSemiDuplicate(p->element);

  for (int i = 0; i < 2; i++) {
    item nodeNdx = edge->ends[i];
    Node* node = getNode(nodeNdx);
    if (!(node->flags & _graphComplete)) {
      result += graphCostFull(nodeNdx);
    } else if (dupe) {
      result += reconfigureCost(nodeNdx, edge->config);
    }
  }

  return result;
}

money getTotalPlansCost() {
  return getTotalPlansCost(true);
}

money getTotalUnsetPlansCost() {
  return getTotalPlansCost(false);
}

bool buyAllPlans(bool isSet, bool discardAfter) {
  unordered_map<BudgetLine, money> costs = getTotalPlansCostTable(isSet);
  if (!canBuy(RoadBuildExpenses, costs[NullBudget])) return false;

  for (std::pair<BudgetLine, money> m : costs) {
    if (m.first != NullBudget) {
      transaction(m.first, -m.second);
    }
  }

  //bool tutorialReport = false;
  for (int i = 1; i <= plans->size(); i++) {
    Plan* p = getPlan(i);
    if ((p->flags & _planExists) && bool(p->flags & _planSet) == isSet) {
      if (p->legalMessage == 0) {
        /*
        // Check if we should report to the tutorial
        if (!tutorialReport) {
          Edge* edge = getEdge(p->element);
          if (edge != 0) {
            tutorialReport = true;
            item type = edge->config.type;
            reportTutorialUpdate(
                type == ConfigTypeRoad ? BuildRoad :
                type == ConfigTypeExpressway ? BuildExpressway :
                type == ConfigTypeHeavyRail ? BuildRail :
                NoAction);
          }
        }
        */
        completePlan(i);
      } else if (discardAfter) {
        discardPlan(i);
      }
    }
  }

  return true;
}

bool buyAllUnsetPlans() {
  return buyAllPlans(false, false);
}

bool buyAllPlans() {
  return buyAllPlans(true, false);
}

void completeAllPlans() {
  for (int i = 1; i <= plans->size(); i++) {
    Plan* p = getPlan(i);
    if (p->flags & _planExists) {
      if (p->legalMessage == 0) {
        completePlan(i);
      } else {
        discardPlan(i);
      }
    }
  }
}

bool anyIllegalPlans() {
  for (int i = 1; i <= plans->size(); i++) {
    Plan* plan = getPlan(i);
    if (plan->legalMessage != 0) {
      return true;
    }
  }
  return false;
}

item addPlan(item planType, item element) {
  item ndx = plans->create();
  Plan* plan = getPlan(ndx);
  plan->flags = _planExists;
  plan->planType = planType;
  plan->element = element;
  for (int i = 0; i < numPlanIconEntities; i++) {
    plan->iconEntity[i] = 0;
  }
  plan->textEntity = 0;
  unsetPlans.insert(ndx);

  if (plan->planType == GraphElementPlan) {
    if (plan->element > 0) {
      Line l = getLine(plan->element);
      plan->location = (l.start+l.end)/2.f + planHeight;
      Edge* edge = getEdge(element);
      edge->plan = ndx;
    } else {
      plan->location = getNode(element)->center + planHeight;
    }

  } else if (plan->planType == PillarPlan) {
    Pillar* pillar = getPillar(element);
    pillar->plan = ndx;
    plan->location = pillar->location + planHeight;

  } else if (plan->planType == StopPlan) {
    Stop* stop = getStop(element);
    stop->plan = ndx;
    plan->location = stop->location + planHeight;

  } else if (plan->planType == BuildingPlan) {
    Building* building = getBuilding(element);
    building->plan = ndx;
    plan->location = building->location + planHeight;

  } else {
    handleError("invalid planType in addPlan\n");
  }

  toUpdateCosts.insert(ndx);
  toRender.insert(ndx);
  return ndx;
}

item addPlan(item planType, item element, Configuration config) {
  item ndx = addPlan(planType, element);
  Plan* plan = getPlan(ndx);
  plan->config = config;
  return ndx;
}

void removePlan(item ndx) {
  Plan* plan = getPlan(ndx);
  if (!(plan->flags & _planExists)) {
    return;
  }

  unsetPlans.erase(ndx);
  if (plan->planType == GraphElementPlan && (plan->flags & _planSet)) {
    numGraphPlans --;
  }

  plan->flags = 0;

  if (plan->element != 0) {
    if (plan->planType == GraphElementPlan) {
      getEdge(plan->element)->plan = 0;
    } else if (plan->planType == PillarPlan) {
      getPillar(plan->element)->plan = 0;
    } else if (plan->planType == StopPlan) {
      getStop(plan->element)->plan = 0;
    } else if (plan->planType == BuildingPlan) {
      getBuilding(plan->element)->plan = 0;

    } else {
      handleError("invalid planType in removePlan\n");
    }
  }

  if (plan->textEntity != 0) {
    removeEntityAndMesh(plan->textEntity);
    for (int i = 0; i < numPlanIconEntities; i++) {
      removeEntityAndMesh(plan->iconEntity[i]);
    }
    plan->textEntity = 0;
  }
  toFree.push_back(ndx);
}

Plan* getPlan(item ndx) {
  return plans->get(ndx);
}

void updatePlanCost(item ndx) {
  Plan* plan = getPlan(ndx);
  if (!plan->flags & _planExists) return;

  plan->legalMessage = planLegalMessage(ndx);
  if (plan->legalMessage != 0) {
    plan->cost = 0;

  } else if (plan->planType == GraphElementPlan) {
    Edge* edge = getEdge(plan->element);
    int cost = graphCostFull(plan->element);
    /*for (int i=0; i < 2; i++) {
      item nodeNdx = edge->ends[i];
      Node* n = getNode(nodeNdx);
      if (!(n->flags & _graphComplete)) {
        cost += graphCostFull(nodeNdx);
      }
    }*/
    plan->cost = cost;

  } else if (plan->planType == PillarPlan) {
    plan->cost = pillarCost(plan->element);

  } else if (plan->planType == StopPlan) {
    plan->cost = stopCost(plan->element);

  } else if (plan->planType == BuildingPlan) {
    vector<item> collisions = collideBuilding(plan->element);
    money valueDestroyed = 0;
    for (int i = 0; i < collisions.size(); i++) {
      Building* b = getBuilding(collisions[i]);
      if (!(b->flags & _buildingExists) || !(b->flags & _buildingComplete)) continue;
      float cost = getBuildingValue(collisions[i]);
      if (b->zone == GovernmentZone) {
        cost *= -1 * c(CBuildingSalesFactor);
      }
      valueDestroyed += cost;
    }

    Building* b = getBuilding(plan->element);
    Design* d = getDesign(b->design);
    float cost = d->zone == GovernmentZone ? (d->cost * c(CAmenityCostMult)) :
      getDesignValue(b->design);
    cost *= getInflation();
    if (!(d->flags & _designNoEminentDomain)) {
      cost += valueDestroyed * c(CEminentDomainFactor);
    }
    plan->cost = cost;

  } else {
    handleError("invalid planType in planCost\n");
  }

  if (canBuy(getPlanBudgetLine(ndx), plan->cost)) {
    plan->flags |= _planAffordable;
  } else {
    plan->flags &= ~_planAffordable;
  }

  toRender.insert(ndx);
}

void updatePlanCosts() {
  for (std::set<item>::iterator it = toUpdateCosts.begin();
      it != toUpdateCosts.end(); it++) {
    item i = *it;
    updatePlanCost(i);
  }
  toUpdateCosts.clear();
}

bool isPlanCompletable(item ndx) {
  Plan* plan = getPlan(ndx);
  if (plan->legalMessage != 0) {
    return false;
  }

  if (plan->planType == GraphElementPlan) {
    Edge* edge = getEdge(plan->element);
    for (int i=0; i < 2; i++) {
      Node* n = getNode(edge->ends[i]);
      if (n->pillar) {
        Pillar* pillar = getPillar(n->pillar);
        if (!(pillar->flags & _pillarComplete)) {
          return false;
        }
      }
    }
    return true;

  } else {
    return true;
  }
}

const char* planLegalMessage(item ndx) {
  Plan* plan = getPlan(ndx);

  if (plan->planType == GraphElementPlan) {
    return legalMessage(plan->element);

  } else if (plan->planType == PillarPlan) {
    return pillarLegalMessage(plan->element);

  } else if (plan->planType == BuildingPlan) {
    return buildingLegalMessage(plan->element);

  } else if (plan->planType == StopPlan) {
    return stopLegalMessage(plan->element);

  } else {
    handleError("invalid planType in planLegalMessage\n");
    return "";
  }
}

void setPlan(item ndx) {
  setPlan(ndx, true);
}

void setPlan(item ndx, bool deintersect) {
  Plan* plan = getPlan(ndx);
  if (plan->flags & _planSet) return;
  plan->flags |= _planSet;
  unsetPlans.erase(ndx);

  if (plan->planType == GraphElementPlan) {
    numGraphPlans ++;
    if (deintersect) {
      if (split(plan->element)) return;
      dedupe(plan->element);
    }
    queueCollide(plan->element);
  }

  toRender.insert(ndx);
}

void completePlan(item ndx) {
  setPlan(ndx, false);
  Plan* plan = getPlan(ndx);
  if (plan->planType == GraphElementPlan) {
    Edge* edge = getEdge(plan->element);
    complete(plan->element, plan->config);
    //plan->element = 0;

  } else if (plan->planType == PillarPlan) {
    completePillar(plan->element);

  } else if (plan->planType == BuildingPlan) {
    completeBuilding(plan->element);

  } else if (plan->planType == StopPlan) {
    completeStop(plan->element);

  } else {
    handleError("invalid planType in planCost\n");
  }

  removePlan(ndx);
}

void discardPlan(item ndx) {
  Plan* plan = getPlan(ndx);

  if (!(plan->flags & _planExists)) {
    return;
  }

  if (plan->planType == GraphElementPlan) {
    removeElement(plan->element);

  } else if (plan->planType == PillarPlan) {
    removePillar(plan->element);

  } else if (plan->planType == BuildingPlan) {
    removeBuilding(plan->element);

  } else if (plan->planType == StopPlan) {
    removeStop(plan->element);

  } else {
    handleError("invalid planType in planCost\n");
  }

  removePlan(ndx);
}

item lastHoverPlan = 0;
bool hoverDiscard = false;
bool handlePlanMouseMove(Line mouseLine) {
  if (lastHoverPlan > 0) {
    Plan* plan = getPlan(lastHoverPlan);
    for (int i = 0; i < numPlanIconEntities; i++) {
      setEntityHighlight(plan->iconEntity[i], false);
    }
    lastHoverPlan = 0;
    hoverDiscard = false;
  }

  if (getCameraDistance() > planCull) return false;

  float angle = getHorizontalCameraAngle() - pi_o/2;
  float pitch = -getVerticalCameraAngle();
  vec3 completeBoxRot = rotate(rotate(planCompleteBoxCenter,
    pitch, vec3(1,0,0)),
    angle, vec3(0,0,1));
  vec3 discardBoxRot = rotate(rotate(planDiscardBoxCenter,
    pitch, vec3(1,0,0)),
    angle, vec3(0,0,1));

  for (int i = 1; i <= plans->size(); i++) {
    Plan* plan = getPlan(i);
    if (!plansVisible && plan->legalMessage == 0) continue;

    if (plan->flags & _planSet) {
      vec3 cb = plan->location + completeBoxRot;
      vec3 db = plan->location + discardBoxRot;

      if (plan->legalMessage == 0 && plan->flags & _planAffordable &&
          pointLineDistance(cb, mouseLine) < boxClickSize) {
        lastHoverPlan = i;
        setEntityHighlight(plan->iconEntity[planYesIconEntity], true);
        return true;

      } else if (pointLineDistance(db, mouseLine) < boxClickSize) {
        lastHoverPlan = i;
        setEntityHighlight(plan->iconEntity[planNoIconEntity], true);
        hoverDiscard = true;
        return true;
      }
    }
  }
  return false;
}

bool buyPlan(item ndx) {
  updatePlanCost(ndx);
  if (isPlanCompletable(ndx) &&
      transaction(getPlanBudgetLine(ndx), -getPlanFullCost(ndx))) {
    completePlan(ndx);
    return true;
  } else {
    return false;
  }
}

void rerenderPlan(item ndx) {
  toRender.insert(ndx);
}

bool handlePlanClick(Line mouseLine) {
  if (lastHoverPlan != 0) {
    if (hoverDiscard) {
      discardPlan(lastHoverPlan);
    } else {
      buyPlan(lastHoverPlan);
    }
    return true;
  } else {
    return false;
  }
}

void updatePlan(item ndx) {
  toUpdateCosts.insert(ndx);
}

void updatePlans() {
  for (int i=1; i <= plans->size(); i++) {
    Plan* plan = getPlan(i);
    if (!plan->flags & _planExists) continue;
    if (!(plan->flags & _planAffordable) &&
        canBuy(getPlanBudgetLine(i), plan->cost)) {
      toUpdateCosts.insert(i);
    }
  }

  updatePlanCosts();

  for (std::set<item>::iterator it = toRender.begin();
      it != toRender.end(); it++) {
    item i = *it;
    renderPlan(i);
  }
  toRender.clear();

  for (int i = 0; i < toFree.size(); i++) {
    plans->free(toFree[i]);
  }
  toFree.clear();

  plans->defragment("plans");
}

void cleanPlans() {
  for (int i=1; i <= plans->size(); i++) {
    Plan* plan = getPlan(i);
    if (!(plan->flags & _planExists)) continue;

    if (!(plan->flags & _planSet)) {
      discardPlan(i);
    } else {

      if (plan->planType == GraphElementPlan) {
        Edge* edge = getEdge(plan->element);
        if (!(edge->flags & _graphExists) || (edge->flags & _graphComplete)) {
          discardPlan(i);
        }

      } else if (plan->planType == PillarPlan) {
        Pillar* pillar = getPillar(plan->element);
        if (!(pillar->flags & _pillarExists) ||
          (pillar->flags & _pillarComplete)) {

          discardPlan(i);
        }

      } else if (plan->planType == StopPlan) {
        Stop* stop = getStop(plan->element);
        if (!(stop->flags & _stopExists) ||
          (stop->flags & _stopComplete)) {

          discardPlan(i);
        }

      } else if (plan->planType == BuildingPlan) {
        Building* building = getBuilding(plan->element);
        if (!(building->flags & _buildingExists) ||
          (building->flags & _buildingComplete)) {

          discardPlan(i);
        }

      } else {
        handleError("invalid planType in cleanPlans\n");
      }
    }
  }
}

void resetPlans() {
  numGraphPlans = 0;
  plansEnabled = false;
  showPlans = true;
  plansVisible = false;
  plansActive = false;
  plans->clear();
  unsetPlans.clear();
}

void readPlan(FileBuffer* file, int version, item ndx) {
  Plan* plan = getPlan(ndx);
  plan->flags = fread_int(file);
  plan->planType = fread_item(file, version);
  plan->element = fread_item(file, version);
  plan->location = fread_vec3(file);
  plan->config = readConfiguration(file, version);
  for (int i = 0; i < numPlanIconEntities; i++) {
    plan->iconEntity[i] = 0;
  }
  plan->textEntity = 0;
  if (plan->flags & _planExists) {
    toUpdateCosts.insert(ndx);
    toRender.insert(ndx);
    if (!(plan->flags & _planSet)) {
      unsetPlans.insert(ndx);
    }
    if (plan->planType == GraphElementPlan && plan->flags & _planSet) {
      numGraphPlans ++;
    }
  }
}

void writePlan(FileBuffer* file, item ndx) {
  Plan* plan = getPlan(ndx);
  fwrite_int  (file, plan->flags);
  fwrite_item(file, plan->planType);
  fwrite_item(file, plan->element);
  fwrite_vec3 (file, plan->location);
  writeConfiguration(file, plan->config);
}

void readPlans(FileBuffer* file, int version) {
  plans->read(file, version);

  for(int i = 1; i <= plans->size(); i++) {
    readPlan(file, version, i);
  }

  for(int i = 1; i <= plans->size(); i++) {
    Plan* plan = getPlan(i);
    if ((plan->flags & _planExists) && !(plan->flags & _planSet)) {
      discardPlan(i);
    }
  }
}

void writePlans(FileBuffer* file) {
  plans->write(file);

  for(int i = 1; i <= plans->size(); i++) {
    writePlan(file, i);
  }
}

