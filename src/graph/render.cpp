#include "spdlog/spdlog.h"

const int _rpIsEnd = 1 << 0;

struct RingPoint {
  uint32_t flags;
  vec3 place;
  vec3 norm;
  vec3 normTrap;
  float normTrapDist;
};

struct Ring {
  vec3 center;
  vec3 offset;
  vector<RingPoint> points;
};

Mesh* mesh;
Ring ring;

RingPoint ringPoint(Ring* ring, vec3 place, vec3 norm,
    vec3 normTrapLoc, uint32_t flags) {
  RingPoint result;
  result.flags = flags;
  result.place = place;
  result.norm = normalize(norm);
  result.normTrap = normTrapLoc;
  result.normTrapDist = length(normTrapLoc - place);
  ring->points.push_back(result);
  return result;
}

RingPoint ringPoint(Ring* ring, vec3 place, vec3 norm, uint32_t flags) {
  RingPoint result;
  result.flags = flags;
  result.place = place;
  result.norm = normalize(norm);
  result.normTrap = vec3(0,0,0);
  result.normTrapDist = -1;
  ring->points.push_back(result);
  return result;
}

vector<vec3> getRingVec3s(Ring ring) {
  vector<vec3> result;
  for (int i = 0; i < ring.points.size(); i++) {
    result.push_back(ring.points[i].place);
  }
  return result;
}

vec3 convertRingPoint(Ring ring, int i, vec3 place) {
  RingPoint p = ring.points[i];
  float x = place.x;
  vec3 loc = p.place;
  vec3 norm = p.norm;

  if (p.normTrapDist > 0 && x > p.normTrapDist) {
    loc = p.normTrap;
    norm = p.normTrap - ring.center;
    x -= p.normTrapDist;
  }

  loc += norm * x;
  loc.z += place.z;

  return loc;
}

vec3 covertRingNorm(Ring ring, int i, vec3 along) {
  RingPoint p = ring.points[i];
  return normalize(p.norm*along.x + vec3(0,0,1)*along.z);
}

void makeCenterRing(vec3 place, vec3 tx) {
  int ringSize = ring.points.size();
  vec3 center = ring.center - ring.offset;
  SPDLOG_INFO("makeCenterRing ringSize:{}", ringSize);
  for (int i = 0; i < ringSize; i++) {
    int i1 = (i+1) % ringSize;
    vec3 r0 = convertRingPoint(ring, i, place) - ring.offset;
    vec3 r1 = convertRingPoint(ring, i1, place) - ring.offset;
    vec3 upn0 = up + ring.points[i].norm * roadMound;
    vec3 upn1 = up + ring.points[i1].norm * roadMound;
    Triangle t = {{
      {r0, upn0, tx},
      {center, up, tx},
      {r1, upn1, tx},
    }};
    pushTriangle(mesh, t);
  }
}

void makeRing(vec3 start, vec3 along, vec3 tx) {
  int ringSize = ring.points.size();
  vec3 center = ring.center - ring.offset;
  SPDLOG_INFO("makeRing ringSize:{}", ringSize);
  for (int i = 0; i < ringSize; i++) {
    int i1 = (i+1) % ringSize;
    vec3 r0 = convertRingPoint(ring, i, start) - ring.offset;
    vec3 r1 = convertRingPoint(ring, i1, start) - ring.offset;
    vec3 r2 = convertRingPoint(ring, i, start+along) - ring.offset;
    vec3 r3 = convertRingPoint(ring, i1, start+along) - ring.offset;
    vec3 n0 = covertRingNorm(ring, i, along);
    vec3 n1 = covertRingNorm(ring, i1, along);

    vec3 upn0 = up + ring.points[i].norm * roadMound;
    vec3 upn1 = up + ring.points[i1].norm * roadMound;
    makeQuad(mesh, r0, r1, r2, r3, n0, n1, n0, n1, tx, tx);
  }
}

Ring getEdgeRing(item edgeNdx) {
  Edge* edge = getEdge(edgeNdx);
  Ring ring;
  ring.center = (edge->line.start + edge->line.end) * .5f;
  float width = edgeWidth(edgeNdx);
  vec3 norm = uzNormal(edge->line.end - edge->line.start) * width;
  ringPoint(&ring, edge->line.start+norm,  norm, 0);
  ringPoint(&ring, edge->line.start-norm, -norm, _rpIsEnd);
  ringPoint(&ring, edge->line.end+norm,    norm, 0);
  ringPoint(&ring, edge->line.end-norm,   -norm, _rpIsEnd);
  return ring;
}

