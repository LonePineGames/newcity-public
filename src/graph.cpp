#include "graph.hpp"

#include "graph/graphParent.hpp"
#include "graph/stop.hpp"

#include "building/building.hpp"
#include "collisionTable.hpp"
#include "draw/entity.hpp"
#include "economy.hpp"
#include "error.hpp"
#include "game/feature.hpp"
#include "game/game.hpp"
#include "heatmap.hpp"
#include "intersection.hpp"
#include "land.hpp"
#include "line.hpp"
#include "lot.hpp"
#include "money.hpp"
#include "name.hpp"
#include "pillar.hpp"
#include "plan.hpp"
#include "pool.hpp"
#include "option.hpp"
#include "renderGraph.hpp"
#include "selection.hpp"
#include "string_proxy.hpp"
#include "time.hpp"
#include "tools/road.hpp"
#include "tutorial.hpp"
#include "util.hpp"
#include "vehicle/laneLoc.hpp"
#include "vehicle/update.hpp"
#include "zone.hpp"

#include "parts/messageBoard.hpp"

#include <algorithm>
#include <stdio.h>

const float roadShine = .4f;
const double wearRate = 1e-7;
const char* expresswayName = "";
const static float maxSpeed = 50;
const static float minSpeed = 5;
const float minExpresswayAngle = 3.1415 * 11 / 16;
const float minIntersectionSpacing = 5;
const float minIntersectionSize = 8;
const float sampleLength = tileSize*5;
float maxWear = 0;
static int completeEdges = 0;
static unordered_map<item, vector<item>> collideBuildingCache;
static bool automaticBridgesEnabled = true;
static bool areTunnelsVisible = true;
static item lastVisualUpdate = 0;
static bool leftHandTraffic = false;

Pool<Node>* nodes = Pool<Node>::newPool(2000);
Pool<Edge>* edges = Pool<Edge>::newPool(2000);

void complete(item ndx, bool deintersect, Configuration config);
item detectDuplicate(item ndx);
void repositionEdge(item edgeNdx);
void resizeNode(item nodeNdx);
void resizeEdge(item edgeNdx);
#include "graphModify.cpp"

void setAutomaticBridges(bool val) {
  automaticBridgesEnabled = val;
}

item numNodes() {
  return nodes->count();
}

item sizeNodes() {
  return nodes->size();
}

item sizeEdges() {
  return edges->size();
}

item numCompleteEdges() {
  return completeEdges;
}

Node* getNode(item ndx) {
  return nodes->get(-ndx);
}

bool isLeftHandTraffic() {
  return leftHandTraffic;
}

void setLeftHandTraffic(bool val) {
  leftHandTraffic = val;
}

float trafficHandedness() {
  return isLeftHandTraffic() ? -1 : 1;
}

money laneCost(item type) {
  if (type == ConfigTypeExpressway) {
    return c(CLaneCostExpwy);
  } else if (type == ConfigTypePedestrian) {
    return c(CWalkwayCost);
  } else if (type == ConfigTypeHeavyRail) {
    return c(CLaneCostRail);
  } else {
    return c(CLaneCostRoad);
  }
}

money minReconfigureCost(item type) {
  if (type == ConfigTypeExpressway) {
    return c(CMinReconfigureCostExpwy);
  } else if (type == ConfigTypePedestrian) {
    return c(CMinReconfigureCostRoad);
  } else if (type == ConfigTypeHeavyRail) {
    return c(CMinReconfigureCostRail);
  } else {
    return c(CMinReconfigureCostRoad);
  }
}

money strategyCost(item type) {
  if (type == StopSignStrategy) {
    return c(CStrategyCostStopSign);
  } else if (type == TrafficLightStrategy) {
    return c(CStrategyCostTrafficLight);
  } else if (type == JointStrategy) {
    return c(CStrategyCostJunction);
  }

  return c(CStrategyCostStopSign);
}

money graphCostPlusElevation(item ndx);
void adjustGraphStats(item ndx, float mult) {
  item econ = ourCityEconNdx();
  if (ndx < 0) {
    if (getNode(ndx)->flags & _graphCity) return;
    adjustStat(econ, NumIntersections, 1);
    adjustStat(econ, PavementMSq, mult*pow(getNode(ndx)->intersectionSize, 2));
  } else if (ndx > 0) {
    if (getEdge(ndx)->flags & _graphCity) return;
    float mSqrd = length(getLine(ndx)) * edgeWidth(ndx);
    adjustStat(econ, PavementMSq, mult*mSqrd);
  } else {
    return;
  }

  adjustStat(econ, RoadSystemValue,
      mult*graphCostPlusElevation(ndx)/getInflation());
}

item getRandomNode() {
  if (nodes->count() == 0) {
    return 0;
  }

  Node* result;
  item resultNdx = 0;
  for (int i=0; i < 100; i++) {
    resultNdx = -randItem(nodes->size()) - 1;
    result = getNode(resultNdx);
    if (result->flags & _graphExists) {
      return resultNdx;
    }
  }
  return 0;
}

Edge* getEdge(item ndx) {
  return edges->get(ndx);
}

item addNode(vec3 center, Configuration config) {
  //if (center.z < beachLine+1) center.z = beachLine+1;
  item ndx = -nodes->create();
  Node* node = getNode(ndx);
  node->center = center;
  node->edges.clear();
  node->laneBlocks.clear();
  node->phaseMins.clear();
  node->phaseMaxs.clear();
  node->entity = 0;
  node->signEntity = 0;
  node->tunnelEntity = 0;
  node->flags = _graphExists;
  node->pillar = 0;
  node->config = config;
  bool oneWay = config.flags & _configOneWay;
  node->intersectionSize = minIntersectionSize;
  node->phase = 0;
  node->timeInPhase = 0;
  node->spawnProbability = getGameMode() == ModeBuildingDesigner ? 0.01 : 0.1;
  addToCollisionTable(GraphCollisions, getGraphBox(ndx), ndx);
  toCollide.insert(ndx);
  toRender.insert(ndx);

  if ((node->config.flags & _configDontMoveEarth) &&
      pointOnLand(node->center).z+2 > node->center.z &&
      pointOnLandNatural(node->center).z > node->center.z) {
    node->flags |= _graphUnderground;
  }

  if (!validate(center)) handleError("NaN nod loc");
  return ndx;
}

item addPillarNode(item pillarNdx, Configuration config) {
  Pillar* pillar = getPillar(pillarNdx);
  vec3 center = pillar->location;
  if (!validate(pillar->location)) handleError("NaN pillar loc");
  item ndx = addNode(center, config);
  Node* node = getNode(ndx);
  node->pillar = pillarNdx;
  pillar->node = ndx;
  return ndx;
}

void removeNode(item ndx) {
  removeFromCollisionTable(GraphCollisions, ndx);
  Node* node = getNode(ndx);
  if (!(node->flags & _graphExists)) {
    return;
    //handleError("removeNode() on node which has already been removed");
  }
  for(int i = 0; i < node->laneBlocks.size(); i++) {
    removeLaneBlock(node->laneBlocks[i]);
  }
  if (node->pillar > 0) {
    Pillar* pillar = getPillar(node->pillar);
    pillar->node = 0;
  }

  deselect(SelectionGraphElement, ndx);
  removeMessageByObject(GraphMessage, ndx);

  if (node->flags & _graphComplete) {
    orphanLots(ndx);
    removeGraphElevator(ndx);
    adjustGraphStats(ndx, -1);
  }

  if (node->entity != 0) {
    removeEntityAndMesh(node->entity);
    removeEntityAndMesh(node->signEntity);
    node->entity = 0;
    node->signEntity = 0;
  }
  if (node->tunnelEntity != 0) {
    removeEntityAndMesh(node->tunnelEntity);
    node->tunnelEntity = 0;
  }
  node->flags = 0;
  toFree.push_back(ndx);
}

void sortEdgesInNode(item ndx) {
  Node* node = getNode(ndx);
  int numEdges = node->edges.size();
  float* thetas = (float*) alloca(sizeof(float)*numEdges);
  int* indices = (int*) alloca(sizeof(int)*numEdges);
  int* itemNdxs = (int*) alloca(sizeof(int)*numEdges);
  vec3 center = node->center;

  for(int i = 0; i < numEdges; i++) {
    Edge* edge = getEdge(node->edges[i]);
    item otherEnd = edge->ends[0] == ndx ? edge->ends[1] : edge->ends[0];
    vec3 dir = getNode(otherEnd)->center - center;
    float theta = atan2(dir.x, dir.y);
    thetas[i] = theta;
    indices[i] = i;
    itemNdxs[i] = node->edges[i];
  }

  // Insertion sort
  for (int i = 1; i < numEdges; i++) {
    item x = indices[i];
    float xTheta = thetas[x];
    int j = i-1;
    for (; j >= 0 && thetas[indices[j]] > xTheta; j--) {
      indices[j+1] = indices[j];
    }
    indices[j+1] = x;
  }

  for (int i = 0; i < numEdges; i++) {
    node->edges.set(i, itemNdxs[indices[i]]);
  }
}

float elementWidth(item ndx) {
  if (ndx < 0) {
    Node* node = getNode(ndx);
    return node->intersectionSize;
  } else if (ndx > 0) {
    return edgeWidth(ndx);
  } else {
    handleError("elementWidth(0)");
    return 0;
  }
}

float buildDistance(item element) {
  if (element >= 0) return c(CBuildDistance);
  return std::max(c(CBuildDistance), elementWidth(element)+0.01f);
}

float edgeWidth(Configuration config) {
  if (config.type == ConfigTypePedestrian) {
    //float result = c(CLaneWidth) + c(CSidewalkWidth);
    float result = c(CSidewalkWidth);
    if (config.flags & _configToll) {
      result = c(CLaneWidth)*(config.numLanes*.5f+2);
    }
    //if (config.flags & _configDontMoveEarth) result += c(CSidewalkWidth);
    return result;
  }
  bool oneWay = config.flags & _configOneWay;
  bool median = config.flags & _configMedian;
  int numLanes = config.numLanes;
  float medianWidth = median ? c(CLaneWidth)*2 : 0;
  return numLanes * c(CLaneWidth) * (oneWay ? 1 : 2) + c(CSholderWidth)*2 +
    medianWidth;
}

float edgeWidth(item ndx) {
  Edge* edge = getEdge(ndx);
  return edgeWidth(edge->config);
}

int getSpeedLimit(item ndx) {
  if (ndx < 0) {
    float max = 0;
    Node* node = getNode(ndx);
    for (int i = 0; i < node->edges.size(); i++) {
      Edge* edge = getEdge(node->edges[i]);
      max = std::max(speedLimits[edge->config.speedLimit], max);
    }
    return max;
  } else if (ndx > 0) {
    Edge* edge = getEdge(ndx);
    return speedLimits[edge->config.speedLimit];
  } else {
    handleError("getSpeedLimit(0)");
    return 0;
  }
}

EndDescriptor getNodeEndDescriptor(item edgeNdx, item nodeNdx) {
  Edge* edge = getEdge(edgeNdx);
  int end = edge->ends[0] == nodeNdx;
  Node* node0 = getNode(edge->ends[!end]);
  Node* node1 = getNode(edge->ends[end]);
  vec3 center = node0->center;

  EndDescriptor result;
  result.location = end ? edge->line.start : edge->line.end;
  vec3 otherEnd = !end ? edge->line.start : edge->line.end;
  vec3 dir = normalize(otherEnd - result.location);
  result.normal = vec3(dir.y, -dir.x, 0) *
    float(isLeftHandTraffic() ? -1 : 1);

  if (edge->config.flags & _configOneWay) {
    result.location += result.normal *
      (float(edge->config.numLanes) * c(CLaneWidth)*.5f * (end ? 1.f : -1.f));
  } else if (edge->config.flags & _configMedian) {
    result.median = result.normal * c(CLaneWidth);
  }

  if (end) {
    result.source = edge->laneBlocks[1];
    result.drain = edge->laneBlocks[0];
  } else {
    result.source = edge->laneBlocks[0];
    result.drain = edge->laneBlocks[1];
  }

  return result;
}

EndDescriptor getEdgeEndDescriptor(item edge, item nodeNdx) {
  EndDescriptor desc = getNodeEndDescriptor(edge, nodeNdx);
  desc.graphElement = nodeNdx;
  desc.normal *= -1;
  desc.median *= -1;
  return desc;
}

