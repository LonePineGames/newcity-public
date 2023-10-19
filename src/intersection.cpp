#include "intersection.hpp"

#include "draw/culler.hpp"
#include "draw/entity.hpp"
#include "draw/texture.hpp"
#include "game/game.hpp"
#include "name.hpp"
#include "renderGraph.hpp"
#include "string_proxy.hpp"
#include "vehicle/laneLoc.hpp"
#include "vehicle/vehicle.hpp"
#include "vehicle/physics.hpp"
#include "util.hpp"

#include "spdlog/spdlog.h"
#include <algorithm>

const int maxPhases = 12;
const int maxNodeBlocks = 40;

item numNodes_v = 0;
Cup<char> nodeStrategy;
Cup<float> nodeSpawnProb;
Cup<item> nodeStartBlock;
Cup<item> spawnDestBlocks;
Cup<char> nodeNumPhases;
Cup<item> nodePhaseBlks;
Cup<item> nodeBlks;
Cup<char> nodePhase;
Cup<float> nodeTimeInPhase;

vector<item> getCompletedEdges(item ndx, bool isRail) {
  Node* node = getNode(ndx);
  int numEdges = node->edges.size();
  vector<item> result;
  for (int i = 0; i < numEdges; i++) {
    item eNdx = node->edges[i];
    Edge* e = getEdge(eNdx);
    if ((e->flags & _graphExists) && (e->flags & _graphComplete)
        && isRail == (e->config.type == ConfigTypeHeavyRail)
        && e->config.type != ConfigTypePedestrian) {
      result.push_back(eNdx);
    }
  }
  return result;
}

vector<item> getCompletedEdges(item ndx) {
  Node* node = getNode(ndx);
  int numEdges = node->edges.size();
  vector<item> result;
  for (int i = 0; i < numEdges; i++) {
    item eNdx = node->edges[i];
    Edge* e = getEdge(eNdx);
    if ((e->flags & _graphExists) && (e->flags & _graphComplete)) {
      result.push_back(eNdx);
    }
  }
  return result;
}

vector<item> getRenderableEdges(item ndx) {
  Node* node = getNode(ndx);
  if (node->flags & _graphComplete) {
    return getCompletedEdges(ndx);
  } else {
    return node->edges.toVector();
  }
}

vector<item> getNodeInputs(item ndx, bool completed) {
  Node* node = getNode(ndx);
  int numEdges = node->edges.size();
  vector<item> result;
  for (int i = 0; i < numEdges; i++) {
    item eNdx = node->edges[i];
    Edge* e = getEdge(eNdx);
    if ((e->flags & _graphExists) &&
        (!completed || (e->flags & _graphComplete)) &&
        (!(e->config.flags & _configOneWay) || e->ends[1] == ndx)) {
      result.push_back(eNdx);
    }
  }
  return result;
}

vector<item> getNodeOutputs(item ndx, bool completed) {
  Node* node = getNode(ndx);
  int numEdges = node->edges.size();
  vector<item> result;
  for (int i = 0; i < numEdges; i++) {
    item eNdx = node->edges[i];
    Edge* e = getEdge(eNdx);
    if ((e->flags & _graphExists) &&
        (!completed || (e->flags & _graphComplete)) &&
        (!(e->config.flags & _configOneWay) || e->ends[0] == ndx)) {
      result.push_back(eNdx);
    }
  }
  return result;
}

