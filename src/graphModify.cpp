#include <set>
using namespace std;

typedef set<item> itemset;
typedef itemset::iterator itemsetIter;
static itemset toRender;
static vector<item> toFree;
static itemset toSimplify;
static itemset toElevate;
static itemset toCollide;
static itemset toName;
static itemset toSetupLaneNodes;
static itemset toSetupLaneEdges;
static itemset toMakeLots;
static itemset toResizeNodes;
static itemset toResizeEdges;
static itemset toRepositionEdges;

void queueCollide(item ndx) {
  toCollide.insert(ndx);
}

bool canSimplifyNode(item ndx, bool byPlayer) {
  Node* node = getNode(ndx);
  if (node->flags & _graphCity) return false;
  if (node->pillar != 0) return false;
  if (node->edges.size() != 2) return false;
  if (isCutTool()) return false;

  item e0ndx = node->edges[0];
  item e1ndx = node->edges[1];
  Edge* e0 = getEdge(e0ndx);
  Edge* e1 = getEdge(e1ndx);
  if ((e0->flags & _graphComplete) != (e1->flags & _graphComplete)) {
    return false;
  }
  Configuration config = e0->config;
  if (!configsEqualEdge(config, e1->config)) return false;

  item end0 = e0->ends[0] == ndx;
  item end1 = e1->ends[0] == ndx;
  if ((config.flags & _configOneWay) && end0 == end1) return false;
  if (detectSemiDuplicate(e0ndx) != 0) return false;

  Line l0 = e0->line;
  Line l1 = e1->line;
  vec3 a0 = l0.start;
  vec3 a1 = l0.end;
  vec3 b0 = l1.start;
  vec3 b1 = l1.end;
  vec3 A = normalize(a1 - a0);
  vec3 B = normalize(b1 - b0);
  vec3 crossVec = cross(A, B);
  float denom0 = length(cross(A, B));
  float denom1 = length(cross(A, -B));
  float denom = std::min(denom0, denom1);
  float threshold = byPlayer ? 0.05 : 0.01;
  if (denom > threshold) return false;

  // Node is overcomplex, delete it
  item n0 = end0 ? e0->ends[1] : e0->ends[0];
  item n1 = end1 ? e1->ends[1] : e1->ends[0];
  if (n0 == ndx || n1 == ndx || n0 == n1) return false; // Sanity check
  return true;
}

bool simplifyNode(item ndx, bool byPlayer) {
  if (!canSimplifyNode(ndx, byPlayer)) return false;
  Node* node = getNode(ndx);
  item e0ndx = node->edges[0];
  item e1ndx = node->edges[1];
  Edge* e0 = getEdge(e0ndx);
  Edge* e1 = getEdge(e1ndx);
  Configuration config = e0->config;
  item end0 = e0->ends[0] == ndx;
  item end1 = e1->ends[0] == ndx;
  item n0 = end0 ? e0->ends[1] : e0->ends[0];
  item n1 = end1 ? e1->ends[1] : e1->ends[0];
  removeEdge(e0ndx, false);
  removeEdge(e1ndx, false);
  removeNode(ndx);
  item newEdge = end0 ? addEdge(n1, n0, config) : addEdge(n0, n1, config);
  complete(newEdge, false, config);
  return true;
}

void simplifyNodeIter(item ndx, int flags) {
  simplifyNode(ndx, false);
}

void resizeNode(item ndx, int flags) {
  resizeNode(ndx);
}

void resizeEdge(item ndx, int flags) {
  resizeEdge(ndx);
}

void repositionEdge(item ndx, int flags) {
  repositionEdge(ndx);
}

void makeLots(item ndx, int flags) {
  if (!(flags & _graphComplete)) return;
  if (flags & _graphCity) return;
  if (getElementConfiguration(ndx).type != ConfigTypeRoad) return;

  if (ndx < 0) {
    Node* node = getNode(ndx);
    vector<item> edges = getCompletedEdges(ndx);
    if (edges.size() == 0) return;
    Line el = getLine(edges[0]);
    vec3 norm = el.end-el.start;
    norm.z = 0;
    norm = normalize(norm) * (node->intersectionSize + c(CBuildDistance));
    makeNodeLots(node->center, norm, ndx);

  } else {
    float width = edgeWidth(ndx);
    Line line = getLine(ndx);
    makeLots(line, width/2 + tileSize * .25f, ndx);
  }
}