bool nameEdge(item edgeNdx, item endNdx) {
  Edge* edge = getEdge(edgeNdx);
  if (edge->config.type == ConfigTypeExpressway) {
    edge->name = strdup_s(expresswayName);
    //SPDLOG_INFO("naming edge {}:expressway", edgeNdx);
    return true;

  } else if (edge->config.type == ConfigTypeHeavyRail) {
    edge->name = strdup_s("Railroad");
    //SPDLOG_INFO("naming edge {}:expressway", edgeNdx);
    return true;

  } else if (edge->config.type == ConfigTypePedestrian) {
    edge->name = strdup_s("Walkway");
    //SPDLOG_INFO("naming edge {}:expressway", edgeNdx);
    return true;
  }

  item nodeNdx = edge->ends[endNdx];
  Node* node = getNode(nodeNdx);
  vector<item> edges = node->edges.toVector();
  item oddManOut = getOddManOut(nodeNdx, edges);
  int numEdges = edges.size();
  if (numEdges <= 1) {
    //SPDLOG_INFO("not naming edge {}:single", edgeNdx);
    return false;
  }

  //Get itemInNode, and build names item
  item itemInNode = -1;
  const char** names = (const char**)alloca(sizeof(char*)*numEdges);
  for (int i = 0; i < numEdges; i++) {
    if (edges[i] == edgeNdx) {
      itemInNode = i;
    }
    names[i] = getEdge(edges[i])->name;
  }
  if (itemInNode == oddManOut) {
    //SPDLOG_INFO("not naming edge {}:odd man out", edgeNdx);
    return false;
  }

  int pairNum = itemInNode;
  for (int i = 0; i < numEdges/2; i++) {
    pairNum = (pairNum+1)%numEdges;
    if (pairNum == oddManOut) pairNum = (pairNum+1)%numEdges;
  }

  //int pairNum = (itemInNode + numEdges/2) % numEdges;
  //SPDLOG_INFO("pairNum:{}/{} ({}+{}), omo:{}",
      //pairNum, numEdges, itemInNode, numEdges/2, oddManOut);
  /*
  if (oddManOut >= 0) {
    if (itemInNode < oddManOut && pairNum >= oddManOut) {
      pairNum = (pairNum + 1) % numEdges;
    } else if (pairNum == oddManOut) {
      pairNum = (pairNum + 1) % numEdges;
      //pairNum = (pairNum + numEdges - 1) % numEdges;
    }
  }

  SPDLOG_INFO("pairNum:{}/{} ({}+{}), omo:{}",
      pairNum, numEdges, itemInNode, numEdges/2, oddManOut);
      */

  //Consider name
  Edge* otherEdge = getEdge(edges[pairNum]);
  const char* newName = otherEdge->name;
  if (newName == 0 || strlen(newName) == 0) {
    //SPDLOG_INFO("not naming edge {}:{}:no name", edgeNdx, edges[pairNum]);
    return false;
  }

  //Don't take name if >=90 deg angle
  vec3 v0 = edge->line.end - edge->line.start;
  vec3 v1 = otherEdge->line.end - otherEdge->line.start;
  float neg = otherEdge->ends[0] == edge->ends[0] ||
    otherEdge->ends[1] == edge->ends[1] ? -1 : 1;
  if (glm::dot(v0, v1) * neg <= 0.1) {
    //SPDLOG_INFO("not naming edge {}:extreme angle", edgeNdx);
    return false;
  }

  //Don't take name if already taken
  for (int i = 0; i < numEdges; i++) {
    if (i != itemInNode && i != pairNum && streql(names[i], newName)) {
      //SPDLOG_INFO("not naming edge {}:duplicate", edgeNdx);
      return false;
    }
  }

  char* name = strdup_s(newName);
  if (edge->name != 0) free(edge->name);
  edge->name = name;
  //SPDLOG_INFO("naming edge {}:{}", edgeNdx, name);
  return true;
}

void nameEdge(item edgeNdx) {
  Edge* edge = getEdge(edgeNdx);
  if (edge->flags & _graphComplete) {
    return;
  }
  if (!nameEdge(edgeNdx, 0)) {
    if (!nameEdge(edgeNdx, 1)) {
      //edge->name = randomName(RoadName);
    }
  }
}

void chainRenameEdge(item edgeNdx, bool followCorners) {
  Edge* edge = getEdge(edgeNdx);

  for (int n = 0; n < 2; n++) {
    item cursorNdx = edgeNdx;
    Edge* cursor = edge;
    item lastEnd = edge->ends[!n];

    for (int k = 0; k < 100; k++) {
      item end = cursor->ends[0] == lastEnd ? 1 : 0;
      item nodeNdx = cursor->ends[end];
      Node* node = getNode(nodeNdx);
      vector<item> edges = node->edges.toVector();
      item oddManOut = getOddManOut(nodeNdx, edges);
      int numEdges = edges.size();
      if (numEdges <= 1) {
        break;
      }

      //Get itemInNode, and build names item
      item itemInNode = -1;
      for (int i = 0; i < numEdges; i++) {
        if (edges[i] == cursorNdx) {
          itemInNode = i;
          break;
        }
      }

      if (itemInNode == oddManOut) {
        break;
      }

      int pairNum = itemInNode;
      for (int i = 0; i < numEdges/2; i++) {
        pairNum = (pairNum+1)%numEdges;
        if (pairNum == oddManOut) pairNum = (pairNum+1)%numEdges;
      }

      //Consider name
      item otherNdx = edges[pairNum];
      Edge* other = getEdge(otherNdx);

      if (!followCorners) {
        //Don't take name if >=90 deg angle
        vec3 v0 = cursor->line.end - cursor->line.start;
        vec3 v1 = other->line.end - other->line.start;
        float neg = other->ends[0] == cursor->ends[0] ||
          other->ends[1] == cursor->ends[1] ? -1 : 1;
        if (glm::dot(v0, v1) * neg <= 0.1) {
          break;
        }
      }

      char* name = strdup_s(edge->name);
      if (other->name != 0) free(other->name);
      other->name = name;
      toRender.insert(otherNdx);
      cursor = other;
      cursorNdx = otherNdx;
      lastEnd = nodeNdx;
    }
  }
}

void addEdgeToNode(item nodeNdx, item edgeNdx) {
  Node* node = getNode(nodeNdx);
  Edge* edge = getEdge(edgeNdx);
  vec3 center = node->center;

  if (edge->config.type != ConfigTypeExpressway &&
      node->config.type == ConfigTypeExpressway) {
    node->config = edge->config;
  }

  #ifdef LP_DEBUG
  if (debugMode()) {
    item otherEnd = edge->ends[0] == nodeNdx ? edge->ends[1] : edge->ends[0];
    for (int i = 0; i < node->edges.size(); i++) {
      if (node->edges[i] == edgeNdx) {
        handleError("Edge added to node twice");
      }
      /*
      Edge* e = getEdge(node->edges[i]);
      if ((e->ends[0] == nodeNdx && e->ends[1] == otherEnd) ||
        (e->ends[1] == nodeNdx && e->ends[0] == otherEnd)) {
        handleError("Duplicate edge added");
      }
      */
    }
  }
  #endif

  node->edges.push_back(edgeNdx);
  if (!(node->flags & _graphComplete)) {
    toResizeNodes.insert(nodeNdx);
    toRender.insert(nodeNdx);
  }
  //nameEdge(edgeNdx);
}

bool areClose(vec3 l0, vec3 l1, float dist) {
  return distance2DSqrd(l0, l1) < dist*dist &&
    abs(l0.z-l1.z) < c(CZTileSize)*.75f;
}

bool areClose(vec3 l0, vec3 l1) {
  return areClose(l0, l1, c(CMinNodeDistance));
}

bool areClose(vec3 p, Line l, float dist) {
  vec3 lp = nearestPointOnLine(p, l);
  return areClose(p, lp, dist);
}

bool areClose(vec3 p, Line l) {
  return areClose(p, l, c(CMinNodeDistance));
}

item detectDuplicate(item ndx) {
  Edge* e = getEdge(ndx);
  item n0i = e->ends[0];
  item n1i = e->ends[1];
  Node* n0 = getNode(n0i);
  Node* n1 = getNode(n1i);
  for (int i = 0; i < n0->edges.size(); i++) {
    item e1i = n0->edges[i];
    if (e1i == ndx) continue;
    Edge* e1 = getEdge(e1i);
    if (e1->ends[0] == n1i || e1->ends[1] == n1i) {
      return e1i;
    }
  }
  return 0;
}

item detectSemiDuplicate(item ndx) {
  Edge* e = getEdge(ndx);
  item n0i = e->ends[0];
  item n1i = e->ends[1];
  Node* n0 = getNode(n0i);
  Node* n1 = getNode(n1i);

  for (int i = 0; i < n0->edges.size(); i++) {
    item e1i = n0->edges[i];
    if (e1i == ndx) continue;
    Edge* e1 = getEdge(e1i);
    if (e1->ends[0] == n1i || e1->ends[1] == n1i) return e1i;
    if (areClose(n1->center, e1->line, edgeWidth(e1i))) return e1i;
  }

  for (int i = 0; i < n1->edges.size(); i++) {
    item e1i = n1->edges[i];
    if (e1i == ndx) continue;
    Edge* e1 = getEdge(e1i);
    if (areClose(n0->center, e1->line, edgeWidth(e1i))) return e1i;
  }
  return 0;
}

/*
vec3 getRepositionLoc(item eNdx, item oNdx, item nNdx) {
  Edge* edge = getEdge(eNdx);
  Node* node = getNode(nNdx);
  vec3 center = node->center;
  int end = edge->ends[1] == nNdx;
  Node* oNode = getNode(edge->ends[!end]);
  vec3 eAlong = node->center - oNode->center;
  float eAlongLength = length(eAlong);
  vec3 eALngthC = (eAlongLength - node->intersectionSize)/eAlongLength;
  vec3 eLoc = oNode->center + eAlong * eALngthC;
  float eWidth = edgeWidth(edgeNdx)*.5f;
  //vec3 eNorm = normalize(zNormal(center - eLoc)) * eWidth;

  Edge* other = getEdge(oNdx);
  float oWidth = edgeWidth(oNdx)*.5f;
  vec3 oLoc = other->ends[1] == nodeNdx ?
    other->line.end : other->line.start;
  vec3 diff = eLoc - oLoc;

  if (length(diff) > oWidth + eWidth + minIntersectionSpacing) {
    return eLoc;
  }

  vec3 oLLoc = other->ends[0] == nodeNdx ?
    other->line.end : other->line.start;
  vec3 oNorm = uzNormal(oOLoc - oLoc) * oWidth;

  vec3 result = cross(oNorm, diff) + oLoc;
  return result;
}
*/

void repositionEdge(item edgeNdx) {
  if (!c(CEnableEdgePlacement)) return;
  Edge* edge = getEdge(edgeNdx);
  item dupe = detectSemiDuplicate(edgeNdx);
  toRender.insert(edgeNdx);

  if (dupe == 0) {
    edge->flags &= ~_graphIsDuplicate;
  } else {
    edge->flags |= _graphIsDuplicate;
  }

  for (int end = 0; end < 2; end ++) {
    item nodeNdx = edge->ends[end];
    toRender.insert(nodeNdx);
    Node* node = getNode(nodeNdx);
    vec3 center = node->center;
    vec3 eLoc = end ? edge->line.end : edge->line.start;
    float eWidth = edgeWidth(edgeNdx)*.5f;
    vec3 eNorm = normalize(zNormal(center - eLoc)) * eWidth;

    for (int i = 0; i < node->edges.size(); i++) {
      item oNdx = node->edges[i];
      if (oNdx == edgeNdx) continue;
      if (oNdx == dupe) continue;
      Edge* other = getEdge(oNdx);
      float oWidth = edgeWidth(oNdx)*.5f;
      vec3 oLoc = other->ends[1] == nodeNdx ?
        other->line.end : other->line.start;
      vec3 diff = eLoc - oLoc;
      if (other->plan != 0) {
        rerenderPlan(other->plan);
      }

      if (length(diff) < 0.0001) { // Undetected semi-duplicate probably
        // pass
      } else if (length(diff) < oWidth + eWidth + minIntersectionSpacing) {
        if (!validate(oLoc)) handleError("NaN edge loc");
        diff = normalize(diff) * (oWidth + eWidth + minIntersectionSpacing);
        if (!validate(diff)) handleError("NaN edge loc");
        eLoc = oLoc + diff;
        if (!validate(eLoc)) handleError("NaN edge loc");
      }
    }

    if (end) {
      edge->line.end = eLoc;
    } else {
      edge->line.start = eLoc;
    }

    toRender.insert(nodeNdx);
  }
}