void makeLaneBlocks(item nodeNdx, int i, int pair, bool isRail) {
  Node* node = getNode(nodeNdx);
  vector<item> edges = getCompletedEdges(nodeNdx, isRail);
  int numEdges = edges.size();
  int numEdgesTotal = getCompletedEdges(nodeNdx).size();
  item edgeNdx = edges[i];
  Edge* edge = getEdge(edgeNdx);
  EndDescriptor blocks = getNodeEndDescriptor(edgeNdx, nodeNdx);
  if (blocks.source == 0) return;
  blocks.graphElement = edgeNdx;
  bool fullLanes = getNodeInputs(nodeNdx, true).size() <= 2;

  for(int j = 0; j < numEdges; j ++) {
    int k = (i + j) % numEdges;

    if (i == k && numEdges == 2) continue;

    //Make a U-Turn?
    if (getGameMode() == ModeBuildingDesigner && i == k) continue;
    if (i == k && numEdges > 1 && !isRail &&
        (!c(CEnableUTurn) || edge->config.numLanes == 1)) continue;

    item otherEdgeNdx = edges[k];
    Edge* otherEdge = getEdge(otherEdgeNdx);
    EndDescriptor otherBlocks = getNodeEndDescriptor(otherEdgeNdx, nodeNdx);
    if (otherBlocks.drain == 0) continue;
    otherBlocks.graphElement = otherEdgeNdx;

    float angle = dot(blocks.normal, otherBlocks.normal);
    if (i != k && angle > 0.75) continue;

    int numLanes = 1;
    int startLane = 0, otherStartLane = 0;
    if (k == pair || fullLanes) {
      numLanes = std::min(edge->config.numLanes,
        otherEdge->config.numLanes);
    } else if (i == k) {
      otherStartLane = otherEdge->config.numLanes - 1;
    } else if (j <= numEdges/2) {
      startLane = edge->config.numLanes - 1;
      otherStartLane = otherEdge->config.numLanes - 1;
    }

    item newLaneBlock = createConnectingLaneBlock(nodeNdx, blocks,
      otherBlocks, startLane, otherStartLane, numLanes);
    node->laneBlocks.push_back(newLaneBlock);

    LaneBlock* block = getLaneBlock(newLaneBlock);
    if (k == pair || fullLanes) {
      block->flags |= _laneStraight;
    } else if (i == k) {
      block->flags |= _laneU;
    } else if (j <= numEdges/2) {
      block->flags |= _laneRight;
    } else {
      block->flags |= _laneLeft;
    }

    if (fullLanes) {
      block->flags |= _laneActive | _laneAlwaysActive;
    //} else if (j == 1 && node->config.strategy == TrafficLightStrategy) {
      //block->flags |= _laneActive | _laneAlwaysActive;
    } else if (numEdgesTotal <= 2) {
      block->flags |= _laneActive;
    } else {
      block->flags &= ~_laneActive;
    }
  }
}

int getOddManOut(item nodeNdx, vector<item> edges) {
  Node* node = getNode(nodeNdx);
  int numEdges = edges.size();
  if (numEdges == 1) {
    return 0;
  }

  //Determine Odd Man Out
  int oddManOut = numEdges+1;
  if (numEdges % 2 == 1) {
    vec3 *norms = (vec3*)alloca(sizeof(vec3)*numEdges);
    int *laneNums = (int*)alloca(sizeof(int)*numEdges);
    vec3 center = node->center;
    for(int i = 0; i < numEdges; i++) {
      Edge* edge = getEdge(edges[i]);
      vec3 end = edge->ends[0] == nodeNdx ? edge->line.start : edge->line.end;
      vec3 dir = end - center;
      vec3 norm = normalize(dir);
      norms[i] = norm;
      laneNums[i] = edge->config.numLanes;
    }

    float oddManOutScore = -FLT_MAX;
    for(int i = 0; i < numEdges; i++) {
      Edge* edge = getEdge(edges[i]);
      int i0 = (i + numEdges/2    ) % numEdges;
      int i1 = (i + numEdges/2 + 1) % numEdges;
      float score0 = abs(cross(norms[i], norms[i0]).z);
        //std::min(laneNums[i], laneNums[i0]);
      float score1 = abs(cross(norms[i], norms[i1]).z);
        //std::min(laneNums[i], laneNums[i1]);

      float score = std::min(score0, score1);
      if (score > oddManOutScore) {
        oddManOut = i;
        oddManOutScore = score;
      }
    }
  }

  return oddManOut;
}

void printIntersection(const char* title, Node* node, item ndx) {
  if (true) {return;}
  printf("Edges %s (Node %d):\n", title, ndx);
  for (int i=0; i < node->edges.size(); i++) {
    printf("  Edge %d:\n", node->edges[i]);
    Edge* edge = getEdge(node->edges[i]);
    int end = (edge->ends[0] != ndx);
    for (int j=0; j < 2; j ++) {
      item ndx = edge->laneBlocks[j];
      LaneBlock* block = getLaneBlock(ndx);
      char c = (j == end) ? '>' : '<';
      printf("  %c LaneBlock %d: %d source, %d drains\n",
        c, ndx, block->sources.size(), block->drains.size());

      for(int k=0; k < block->numLanes; k++) {
        Lane* lane = &block->lanes[k];
        printf("      Lane %d: %d source, %d drains\n",
          ndx+k, lane->sources.size(), lane->drains.size());
      }
    }
  }
}