Ring getNodeRing(item nodeNdx, bool simple) {
  Node* node = getNode(nodeNdx);
  vector<item> edges = getRenderableEdges(nodeNdx);
  Ring ring;
  vec3 center = node->center;
  ring.center = center;
  int numEdges = edges.size();
  int gameMode = getGameMode();
  bool isExpwy = node->config.type == ConfigTypeExpressway;

  for(int i = 0; i < numEdges; i++) {
    Edge* edge = getEdge(edges[i]);
    if (debugMode() && !(edge->flags & _graphExists)) {
      handleError("edge doesn't exist");
    }

    // make culdesac
    if (numEdges == 1) {
      vec3 edgeLoc = edge->ends[0] == nodeNdx ?
        edge->line.start : edge->line.end;
      vec3 nalong = edgeLoc - center;
      vec3 dir = normalize(edgeLoc - center) *
        node->intersectionSize;
      vec3 unorm = normalize(vec3(dir.y, -dir.x, 0));
      vec3 norm = unorm * (edgeWidth(edges[i])*.5f);
      vec3 loc = center + nalong;
      ringPoint(&ring, loc - norm, -norm, 0);
      ringPoint(&ring, loc + norm,  norm, _rpIsEnd);

      if (gameMode == ModeTest || isExpwy) {
        loc = center - nalong;
        ringPoint(&ring, loc + norm,  norm, 0);
        ringPoint(&ring, loc - norm, -norm, 0);

      } else if (gameMode == ModeGame) {
        int culdesacVerticies = simple ? 6 : 24;
        for(int i = 0; i < culdesacVerticies; i ++) {
          float theta =  - i * pi_o * 1.25 / (culdesacVerticies - 1 ) +
            pi_o * 0.625;
          vec3 rdir = rotateZ(nalong, theta);
          ringPoint(&ring, center - rdir, -rdir, 0);
        }
      }

    } else { // Make a round corner
      item otherNdx = edges[(i+1)%numEdges];
      Edge* other = getEdge(otherNdx);
      Edge* cornerEdges[2] = {edge, other};
      float widths[2] = {edgeWidth(edges[i]), edgeWidth(otherNdx)};
      vec3 cnorms[2];
      Line l[2];

      for (int j = 0; j < 2; j++) {
        if (cornerEdges[j]->ends[1] == nodeNdx) {
          l[j] = line(cornerEdges[j]->line.end, cornerEdges[j]->line.start);
        } else {
          l[j] = cornerEdges[j]->line;
        }

        l[j].start.z = center.z;
        l[j].end.z = center.z;
        vec3 along = l[j].end - l[j].start;
        vec3 unorm = normalize(vec3(along.y, -along.x, 0));
        if (j == 1) unorm *= -1;
        vec3 norm = unorm * widths[j] * .5f;
        cnorms[j] = norm;
        l[j].start += norm;
        l[j].end += norm;
      }

      vec3 normTrap = pointOfIntersection(
          line(l[0].start, l[0].start+cnorms[0]*100.f),
          line(l[1].start, l[1].start+cnorms[1]*100.f));
      Spline s = spline(l[0], l[1]);

      int numCurveSegments = simple ? 1 : 12;
      for (int j = 0; j <= numCurveSegments; j++) {
        float theta = float(j)/numCurveSegments;
        vec3 loc = interpolateSpline(s, theta);
        vec3 norm = cnorms[0] * (1-theta) + cnorms[1] * theta;
        uint32_t flags = (j == 0) ? _rpIsEnd : 0;
        ringPoint(&ring, loc, norm, normTrap, 0);
      }
    }
  }

  return ring;
}

void renderGraphElement(item ndx, vec3 node_tx) {
}

  /* push functions and arguments */
  //lua_getglobal(L, "f");  /* function to be called */
  //lua_pushnumber(L, ndx);   /* push 1st argument */
    //
      /* do the call (2 arguments, 1 result) */
      //if (lua_pcall(L, 2, 1, 0) != 0)
        //error(L, "error running function `f': %s",
                 //lua_tostring(L, -1));
    //
      /* retrieve result */
      //if (!lua_isnumber(L, -1))
        //error(L, "function `f' must return a number");
      //z = lua_tonumber(L, -1);
      //lua_pop(L, 1);  /* pop returned value */
      //return z;
//}