void resizeEdge(item edgeNdx) {
  Edge* edge = getEdge(edgeNdx);
  for (int end = 0; end < 2; end ++) {
    item nodeNdx = edge->ends[end];
    Node* node = getNode(nodeNdx);

    vec2 center = vec2(node->center);
    float z = node->center.z;
    float size = node->intersectionSize;
    vec2 basis = vec2(end ? edge->line.start : edge->line.end);
    vec2 loc = vec2(!end ? edge->line.start : edge->line.end);
    vec2 along = loc - basis;
    vec2 ray = basis - center;
    vec2 solution;

    float a = dot(along, along);
    float b = 2.f * dot(ray, along);
    float c = dot(ray, ray) - size*size;
    float discriminant = b*b - 4*a*c;
    if (!validate(discriminant) || discriminant < 0 ||
        !validate(a) || abs(a) < 0.000001) {
      // no intersection
      solution = loc;

    } else {
      discriminant = sqrt(discriminant);
      float t1 = (-b - discriminant)/(2*a);
      solution = t1*along + basis;
    }

    vec3 eLoc = vec3(solution, z);
    if (!validate(eLoc)) handleError("NaN edge loc");
    if (end) {
      edge->line.end = eLoc;
    } else {
      edge->line.start = eLoc;
    }

    if (!(node->flags & _graphComplete)) {
      toRender.insert(nodeNdx);
    }
  }

  toRender.insert(edgeNdx);
  toSetupLaneEdges.insert(edgeNdx);
}

void coerceNodeType(item ndx) {
  Node* node = getNode(ndx);
  item nodeType = node->config.type;
  bool anyExpwy = false;
  bool allExpwy = true;
  bool anyRail = false;
  bool allRail = true;
  bool anyPed = false;
  bool allPed = true;

  for (int i = 0; i < node->edges.size(); i++) {
    item type = getEdge(node->edges[i])->config.type;

    if (type == ConfigTypeExpressway) anyExpwy = true;
    else allExpwy = false;

    if (type == ConfigTypeHeavyRail) anyRail = true;
    else allRail = false;

    if (type == ConfigTypePedestrian) anyPed = true;
    else allPed = false;
  }

  bool changeMade = false;
  if (allRail && anyRail) {
    if (nodeType != ConfigTypeHeavyRail) {
      changeMade = true;
      node->config.type = ConfigTypeHeavyRail;
      node->config.strategy = JunctionStrategy;
    }

  } else if (anyExpwy && allExpwy) {
    if (nodeType != ConfigTypeExpressway) {
      changeMade = true;
      node->config.type = ConfigTypeExpressway;
      node->config.strategy = JointStrategy;
      node->config.flags |= _configOneWay;
    }

  } else if (anyPed && allPed) {
    if (nodeType != ConfigTypePedestrian) {
      changeMade = true;
      node->config.type = ConfigTypePedestrian;
      node->config.strategy = UnregulatedStrategy;
    }

  } else if (nodeType != ConfigTypeRoad) {
    changeMade = true;
    node->config.type = ConfigTypeRoad;
    item strategy = node->config.strategy;
    if (strategy != StopSignStrategy &&
        strategy != TrafficLightStrategy) {
      node->config.strategy = StopSignStrategy;
    }
  }

  if (changeMade) {
    toRender.insert(ndx);
    toResizeNodes.insert(ndx);
    toSetupLaneNodes.insert(ndx);
  }
}

void resizeNode(item ndx) {
  coerceNodeType(ndx);
  sortEdgesInNode(ndx);
  Node* node = getNode(ndx);

  float newSize = minIntersectionSize;
  vector<item> edges = getRenderableEdges(ndx);
  int numEdges = edges.size();
  int numLanes = 0;
  int numRail = 0;
  for(int i = 0; i < numEdges; i ++) {
    Edge* edge = getEdge(edges[i]);
    float width = edgeWidth(edges[i]);
    numLanes += edge->config.numLanes;
    if (width > newSize) {
      newSize = width;
    }
    if (edge->config.type == ConfigTypeHeavyRail) {
      numRail ++;
    }
  }

  if (node->config.type == ConfigTypeExpressway) {
    newSize *= 2;
  } else {
    newSize *= 1 + (numEdges-1)*.05 + numLanes*.01;
  }
  if (numRail > 2) newSize = std::max(newSize, 40.f);

  if (abs(newSize - node->intersectionSize) > 0.1) {
    node->intersectionSize = newSize;

    toRender.insert(ndx);
    if (node->flags & _graphComplete) {
      toSetupLaneNodes.insert(ndx);
    }
  }

  for(int i = 0; i < node->edges.size(); i ++) {
    item edgeNdx = node->edges[i];
    toResizeEdges.insert(edgeNdx);
    toRender.insert(edgeNdx);
    toSetupLaneEdges.insert(edgeNdx);
  }
}

item addEdge(item end0ndx, item end1ndx, Configuration config) {

  Node* n0 = getNode(end0ndx);
  Node* n1 = getNode(end1ndx);
  vec3 c0 = n0->center;
  vec3 c1 = n1->center;
  vec3 along = c1 - c0;
  float lngth = length(along);
  if (lngth < 0.0001) {
    //handleError("Zero length edge");
    return 0;
  }
  vec3 ualong = along / lngth;
  vec3 offset0 = -ualong * n0->intersectionSize;
  vec3 offset1 = ualong * n1->intersectionSize;
  offset0.z = 0;
  offset1.z = 0;

  item ndx = edges->create();
  Edge* edge = getEdge(ndx);
  edge->name = strdup_s(""); //randomName(RoadName);
  edge->config = config;
  edge->entity = 0;
  edge->wearEntity = 0;
  edge->textEntity = 0;
  edge->signEntity = 0;
  edge->tunnelEntity = 0;
  edge->flags = _graphExists;
  edge->wear = 0;
  edge->plan = 0;
  edge->ends[0] = end0ndx;
  edge->ends[1] = end1ndx;
  edge->laneBlocks[0] = 0;
  edge->laneBlocks[1] = 0;
  edge->line = line(c0 - offset0, c1 - offset1);
  if (!validate(edge->line)) handleError("NaN edge loc");

  addEdgeToNode(end0ndx, ndx);
  addEdgeToNode(end1ndx, ndx);

  if ((edge->config.flags & _configDontMoveEarth) &&
      (n0->flags & _graphUnderground) &&
      (n1->flags & _graphUnderground)) {
    edge->flags |= _graphUnderground;
  }

  addToCollisionTable(GraphCollisions, getGraphBox(ndx), ndx);
  toCollide.insert(ndx);
  toRepositionEdges.insert(ndx);
  toRender.insert(ndx);
  return ndx;
}

bool isOpen(item ndx) {
  if (ndx < 0) {
    Node* node = getNode(ndx);
    return node->flags & _graphOpen;
  } else if (ndx > 0) {
    Edge* edge = getEdge(ndx);
    return edge->flags & _graphOpen;
  } else {
    handleError("isOpen(0)");
    return false;
  }
}

money elevatorCost(Configuration config, float z) {
  if (abs(z) < c(CRoadRise)*2.f) return 0;
  bool dontMoveEarth = config.flags & _configDontMoveEarth;
  return abs(z) * (dontMoveEarth ?
      (z > 0 ? c(CViaductCost) : c(CTunnelCost)) :
      (z > 0 ? c(CEmbankmentCost) : c(CTrenchCost)));
}

money edgeCost(vec3 c0, vec3 c1,
    Configuration e, Configuration n0, Configuration n1) {
  float width = edgeWidth(e);
  float ln = length(c1-c0);
  money landCost = 0;
  int num = ln/tileSize;
  for (int i = 0; i < num; i++) {
    float z = mix(c0.z, c1.z, float(i)/num);
    landCost += elevatorCost(e, z);
  }
  float platform = (e.flags & _configPlatform) ?
    c(CPlatformCost) : 0;
  return ln/1000 * (width/c(CLaneWidth) * laneCost(e.type) + platform) +
    strategyCost(n0.strategy) + strategyCost(n1.strategy) +
    landCost;
}

money graphCostPlusElevation(item ndx) {
  money landCost = 0;
  if (ndx < 0) {
    return 0;
    //Node* node = getNode(ndx);
    //landDiff = abs(pointOnLand(node->center).z - node->center.z);
  } else if (ndx > 0) {
    Edge* edge = getEdge(ndx);
    vec3 cursor = edge->line.start;
    vec3 along = edge->line.end - edge->line.start;
    float dist = length(along);
    int num = dist/tileSize;
    vec3 ualong = along*(tileSize/dist);
    for (int k = 0; k < num; k++) {
      cursor += ualong;
      float natH = std::max(getNaturalHeight(cursor), 0.f);
      float z = cursor.z - natH - c(CRoadRise)*2;
      landCost += elevatorCost(edge->config, z);
    }
  } else {
    handleError("graphCost(0)");
  }
  return graphCostSimple(ndx) + landCost * getInflation();
}

bool isElementUnderground(item ndx) {
  if (ndx < 0) return getNode(ndx)->flags & _graphUnderground;
  else if (ndx > 0) return getEdge(ndx)->flags & _graphUnderground;
  else return false;
}

vector<item> collideBuildingWithGraph(item ndx) {
  if (isElementUnderground(ndx)) return vector<item>();
  auto result = collideBuildingCache.find(ndx);
  if (result == collideBuildingCache.end()) {
    Box b = getGraphBox(ndx);
    vector<item> collisions = collideBuilding(b, 0);
    collideBuildingCache[ndx] = collisions;
    return collisions;
  } else {
    return result->second;
  }
}

money graphCostFull(item ndx) {
  float eminentDomainCost = 0;
  vector<item> collisions = collideBuildingWithGraph(ndx);
  for (int i = 0; i < collisions.size(); i++) {
    Building* b = getBuilding(collisions[i]);
    if (b->zone == GovernmentZone) {
      eminentDomainCost -= getBuildingValue(collisions[i])*
        c(CBuildingSalesFactor);
    } else {
      eminentDomainCost += getBuildingValue(collisions[i]) *
        c(CEminentDomainFactor);
    }
  }

  if (ndx > 0) {
    item otherEdge = detectDuplicate(ndx);
    if (otherEdge != 0) {
      Edge* edge = getEdge(ndx);
      if (getEdge(otherEdge)->flags & _graphComplete) {
        return eminentDomainCost + reconfigureCost(otherEdge, edge->config);
      }
    }
    otherEdge = detectSemiDuplicate(ndx);
    if (otherEdge != 0) {
      Edge* edge = getEdge(ndx);
      Edge* other = getEdge(otherEdge);
      if (other->flags & _graphComplete) {
        Node* n0 = getNode(edge->ends[0]);
        Node* n1 = getNode(edge->ends[1]);
        float widthDiff = edgeWidth(ndx) - edgeWidth(otherEdge);
        float platform = (edge->config.flags & _configPlatform) &&
          !(other->config.flags & _configPlatform) ?
          c(CPlatformCost) : 0;
        money cost = length(n1->center - n0->center)/1000 *
          (widthDiff / c(CLaneWidth) * laneCost(edge->config.type) + platform);
        cost = std::max(minReconfigureCost(edge->config.type), cost);
        return cost * getInflation() + eminentDomainCost;
      }
    }

  } else {
    Node* node = getNode(ndx);
    item collider = nearestEdge(node->center, false);
    if (collider != 0) {
      vec3 loc = nearestPointOnLine(node->center, getLine(collider));
      if (vecDistance(loc, node->center) < 1) {
        return strategyCost(node->config.strategy)*2 + eminentDomainCost;
      }
    }
  }

  return graphCostPlusElevation(ndx) + eminentDomainCost;
}

money graphCostSimple(item ndx) {
  if (ndx < 0) {
    return 0;
    //Node* node = getNode(ndx);
    //(intersectionCost[node->config.type] +
     // strategyCost[node->config.strategy]) * getInflation();

  } else if (ndx > 0) {
    Edge* edge = getEdge(ndx);
    Node* n0 = getNode(edge->ends[0]);
    Node* n1 = getNode(edge->ends[1]);
    float platform = (edge->config.flags & _configPlatform) ?
      c(CPlatformCost) : 0;
    float lngth = length(n1->center - n0->center)/1000;
    money cost = lngth * (platform +
        edgeWidth(ndx) / c(CLaneWidth) * laneCost(edge->config.type));
    cost += strategyCost(n0->config.strategy);
    cost += strategyCost(n1->config.strategy);
    return cost * getInflation();

  } else {
    handleError("graphCost(0)");
    return 0;
  }
}