void rebuildJunction(item nodeNdx) {
  Node* node = getNode(nodeNdx);
  vector<item> edges = getCompletedEdges(nodeNdx, false);
  int numEdges = edges.size();
  vector<item> inputs;
  vector<item> outputs;
  int numInputLanes = 0;
  int numOutputLanes = 0;
  int oddManOutNum = getOddManOut(nodeNdx, edges);
  item oddManOut = 0;
  if (oddManOutNum >= 0 && oddManOutNum < numEdges) {
    oddManOut = edges[oddManOutNum];
  }

  for (int i = 0; i < numEdges; i++) {
    item edgeNdx = edges[i];
    Edge* edge = getEdge(edgeNdx);
    if (!(edge->flags & _graphComplete)) {
      continue;
    } else if (edge->ends[0] == nodeNdx) {
      outputs.push_back(edgeNdx);
      numOutputLanes += edge->config.numLanes;
    } else {
      inputs.push_back(edgeNdx);
      numInputLanes += edge->config.numLanes;
    }
  }

  if (inputs.size() == 0 || outputs.size() == 0 ||
      (inputs.size() != 1 && outputs.size() != 1)) {
    return;

  } else if (inputs.size() == 1) {
    EndDescriptor blocks0 = getNodeEndDescriptor(inputs[0], nodeNdx);
    blocks0.graphElement = inputs[0];
    float laneMult = (numInputLanes+1.f) / numOutputLanes;
    if (laneMult > 1) laneMult = 1;
    for (int i = 0; i < outputs.size(); i++) {
      Edge* e = getEdge(outputs[i]);
      float numLanesFlt = e->config.numLanes * laneMult;
      int numLanes = outputs[i] == oddManOut ?
        int(numLanesFlt) : ceil(numLanesFlt);
      int startLane = i != 0 ? 0 : numInputLanes - numLanes;
      int endLane = i == 0 ? 0 : e->config.numLanes - numLanes;
      EndDescriptor blocks1 = getNodeEndDescriptor(outputs[i], nodeNdx);
      blocks1.graphElement = outputs[i];
      item newLaneBlock = createConnectingLaneBlock(nodeNdx, blocks0,
        blocks1, startLane, endLane, numLanes);
      node->laneBlocks.push_back(newLaneBlock);

      LaneBlock* block = getLaneBlock(newLaneBlock);
      if (outputs[i] != oddManOut) {
        block->flags |= _laneStraight;
      } else if (i == 0) {
        block->flags |= _laneRight;
      } else {
        block->flags |= _laneLeft;
      }
      block->flags |= _laneActive | _laneAlwaysActive;
    }

  } else {
    EndDescriptor blocks1 = getNodeEndDescriptor(outputs[0], nodeNdx);
    blocks1.graphElement = outputs[0];
    float laneMult = (numOutputLanes+1.f) / numInputLanes;
    if (laneMult > 1) laneMult = 1;
    for (int i = 0; i < inputs.size(); i++) {
      Edge* e = getEdge(inputs[i]);
      float numLanesFlt = e->config.numLanes * laneMult;
      int numLanes = inputs[i] == oddManOut ?
        int(numLanesFlt) : ceil(numLanesFlt);
      int startLane = i != 0 ? 0 : e->config.numLanes - numLanes;
      int endLane = i == 0 ? 0 : numOutputLanes - numLanes;
      EndDescriptor blocks0 = getNodeEndDescriptor(inputs[i], nodeNdx);
      blocks0.graphElement = inputs[i];
      item newLaneBlock = createConnectingLaneBlock(nodeNdx, blocks0,
        blocks1, startLane, endLane, numLanes);
      node->laneBlocks.push_back(newLaneBlock);

      LaneBlock* block = getLaneBlock(newLaneBlock);
      block->flags |= _laneStraight;
      /*
      if (inputs[i] != oddManOut) {
        block->flags |= _laneStraight;
      } else if (i == 0) {
        block->flags |= _laneLeft;
      } else {
        block->flags |= _laneRight;
      }
      */
      block->flags |= _laneActive | _laneAlwaysActive;
    }
  }
}