void renderNodeNew(item ndx) {
  if (!c(CGraphVisible)) return;
  if (!isRenderEnabled()) { return; }
  int gameMode = getGameMode();
  if (gameMode == ModeBuildingDesigner) { return; }
  Node* node = getNode(ndx);
  if (!(node->flags & _graphExists)) return;

  if (node->entity == 0) {
    node->entity = addEntity(RoadShader);
    node->signEntity = addEntity(SignShader);
  }

  vec3 center = node->center;
  vector<item> edges = getRenderableEdges(ndx);
  int numEdges = edges.size();
  bool isUnderground = isNodeUnderground(ndx);
  float landZ = pointOnLand(center).z;
  bool makeTunnel = isUnderground && landZ - center.z < 4;
  bool complete = node->flags & _graphComplete;
  bool drawDirt = !isUnderground && complete &&
    abs(center.z-landZ) < c(CRoadRise)*4;
  bool drawGirder = !isUnderground && !drawDirt && complete;
  bool isExpwy = node->config.type == ConfigTypeExpressway;
  bool isRail = node->config.type == ConfigTypeHeavyRail;
  bool isPed = node->config.type == ConfigTypePedestrian;
  bool isRoadway = !isRail && !isPed;
  bool isRoad = node->config.type == ConfigTypeRoad;
  bool drawSidewalk = isRoad &&
    (drawGirder || isUnderground || shouldDrawSidewalk(center));

  Entity* entity = getEntity(node->entity);
  item texture = isExpwy ? expresswayTexture :
    isRail ? railTexture : roadTexture;
  vec3 node_tx = isRail ? node_x : isPed ? sidewalk_xs : node_x;
  entity->texture = texture;
  placeEntity(node->entity, node->center, 0, 0);
  setCull(node->entity, node->intersectionSize, 100000);
  entity->simpleDistance = graphSimpleDistance * node->intersectionSize/10;

  createMeshForEntity(node->entity);
  createSimpleMeshForEntity(node->entity);
  createMeshForEntity(node->signEntity);

  Entity* tunnelEntity = 0;
  Mesh* tunnelMesh = 0;
  //if (node->flags & _graphUnderground) {
    if (node->tunnelEntity == 0) {
      node->tunnelEntity = addEntity(RoadShader);
    }
    tunnelEntity = getEntity(node->tunnelEntity);
    tunnelEntity->texture = texture;
    setCull(node->tunnelEntity, node->intersectionSize, 100000);
    placeEntity(node->tunnelEntity, node->center, 0, 0);
    setEntityVisible(node->tunnelEntity, tunnelsVisible());
    createMeshForEntity(node->tunnelEntity);
    tunnelMesh = getMesh(tunnelEntity->mesh);
  //}

  Mesh* complexMesh = getMesh(entity->mesh);
  Mesh* simpleMesh = getMesh(entity->simpleMesh);

  if (edges.size() <= 2 && !(node->flags & _graphComplete)) {
    bufferMesh(entity->mesh);
    bufferMesh(entity->simpleMesh);
    bufferMesh(getEntity(node->signEntity)->mesh);
    setEntityVisible(node->entity, false);
    return;
  }

  // Make railroads
  for (int i = 0; i < node->laneBlocks.size(); i ++) {
    LaneBlock* block = getLaneBlock(node->laneBlocks[i]);
    if (!(block->flags & _laneRailway)) continue;
    for (int j = 0; j < block->numLanes; j ++) {
      makeRailroad(complexMesh, block, j, node->center, isRail);
    }
  }

  if (abs(node->center.z-pointOnLand(node->center).z) > c(CRoadRise)*4) {
    makeSupportPillar(ndx, complexMesh, simpleMesh, vec3(0,0,0), node->center);
  }

  for (int simple = 0; simple < 2; simple++) {
    ring = getNodeRing(ndx, simple);
    ring.offset = node->center;
    mesh = simple ? simpleMesh : complexMesh;
    renderGraphElement(ndx, node_tx);

    makeCenterRing(vec3(0,0,0), node_tx);
    if (drawSidewalk) {
      makeRing(vec3(0, 0, 0), vec3(10, 0, 0), sidewalk_xs);
      makeRing(vec3(10, 0, 0), vec3(10, 0, -10), sholder_xs);
    } else {
      // Sholder
      makeRing(vec3(0, 0, 0), vec3(10, 0, -10), sholder_xs);
    }

    if (!simple && node->config.strategy == TrafficLightStrategy) {
      renderTrafficLights(ndx, getRingVec3s(ring));
    }
  }

  bufferMesh(entity->mesh);
  bufferMesh(entity->simpleMesh);
  if (tunnelEntity != 0) {
    bufferMesh(tunnelEntity->mesh);
  }

  // Position bridge pillar, if any
  if (node->pillar && numEdges > 0) {
    renderPillar(node->pillar);
  }

  // Render stop signs or traffic lights
  if (node->config.strategy == StopSignStrategy) {
    renderStopSigns(ndx);
  //} else if (node->config.strategy == TrafficLightStrategy) {
    //renderTrafficLights(ndx, getRingVec3s(ring));
  } else if (node->config.strategy == JointStrategy) {
    if (node->signEntity != 0) {
      setEntityVisible(node->signEntity, false);
    }
  }
}