Box getInflatedGraphBox(item i);
vector<item> findCollisions(item ndx) {
  vector<item> collisions;
  Line l0 = getLine(ndx);
  vector<item> ignorable;
  float matchDistance = 0;
  bool isExpwy = getElementConfiguration(ndx).type == ConfigTypeExpressway;
  vec3 along = l0.end - l0.start;
  float dist2D = sqrt(along.x*along.x + along.y*along.y);
  float z0base = l0.start.z;// - pointOnLandNatural(l0.start).z;
  float z0end = l0.end.z;// - pointOnLandNatural(l0.end).z;
  float z0slope = (z0end - z0base) / dist2D;

  if (ndx < 0) {
    Node* node = getNode(ndx);
    if (!(node->flags & _graphExists)) {
      return collisions;
    }
    ignorable = node->edges.toVector();
    matchDistance = c(CLaneWidth); //node->intersectionSize;

  } else if (ndx > 0) {
    Edge* edge = getEdge(ndx);
    if (!(edge->flags & _graphExists)) {
      return collisions;
    }
    Node* n0 = getNode(edge->ends[0]);
    Node* n1 = getNode(edge->ends[1]);
    ignorable = n0->edges.toVector();
    for (int i = 0; i < n1->edges.size(); i++) {
      ignorable.push_back(n1->edges[i]);
    }
    //ignorable.push_back(edge->ends[0]);
    //ignorable.push_back(edge->ends[1]);
    matchDistance = c(CLaneWidth); //edgeWidth(ndx);

  } else {
    handleError("findCollisions(0)");
  }

  //for (int i = -nodes->size(); i <= edges->size(); i ++) {
  Box bbox = getInflatedGraphBox(ndx);
  vector<item> boxCollisions = getCollisions(GraphCollisions, bbox, 0);
  for (int k = 0; k < boxCollisions.size(); k++) {
    item i = boxCollisions[k];
    if (i == ndx || i == 0) continue;
    if ((i < 0 && !(getNode(i)->flags & _graphExists)) ||
      (i > 0 && !(getEdge(i)->flags & _graphExists))) {
      continue;
    }

    Line l1 = getLine(i);
    float myMatchDist = matchDistance;
    //if (i < 0) {
      //myMatchDist += tileSize; //getNode(i)->intersectionSize;
    //}
    if (lineDistance2D(l0, l1) < myMatchDist) {
      bool ignore = false;
      for (int j = 0; j < ignorable.size(); j++) {
        if (ignorable[j] == i) {
          ignore = true;
          break;
        }
      }
      if (ignore) {
        continue;
      }

      vec3 pa = pointOfIntersection(l0, l1);
      vec3 pb = nearestPointOnLine(pa, l1);
      float z1 = pb.z;// - pointOnLandNatural(pb).z;
      vec3 along1 = pa - l0.start;
      float dist2D1 = sqrt(along1.x*along1.x + along1.y*along1.y);
      float z0 = z0base + z0slope * dist2D1;
      if (abs(z1-z0) < c(CZTileSize)*.75f) {
        collisions.push_back(i);
      }
    }
  }
  return collisions;
}

class sort_distances
{
   private:
     vec3 orig;
   public:
     sort_distances(vec3 orig1) : orig(orig1) {}
     bool operator()(int i, int j) const {
       float d0 = vecDistance(getNode(i)->center, orig);
       float d1 = vecDistance(getNode(j)->center, orig);
       return d0 < d1;
     }
};

void dedupe(item ndx) {
  Edge* edge = getEdge(ndx);
  if (!(edge->flags & _graphExists)) return;

  item dupe = detectDuplicate(ndx);
  if (dupe == 0) return;

  Edge* other = getEdge(dupe);
  Configuration config = edge->config;
  item end0 = edge->ends[0];
  item end1 = edge->ends[1];
  bool wasComplete = edge->flags & _graphComplete;

  if (!wasComplete && (other->flags & _graphComplete)) return;

  if (!wasComplete && !(other->flags & _graphComplete)) {
    if (other->plan != 0 && edge->plan != 0) {
      Plan* pe = getPlan(edge->plan);
      Plan* po = getPlan(other->plan);
      if ((po->flags & _planSet) && !(pe->flags & _planSet)) return;
    }
  }

  removeEdge(dupe, false);
  setGraphParent(ndx, getGraphParent(dupe));

  //reconfigure(dupe, config);
  //if (config.flags & _configOneWay &&
      //other->ends[0] == end1 && other->ends[1] == end0) {
    //switchDirection(dupe);
  //}

  if (wasComplete) {
    reconfigure(end0, config);
    reconfigure(end1, config);
    complete(end0, false, config);
    complete(end1, false, config);
    //complete(dupe, false, config);

  } else {
    if (!(getNode(end0)->flags & _graphComplete)) {
      reconfigure(end0, config);
    }
    if (!(getNode(end1)->flags & _graphComplete)) {
      reconfigure(end1, config);
    }
  }

  toRender.insert(end0);
  toRender.insert(end1);
  toRender.insert(ndx);
}

void split(item nodeNdx, item edgeNdx, Configuration nodeConfig) {
  Edge* edge = getEdge(edgeNdx);
  char* name = strdup_s(edge->name);
  Configuration edgeConfig = edge->config;
  item n0 = edge->ends[0];
  item n1 = edge->ends[1];
  bool wasComplete = edge->flags & _graphComplete;
  bool wasSet = true;
  if (edge->plan) {
    Plan* p = getPlan(edge->plan);
    wasSet = p->flags & _planSet;
  }

  removeEdge(edgeNdx, false);

  item ei0 = addEdge(n0, nodeNdx, edgeConfig);
  item ei1 = addEdge(nodeNdx, n1, edgeConfig);
  Edge* e0 = getEdge(ei0);
  Edge* e1 = getEdge(ei1);

  item parent = getGraphParent(edgeNdx);
  setGraphParent(ei0, edgeNdx);
  setGraphParent(ei1, edgeNdx);

  if (wasComplete) {
    complete(ei0, false, nodeConfig);
    complete(ei1, false, nodeConfig);
    toName.erase(ei0);
    toName.erase(ei1);

  } else if (wasSet) {
    setPlan(addPlan(GraphElementPlan, ei0, e0->config), false);
    setPlan(addPlan(GraphElementPlan, ei1, e1->config), false);

  } else {
    addPlan(GraphElementPlan, ei0, e0->config);
    addPlan(GraphElementPlan, ei1, e1->config);
  }

  //if (wasComplete) {
    toRepositionEdges.erase(ei0);
    toRepositionEdges.erase(ei1);
  //}
  e0->name = strdup_s(name);
  e1->name = name;
}

float getSlope(vec3 l0, vec3 l1) {
  float length = distance2D(l0, l1);
  float z = l1.z - l0.z;
  if (length < 0.0001) return z;
  return z / length;
}

vector<vec3> getLengthSplits(vec3 h0, vec3 h1) {
  vec3 along = h1 - h0;
  float dist2D = sqrt(along.x*along.x + along.y*along.y);
  float lastDev = 0;

  if (dist2D < sampleLength) return vector<vec3>();

  float hz0 = h0.z;
  float hz1 = h1.z;
  float hd0 = hz0 - pointOnLandNatural(h0).z;
  float hd1 = hz1 - pointOnLandNatural(h1).z;
  float hd = abs(hd0) > abs(hd1) ? hd1 : hd0;
  float dist = length(along);
  float sampleAlpha = sampleLength / dist2D;
  float segmentLength = dist*sampleAlpha;
  vec3 segAlong = along/dist * segmentLength;
  vec3 cursor = h0;
  int numSegments = dist2D / sampleLength;
  if (length(along - segAlong*float(numSegments)) < tileSize*2) {
    numSegments -= 1;
  }

  vec3 *cs = (vec3*)alloca(sizeof(vec3)*numSegments);
  float *lzs = (float*)alloca(sizeof(float)*numSegments);
  //float *minzs = (float*)alloca(sizeof(float)*numSegments);
  //float *maxzs = (float*)alloca(sizeof(float)*numSegments);
  float distSinceLand = 0;
  int best = -1;

  for (int k = 0; k < numSegments; k++) {
    cursor += segAlong;
    cs[k] = cursor;
    float lz = pointOnLandNatural(cursor).z + hd;
    if (lz < beachLine && hd1 > 0) {
      lzs[k] = beachLine;
      distSinceLand += sampleLength;
      if (distSinceLand + sampleLength > c(CMaxBridgeLength)) {
        best = k;
        break;
      }

    } else {
      distSinceLand = 0;
      float alpha = (k+1) * sampleLength;
      float z0 = alpha*(c(CMaxGrade)-.1);
      float z1 = (dist-alpha)*.1; //(maxGrade-.1);
      float min = std::min(lz-maxMoundHeight, std::max(hz0 - z0, hz1 - z1));
      float max = std::max(lz+maxMoundHeight, std::min(hz0 + z0, hz1 + z1));
      if (min > max) {
        lz = (max + min)*.5f;
      } else if (lz < min) {
        lz = min;
      } else if (lz > max) {
        lz = max;
      }
      lzs[k] = lz;
    }
  }

  if (best < 0) {
    float maxDev = -1;
    for (int k = 1; k < numSegments; k++) {
      float dev = abs(cs[k].z - lzs[k]);
      vec3 lk = cs[k];
      lk.z = lzs[k];
      float slope0 = abs(getSlope(cs[0], lk));
      float slope1 = abs(getSlope(lk, cs[numSegments-1]));
      float slope = std::max(slope0, slope1);
      if (slope >= c(CMaxGrade)) {
      }
      if (dev > maxDev && slope < c(CMaxGrade)) {
        best = k;
        maxDev = dev;
      }
    }
    if (maxDev < c(CZTileSize)*.5f) return vector<vec3>();
  }

  if (best < 0) return vector<vec3>();

  vec3 ba = cs[best];
  ba.z = lzs[best];
  vector<vec3> r0 = getLengthSplits(h0, ba);
  vector<vec3> r1 = getLengthSplits(ba, h1);
  r0.insert(r0.end(), r1.begin(), r1.end());
  r0.push_back(ba);
  return r0;
}