void rebuildIntersection(item nodeNdx) {
  Node* node = getNode(nodeNdx);
  if (!(node->flags & _graphExists)) return;
  vector<item> edges = node->edges.toVector();
  int numEdges = edges.size();
  printIntersection("before cleanup", node, nodeNdx);

  //Cleanup
  node->phaseMins.clear();
  node->phaseMaxs.clear();
  for(int i = node->laneBlocks.size() - 1; i >= 0; i--) {
    removeLaneBlock(node->laneBlocks[i]);
  }
  for(int i=0; i < numEdges; i++) {
    Edge* edge = getEdge(edges[i]);
    int end = (edge->ends[0] != nodeNdx);
    bool oneWay = edge->config.flags & _configOneWay;
    for (int b = 0; b < 2; b++) {
      item blockNdx = edge->laneBlocks[b];
      if (blockNdx == 0) continue;
      if (end == b) {
        clearSources(edge->laneBlocks[b]);
      } else {
        clearDrains(edge->laneBlocks[b]);
      }
    }
  }
  node->laneBlocks.clear();

  if (!(node->flags & _graphComplete)) return;

  printIntersection("before rebuild", node, nodeNdx);
  if (node->config.type == ConfigTypeExpressway) {
    rebuildJunction(nodeNdx);
    return;
  }

  node->phaseMins.push_back(0);

  for (int r = 0; r < 2; r++) {
    edges = getCompletedEdges(nodeNdx, r);
    numEdges = edges.size();
    int oddManOut = getOddManOut(nodeNdx, edges);
    //int *stopLightTextureIndex = (int*)alloca(sizeof(int)*numEdges);

    //printf("Odd man out: %d\n", oddManOut);

    //Construct Lane Blocks and assign them to phases
    for(int k = 0; k < (numEdges+1)/2; k ++) {
      int i = (k + oddManOut) % numEdges;
      int pairNum = (i + numEdges/2) % numEdges;
      if (i == oddManOut) {
        pairNum = -1;
      }
      makeLaneBlocks(nodeNdx, i, pairNum, r);
      //stopLightTextureIndex[i] = node->phaseMaxs.size() * 2;

      if (pairNum >= 0) {
        makeLaneBlocks(nodeNdx, pairNum, i, r);
        //stopLightTextureIndex[pairNum] = node->phaseMaxs.size() * 2;
      }

      int size = node->laneBlocks.size();
      node->phaseMins.push_back(size);
      node->phaseMaxs.push_back(size - 1);
    }
  }

  node->phaseMins.pop_back();
  node->phase = 0;
  node->timeInPhase = 0;
  printIntersection("after rebuild", node, nodeNdx);
  //printf("\n");
}

void activateLanes(Node* node, item min, item max, bool activate, bool yellow) {
  for(int i=min; i <= max; i++) {
    item ndx = node->laneBlocks[i];
    LaneBlock* laneBlock = getLaneBlock(ndx);
    if (laneBlock->flags & _laneAlwaysActive) {
      laneBlock->flags |= _laneActive;
    } else if (activate) {
      laneBlock->flags |= _laneActive;
    } else {
      laneBlock->flags &= ~_laneActive;
    }

    if (yellow) {
      laneBlock->flags |= _laneYellow;
    } else {
      laneBlock->flags &= ~_laneYellow;
    }
  }
}

void setStopLightPhaseTexture(item nodeNdx) {
  Node* node = getNode(nodeNdx);
  if (!(node->flags & _graphExists) || !(node->flags & _graphComplete)) return;
  if (node->signEntity != 0 && node->config.strategy == TrafficLightStrategy) {

    item arrNdx = -nodeNdx;
    float yellowDuration = c(CStopLightMaxPhaseDuration) -
      c(CStopLightYellowDuration);
    int number = nodePhase[arrNdx]*4;
    //if (nodeTimeInPhase[arrNdx] >= yellowDuration) number ++;
    number %= 12;

    Entity* signEntity = getEntity(node->signEntity);
    signEntity->texture = stopLight0 + number;
    markEntityDirty_g(node->signEntity);
  }
}