void setupLanes(item ndx, int flags) {
  if (!(flags & _graphComplete)) return;

  if (ndx < 0) {
    Node* node = getNode(ndx);
    int numEdges = node->edges.size();
    if (numEdges > 0) {
      rebuildIntersection(ndx);
      for (int i = 0; i < node->laneBlocks.size(); i++) {
        setLaneBlockState(node->laneBlocks[i]);
      }
    }

  } else if (ndx > 0) {
    Edge* edge = getEdge(ndx);
    Configuration config = edge->config;

    if (config.type == ConfigTypePedestrian) {
      if (edge->laneBlocks[0] != 0) removeLaneBlock(edge->laneBlocks[0]);
      if (edge->laneBlocks[1] != 0) removeLaneBlock(edge->laneBlocks[1]);
      edge->laneBlocks[0] = 0;
      edge->laneBlocks[1] = 0;
      return;
    }

    item end0ndx = edge->ends[0];
    item end1ndx = edge->ends[1];
    EndDescriptor endDesc0 = getEdgeEndDescriptor(ndx, end0ndx);
    EndDescriptor endDesc1 = getEdgeEndDescriptor(ndx, end1ndx);

    if (edge->laneBlocks[0] == 0) {
      edge->laneBlocks[0] = addLaneBlock(ndx, endDesc0, endDesc1,
          config.numLanes);
    } else {
      reconfigureLaneBlock(edge->laneBlocks[0], endDesc0, endDesc1,
        config.numLanes);
    }

    if (edge->config.flags & _configOneWay) {
      if (edge->laneBlocks[1] != 0) {
        removeLaneBlock(edge->laneBlocks[1]);
        edge->laneBlocks[1] = 0;
      }
    } else if (edge->laneBlocks[1] == 0) {
      edge->laneBlocks[1] = addLaneBlock(ndx, endDesc1, endDesc0,
          config.numLanes);
    } else {
      reconfigureLaneBlock(edge->laneBlocks[1], endDesc1, endDesc0,
        config.numLanes);
    }

    setLaneBlockState(edge->laneBlocks[0]);
    setLaneBlockState(edge->laneBlocks[1]);
  }
}

void setupEdgeLanes(item ndx) {
  setupLanes(ndx, getEdge(ndx)->flags);
}

void resetCollisions(item ndx, int flags) {
  if (flags & _graphCity) return;
  removeFromCollisionTable(GraphCollisions, ndx);
  Box b = getGraphBox(ndx);
  addToCollisionTable(GraphCollisions, b, ndx);
  if (!(flags & _graphComplete)) return;
  if (flags & _graphUnderground) return;
  removeLots(b);
  removeCollidingBuildings(b);
}

bool isNodeUnderground(item ndx) {
  Node* node = getNode(ndx);
  if (node->flags & _graphCity) return false;
  return (node->config.flags & _configDontMoveEarth) &&
    pointOnLand(node->center).z+2 > node->center.z &&
    pointOnLandNatural(node->center).z > node->center.z;
}

bool isEdgeUnderground(item ndx) {
  Edge* edge = getEdge(ndx);
  return (edge->config.flags & _configDontMoveEarth) &&
    (isNodeUnderground(edge->ends[0]) || isNodeUnderground(edge->ends[1]));
}

void addGraphElevator(item ndx, int flags) {
  if (!(flags & _graphComplete)) return;
  if (flags & _graphCity) return;
  if (ndx < 0) {
    if (!(getElementConfiguration(ndx).flags & _configDontMoveEarth)) {
      addGraphElevator(ndx, true);
    } else {
      clearTrees(getGraphBox(ndx));
    }

  } else if (ndx > 0) {
    Edge* edge = getEdge(ndx);
    //bool n0u = isNodeUnderground(edge->ends[0]);
    //bool n1u = isNodeUnderground(edge->ends[1]);
    bool edme = edge->config.flags & _configDontMoveEarth;
    if (!edme) {
      addGraphElevator(ndx, true);
    } else {
      clearTrees(getGraphBox(ndx));
    }

    /*
    if (edge->config.type == ConfigTypePedestrian && edme) {
      edge->config.flags |= _configDontMoveEarth;

    } else if (!edme || (n0u && !n1u) || (!n0u && n1u)) {
      edge->config.flags &= ~_configDontMoveEarth;
      addGraphElevator(ndx, true);

    } else {
      edge->config.flags |= _configDontMoveEarth;
    }
    */
  }
}

/*
void resetGraphElevator(item ndx, int flags) {
  removeGraphElevator(ndx);
  addGraphElevator(ndx, flags);
}
*/

void clearName(item edgeNdx, int flags) {
  Edge* edge = getEdge(edgeNdx);
  if (edge->name != 0) free(edge->name);
  edge->name = strdup_s("");
}

bool nameEdge(item edgeNdx, item end);
void nameEdgeCallback(item edgeNdx, int flags) {
  Edge* edge = getEdge(edgeNdx);
  //if (edge->flags & _graphComplete) return;
  //SPDLOG_INFO("edge {}:start", edgeNdx);
  if (!nameEdge(edgeNdx, 0)) {
    //SPDLOG_INFO("edge {}:end", edgeNdx);
    if (!nameEdge(edgeNdx, 1)) {
      //SPDLOG_INFO("edge {}:try again", edgeNdx);
      toName.insert(edgeNdx);
      //edge->name = randomName(RoadName);
    }
  }
}