bool split(item ndx, Configuration nodeConfig, bool doComplete) {
  if (ndx < 0) {
    vector<item> collisions = findCollisions(ndx);
    if (collisions.size() > 0) {
      for (int i = 0; i < collisions.size(); i ++) {
        if (collisions[i] < 0) continue;
        Edge* e = getEdge(collisions[i]);
        if (!(e->flags & _graphExists)) continue;
        if (!doComplete && e->flags & _graphComplete) continue;
        split(ndx, collisions[i], nodeConfig);
      }
      return true;
    } else {
      return false;
    }

  } else if (ndx > 0) {
    vector<item> collisions = findCollisions(ndx);
    Edge* edge = getEdge(ndx);
    if (!(edge->flags & _graphExists)) return false;

    /*
    if (collisions.size() == 0) {
      Node* n0 = getNode(edge->ends[0]);
      Node* n1 = getNode(edge->ends[1]);
      if (n0->pillar != 0 || n1->pillar != 0) return false;
      vec3 c0 = n0->center;
      vec3 c1 = n1->center;
      vector<vec3> splits = getLengthSplits(c0, c1);
      if (splits.size() == 0) {
        return false;
      }
    }
    */

    float eWidth = edgeWidth(ndx);
    vector<item> nodes;
    Configuration edgeConfig = edge->config;
    vec3 end0 = getNode(edge->ends[0])->center;
    Line edgeLine = edge->line;
    nodes.push_back(edge->ends[0]);
    nodes.push_back(edge->ends[1]);
    bool wasSet = true;
    if (edge->plan) {
      Plan* p = getPlan(edge->plan);
      wasSet = p->flags & _planSet;
    }

    removeEdge(ndx, false);

    for (int i= 0; i < collisions.size(); i ++) {
      if (collisions[i] < 0) {
        nodes.push_back(collisions[i]);

      } else if (collisions[i] > 0) {
        Edge* e = getEdge(collisions[i]);
        Line collLine = e->line;
        vec3 poi = pointOfIntersection(collLine, edgeLine);
        if (poi.x >= 0 || poi.y >= 0) {
          float cWidth = edgeWidth(collisions[i]);
          float mWidth = cWidth + eWidth + tileSize;
          if (length(poi-collLine.start) < mWidth) {
            nodes.push_back(e->ends[0]);
          } else if (length(poi-collLine.end) < mWidth) {
            nodes.push_back(e->ends[1]);
          } else {
            nodes.push_back(getOrCreateNodeAt(poi, nodeConfig));
          }
        }
      }
    }

    sort(nodes.begin(), nodes.end(), sort_distances(end0));
    //De-dupe
    for (int i = 0; i < nodes.size()-1; i++) {
      if (nodes[i] == nodes[i+1]) {
        nodes.erase(nodes.begin()+i);
        i --;
      }
    }

    // Split for length
    int nodesSizePreLengthSplit = nodes.size();
    for (int i = 0; i < nodesSizePreLengthSplit-1; i++) {
      Node* n0 = getNode(nodes[i]);
      Node* n1 = getNode(nodes[i+1]);
      if (n0->pillar != 0 || n1->pillar != 0) continue;
      //if ((n0->flags & _graphCity) || (n1->flags & _graphCity)) continue;
      vec3 c0 = n0->center;
      vec3 c1 = n1->center;
      vector<vec3> splits = getLengthSplits(c0, c1);
      for (int j = 0; j < splits.size(); j++) {
        vec3 point = splits[j];
        vec2 normed = vec2(point)/getMapSize();
        if (normed.x < 0 || normed.x > 1 ||
          normed.y < 0 || normed.y > 1) continue;
        nodes.push_back(getOrCreateNodeAt(splits[j], nodeConfig));
      }
    }

    sort(nodes.begin(), nodes.end(), sort_distances(end0));
    //De-dupe
    for (int i = 0; i < nodes.size()-1; i++) {
      if (nodes[i] == nodes[i+1]) {
        nodes.erase(nodes.begin()+i);
        i --;
        continue;
      }
      Node* n0 = getNode(nodes[i]);
      Node* n1 = getNode(nodes[i+1]);
      if (vecDistance(n0->center, n1->center) < 0.001) {
        nodes.erase(nodes.begin()+i);
        i --;
      }
    }

    if (!doComplete && isFeatureEnabled(FRoadPillars) &&
        automaticBridgesEnabled) {
      bool doBridge = false;
      for (int i = 0; i < nodes.size(); i++) {
        Node* n = getNode(nodes[i]);
        if (n->edges.size() > 0) continue;
        //if (n->pillar != 0) continue;
        float landZ = pointOnLandNatural(n->center).z;
        if (landZ < -beachLine*2 && landZ <= n->center.z+1) {
          doBridge = true;
          break;
        }
      }

      if (doBridge) {
        for (int i = 0; i < nodes.size(); i++) {
          Node* n = getNode(nodes[i]);
          if (n->pillar != 0) continue;
          if (n->edges.size() > 0) continue;
          if (pointOnLandNatural(n->center).z < beachLine-1) {
            item pillarNdx = addPillar(n->center, false);
            addPlan(PillarPlan, pillarNdx);
            Pillar* pillar = getPillar(pillarNdx);
            n->pillar = pillarNdx;
            n->center = pillar->location;
            pillar->node = nodes[i];
          }
        }
      }
    }

    for (int i= 0; i < nodes.size() - 1; i ++) {
      item newEdge = addEdge(nodes[i], nodes[i+1], edgeConfig);
      if (doComplete && legalMessage(newEdge) == 0) {
        complete(newEdge, false, nodeConfig);
      } else {
        item plan = addPlan(GraphElementPlan, newEdge);
        if (wasSet) {
          setPlan(plan, false);
        }
      }
      dedupe(newEdge);
    }

    if (wasSet || doComplete) {
      for (int i= 0; i < nodes.size(); i ++) {
        split(nodes[i]);
      }
    }

    return true;

  } else {
    handleError("split(0)");
    return false;
  }
}

bool split(item ndx) {
  return split(ndx, getElementConfiguration(ndx), false);
}

void complete(item ndx, Configuration nodeConfig) {
  complete(ndx, getGameMode() != ModeBuildingDesigner, nodeConfig);
}

void complete(item ndx, bool deintersect, Configuration nodeConfig) {
  if (ndx < 0) {
    Node* node = getNode(ndx);
    if (node->flags & _graphComplete) return;
    if (node->flags & _graphCity) deintersect = false;

    // Split
    if (deintersect && split(ndx, nodeConfig, true)) return;

    node->flags |= _graphOpen | _graphComplete;
    node->flags &= ~_graphIsColliding;

    if ((node->config.flags & _configDontMoveEarth) &&
        pointOnLand(node->center).z+2 > node->center.z &&
        pointOnLandNatural(node->center).z > node->center.z) {
      node->flags |= _graphUnderground;
    } else {
      node->flags &= ~_graphUnderground;
    }

    toElevate.insert(ndx);
    toCollide.insert(ndx);
    toMakeLots.insert(ndx);
    toSimplify.insert(ndx);
    toSetupLaneNodes.insert(ndx);
    toRender.insert(ndx);
    toResizeNodes.insert(ndx);
    adjustGraphStats(ndx, 1);

  } else if (ndx > 0) {
    Edge* edge = getEdge(ndx);
    if (edge->flags & _graphComplete) return;
    if (edge->flags & _graphCity) deintersect = false;
    if (deintersect && split(ndx, nodeConfig, true)) return;

    if (edge->plan != 0) removePlan(edge->plan);
    edge->flags |= _graphOpen | _graphComplete;
    edge->flags &= ~_graphIsColliding;
    if (!(edge->flags & _graphCity)) {
      completeEdges ++;
    }

    complete(edge->ends[0], true, nodeConfig);
    complete(edge->ends[1], true, nodeConfig);

    if ((edge->config.flags & _configDontMoveEarth) &&
        (edge->config.type == ConfigTypePedestrian || (
        (getNode(edge->ends[0])->flags & _graphUnderground) &&
        (getNode(edge->ends[1])->flags & _graphUnderground)))) {
      edge->flags |= _graphUnderground;
    } else {
      edge->flags &= ~_graphUnderground;
    }

    toElevate.insert(ndx);
    toCollide.insert(ndx);
    toSetupLaneEdges.insert(ndx);
    toMakeLots.insert(ndx);
    toRender.insert(ndx);
    toName.insert(ndx);
    toResizeEdges.insert(ndx);
    toRepositionEdges.insert(ndx);
    toResizeNodes.insert(edge->ends[0]);
    toResizeNodes.insert(edge->ends[1]);
    toSetupLaneNodes.insert(edge->ends[0]);
    toSetupLaneNodes.insert(edge->ends[1]);
    adjustGraphStats(ndx, 1);

    if (deintersect) dedupe(ndx);

  } else {
    handleError("Trying to complete null graphLoc");
  }

}

Configuration getElementConfiguration(item ndx) {
  if (ndx < 0) {
    return getNode(ndx)->config;
  } else if (ndx > 0) {
    return getEdge(ndx)->config;
  } else {
    handleError("getElementConfiguration(0)");
    Configuration result;
    return result;
  }
}

item getNodeAt(vec3 point) {
  for(int i = 0; i < nodes->size(); i ++) {
    item ndx = -i-1;
    Node* node = getNode(ndx);
    if (!(node->flags & _graphExists)) continue;
    if (areClose(node->center, point)) {
      return ndx;
    }
  }
  return 0;
}

item getOrCreateNodeAt(vec3 point, Configuration nodeConfig) {
  item ndx = getNodeAt(point);
  if (ndx < 0) {
    return ndx;

  } else {
    //point = unitize(point);
    item elemNdx = nearestElement(point, true);
    if (elemNdx < 0) {
      if (areClose(getNode(elemNdx)->center, point)) {
        return elemNdx;
      }

    } else if (elemNdx > 0) {
      Line l = getLine(elemNdx);
      if (areClose(point, l)) {
        vec3 loc = nearestPointOnLine(point, l);
        item nodeNdx = addNode(loc, nodeConfig);
        Node* node = getNode(nodeNdx);
        node->flags |= _graphIsColliding;
        return nodeNdx;
      }
    }

    return addNode(point, nodeConfig);
  }
}

item getOrCreatePillarNodeAt(item pillarNdx, Configuration nodeConfig) {
  Pillar* pillar = getPillar(pillarNdx);
  if (pillar->node == 0) {
    return addPillarNode(pillarNdx, nodeConfig);
  } else {
    return pillar->node;
  }
}

void removeEdgeFromNode(item ndx, item edge, bool deleteIfEmpty) {
  Node* node = getNode(ndx);

  for (int i = node->edges.size() - 1; i >= 0; i--) {
    if (node->edges[i] == edge) {
      node->edges.remove(i);
    } else {
      Edge* other = getEdge(node->edges[i]);
      if (other->plan != 0) {
        rerenderPlan(other->plan);
      }
    }
  }

  if (node->edges.size() == 0 && deleteIfEmpty &&
      !(node->flags & _graphCity)) {
    removeNode(ndx);
  } else {
    coerceNodeType(ndx);
    toRender.insert(ndx);
    if (!(node->flags & _graphComplete) ||
        (getEdge(edge)->flags & _graphComplete)) {
      toResizeNodes.insert(ndx);
      toSetupLaneNodes.insert(ndx);
    }
  }
}

void removeEdge(item ndx, bool deleteNodes) {
  removeFromCollisionTable(GraphCollisions, ndx);
  Edge* edge = getEdge(ndx);
  if (edge == 0 || !(edge->flags & _graphExists)) {
    SPDLOG_ERROR("removeEdge() on edge which has already been removed");
    logStacktrace();
    return;
  }

  removeEdgeFromNode(edge->ends[0], ndx, deleteNodes);
  removeEdgeFromNode(edge->ends[1], ndx, deleteNodes);

  for (int i = 0; i < 2; i++) {
    item blockNdx = edge->laneBlocks[0];
    if (blockNdx == 0) continue;
    removeLaneBlock(blockNdx);
  }

  if (edge->flags & _graphComplete) {
    if (!(edge->flags & _graphCity)) {
      completeEdges --;
    }
    removeGraphElevator(ndx);
    orphanLots(ndx);
    adjustGraphStats(ndx, -1);
  }

  deselect(SelectionGraphElement, ndx);
  removeMessageByObject(GraphMessage, ndx);
  edge->flags = 0;

  if (edge->plan) {
    removePlan(edge->plan);
  }

  if (edge->entity != 0) {
    removeEntityAndMesh(edge->entity);
    removeEntityAndMesh(edge->wearEntity);
    removeEntityAndMesh(edge->textEntity);
    removeEntityAndMesh(edge->signEntity);
    edge->entity = 0;
    edge->wearEntity = 0;
    edge->textEntity = 0;
    edge->signEntity = 0;
  }
  if (edge->tunnelEntity != 0) {
    removeEntityAndMesh(edge->tunnelEntity);
    edge->tunnelEntity = 0;
  }

  free(edge->name);
  edge->name = 0;
  toFree.push_back(ndx);
}

void removeElement(item ndx) {
  if (ndx < 0) {
    removeNode(ndx);
  } else if (ndx > 0) {
    removeEdge(ndx, true);
  } else {
    handleError("removeElement(0)");
  }
}

money reconfigureCost(item ndx, Configuration config,
    bool includeUnset, bool includeSet) {
  money oldCost = graphCostSimple(ndx);
  money newCost = 0;

  if (ndx < 0) {
    Node* node = getNode(ndx);
    if (!(node->flags & _graphComplete)) return 0;
    if (config.type != node->config.type ||
        configsEqualNode(config, node->config)) {
      return 0;
    }

    int numLegs = 0;
    for (int i = 0; i < node->edges.size(); i++) {
      Edge* e = getEdge(node->edges[i]);
      if (e->flags & _graphComplete) {
        numLegs ++;
      } else if (e->plan != 0) {
        Plan* p = getPlan(e->plan);
        bool isSet = p->flags & _planSet;
        if ((isSet && includeSet) || (!isSet && includeUnset)) {
          numLegs ++;
        }
      }
    }
    oldCost = strategyCost(node->config.strategy) * numLegs;
    newCost = strategyCost(config.strategy) * numLegs;

  } else if (ndx > 0) {
    Edge* edge = getEdge(ndx);
    if (!(edge->flags & _graphComplete)) return 0;
    if (config.type != edge->config.type ||
        configsEqualEdge(config, edge->config)) {
      return 0;
    }
    Node* n0 = getNode(edge->ends[0]);
    Node* n1 = getNode(edge->ends[1]);
    float lngth = length(n1->center - n0->center)/1000;
    float platform = (edge->config.flags & _configPlatform) ?
      c(CPlatformCost) : 0;
    newCost = lngth * (platform + config.numLanes *
      laneCost(config.type) * (config.flags & _configOneWay ? 1 : 2));

  } else {
    handleError("reconfigureCost(0)");
  }

  money diff = newCost - oldCost;
  diff = std::max(diff, minReconfigureCost(config.type));
  //diff += getRepairCost(ndx);
  return diff * getInflation();
}