/*
void spawnVehicle(item nodeNdx, bool force) {
  Node* node = getNode(nodeNdx);
  vector<item> edges = getCompletedEdges(nodeNdx);

  if (edges.size() != 1) {
    return;
  }

  Edge* startEdge = getEdge(edges[0]);
  int startSide = startEdge->ends[1] == nodeNdx;
  bool oneWay = startEdge->config.flags & _configOneWay;
  if (startSide && oneWay) {
    return;
  }

  item startBlockNdx = startEdge->laneBlocks[startSide];
  LaneBlock* startBlock = getLaneBlock(startBlockNdx);
  item startLaneNdx = getLaneIndex(startBlockNdx,
    randItem(startBlock->numLanes));
  if (!force && !canEnterLane(startLaneNdx)) {
    return;
  }

  for (int numIterations = 0; numIterations < 100; numIterations ++) {
    item destNodeNdx = getRandomNode();
    if (destNodeNdx >= 0) continue;
    Node* destNode = getNode(destNodeNdx);
    vector<item> destEdges = getCompletedEdges(destNodeNdx);
    if (destEdges.size() != 1) continue;
    Edge* destEdge = getEdge(destEdges[0]);
    int destSide = destEdge->ends[0] == destNodeNdx;
    if (destEdge->laneBlocks[destSide] == 0) continue;
    bool oneWay = destEdge->config.flags & _configOneWay;
    if (destSide && oneWay) continue;
    item destBlockNdx = destEdge->laneBlocks[destSide];

    GraphLocation start = graphLocation(startLaneNdx);
    start.dap = 0;

    item destLaneNdx = getLaneIndex(destBlockNdx,
      randItem(getLaneBlock(destBlockNdx)->numLanes));
    GraphLocation dest = graphLocation(destLaneNdx);
    dest.dap = getLane(destLaneNdx)->length;

    item vehicleNdx = addTestVehicle(start, dest);
    return;
  }
}
*/

void spawnVehicle(item nodeNdx, bool force) {
  if (nodeNdx > numNodes_v) return;
  item numDests = spawnDestBlocks.size();
  if (numDests == 0) return;
  item startBlk = nodeStartBlock[-nodeNdx];
  item endBlk = spawnDestBlocks[randItem(numDests)];
  if (startBlk < 10 || endBlk < 10) return;
  startBlk += randItem(getBlockLanes(startBlk));
  if (!force && !canEnterLane_g(startBlk)) return;
  GraphLocation start = graphLocation(startBlk, 0);
  GraphLocation end = graphLocation(endBlk, getLaneLength(endBlk));
  addTestVehicle_g(start, end);
}

