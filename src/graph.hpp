#ifndef GRAPH_H
#define GRAPH_H

#include "box.hpp"
#include "configuration.hpp"
#include "cup.hpp"
#include "item.hpp"
#include "lane.hpp"
#include "line.hpp"
#include "main.hpp"
#include "serialize.hpp"

void updateGraph(double duration);
void updateGraphVisuals(bool firstPass);
void rebuildRoadStats();
void resetGraph();
void initGraphEntities();
void renderGraph();
void rerenderGraph();
void finishGraph();
void setAutomaticBridges(bool val);

void complete(item ndx, Configuration config);
bool isOpen(item ndx);
int getBridgeType(item node);
const char* legalMessage(item ndx);
Line getLine(item ndx);
vec3 getElementLoc(item ndx);
item nearestElement(vec3 loc, bool includePlanned);
item nearestElement(Line line, bool includePlanned);
item nearestElement(vec3 loc, bool includePlanned, Configuration config);
item nearestElement(Line line, bool includePlanned, Configuration config);
void writeGraph(FileBuffer* file);
void readGraph(FileBuffer* file, int version);
money graphCostSimple(item ndx);
money graphCostFull(item ndx);
money edgeCost(vec3 c0, vec3 c1,
    Configuration e, Configuration n0, Configuration n1);
void removeElement(item ndx);
int getSpeedLimit(item ndx);
char* graphElementName(item ndx);
bool split(item ndx);
Configuration getElementConfiguration(item ndx);
vector<item> getGraphCollisions(Box bbox);
bool graphIntersect(Box box);
bool graphIntersect(Box bbox, item exclude);
bool graphIntersect(Box box, bool includeUnderground);
bool graphIntersect(Box bbox, item exclude, bool includeUnderground);
Box getGraphBox(item i);
float elementWidth(item ndx);
float buildDistance(item element);
item numVehiclesInElement(item ndx);
float getMaxWear();
void renderGraphElement(item ndx);
bool isElementUnderground(item ndx);
bool simplifyNode(item ndx, bool byPlayer);
bool canSimplifyNode(item ndx, bool byPlayer);

const int _graphExists   = 1 << 0;
const int _graphOpen     = 1 << 1;
const int _graphComplete = 1 << 2;
const int _graphExpressway = 1 << 3;
const int _graphUnderground = 1 << 4;
const int _graphCity = 1 << 5;
const int _graphIsColliding = 1 << 6;
const int _graphIsDuplicate = 1 << 7;

struct Node {
  vec3 center;
  Cup<item> edges;
  vector<item> laneBlocks;
  vector<item> phaseMins;
  vector<item> phaseMaxs;
  item entity;
  item signEntity;
  item tunnelEntity;
  uint32_t flags;
  item pillar;
  Configuration config;
  float intersectionSize;
  int phase;
  float timeInPhase;
  float spawnProbability;
};

item numNodes();
item sizeNodes();
Node* getNode(item ndx);
item getRandomNode();
item addNode(vec3 center, Configuration config);
item getNodeAt(vec3 point);
item getOrCreateNodeAt(vec3 point, Configuration config);
item getOrCreatePillarNodeAt(item pillarNdx, Configuration config);
item addPillarNode(item pillarNdx, Configuration config);
void removeNode(item ndx);
EndDescriptor getNodeEndDescriptor(item edgeNdx, item nodeNdx);
void setSpawnProbGlobal(float amount);
bool isNodeUnderground(item ndx);
void setTunnelsVisible();
bool tunnelsVisible();
bool isLeftHandTraffic();
void setLeftHandTraffic(bool val);
float trafficHandedness();

struct Edge {
  item ends[2];
  item laneBlocks[2];
  item entity;
  item wearEntity;
  item textEntity;
  item signEntity;
  item tunnelEntity;
  item plan;
  Line line;
  uint32_t flags;
  double wear;
  Configuration config;
  char* name;
};

item numCompleteEdges();
item sizeEdges();
Edge* getEdge(item ndx);
item addEdge(item end1ndx, item end2ndx, Configuration config);
void removeEdgeFromNode(item ndx, item edge, bool deleteIfEmpty);
void removeEdge(item ndx, bool deleteNode);
item nearestEdge(vec3 loc, bool includePlanned);
item nearestEdge(Line line, bool includePlanned);
item nearestEdge(Line line, bool includePlanned, Configuration config);
item nearestEdge(vec3 loc, bool includePlanned, Configuration config);
void setupEdgeLanes(item ndx);
float edgeWidth(item ndx);
bool isEdgeUnderground(item ndx);
EndDescriptor getEdgeEndDescriptor(item edge, item nodeNdx);
void reconfigure(item ndx, Configuration config);
void queueCollide(item ndx);
void trimEdge(item ndx, Box bbox);
void dedupe(item ndx);
item detectSemiDuplicate(item ndx);
void switchDirection(item ndx);
void chainRenameEdge(item edgeNdx, bool followCorners);
void setWear(item ndx, double amount);
void wearEdge(item ndx, double amount);
money getRepairCost(item ndx);
money reconfigureCost(item ndx, Configuration config);
money reconfigureCost(item ndx, Configuration config,
    bool includeUnset, bool includeSet);
money strategyCost(item type);
money minReconfigureCost(item type);
money laneCost(item type);
void repairEdge(item ndx);
item graphElementsToRender();

#endif