money reconfigureCost(item ndx, Configuration config) {
  return reconfigureCost(ndx, config, false, false);
}

void reconfigure(item ndx, Configuration config) {
  adjustGraphStats(ndx, -1);

  if (ndx > 0) {
    Edge* edge = getEdge(ndx);
    if (edge->config.type != config.type) {
      return;
    }

    edge->config = config;
    toRepositionEdges.insert(ndx);
    toResizeEdges.insert(ndx);
    toRender.insert(ndx);
    if (edge->flags & _graphComplete) {
      toSetupLaneEdges.insert(ndx);
      toSetupLaneNodes.insert(edge->ends[0]);
      toSetupLaneNodes.insert(edge->ends[1]);
      toResizeNodes.insert(edge->ends[0]);
      toResizeNodes.insert(edge->ends[1]);

    } else {
      if (!(getNode(edge->ends[0])->flags & _graphComplete)) {
        toSetupLaneNodes.insert(edge->ends[0]);
        toResizeNodes.insert(edge->ends[0]);
      }
      if (!(getNode(edge->ends[1])->flags & _graphComplete)) {
        toSetupLaneNodes.insert(edge->ends[1]);
        toResizeNodes.insert(edge->ends[1]);
      }
    }

    edge->wear = 0;

  } else if (ndx < 0) {
    Node* node = getNode(ndx);
    if (node->config.type != config.type) {
      return;
    }
    node->config = config;
    toSetupLaneNodes.insert(ndx);
    toResizeNodes.insert(ndx);
    toRender.insert(ndx);
  } else {
    // no-op
  }

  adjustGraphStats(ndx, 1);
}

void switchDirection(item ndx) {
  Edge* edge = getEdge(ndx);
  if (!(edge->flags & _graphExists)) return;
  if (!edge->config.flags & _configOneWay) return;
  item swap = edge->ends[0];
  edge->ends[0] = edge->ends[1];
  edge->ends[1] = swap;

  vec3 swapVec = edge->line.start;
  edge->line.start = edge->line.end;
  edge->line.end = swapVec;

  if (edge->laneBlocks[0] != 0) {
    removeLaneVehicles(edge->laneBlocks[0]);
    if (edge->laneBlocks[1] != 0) {
      item swap = edge->laneBlocks[0];
      edge->laneBlocks[0] = edge->laneBlocks[1];
      edge->laneBlocks[1] = swap;
    }
  }

  if (edge->flags & _graphComplete) {
    toSetupLaneEdges.insert(ndx);
    toSetupLaneNodes.insert(edge->ends[0]);
    toSetupLaneNodes.insert(edge->ends[1]);
    toRender.insert(edge->ends[0]);
    toRender.insert(edge->ends[1]);
  }
  toRender.insert(ndx);
}

void resetGraph() {
  resetGraphParents();
  automaticBridgesEnabled = true;
  areTunnelsVisible = true;
  completeEdges = 0;
  maxWear = 0;
  for (int i = -1; i >= -nodes->size(); i--) {
    Node* node = getNode(i);
    node->edges.clear();
    node->phaseMins.clear();
    node->phaseMaxs.clear();
  }

  for (int i = 1; i <= edges->size(); i++) {
    Edge* edge = getEdge(i);
    if (edge->name) free(edge->name);
  }

  edges->clear();
  nodes->clear();
}

void initGraphEntities() {
  for (int i = -1; i >= -nodes->size(); i--) {
    Node* node = getNode(i);
    if (!(node->flags & _graphExists)) {
      continue;
    }
    if (node->entity == 0) {
      node->entity = addEntity(RoadShader);
      node->signEntity = addEntity(SignShader);
      createMeshForEntity(node->entity);
      createMeshForEntity(node->signEntity);
      vec3 nodePt = node->center;
      if (isNodeUnderground(i)) {
        node->tunnelEntity = addEntity(SignShader);
        createMeshForEntity(node->tunnelEntity);
      }
    }
  }

  for (int i = 1; i <= edges->size(); i++) {
    Edge* edge = getEdge(i);
    if (!(edge->flags & _graphExists)) {
      continue;
    }
    if (edge->entity == 0) {
      edge->entity = addEntity(RoadShader);
      edge->wearEntity = addEntity(WearShader);
      edge->textEntity = addEntity(TextShader);
      edge->signEntity = addEntity(SignShader);
      createMeshForEntity(edge->entity);
      createMeshForEntity(edge->wearEntity);
      createMeshForEntity(edge->textEntity);
      createMeshForEntity(edge->signEntity);
      vec3 edgePtStart = edge->line.start;
      vec3 edgePtEnd = edge->line.end;
      if (isEdgeUnderground(i)) {
        edge->tunnelEntity = addEntity(SignShader);
        createMeshForEntity(edge->tunnelEntity);
      }
    }
  }
}

void updateGraphVisuals(bool firstPass);
void rerenderGraph() {
  for (int i = 1; i <= edges->size(); i++) {
    renderEdge(i);
  }
  for (int i = -1; i >= -nodes->size(); i--) {
    renderNode(i);
  }
  updateGraphVisuals(true);
}

vec3 getElementLoc(item ndx) {
  if (ndx < 0) {
    return getNode(ndx)->center;
  } else if (ndx > 0) {
    Line l = getEdge(ndx)->line;
    return (l.start + l.end)*.5f;
  } else {
    handleError("Trying to get location of null graphLoc");
    return vec3(0,0,0);
  }
}

Line getLine(item ndx) {
  if (ndx < 0) {
    Node* node = getNode(ndx);
    return line(node->center, node->center + vec3(0.01, 0, 0));
  } else if (ndx > 0) {
    Edge* edge = getEdge(ndx);
    return edge->line;
  } else {
    handleError("Trying to get line of null graphLoc");
    return line(vec3(0,0,0), vec3(0,0,0));
  }
}

int getBridgeType(item ndx) {
  Edge* edge = getEdge(ndx);
  Node* node0 = getNode(edge->ends[0]);
  Node* node1 = getNode(edge->ends[1]);
  bool isBridge = (node0->pillar != 0) || (node1->pillar != 0);

  if (isBridge) {
    bool isSuspension = false;

    if (node0->pillar != 0) {
      Pillar* p = getPillar(node0->pillar);
      if (p->flags & _pillarSuspension) isSuspension = true;
    }

    if (node1->pillar != 0) {
      Pillar* p = getPillar(node1->pillar);
      if (p->flags & _pillarSuspension) isSuspension = true;
    }

    return isSuspension;
  } else {
    return -1;
  }
}

const char* legalMessage(item ndx) {
  if (ndx < 0) {
    coerceNodeType(ndx);
    Node* node = getNode(ndx);

    if (!(node->flags & _graphCity)) {
      vec2 centerNormed = vec2(node->center)/getMapSize();
      if (centerNormed.x < 0 || centerNormed.x > 1 ||
        centerNormed.y < 0 || centerNormed.y > 1) {
        return "Beyond City Limits";
      }
    }

    int numEdges = node->edges.size();
    bool *isDupe = (bool*)alloca(sizeof(bool)*numEdges);
    int numNonDupes = numEdges;

    for (int i = 0; i < numEdges; i++) {
      item eNdx = node->edges[i];
      Edge* e = getEdge(eNdx);
      bool isEDupe = false;
      isDupe[i] = false;
      item dupeNdx = detectSemiDuplicate(eNdx);

      if (dupeNdx != 0 && e->plan != 0) {
        Plan* p = getPlan(e->plan);
        if (!(p->flags & _planSet)) {
          isEDupe = true;
        } else {
          Edge* dupe = getEdge(dupeNdx);
          if (dupe->flags & _graphComplete) {
            isEDupe = true;
          }
        }

        if (isEDupe) {
          numNonDupes --;
          isDupe[i] = true;
        }
      }
    }

    if (numNonDupes > 6 ||
      (node->pillar != 0 && numNonDupes > 2) ||
      (node->config.type == ConfigTypeExpressway && numNonDupes > 3)) {
      return "Too Many Roads";
    }

    if (numEdges > 1) {
      float *thetas = (float*)alloca(sizeof(float)*numEdges);
      bool *isOutput = (bool*)alloca(sizeof(bool)*numEdges);
      bool *isInput = (bool*)alloca(sizeof(bool)*numEdges);
      vec3 center = node->center;
      int numInputs = 0;
      int numOutputs = 0;

      for(int i = 0; i < numEdges; i++) {
        if (isDupe[i]) continue;
        Edge* edge = getEdge(node->edges[i]);
        item end = edge->ends[1] == ndx;
        item otherEnd = edge->ends[!end];
        vec3 dir = getNode(otherEnd)->center - center;
        float theta = atan2(dir.x, dir.y);
        thetas[i] = theta;
        if (edge->config.flags & _configOneWay) {
          if (end) {
            numOutputs ++;
            isOutput[i] = true;
            isInput[i] = false;
          } else {
            numInputs ++;
            isOutput[i] = false;
            isInput[i] = true;
          }
        } else {
          numInputs ++;
          numOutputs ++;
          isOutput[i] = true;
          isInput[i] = true;
        }
      }

      if (node->config.type == ConfigTypeExpressway &&
        numInputs > 1 && numOutputs > 1
      ) {
        return "Too Many Roads";
      }

      bool isExpwy = node->config.type == ConfigTypeExpressway;
      if (isExpwy || node->pillar != 0) {
        for (int i = 0; i < numEdges; i ++) {
          int j = (i+1)%numEdges;
          if (isDupe[i] || isDupe[j]) continue;
          float diff = abs(thetas[j] - thetas[i]);
          diff = std::min(diff, (float)(pi_o * 2.f - diff));

          if (node->pillar) {
            Pillar* p = getPillar(node->pillar);
            bool suspension = p->flags & _pillarSuspension;
            float minDiff = suspension ?
              c(CMinSuspensionPillarAngle) : c(CMinPillarAngle);
            if (diff < minDiff) return "Bad Angle";
          }

          if (isExpwy && diff < minExpresswayAngle &&
            ((isOutput[i] && isInput[j]) || (isInput[i] && isOutput[j]))
          ) {
            return "Bad Angle";
          }
        }
      }
    }

    if (node->flags & _graphComplete) {
      return NULL;
    }

    if (node->pillar) {
      return NULL;
    }
    float landZ = pointOnLandNatural(node->center).z;
    //if (landZ < 0) {
      //return "Need Bridge Pillar";
    //} else
    //if (node->center.z < beachLine-1 || landZ < beachLine-1) {
    if (landZ < beachLine-1 && node->center.z > landZ-4) {
      return "Below Waterline";
    }

    if (node->center.z < beachLine-1) {
      if (!(node->config.flags & _configDontMoveEarth)) {
        return "Use Tunnel/Viaduct (H) to build an underwater tunnel";
      }
      if (node->config.type != ConfigTypeHeavyRail &&
          node->center.z < -50) {
        return "Cannot be built under deep water";
      }
    }

    /*
    vector<item> collisions = findCollisions(ndx);
    if (collisions.size() > 0) {
      for (int i = 0; i < collisions.size(); i++) {
        if (collisions[i] > 0) {
          item elem = collisions[i];
          Edge* edge = getEdge(elem);
          vec3 p = nearestPointOnLine(node->center, getLine(elem));
          if (vecDistance(p, edge->line.start) < minRoadLength ||
            vecDistance(p, edge->line.end) < minRoadLength) {

            return "Too Close";
          }
        }
      }
    }
    */

    /*
    vector<item> buildingCollisions = collideBuildingWithGraph(ndx);
    for (int i = 0; i < buildingCollisions.size(); i ++) {
      Building* b = getBuilding(buildingCollisions[i]);
      if (b->zone == GovernmentZone || b->flags & _buildingHistorical) {
        return "Building in the Way";
      }
    }
    */

    return NULL;

  } else if (ndx > 0) {
    Edge* edge = getEdge(ndx);
    if (edge->flags & _graphComplete) {
      return NULL;
    }

    if (edge->ends[0] == edge->ends[1]) {
      return "Invalid";
    }

    Node* node0 = getNode(edge->ends[0]);
    Node* node1 = getNode(edge->ends[1]);
    int bridgeType = getBridgeType(ndx);
    float roadLength =
      distance2DSqrd(node0->center, node1->center);
    float minRoad = c(CMinRoadLength);

    if (bridgeType >= 0) {
      float maxBridge = bridgeType ?
        c(CMaxSuspensionBridgeLength) : c(CMaxBridgeLength);
      if (roadLength > maxBridge*maxBridge) {
        return "Too Long";
      }
    }

    if (roadLength < minRoad*minRoad) {
      return "Too Close";
    }
    if (slope(getLine(ndx)) > c(CMaxGrade)) {
      return "Too Steep";
    }

    vector<item> buildingCollisions = collideBuildingWithGraph(ndx);
    for (int i = 0; i < buildingCollisions.size(); i ++) {
      Building* b = getBuilding(buildingCollisions[i]);
      if (b->zone == GovernmentZone || b->flags & _buildingHistorical) {
        return "Building in the way";
      }
    }

    for (int i = 0; i < 2; i++) {
      const char* nodeResult = legalMessage(edge->ends[i]);
      if (nodeResult) {
        return nodeResult;
      }
    }

    return NULL;

  } else {
    handleError("Trying to legal message of null graphLoc");
    return "Element doesn't exist";
  }
}