void swapOneIntersection_v(item nodeNdx, bool fromSave) {
  Node* node = getNode(nodeNdx);
  item arrNdx = -nodeNdx;

  for (int i = 0; i < maxPhases; i++) {
    nodePhaseBlks.set(arrNdx*maxPhases + i, 0);
  }
  for (int i = 0; i < maxNodeBlocks; i++) {
    nodeBlks.set(arrNdx*maxNodeBlocks + i, 0);
  }

  if (!(node->flags & _graphExists)) {
    nodeStrategy.set(arrNdx, UnregulatedStrategy);
    nodePhase.set(arrNdx, 0);
    nodeSpawnProb.set(arrNdx, 0);
    nodeNumPhases.set(arrNdx, 0);
    nodeTimeInPhase.set(arrNdx, 0);
  }

  vector<item> edges = getCompletedEdges(nodeNdx);
  int numEdges = edges.size();
  IntersectionStrategies strat = UnregulatedStrategy;

  if (node->flags & _graphCity) {
    strat = SpawnStrategy;
  } else if (numEdges == 1 && getGameMode() != ModeGame &&
      (node->config.type == ConfigTypeRoad ||
      node->config.type == ConfigTypeExpressway)) {
    strat = SpawnStrategy;
  } else if (numEdges <= 2) {
    strat = UnregulatedStrategy;
  } else {
    strat = (IntersectionStrategies)node->config.strategy;
  }

  nodeStrategy.set(arrNdx, strat);
  nodeSpawnProb.set(arrNdx, node->spawnProbability);

  if (fromSave) {
    nodePhase.set(arrNdx, node->phase);
    nodeTimeInPhase.set(arrNdx, node->timeInPhase);
  } else {
    node->phase = nodePhase[arrNdx];
    node->timeInPhase = nodeTimeInPhase[arrNdx];
  }

  int numPhases = node->phaseMins.size();
  nodeNumPhases.set(arrNdx, numPhases);
  int numBlks = node->laneBlocks.size();
  int ndx = 0;
  item phase = 0;

  for (int i = 0; i < numBlks; i++) {
    item blkNdx = node->laneBlocks[i];
    LaneBlock* b = getLaneBlock(blkNdx);
    if (strat == TrafficLightStrategy &&
        (b->flags & _laneAlwaysActive)) continue;

    nodeBlks.set(arrNdx*maxNodeBlocks + ndx, blkNdx);
    while (phase < numPhases && node->phaseMaxs[phase] < i) {
      nodePhaseBlks.set(arrNdx*maxPhases + phase, ndx);
      phase++;
    }
    ndx++;
  }

  for (; phase < numBlks; phase++) {
    nodePhaseBlks.set(arrNdx*maxPhases + phase, ndx);
  }

  for (; ndx < maxNodeBlocks; ndx++) {
    nodeBlks.set(arrNdx*maxNodeBlocks + ndx, 0);
    ndx ++;
  }

  if (strat == SpawnStrategy) {
    Edge* edge = getEdge(edges[0]);
    if (edge->config.type == ConfigTypePedestrian) {
      nodeStartBlock.set(arrNdx, 0);

    } else {
      int end = edge->ends[1] == nodeNdx;
      bool oneWay = edge->config.flags & _configOneWay;
      bool hasStart = !oneWay || !end;
      bool hasEnd = !oneWay || end;

      if (hasStart) {
        nodeStartBlock.set(arrNdx, edge->laneBlocks[end]);
      } else {
        nodeStartBlock.set(arrNdx, 0);
      }

      if (hasEnd) {
        item endBlockNdx = edge->laneBlocks[!end];
        LaneBlock* endBlock = getLaneBlock(endBlockNdx);
        for (int i = 0; i < endBlock->numLanes; i++) {
          spawnDestBlocks.push_back(endBlockNdx+i);
        }
      }
    }

  } else {
    nodeStartBlock.set(arrNdx, 0);
  }
}

void swapIntersections_v(bool fromSave) {
  numNodes_v = sizeNodes();

  if (fromSave) {
    nodeStrategy.clear();
    nodeSpawnProb.clear();
    nodeStartBlock.clear();
    spawnDestBlocks.clear();
    nodeNumPhases.clear();
    nodePhaseBlks.clear();
    nodeBlks.clear();
    nodePhase.clear();
    nodeTimeInPhase.clear();
  }

  nodeStrategy.ensureSize(numNodes_v+1);
  nodeSpawnProb.ensureSize(numNodes_v+1);
  nodeNumPhases.ensureSize(numNodes_v+1);
  nodePhase.ensureSize(numNodes_v+1);
  nodeTimeInPhase.ensureSize(numNodes_v+1);
  nodePhaseBlks.ensureSize((numNodes_v+2)*maxPhases);
  nodeBlks.ensureSize((numNodes_v+1)*maxNodeBlocks);
  nodeStartBlock.ensureSize((numNodes_v+1)*maxNodeBlocks);
  spawnDestBlocks.clear();

  for (int i = 1; i <= numNodes_v; i++) {
    item nodeNdx = -i;
    swapOneIntersection_v(nodeNdx, fromSave);
  }
}

void activateLanes_v(item nodeNdx, item start, item end, bool activate,
    bool yellow) {
  item arrNdx = -nodeNdx;
  item* blocks = nodeBlks.get(arrNdx*maxNodeBlocks);
  for (int i = start; i < end; i++) {
    item block = blocks[i];
    if (block == 0) continue;
    activateBlock_v(block, activate, yellow);
  }
}