static void iterate(itemset& s, void (*callback)(item ndx, int flags),
    bool clear) {

  for (itemsetIter it = s.begin(); it != s.end(); it++) {
    item el = *it;
    if (el == 0) continue;
    int flags = el < 0 ? getNode(el)->flags : getEdge(el)->flags;
    if (!(flags & _graphExists)) continue;
    callback(el, flags);
  }

  if (clear) s.clear();
}

static void iterate(itemset& s, void (*callback)(item ndx, int flags)) {
  iterate(s, callback, true);
}

void finishGraph() {
  bool doRepositionTransitStops =
    toFree.size() > 0 || toRepositionEdges.size() > 0;

  iterate(toSimplify, simplifyNodeIter);
  iterate(toResizeNodes, resizeNode);
  iterate(toResizeEdges, resizeEdge);
  iterate(toRepositionEdges, repositionEdge);
  iterate(toSetupLaneEdges, setupLanes);
  iterate(toSetupLaneNodes, setupLanes);
  //iterate(toReelevate, resetGraphElevator);
  iterate(toElevate, addGraphElevator);
  iterate(toCollide, resetCollisions);
  iterate(toMakeLots, makeLots);
  collideBuildingCache.clear();

  if (doRepositionTransitStops) {
    repositionTransitStops();
    finishLots();
  }

  int toNameS = toName.size();
  //if (toNameS > 0) SPDLOG_INFO("\n\n\n ### naming");
  iterate(toName, clearName, false);
  while (toNameS > 0) {
    itemset toNameTemp = toName;
    toName.clear();
    //SPDLOG_INFO("\n# to name {}", toNameS);
    iterate(toNameTemp, nameEdgeCallback, false);

    if (toNameS == toName.size()) {
      item ndx = *toName.begin();
      Edge* edge = getEdge(ndx);
      edge->name = randomName(RoadName);
      //SPDLOG_INFO("force naming edge {}:{}", ndx, edge->name);
      toName.erase(ndx);
    }
    toNameS = toName.size();
  }

  // Free
  for (int i = 0; i < toFree.size(); i++) {
    item ndx = toFree[i];
    if (ndx < 0) {
      nodes->free(-ndx);
    } else if (ndx > 0) {
      clearGraphChildren(ndx);
      edges->free(ndx);
    }
  }
  toFree.clear();

  freeLaneBlocks();
}

void renderGraph() {
  // Render
  for (itemsetIter it = toRender.begin(); it != toRender.end(); it++) {
    item el = *it;
    if (el < 0) {
      renderNode(el);
    } else {
      renderEdge(el);
      item plan = getEdge(el)->plan;
      if (plan != 0) {
        updatePlan(plan);
      }
    }
  }
  toRender.clear();
}

item graphElementsToRender() {
  return toRender.size();
}

void renderGraphElement(item ndx) {
  toRender.insert(ndx);
}

vec3 edgeCutsStartPoint;

struct compareCuts : binary_function <vec3, vec3, bool> {
  bool operator() (vec3 const& x, vec3 const& y) const {
    return vecDistance(edgeCutsStartPoint, x) < vecDistance(edgeCutsStartPoint, y);
  }
};

void split(item nodeNdx, item edgeNdx, Configuration nodeConfig);
void trimEdge(item ndx, Box bbox) {
  Edge* original = getEdge(ndx);
  if (!(original->flags & _graphExists)) return;

  bbox = growBox(bbox, tileSize);
  Line l = getLine(ndx);
  vector<vec3> cuts = boxLineIntersect(bbox, l);
  SPDLOG_INFO("trimEdge({}, box) {} cuts", ndx, cuts.size());

  if (cuts.size() == 0) {
    // No intersection, let's check if it's inside the box
    float dist = boxDistance(bbox, l.start);
    dist = std::min(dist, boxDistance(bbox, l.end));
    if (dist <= 0) {
      removeEdge(ndx, true);
    }
    return;
  }

  edgeCutsStartPoint = l.start;
  compareCuts comparator;
  sort(cuts.begin(), cuts.end(), comparator);
  vector<item> newEdges;
  item runningEdge = ndx;
  Configuration config = getElementConfiguration(ndx);

  for (int i = 0; i < cuts.size(); i++) {
    item nodeNdx = getOrCreateNodeAt(cuts[i], config);
    if (nodeNdx == 0) continue;
    split(nodeNdx, runningEdge, config);
    Node* node = getNode(nodeNdx);
    SPDLOG_INFO("split node#{} {} edges", nodeNdx, node->edges.size());
    if (node->edges.size() > 1) {
      runningEdge = node->edges[1];
      newEdges.push_back(node->edges[0]);
    }
  }
  newEdges.push_back(runningEdge);

  for (int i = 0; i < newEdges.size(); i++) {
    item newEdgeNdx = newEdges[i];
    Edge* newEdge = getEdge(newEdgeNdx);
    if (!(newEdge->flags & _graphExists)) continue;
    Line nl = getLine(newEdgeNdx);
    float dist = boxDistance(bbox, nl.start);
    dist = std::min(dist, boxDistance(bbox, nl.end));
    if (dist <= 0) {
      removeEdge(newEdgeNdx, true);
    }
  }
}