item nearestEdge(vec3 location, bool includePlanned) {
  float bestDistance = FLT_MAX;
  item bestIndex = 0;
  for (int i = 1; i <= edges->size(); i ++) {
    Edge* edge = getEdge(i);
    if (!(edge->flags & _graphExists) ||
      (!includePlanned && !(edge->flags & _graphOpen))) {
      continue;
    }
    float distance = pointLineDistance(location, getLine(i));
    if (distance < bestDistance) {
      bestDistance = distance;
      bestIndex = i;
    }
  }
  return bestIndex;
}

item nearestEdge(Line l, bool includePlanned) {
  float bestDistance = FLT_MAX;
  item bestIndex = 0;
  for (int i = 1; i <= edges->size(); i ++) {
    Edge* edge = getEdge(i);
    if (!(edge->flags & _graphExists) ||
      (!includePlanned && !(edge->flags & _graphOpen))) {
      continue;
    }
    float distance = lineDistance(l, getLine(i));
    if (distance < bestDistance) {
      bestDistance = distance;
      bestIndex = i;
    }
  }
  return bestIndex;
}

item nearestEdge(Line l, bool includePlanned, Configuration config) {
  float bestDistance = FLT_MAX;
  item bestIndex = 0;
  for (int i = 1; i <= edges->size(); i ++) {
    Edge* edge = getEdge(i);
    if (!(edge->flags & _graphExists) ||
      (!includePlanned && !(edge->flags & _graphOpen))) {
      continue;
    }
    if (edge->config.type != config.type) continue;
    if ((edge->config.flags | config.flags) != edge->config.flags) continue;
    float distance = lineDistance(l, getLine(i));
    if (distance < bestDistance) {
      bestDistance = distance;
      bestIndex = i;
    }
  }
  return bestIndex;
}

item nearestEdge(vec3 loc, bool includePlanned, Configuration config) {
  float bestDistance = FLT_MAX;
  item bestIndex = 0;
  for (int i = 1; i <= edges->size(); i ++) {
    Edge* edge = getEdge(i);
    if (!(edge->flags & _graphExists) ||
      (!includePlanned && !(edge->flags & _graphOpen))) {
      continue;
    }
    if (edge->config.type != config.type) continue;
    if ((edge->config.flags | config.flags) != edge->config.flags) continue;
    float distance = pointLineDistance(loc, getLine(i));
    if (distance < bestDistance) {
      bestDistance = distance;
      bestIndex = i;
    }
  }
  return bestIndex;
}

item nearestNode(vec3 location, bool includePlanned) {
  float bestDistance = FLT_MAX;
  item bestIndex = 0;
  for (int i = 1; i <= nodes->size(); i ++) {
    Node* node = getNode(-i);
    if (!(node->flags & _graphExists) ||
      (!includePlanned && !(node->flags & _graphOpen))) {
      continue;
    }
    float distance = pointLineDistance(location, getLine(-i));
    if (distance < bestDistance) {
      bestDistance = distance;
      bestIndex = -i;
    }
  }
  return bestIndex;
}

item nearestNode(vec3 location, bool includePlanned, Configuration config) {
  float bestDistance = FLT_MAX;
  item bestIndex = 0;
  for (int i = 1; i <= nodes->size(); i ++) {
    Node* node = getNode(-i);
    if (!(node->flags & _graphExists) ||
      (!includePlanned && !(node->flags & _graphOpen))) {
      continue;
    }
    if (node->config.type != config.type) continue;
    if ((node->config.flags | config.flags) != node->config.flags) continue;
    float distance = pointLineDistance(location, getLine(-i));
    if (distance < bestDistance) {
      bestDistance = distance;
      bestIndex = -i;
    }
  }
  return bestIndex;
}

item nearestNode(Line l, bool includePlanned) {
  float bestDistance = FLT_MAX;
  item bestIndex = 0;
  for (int i = 1; i <= nodes->size(); i ++) {
    Node* node = getNode(-i);
    if (!(node->flags & _graphExists) ||
      (!includePlanned && !(node->flags & _graphOpen))) {
      continue;
    }
    float distance = lineDistance(l, getLine(-i));
    if (distance < bestDistance) {
      bestDistance = distance;
      bestIndex = -i;
    }
  }
  return bestIndex;
}

item nearestNode(Line l, bool includePlanned, Configuration config) {
  float bestDistance = FLT_MAX;
  item bestIndex = 0;
  for (int i = 1; i <= nodes->size(); i ++) {
    Node* node = getNode(-i);
    if (!(node->flags & _graphExists) ||
      (!includePlanned && !(node->flags & _graphOpen))) {
      continue;
    }
    if (node->config.type != config.type) continue;
    if ((node->config.flags | config.flags) != node->config.flags) continue;
    float distance = lineDistance(l, getLine(-i));
    if (distance < bestDistance) {
      bestDistance = distance;
      bestIndex = -i;
    }
  }
  return bestIndex;
}

item nearestElement(vec3 location, bool includePlanned) {
  item node = nearestNode(location, includePlanned);
  item edge = nearestEdge(location, includePlanned);
  if (node == 0) return edge;
  if (edge == 0) return node;
  return vecDistance(location, getNode(node)->center) <
    pointLineDistance(location, getLine(edge)) ?  node : edge;
}

item nearestElement(vec3 location, bool includePlanned, Configuration config) {
  item node = nearestNode(location, includePlanned, config);
  item edge = nearestEdge(location, includePlanned, config);
  if (node == 0) return edge;
  if (edge == 0) return node;
  return vecDistance(location, getNode(node)->center) <
    pointLineDistance(location, getLine(edge)) ?  node : edge;
}

Box getGraphBox(item i) {
  if (i < 0) {
    Node* node = getNode(i);
    float size = node->intersectionSize*2;
    vec2 axis0 = vec2(size, 0);
    vec2 axis1 = vec2(0, size);
    vec2 start = vec2(node->center) - .5f*(axis0 + axis1);
    return box(start, axis0, axis1);
  } else if (i > 0) {
    Edge* e = getEdge(i);
    return box(e->line, edgeWidth(i)*.5f);
  } else {
    handleError("getGraphBox(0)\n");
    return box(line(vec3(0,0,0), vec3(0,0,0)), 0);
  }
}

Box getInflatedGraphBox(item i) {
  if (i < 0) {
    Node* node = getNode(i);
    float size = node->intersectionSize*4;
    vec2 axis0 = vec2(size, 0);
    vec2 axis1 = vec2(0, size);
    vec2 start = vec2(node->center) - .5f*(axis0 + axis1);
    return box(start, axis0, axis1);
  } else if (i > 0) {
    Edge* e = getEdge(i);
    Line l = line(getNode(e->ends[0])->center, getNode(e->ends[1])->center);
    return box(l, edgeWidth(i));
  } else {
    handleError("getGraphBox(0)\n");
    return box(line(vec3(0,0,0), vec3(0,0,0)), 0);
  }
}

vector<item> getGraphCollisions(Box bbox) {
  return getCollisions(GraphCollisions, bbox, 0);
}

bool graphIntersect(Box bbox, item exclude, bool includeUnderground) {
  // Disable node collision for now
  vector<item> collisions = getGraphCollisions(bbox);
  int size = collisions.size();
  for (int i = 0; i < size; i++) {
    item coll = collisions[i];
    if (coll > 0 && coll != exclude &&
        (includeUnderground || !isElementUnderground(coll))) {
      return true;
    }
  }
  return false;
  //return getCollisions(GraphCollisions, bbox, 0).size() != 0;
}

bool graphIntersect(Box bbox, bool includeUnderground) {
  return graphIntersect(bbox, 0, includeUnderground);
}

bool graphIntersect(Box bbox, item exclude) {
  return graphIntersect(bbox, 0, true);
}

bool graphIntersect(Box bbox) {
  return graphIntersect(bbox, 0, true);
}

item nearestElement(Line l, bool includePlanned) {
  item node = nearestNode(l, includePlanned);
  item edge = nearestEdge(l, includePlanned);
  if (node == 0) return edge;
  if (edge == 0) return node;
  return lineDistance(l, getLine(node)) <
    lineDistance(l, getLine(edge)) ?  node : edge;
}

item nearestElement(Line l, bool includePlanned, Configuration config) {
  item node = nearestNode(l, includePlanned, config);
  item edge = nearestEdge(l, includePlanned, config);
  if (node == 0) return edge;
  if (edge == 0) return node;
  return lineDistance(l, getLine(node)) <
    lineDistance(l, getLine(edge)) ?  node : edge;
}

void setWear(item ndx, double amount) {
  Edge* edge = getEdge(ndx);
  edge->wear = amount;
  float oldSpeedLimit = speedLimits[edge->config.speedLimit];
  float k = glm::clamp(1. - amount*1.5, 0., 1.);
  float newSpeedLimit = std::min(double(oldSpeedLimit),
    k*(oldSpeedLimit*1.1 - minSpeed)+minSpeed);
  //float k = (oldSpeedLimit - newSpeedLimit)/(oldSpeedLimit-minSpeed);
  //if (k > 1) k = 1;
  maxWear = std::max(k, maxWear);

  for (int i = 0; i < 2; i++) {
    item blockNdx = edge->laneBlocks[0];
    if (blockNdx == 0) continue;
    getLaneBlock(blockNdx)->speedLimit = newSpeedLimit;
  }

  //SPDLOG_INFO("setWear {} {} {} {}", ndx, amount, newSpeedLimit, edge->wearEntity);
  if (edge->wearEntity != 0) {
    Entity* wearEntity = getEntity(edge->wearEntity);
    if (amount > 0.1) {
      wearEntity->flags &= ~(255 << 23);
      wearEntity->flags |= (int(255*amount) << 23);
      setEntityVisible(edge->wearEntity, true);
    } else {
      setEntityVisible(edge->wearEntity, false);
    }
  }
}

float getMaxWear() {
  return maxWear;
}

void wearEdge(item ndx, double amount) {
  if (ndx > 0 && getGameMode() == ModeGame) {
    Edge* edge = getEdge(ndx);
    if (!(edge->flags & _graphComplete) || (edge->flags & _graphCity)) {
      return;
    }
    amount *= c(CWearRate) / length(edge->line) / edge->config.numLanes;
    float newWear = std::min(edge->wear+amount, 1.0);
    float controlEffect = getBudgetControl(RepairExpenses);
    if ((1.0f - newWear)*1.5f < controlEffect) {
      float cost = getRepairCost(ndx) * .5f;
      SPDLOG_INFO("wearEdge repair {} {} {} {}", ndx, newWear, controlEffect, cost);
      if (transaction(RepairExpenses, -cost)) {
        setWear(ndx, newWear * .5f);
        //repairEdge(ndx);
      }
    } else {
      setWear(ndx, newWear);
    }
  }
}

void setSpawnProbGlobal(float amount) {
  amount *= c(CMaxSpawn);
  for (int i = -1; i >= -numNodes(); i--) {
    Node* node = getNode(i);
    node->spawnProbability = amount;
  }
}