void updateIntersection_v(item nodeNdx, float duration) {
  item arrNdx = -nodeNdx;
  item phase = nodePhase[arrNdx];
  item numPhases = nodeNumPhases[arrNdx];
  item strat = nodeStrategy[arrNdx];
  float timeInPhase = nodeTimeInPhase[arrNdx];
  item* blocks = nodeBlks.get(arrNdx*maxNodeBlocks);
  item* phaseBlks = nodePhaseBlks.get(arrNdx*maxPhases);
  item numBlocks = phaseBlks[maxPhases-1];

  if (strat == StopSignStrategy) {
    timeInPhase += duration;
    activateLanes_v(nodeNdx, 0, maxNodeBlocks, false, false);

    if (numPhases == 0) {
      activateLanes_v(nodeNdx, 0, maxNodeBlocks, true, false);

    } else if (phase < 0 || timeInPhase > c(CStopSignDuration)) {
      timeInPhase = 0;
      bool anyMatched = false;

      for (int i=1; i <= numPhases; i++) {
        int ip = (i+phase)%numPhases;
        if (ip == phase) {
          continue;
        }

        bool carsWaiting = false;
        item phaseStart = ip == 0 ? 0 : phaseBlks[ip-1];
        item phaseEnd = phaseBlks[ip];
        for(int j=phaseStart; j < phaseEnd; j++) {
          if (detectBlock_v(blocks[j])) {
            carsWaiting = true;
            break;
          }
        }

        if (carsWaiting) {
          phase = ip;
          activateLanes_v(nodeNdx, phaseStart, phaseEnd, true, false);
          anyMatched = true;
          break;
        }
      }

      if (!anyMatched) {
        phase = (phase+1)%numPhases;
      }

      //if (!anyMatched) {
        //phase = -1;
        //activateLanes_v(nodeNdx, 0, maxNodeBlocks, false, false);
      //}

    } else if (phase >= 0) {
      item phaseStart = phase == 0 ? 0 : phaseBlks[phase-1];
      item phaseEnd = phaseBlks[phase];
      activateLanes_v(nodeNdx, phaseStart, phaseEnd, true, false);

    } else {
      activateLanes_v(nodeNdx, 0, maxNodeBlocks, false, false);
    }

  } else if (strat == TrafficLightStrategy) {
    float yellowDuration = c(CStopLightMaxPhaseDuration) -
      c(CStopLightYellowDuration);
    timeInPhase += duration;
    activateLanes_v(nodeNdx, 0, maxNodeBlocks, false, false);
    item phaseStart = phase == 0 ? 0 : phaseBlks[phase-1];
    item phaseEnd = phaseBlks[phase];

    if (numPhases <= 0) {
      phase = 0;
      timeInPhase = 0;

    } else if (timeInPhase > c(CStopLightMaxPhaseDuration)) {
      phase = (phase + 1) % numPhases;
      timeInPhase -= c(CStopLightMaxPhaseDuration);

    } else if (timeInPhase < yellowDuration &&
        timeInPhase > c(CStopLightMinPhaseDuration)) {
      int numVehicles = 0;
      int numRequests = 0;
      for (int i = 0; i < numBlocks; i++) {
        item blk = blocks[i];
        if (i >= phaseStart && i <= phaseEnd) {
          numVehicles += numVehiclesInBlock_v(blk);
        } else if (detectBlock_v(blk)) {
          numRequests ++;
        }
      }

      if (numRequests > numVehicles) {
        timeInPhase = yellowDuration;
      //} else if (numRequests == 0) {
        //timeInPhase = 0;
      }
    }

    if (timeInPhase >= yellowDuration) {
      activateLanes_v(nodeNdx, phaseStart, phaseEnd, false, true);
    } else {
      activateLanes_v(nodeNdx, phaseStart, phaseEnd, true, false);
    }

  } else {
    activateLanes_v(nodeNdx, 0, maxNodeBlocks, true, false);
  }

  nodePhase.set(arrNdx, phase);
  nodeTimeInPhase.set(arrNdx, timeInPhase);
}