money getRepairCost(item ndx) {
  return graphCostPlusElevation(ndx) * c(CRepairCost);
  /*
  if (ndx <= 0) {
    return 0;
  }

  Edge* edge = getEdge(ndx);
  float wear = edge->wear;
  float len = length(edge->line) / 1000;
  bool oneWay = edge->config.flags & _configOneWay;
  int numLanes = edge->config.numLanes * oneWay ? 1 : 2;

  return wear * len * numLanes * laneCost(edge->config.type) * getInflation();
  */
}

void repairEdge(item ndx) {
  setWear(ndx, 0);
}

char* graphElementName(item ndx) {
  if (ndx < 0) {
    Node* node = getNode(ndx);
    int edges = getRenderableEdges(ndx).size();
    if (node->config.type == ConfigTypeExpressway) {
      if (edges == 1) {
        if (getGameMode() == ModeTest) {
          return strdup_s("Spawn Point");
        } else {
          return strdup_s("Dead End");
        }
      } else {
        return strdup_s("Junction");
      }
    }
    switch (edges) {
      case 0: return strdup_s("Disconnected Node");
      case 1:
        if (getGameMode() == ModeTest) {
          return strdup_s("Spawn Point");
        } else {
          return strdup_s("Culdesac");
        }
      case 2: return strdup_s("Joint");
      default: return strdup_s("Intersection");
    }

  } else if (ndx > 0) {
    Edge* edge = getEdge(ndx);
    const char* roadType = "St";
    if (edge->config.type == ConfigTypeExpressway) {
      roadType = "Expressway";
    } else if (edge->config.numLanes > 2) {
      roadType = "Blvd";
    } else if (edge->config.numLanes > 1) {
      roadType = "Ave";
    }

    if (edge->name == 0 || edge->name[0] == 0) {
      return strdup_s(roadType);
    } else {
      return sprintf_o("%s %s", edge->name, roadType);
    }
  } else {
    return strdup_s("Null Element");
  }
}

item numVehiclesInElement(item ndx) {
  if (ndx < 0) {
    Node* node = getNode(ndx);
    int total = 0;
    for (int i = 0; i < node->laneBlocks.size(); i++) {
      item blockNdx = node->laneBlocks[i];
      total += numVehiclesInBlock(blockNdx);
    }
    return total;

  } else if (ndx > 0) {
    Edge* edge = getEdge(ndx);
    int total = 0;
    for (int i = 0; i < 2; i++) {
      item blockNdx = edge->laneBlocks[i];
      if (blockNdx == 0) continue;
      total += numVehiclesInBlock(blockNdx);
    }
    return total;

  } else {
    handleError("numVehiclesInElement(0)\n");
    return 0;
  }
}

bool tunnelsVisible() {
  return areTunnelsVisible;
}

void setTunnelsVisible() {
  bool viz = !isUndergroundView();
  if (viz != areTunnelsVisible) {
    areTunnelsVisible = viz;
    for (int i = 1; i <= nodes->size(); i++) {
      Node* n = getNode(-i);
      if ((n->flags & _graphExists) && n->tunnelEntity != 0) {
        setEntityVisible(n->tunnelEntity, viz);
      }
    }
    for (int i = 1; i <= edges->size(); i++) {
      Edge* e = getEdge(i);
      if ((e->flags & _graphExists) && e->tunnelEntity != 0) {
        setEntityVisible(e->tunnelEntity, viz);
      }
    }
  }
}

void updateGraphVisuals(bool firstPass) {
  int numEdges = edges->size();
  int numNodes = nodes->size();
  int numElem = numEdges + numNodes;
  int numToUpdate = firstPass ? numElem+2 : numElem / c(CHeatMapTime);
  int numUpdated = 0;
  if (firstPass) lastVisualUpdate = -numNodes;
  bool night = getLightLevel() < 0.5;

  for (int numUpdated = 0; numUpdated < numToUpdate; numUpdated++) {
    if (lastVisualUpdate > numEdges || lastVisualUpdate < -numNodes) {
      lastVisualUpdate = -numNodes;
    }
    int i = lastVisualUpdate;
    lastVisualUpdate ++;

    Entity* entity = 0;
    Configuration config;
    vector<item> laneBlocks;
    bool culdesac = false;

    if (i == 0) {
      continue;

    } else if (i < 0) {
      Node* node = getNode(i);
      if (!(node->flags & _graphExists)) continue;
      laneBlocks = node->laneBlocks;
      if (node->entity != 0) {
        entity = getEntity(node->entity);
      }
      config = node->config;
      if (node->edges.size() <= 1) culdesac = true;

    } else {
      Edge* edge = getEdge(i);
      if (!(edge->flags & _graphExists)) continue;
      config = edge->config;

      if (edge->entity != 0) {
        entity = getEntity(edge->entity);
      }
      for (int j = 0; j < 2; j ++) {
        if (edge->laneBlocks[j] != 0) {
          laneBlocks.push_back(edge->laneBlocks[j]);
        }
      }

      if (edge->signEntity != 0) {
        setEntityIlluminated(edge->signEntity, night);
      }
    }

    if (entity != 0 && (getHeatMap() == TrafficHeatMap || firstPass)) {
      float accum = 0;

      for (int l = 0; l < laneBlocks.size(); l++) {
        item blockNdx = laneBlocks[l];
        if (blockNdx == 0) continue;
        float ratio = 0;

        if (firstPass) {
          LaneBlock* blk = getLaneBlock(blockNdx);
          ratio = blk->timeEstimate / blk->staticTimeEstimate;
        } else {
          float traversalTime = getTraversalRecord(blockNdx);
          float timeEstimate = getStaticTimeEstimate(blockNdx);
          ratio = traversalTime / timeEstimate;
        }

        accum += clamp(ratio - 2, 0.f, 100.f);
      }

      float k = clamp(accum/laneBlocks.size()*0.05f, 0.f, 1.f);
      k = k*k;
      if (culdesac) k = 0; // Supress a bug where culdesacs appear to have traffic
      entity->dataFlags &= ~(255 << 16);
      entity->dataFlags |= (int(255*k) << 16);
    }

    if (entity != 0) { // && (getHeatMap() == RoadHeatMap || firstPass)) {

      uint32_t color = 26; // purple
      item type = config.type;
      item numLanes = config.numLanes;
      if (numLanes < 1) numLanes = 1;
      bool oneWay = config.flags & _configOneWay;
      if (type == ConfigTypeRoad) {
        if (oneWay) {
          //color = 19; // light blue
          color = 30; // light gray
        } else if (numLanes >= 3) {
          color = 31; // white
        } else if (numLanes >= 2) {
          color = 30; // light gray
        } else {
          color = 29; // mid gray
        }
      } else if (type == ConfigTypeExpressway) {
        color = 7; // orange
      } else if (type == ConfigTypeHeavyRail) {
        color = 3; // red
      } else if (type == ConfigTypePedestrian) {
        color = 15; // green
      }

      uint32_t colorMask = 31;
      entity->dataFlags &= ~colorMask;
      entity->dataFlags |= color;

      float cullDist = 10000;
      if (isUndergroundView()) {
        cullDist = 100000;
      } else {
        cullDist = 10000*numLanes*numLanes;
      }

      entity->maxCameraDistance = cullDist;
    }
  }
}

void weatherWear(double duration) {
  if (getGameMode() != ModeGame) return;
  item edgeS = edges->size();
  const int numToWear = 10;
  if (edgeS < numToWear) return;
  double yearDur = duration / gameDayInRealSeconds / oneYear;
  float wear = yearDur * edgeS / numToWear;
  for (int i = 0; i < numToWear; i++) {
    item randEdgeNdx = rand() % edges->size() + 1;
    Edge* edge = getEdge(randEdgeNdx);
    if (!(edge->flags & _graphExists) || !(edge->flags & _graphComplete) ||
        edge->flags & _graphCity) {
      continue;
    }

    //SPDLOG_INFO("weatherWear {} {} {}", randEdgeNdx, edge->wear, wear);
    wearEdge(randEdgeNdx, wear);
  }
}

void updateGraph(double duration) {
  updateGraphVisuals(false);
  weatherWear(duration);
}

void rebuildRoadStats() {
  resetStat(NumIntersections);
  resetStat(PavementMSq);
  resetStat(RoadSystemValue);

  for (int i = 1; i <= nodes->size(); i++) {
    Node* n = getNode(-i);
    if (n->flags & _graphExists && n->flags & _graphComplete) {
      adjustGraphStats(-i, 1);
    }
  }
  for (int i = 1; i <= edges->size(); i++) {
    Edge* e = getEdge(i);
    if (e->flags & _graphExists && e->flags & _graphComplete) {
      adjustGraphStats(i, 1);
    }
  }
}

void writeNode(FileBuffer* file, item ndx) {
  Node* node = getNode(ndx);
  fwrite_item(file, node->pillar);
  fwrite_int  (file, node->flags);
  fwrite_vec3 (file, node->center);
  fwrite_float(file, node->intersectionSize);
  fwrite_int  (file, node->phase);
  fwrite_float(file, node->timeInPhase);
  fwrite_float(file, node->spawnProbability);
  writeConfiguration(file, node->config);
  node->edges.write(file);
  fwrite_item_vector(file, &node->laneBlocks);
  fwrite_item_vector(file, &node->phaseMins);
  fwrite_item_vector(file, &node->phaseMaxs);
}

void readNode(FileBuffer* file, int version, item ndx) {
  Node* node = getNode(ndx);

  node->pillar = fread_item(file, version);
  node->flags = fread_int(file);
  node->center = fread_vec3(file);
  node->intersectionSize = fread_float(file);
  node->phase = fread_int(file);
  node->timeInPhase = fread_float(file);
  node->spawnProbability = fread_float(file);
  node->config = readConfiguration(file, version);
  node->edges.read(file, version);
  fread_item_vector(file, &node->laneBlocks, version);
  fread_item_vector(file, &node->phaseMins, version);
  fread_item_vector(file, &node->phaseMaxs, version);

  node->entity = 0;
  node->signEntity = 0;
  node->tunnelEntity = 0;

  if (node->flags & _graphExists) {
    addToCollisionTable(GraphCollisions, getGraphBox(ndx), ndx);
  }

  if (version < 25) {
    addGraphElevator(ndx, false);
  }
}

void writeEdge(FileBuffer* file, item ndx) {
  Edge* edge = getEdge(ndx);

  fwrite_int  (file, edge->flags);
  fwrite_item(file, edge->ends[0]);
  fwrite_item(file, edge->ends[1]);
  fwrite_item(file, edge->plan);
  fwrite_item(file, edge->laneBlocks[0]);
  fwrite_item(file, edge->laneBlocks[1]);
  fwrite_double(file, edge->wear);
  fwrite_line (file, edge->line);
  fwrite_string(file, edge->name);
  writeConfiguration(file, edge->config);
}

void readEdge(FileBuffer* file, int version, item ndx) {
  Edge* edge = getEdge(ndx);

  edge->flags = fread_int(file);
  edge->ends[0] = fread_item(file, version);
  edge->ends[1] = fread_item(file, version);
  edge->plan = fread_item(file, version);
  edge->laneBlocks[0] = fread_item(file, version);
  edge->laneBlocks[1] = fread_item(file, version);
  edge->wear = fread_double(file);
  edge->line = fread_line(file);
  edge->name = fread_string(file);
  edge->config = readConfiguration(file, version);
  edge->entity = 0;
  edge->wearEntity = 0;
  edge->textEntity = 0;
  edge->signEntity = 0;
  edge->tunnelEntity = 0;

  if (edge->flags & _graphExists) {
    addToCollisionTable(GraphCollisions, getGraphBox(ndx), ndx);
    if (edge->flags & _graphComplete) {
      completeEdges ++;
    }
  }

  if (version < 25) {
    addGraphElevator(ndx, false);
  }
}

void writeGraph(FileBuffer* file) {
  nodes->write(file);
  for (int i = 1; i <= nodes->size(); i++) {
    writeNode(file, -i);
  }

  edges->write(file);
  for (int i = 1; i <= edges->size(); i++) {
    writeEdge(file, i);
  }
}

void readGraph(FileBuffer* file, int version) {
  nodes->read(file, version);
  for (int i = 1; i <= nodes->size(); i++) {
    readNode(file, version, -i);
  }

  edges->read(file, version);
  for (int i = 1; i <= edges->size(); i++) {
    readEdge(file, version, i);
  }
}