// Called in Vehicle thread
void updateIntersections_v(float duration) {
  for (int i = 1; i <= numNodes_v; i++) {
    item nodeNdx = -i;
    updateIntersection_v(nodeNdx, duration);
  }
}

// Called in Game thread
void updateIntersections(double duration) {
  bool lanesReady = areLanesReady();
  for (int i = 1; i <= sizeNodes() && i <= numNodes_v; i++) {
    item nodeNdx = -i;
    item strat = nodeStrategy[i];
    if (strat == SpawnStrategy && lanesReady) {
      float spawn = nodeSpawnProb[i] * duration;
      while (spawn > 1) {
        spawnVehicle(nodeNdx, false);
        spawn --;
      }
      if (randFloat() < spawn) {
        spawnVehicle(nodeNdx, false);
      }

    } else if (strat == TrafficLightStrategy) {
      setStopLightPhaseTexture(nodeNdx);
    }
  }
}

/*
void updateIntersection(item nodeNdx, float duration) {
  Node* node = getNode(nodeNdx);
  vector<item> edges = getCompletedEdges(nodeNdx);

  if (edges.size() <= 2) {
    if (getGameMode() != ModeGame && edges.size() == 1) {
      float spawn = node->spawnProbability * duration;
      while (spawn > 1) {
        spawnVehicle(nodeNdx, false);
        spawn --;
      }
      if (randFloat() < spawn) {
        spawnVehicle(nodeNdx, false);
      }
    }
    return;
  }

  if (getNodeInputs(nodeNdx, true).size() <= 2) {
    activateLanes(node, 0, node->laneBlocks.size()-1, true, false);
    return;
  }

  int phase = node->phase;
  int numPhases = node->phaseMins.size();

  if (node->config.strategy == StopSignStrategy) {
    node->timeInPhase += duration;
    activateLanes(node, 0, node->laneBlocks.size()-1, false, false);

    if (phase < 0 || node->timeInPhase > c(CStopSignDuration)) {
      node->timeInPhase = 0;

      for (int i=1; i <= numPhases; i++) {
        int ip = (i+phase)%numPhases;
        if (ip == phase) {
          continue;
        }

        bool carsWaiting = true;
        for(int j=node->phaseMins[ip]; j <= node->phaseMaxs[ip]; j++) {
          if (true) { //detectBlock(node->laneBlocks[j])) {
            carsWaiting = true;
            break;
          }
        }

        if (carsWaiting) {
          node->phase = ip;
          activateLanes(node, node->phaseMins[node->phase],
            node->phaseMaxs[node->phase], true, false);
          return;
        }
      }

      node->phase = -1;
    }

  } else if (node->config.strategy == TrafficLightStrategy) {
    int phase = node->phase;
    node->timeInPhase += duration;
    float time = node->timeInPhase;
    activateLanes(node, 0, node->laneBlocks.size()-1, false, false);
    if (time > phaseDuration) {
      node->phase = (phase + 1) % node->phaseMins.size();
      node->timeInPhase -= phaseDuration;
      activateLanes(node, node->phaseMins[phase],
        node->phaseMaxs[phase], true, false);

    } else {
      if (time > 10 && time < c(CStopLightYellowDuration)) {
        int numVehicles = 0;
        int numRequests = 0;
        for (int i = 0; i < node->laneBlocks.size(); i++) {
          item blk = node->laneBlocks[i];
          if (i >= node->phaseMins[phase] && i <= node->phaseMaxs[phase]) {
            numVehicles += numVehiclesInBlock(blk);
          } else if (detectBlock(blk)) {
            numRequests ++;
          }
        }
        if (numRequests > numVehicles) {
          node->timeInPhase = c(CStopLightYellowDuration);
        }
      }

      if (node->timeInPhase >= c(CStopLightYellowDuration)) {
        activateLanes(node, node->phaseMins[phase],
          node->phaseMaxs[phase], false, true);
      } else {
        activateLanes(node, node->phaseMins[phase],
          node->phaseMaxs[phase], true, false);
      }
    }
    setStopLightPhaseTexture(node);

  } else if (node->config.strategy == JointStrategy) {
    activateLanes(node, 0, node->laneBlocks.size()-1, true, false);
  }
}
*/

