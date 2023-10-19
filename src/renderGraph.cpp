#include "renderGraph.hpp"

#include "color.hpp"
#include "building/design.hpp"
#include "draw/entity.hpp"
#include "draw/texture.hpp"
#include "game/game.hpp"
#include "graph.hpp"
#include "graph/stop.hpp"
#include "graph/transit.hpp"
#include "heatmap.hpp"
#include "icons.hpp"
#include "intersection.hpp"
#include "land.hpp"
#include "option.hpp"
#include "pillar.hpp"
#include "renderPillar.hpp"
#include "renderUtils.hpp"
#include "spline.hpp"
#include "string.hpp"
#include "string_proxy.hpp"
#include "util.hpp"

#include "spdlog/spdlog.h"
#include <glm/gtx/rotate_vector.hpp>
#include <stdio.h>

static vec3 up = vec3(0, 0, 1);
static vec3 railRise = vec3(0, 0, .25f);
static const float roadMound = 0.04f;
static const float graphSimpleDistance = 2000;

//Colors
vec4 roadColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
vec4 roadIllegalColor = vec4(1.0f, 0.5f, 0.5f, 1.0f);
vec4 laneColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
vec4 laneClosedColor = vec4(1.0f, 0.5f, 0.5f, 1.0f);
vec4 laneGreenColor = vec4(0.f, 0.5f, 0.f, 1.0f);
vec4 laneRedColor = vec4(0.5f, 0.f, 0.f, 1.0f);

static const vec3 xGray = vec3(0.5/spriteSheetSize, 4.5/spriteSheetSize, 1);

//Stop light render
static float stopLightHeight = 7.5;
static float stopLightPoleWidth = 0.5;
static float stopLightBoxSize = 1;
static float stopLightBoxHeight = 2;
static float trainSignalBoxSize = 1;
static float trainSignalBoxHeight = 2;
static float signPoleOffset = 2;

float roadTextureSizeX = 8192;
float roadTextureSizeY = 512;
vec3 sholder_xs = vec3(16/roadTextureSizeX, 0.1f, 0);
vec3 sidewalk_xs = vec3(48/roadTextureSizeX, 0.1f, 0);
vec3 bridge_xs = vec3(80/roadTextureSizeX, 0.125f, 0);
vec3 trunk_xs = vec3(80/roadTextureSizeX, 0.375f, 0);
vec3 grass_xs = vec3(80/roadTextureSizeX, 0.75f, 0);
vec3 guard_xs = vec3(120/roadTextureSizeX, 0.0f, 0);
vec3 node_x = vec3(144/roadTextureSizeX, 0.0f, 0);
vec3 suspension_x = vec3(65/roadTextureSizeX, 1.f/512.f, 0);
vec3 rail_x = vec3(67/roadTextureSizeX, 1.f/512.f, 1);
vec3 black_x = vec3(69/roadTextureSizeX, 1.f/512.f, 0);
float twoWayRoadTex[] = {160, 406, 812, 1530};
float oneWayRoadTex[] = {3920, 4080, 4320, 4640, 5040, 5520, 6080};
float undenseRailTex[] = {171, 253, 329, 408, 487};
float denseRailTex[] = {549, 628, 707, 775, 854};

bool shouldDrawSidewalk(vec3 loc) {
  float dens = getGameMode() == ModeBuildingDesigner ?
    getDesign(1)->minDensity : heatMapGet(Density, loc);
  return dens > c(CSidewalkDensity);
}

bool shouldDrawRailDense(vec3 loc) {
  float dens = getGameMode() == ModeBuildingDesigner ?
    getDesign(1)->minDensity : heatMapGet(Density, loc);
  return dens > c(CPavedRailDensity);
}

void renderStopSigns(item ndx) {
  Node* node = getNode(ndx);
  vector<item> edges = getRenderableEdges(ndx);
  int numEdges = edges.size();
  Entity* signEntity = getEntity(node->signEntity);
  signEntity->shader = SignShader;
  setEntityTransparent(node->signEntity, (node->flags & _graphComplete));
  placeEntity(node->signEntity, node->center, 0, 0);
  setCull(node->signEntity, node->intersectionSize, 400);

  Mesh* mesh = getMeshForEntity(node->signEntity);

  if (numEdges <= 2) {
    bufferMesh(signEntity->mesh);
    return;
  }

  signEntity->texture = iconTexture;
  vec3 center = node->center;

  for(int i = 0; i < numEdges; i++) {
    Edge* edge = getEdge(edges[i]);
    bool oneWay = edge->config.flags & _configOneWay;
    if (oneWay && edge->ends[1] != ndx) {
      continue;
    }
    vec3 edgeLoc = edge->ends[0] == ndx ? edge->line.start : edge->line.end;
    vec3 dir = normalize(edgeLoc - center) *
      node->intersectionSize;
    float width = edgeWidth(edges[i])/2.f - c(CSholderWidth)/2.f;
    vec3 norm = normalize(vec3(-dir.y, dir.x, 0)) * width;

    makeSign(mesh, edgeLoc-norm-center, dir, iconStopSign, iconStopSignBack);
  }

  bufferMesh(signEntity->mesh);
}

void renderTrafficLights(item ndx, vector<vec3> ring) {
  Node* node = getNode(ndx);
  vector<item> edges = getRenderableEdges(ndx);
  int numEdges = edges.size();

  Entity* signEntity = getEntity(node->signEntity);
  signEntity->shader = SignShader;
  signEntity->texture = stopLight1;
  setEntityIlluminated(node->signEntity, true);
  placeEntity(node->signEntity, node->center, 0, 0);
  setCull(node->signEntity, node->intersectionSize, 2000);
  Mesh* mesh = getMeshForEntity(node->signEntity);

  if (getNodeInputs(ndx, node->flags & _graphComplete).size() <= 2) {
    bufferMesh(signEntity->mesh);
    return;
  }

  int oddManOut = getOddManOut(ndx, edges);
  int *stopLightTextureIndex = (int*) alloca(numEdges*sizeof(int));
  int m = 0;

  for(int k = 0; k < (numEdges+1)/2; k ++) {
    int i = (k + oddManOut) % numEdges;
    stopLightTextureIndex[i] = m*2;

    if (oddManOut !=i) {
      int pairNum = (i + numEdges/2) % numEdges;
      stopLightTextureIndex[pairNum] = m*2;
    }

    m++;
  }

  //setStopLightPhaseTexture(ndx);
  vec3 center = node->center;

  for(int i = 0; i < numEdges; i++) {
    Edge* edge = getEdge(edges[i]);
    bool oneWay = edge->config.flags & _configOneWay;
    bool isRail = edge->config.type == ConfigTypeHeavyRail;
    if (oneWay && edge->ends[1] != ndx) continue;

    vec3 edgeLoc = edge->ends[0] == ndx ? edge->line.start : edge->line.end;
    vec3 dir = normalize(edgeLoc - center) * node->intersectionSize;
    vec3 norm = normalize(vec3(-dir.y, dir.x, 0)) * trafficHandedness() *
      edgeWidth(edges[i]) / 2.f;
    vec3 cornerLoc = edgeLoc - norm;
    vec3 xppb = vec3(0.02f, 0.25f, 0);
    vec3 xppt = vec3(0.02f, 0.25f, 1);
    vec3 xpbox = vec3(0.02f, 0.75f, 1);

    if (isRail) {
      makeCylinder(mesh, cornerLoc-vec3(0,0,2)-center,
          2, stopLightPoleWidth, 12, xppb, xppb);
      makeAngledCube(mesh, cornerLoc - center,
        normalize(dir)*trainSignalBoxSize, trainSignalBoxHeight, xpbox,
        vec3((8+32*stopLightTextureIndex[i])/200.0f, 0, 1),
        vec3((8+32*(stopLightTextureIndex[i]+1))/200.0f, 1, 1));

    } else {
      Line visionLine = line(cornerLoc - dir*1.5f, cornerLoc - dir*3.f);

      vec3 best = ring[0];
      float bestDist = FLT_MAX;
      for (int j = 0; j < ring.size(); j++) {
        float dist = pointLineDistance(ring[j], visionLine);
        if (dist < bestDist) {
          bestDist = dist;
          best = ring[j];
        }
      }

      // make stoplight pole
      float numLanes = edge->config.numLanes;
      vec3 poleNorm = normalize(norm) * signPoleOffset;
      vec3 poleStart = best - center;
      vec3 poleTop = poleStart + vec3(0, 0, stopLightHeight);
      bool median = edge->config.flags & _configMedian;
      vec3 beamNorm = norm*(numLanes-0.5f-(median?1:0))/numLanes
        * (oneWay ? 2.f : 1.f) + poleNorm;

      makeCylinder(mesh, poleStart, stopLightHeight+0.5f, stopLightPoleWidth,
          12, xppb, xppt);
      makeCylinder(mesh, poleTop, beamNorm, stopLightPoleWidth, 6, xppt);

      // make stoplights
      vec3 boxStart = poleTop + poleNorm + normalize(dir)*stopLightPoleWidth -
        vec3(0, 0, stopLightBoxHeight/2);
      vec3 boxStep = norm / (numLanes + (median ? 1 : 0)) * (oneWay?2.f:1.f);
      for(float j=0; j < numLanes; j++) {
        makeAngledCube(mesh, boxStart + boxStep*j,
          normalize(dir)*stopLightBoxSize, stopLightBoxHeight, xpbox,
          vec3((8+32*stopLightTextureIndex[i])/200.0f, 0, 1),
          vec3((8+32*(stopLightTextureIndex[i]+1))/200.0f, 1, 1));
      }
    }
  }
  //setEntityTransparent(node->signEntity, (node->flags & _graphComplete));
  bufferMesh(signEntity->mesh);
}

void makeRailroad(Mesh* mesh, LaneBlock* block, item laneNdx, vec3 offset,
    bool makeTies) {
  Lane* lane = &block->lanes[laneNdx];
  const int numCurveSegments = lane->length / 2;
  vec3 norm0 = uzNormal(lane->spline.normal[0]);
  vec3 norm1 = -uzNormal(lane->spline.normal[1]);
  vec3* locs = (vec3*) alloca(sizeof(vec3)*(numCurveSegments+1)*2);
  vec3 tie_xs = sidewalk_xs;
  tie_xs.z = 1;

  for (int j = 0; j <= numCurveSegments; j++) {
    float theta = float(j)/numCurveSegments;
    vec3 loc = interpolateSpline(lane->spline, theta) - offset;
    vec3 n = normalize(lerp(norm0, norm1, theta));
    vec3 n0 = n * c(CLaneWidth) * .45f;
    vec3 n1 = uzNormal(n)*.25f;
    vec3 nr = n * c(CRailGauge) * .5f;
    locs[j*2+0] = loc-nr;
    locs[j*2+1] = loc+nr;

    if (makeTies) {
      makeAngledCube(mesh, loc-n0-n1-railRise, n0*2.f, n1*2.f,
          railRise*.95f, true, tie_xs);
    }
  }

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < numCurveSegments; j++) {
      vec3 loc = locs[j*2+i];
      vec3 along = locs[(j+1)*2+i] - loc;
      vec3 uAlong = normalize(along) * .05f;
      loc -= uAlong;
      along += uAlong*2.f;
      vec3 right = uzNormal(along)*.15f;
      loc -= right*.5f;
      makeAngledCube(mesh, loc-railRise*.5f, along,
          right, railRise, true, rail_x);
    }
  }
}

void makeGuardrails(Mesh* mesh, vec3 btl, vec3 along, vec3 right) {
  vec3 bridgeUp = up*2.f;
  vec3 bbl = btl + along;
  vec3 uright = normalize(right);
  vec3 ualong = normalize(along);
  float width = length(right);
  float amount = length(along);
  float i=0;

  makeAngledCube(mesh, btl+up*.5f+uright*.5f,
    uright*.25f, along, up*.5f, true, guard_xs);

  for (; i < amount; i+=10) {
    makeAngledCube(mesh, btl+ualong*i+uright*.25f,
      uright*.25f, ualong*.25f, up, true, guard_xs);
  }
  i = amount-.25f;
  makeAngledCube(mesh, btl+ualong*i+uright*.25f,
    uright*.25f, ualong*.25f, up, true, guard_xs);
}

vector<vec3> renderNodeCore(item ndx, Mesh* mesh, Mesh* tunnelMesh,
    bool simple) {
  Node* node = getNode(ndx);
  vector<item> edges = getRenderableEdges(ndx);
  vec3 center = node->center;
  vector<vec3> ring;
  vector<vec3> outerRing;
  vector<vec3> norms;
  vector<bool> isEnd;
  int numEdges = edges.size();
  float *thetas = (float*)alloca(sizeof(float)*numEdges);
  bool isUnderground = isNodeUnderground(ndx);
  float landZ = pointOnLand(center).z;
  bool makeTunnel = isUnderground && landZ - center.z < 4;
  bool complete = (node->flags & _graphComplete);
  bool showBody = complete && !(node->flags & _graphCity);
  bool drawDirt = !isUnderground && showBody &&
    abs(center.z-landZ) < c(CRoadRise)*4;
  bool drawGirder = !isUnderground && !drawDirt && showBody;
  bool isExpwy = node->config.type == ConfigTypeExpressway;
  bool isRail = node->config.type == ConfigTypeHeavyRail;
  bool isPed = node->config.type == ConfigTypePedestrian;
  bool isRoadway = !isRail && !isPed;
  bool isRoad = node->config.type == ConfigTypeRoad;
  bool isDense = shouldDrawSidewalk(center);
  bool drawSidewalk = isRoad &&
    (drawGirder || isUnderground || isDense);
  vec3 tup = vec3(0,0,7.1);
  vec3 tupi = vec3(0,0,6.1);
  vec3 tdown = vec3(0,0,1.5);
  int gameMode = getGameMode();
  bool railDense = isRail && (isUnderground || drawGirder ||
      shouldDrawRailDense(center));
  vec3 node_tx = isRail && !railDense ? sholder_xs :
    isPed ? sidewalk_xs : node_x;
  node_tx.z = 1;
  vec3 sholdx = railDense ? node_x :
    isUnderground && isPed ? sidewalk_xs :
    drawGirder ? sidewalk_xs : sholder_xs;
  vec3 offset = isRail ? -railRise : vec3(0,0,0);

  // Check for pedestrian paths
  if (!isPed) {
    for(int i = 0; i < numEdges; i++) {
      Edge* edge = getEdge(edges[i]);
      if (edge->config.type == ConfigTypePedestrian) {
        drawSidewalk = true;
        break;
      }
    }
  }

  // Collect points for each edge
  for(int i = 0; i < numEdges; i++) {
    Edge* edge = getEdge(edges[i]);
    if (debugMode() && !(edge->flags & _graphExists)) {
      handleError("edge doesn't exist");
    }
    vec3 edgeLoc = edge->ends[0] == ndx ? edge->line.start : edge->line.end;
    vec3 dir = normalize(edgeLoc - center) *
      node->intersectionSize;
    thetas[i] = atan2(dir.y, dir.x);
    vec3 outerDown = drawDirt ?
      vec3(0,0, -c(CRoadRise)*3) : vec3(0,0, -c(CRoadRise)*1);

    // make culdesac
    if (numEdges == 1) {
      vec3 nalong = edgeLoc - center;
      vec3 unorm = normalize(vec3(dir.y, -dir.x, 0));
      vec3 norm = unorm * (edgeWidth(edges[i])*.5f);
      //if (isUnderground && isPed) norm += unorm*c(CSidewalkWidth);
      vec3 outerNorm =
        drawSidewalk ? norm+unorm*c(CSidewalkWidth) :
        drawGirder ? norm :
        norm*2.f;

      vec3 loc0 = center + nalong - outerNorm;
      vec3 loc1 = center + nalong + outerNorm;
      if (drawDirt) {
        loc0 = pointOnLand(loc0);
        loc1 = pointOnLand(loc1);
      }

      ring.push_back(center + nalong - norm);
      outerRing.push_back(loc0 + outerDown);
      norms.push_back(-norm);

      int rs = ring.size();
      isEnd.resize(rs);
      isEnd[rs-1] = true;

      ring.push_back(center + nalong + norm);
      outerRing.push_back(loc1 + outerDown);
      norms.push_back(norm);

      if (gameMode == ModeTest || isExpwy) {
        ring.push_back(center - nalong + norm);
        outerRing.push_back(center - nalong + outerNorm + outerDown);
        norms.push_back(norm);

        ring.push_back(center - nalong - norm);
        outerRing.push_back(center - nalong - outerNorm + outerDown);
        norms.push_back(-norm);

      } else if (gameMode == ModeGame) {
        int culdesacVerticies = simple ? 6 : 24;
        for(int i = 0; i < culdesacVerticies; i ++) {
          float theta =  - i * pi_o * 1.25 / (culdesacVerticies - 1 ) +
            pi_o * 0.625;
          vec3 rdir = rotateZ(nalong, theta);
          ring.push_back(center - rdir);
          vec3 loc = center - rdir*(drawGirder ? 1 : 1.5f);
          if (drawDirt) {
            loc = pointOnLand(loc);
          }
          outerRing.push_back(loc + outerDown);
          norms.push_back(-rdir);
        }

        if (drawSidewalk) {
          vec3 sDown = vec3(0,0,c(CSidewalkDown));
          vec3 sUp = vec3(0,0,c(CSidewalkRise));
          for (int j = 0; j < 2; j++) {
            vec3 sp = center - nalong + norm*(j==0?1.f:-1.f);
            vec3 spo = sp + outerNorm*(j==0?1.f:-1.f);
            spo.z = sp.z;
            vec3 spd = sp+sDown;
            vec3 spod = spo+sDown;
            if (drawDirt) {
              spod += (spo-sp)*c(CSidewalkBezel)/c(CSidewalkWidth);
            }
            if (j == 1) {
              vec3 swap = sp;
              sp = spo;
              spo = swap;
              swap = spd;
              spd = spod;
              spod = swap;
            }
            makeQuad(mesh, sp+sUp, spo+sUp, spd, spod,
              sidewalk_xs, sidewalk_xs);
          }
        }

        if (drawGirder && !simple) {
          const int numGuardrails = 5;
          vec3 guardrailLocs[numGuardrails+1];
          for (int j = 0; j < numGuardrails+1; j++) {
            float theta =  - j * pi_o * 1.25 / (numGuardrails - 1) +
              pi_o * 0.625;
            vec3 rdir = rotateZ(nalong, theta);
            guardrailLocs[j] = rdir;
          }
          for (int j = 0; j < numGuardrails; j++) {
            vec3 along = guardrailLocs[j+1] - guardrailLocs[j+1];
            makeGuardrails(mesh, guardrailLocs[j], along, uzNormal(along));
          }
        }
      }

    // Make a round corner
    } else {
      item otherNdx = edges[(i+1)%numEdges];
      Edge* other = getEdge(otherNdx);
      Edge* cornerEdges[2] = {edge, other};
      float widths[2] = {edgeWidth(edges[i]), edgeWidth(otherNdx)};
      vec3 cnorms[2];
      Line l[2];
      Line lo[2];

      //if (isUnderground && isPed) {
        //widths[0] += c(CSidewalkWidth);
        //widths[1] += c(CSidewalkWidth);
      //}

      for (int j = 0; j < 2; j++) {
        if (cornerEdges[j]->ends[1] == ndx) {
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

        vec3 outerNorm =
          drawSidewalk ? unorm * c(CSidewalkWidth) :
          drawGirder ? vec3(0,0,0) :
          norm;
        lo[j].start = l[j].start + outerNorm;
        lo[j].end = l[j].end + outerNorm;
        if (drawDirt) {
          lo[j].start = pointOnLand(lo[j].start);
          lo[j].end = pointOnLand(lo[j].end);
        }
        lo[j].start += outerDown;
        lo[j].end += outerDown;
      }

      Spline s = spline(l[0], l[1]);
      Spline so = spline(lo[0], lo[1]);

      int numCurveSegments = simple ? 1 : 12;
      for (int j = 0; j <= numCurveSegments; j++) {
        float theta = float(j)/numCurveSegments;
        vec3 loc = interpolateSpline(s, theta);
        ring.push_back(loc);
        outerRing.push_back(interpolateSpline(so, theta));
        norms.push_back(cnorms[0] * (1-theta) + cnorms[1] * theta);
      }

      int rs = ring.size();
      isEnd.resize(rs);
      isEnd[rs-1] = true;
      vec3 sDown = vec3(0,0,drawDirt?c(CSidewalkDown):0);
      vec3 sUp = vec3(0,0,c(CSidewalkRise));

      if (drawSidewalk && !simple) {
        for (int j = 0; j < 2; j++) {
          vec3 sp = l[j].start - center;
          vec3 spo = lo[j].start - center;
          spo.z = sp.z;
          vec3 spd = sp+sDown;
          vec3 spod = spo+sDown;
          if (drawDirt) {
            spod += (spo-sp)*c(CSidewalkBezel)/c(CSidewalkWidth);
          }
          if (j == 1) {
            vec3 swap = sp;
            sp = spo;
            spo = swap;
            swap = spd;
            spd = spod;
            spod = swap;
          }
          makeQuad(mesh, sp+sUp, spo+sUp, spd, spod,
            sidewalk_xs, sidewalk_xs);
        }
      }

      if (drawGirder && !simple) {
        const int numGuardrails = 4;
        vec3 guardrailLocs[numGuardrails+1];
        for (int j = 0; j < numGuardrails+1; j++) {
          float theta = float(j)/(numGuardrails);
          guardrailLocs[j] = interpolateSpline(s, theta) - center;
        }
        for (int j = 0; j < numGuardrails; j++) {
          vec3 along = guardrailLocs[j+1] - guardrailLocs[j];
          makeGuardrails(mesh, guardrailLocs[j+1], -along, uzNormal(along));
        }

        if (drawSidewalk) {
          for (int j = 0; j < numGuardrails+1; j++) {
            float theta = float(j)/(numGuardrails);
            guardrailLocs[j] = interpolateSpline(so, theta) - center + sUp;
          }
          for (int j = 0; j < numGuardrails; j++) {
            vec3 along = guardrailLocs[j+1] - guardrailLocs[j];
            makeGuardrails(mesh, guardrailLocs[j+1], -along, uzNormal(along));
          }
        }
      }
    }
  }

  // Render the intersection pavement
  int ringSize = ring.size();
  isEnd.resize(ringSize);
  center -= offset;
  for(int i = 0; i < ringSize; i ++) {
    int i1 = (i+1) % ringSize;
    vec3 r0 = ring[i];
    vec3 r1 = ring[i1];
    vec3 r2 = outerRing[i];
    vec3 r3 = outerRing[i1];
    vec3 upn0 = up + norms[i] * roadMound;
    vec3 upn1 = up + norms[i1] * roadMound;
    vec3 n0 = norms[i];
    vec3 n1 = norms[i1];

    if (numEdges != 2) {
      Triangle t = {{
        {r0-center, upn0, node_tx},
        {vec3(0.f,0.f,0.f)+offset, up, node_tx},
        {r1-center, upn1, node_tx},
      }};
      pushTriangle(mesh, t);
    }

    // Tunnel Structure
    if (makeTunnel && tunnelMesh != 0) {
      vec3 r0t = (!isRoad ? r0 : r2) - center;
      vec3 r2t = r0t;
      vec3 r1t = (!isRoad ? r1 : r3) - center;
      vec3 r3t = r1t;
      r2t += normalize(r2 - r0)*1.f;
      r3t += normalize(r3 - r1)*1.f;

      //if (numEdges != 2) {
        Triangle t = {{
          {r2t+tup, up, node_x},
          {offset+tup, up, node_x},
          {r3t+tup, up, node_x},
        }};
        pushTriangle(tunnelMesh, t);
      //}

      if (isEnd[i]) {
        makeQuad(tunnelMesh, r2t+tup, r0t+tupi, r2t-tdown, r0t-tdown,
            node_x, node_x);
        makeQuad(tunnelMesh, r1t+tupi, r3t+tup, r1t-tdown, r3t-tdown,
            node_x, node_x);
        makeQuad(tunnelMesh, r2t+tup, r3t+tup, r0t+tupi, r1t+tupi,
            node_x, node_x);

      } else {
        makeQuad(tunnelMesh, r2t+tup, r3t+tup, r2t-tdown, r3t-tdown,
            node_x, node_x);
        makeQuad(tunnelMesh, r1t+tupi, r0t+tupi, r1t-tdown, r0t-tdown,
            node_x, node_x);
      }
    }

    if (drawGirder) {
      vec3 gr0 = r2-center;
      vec3 gr1 = r3-center;
      if (simple) {
        makeQuad(mesh, gr0, gr1,
            gr0-up*2.5f, gr1-up*2.5f,
            n0, n1, n0, n1, bridge_xs, bridge_xs);

      } else {
        makeQuad(mesh, r0-center, r1-center, r2-center, r3-center,
            n0, n1, n0, n1, node_tx, node_tx);
        //makeQuad(mesh, gr0-up*.5f, gr1-up*.5f,
            //gr0-up*2.5f, gr1-up*2.5f,
            //n0, n1, n0, n1, bridge_xs, bridge_xs);
        makeQuad(mesh, gr0, gr1,
            gr0-up*2.5f, gr1-up*2.5f,
            n0, n1, n0, n1, bridge_xs, bridge_xs);
      }
    }

    if (drawSidewalk) {
      if (simple) {
        //vec3 rn0 = (r2 - r0)*c(CSidewalkBezel)/c(CSidewalkWidth);
        //vec3 rn1 = (r3 - r1)*c(CSidewalkBezel)/c(CSidewalkWidth);
        //r2 += rn0;
        //r3 += rn1;
        r2.z = r0.z;
        r3.z = r1.z;
        makeQuad(mesh, r0-center, r1-center, r2-center, r3-center,
            sidewalk_xs, sidewalk_xs);

      } else {
        vec3 r0d = r0;
        vec3 r1d = r1;
        vec3 r2d = r2;
        vec3 r3d = r3;
        r2.z = r0.z;
        r3.z = r1.z;
        r0.z += c(CSidewalkRise);
        r1.z += c(CSidewalkRise);
        r2.z += c(CSidewalkRise);
        r3.z += c(CSidewalkRise);
        if (!isUnderground && !drawGirder) {
          r2d += (r2d - r0d)*c(CSidewalkBezel)/c(CSidewalkWidth);
          r3d += (r3d - r1d)*c(CSidewalkBezel)/c(CSidewalkWidth);
          r0d.z = r0.z + c(CSidewalkDown);
          r1d.z = r1.z + c(CSidewalkDown);
          r2d.z = r2.z + c(CSidewalkDown);
          r3d.z = r3.z + c(CSidewalkDown);
        }

        makeQuad(mesh, r0-center, r1-center, r2-center, r3-center,
            sidewalk_xs, sidewalk_xs);

        if (!makeTunnel && !isEnd[i]) {
          makeQuad(mesh, r2-center, r3-center, r2d-center, r3d-center,
              sholdx, sholdx);

          makeQuad(mesh, r1-center, r0-center, r1d-center, r0d-center,
              sidewalk_xs, sidewalk_xs);
        }
      }

    } else if (drawDirt) {
      vec3 n0a = cross(cross(n0, up), r2-r0);
      vec3 n1a = cross(cross(n1, up), r3-r1);
      makeQuad(mesh, r0-center, r1-center, r2-center, r3-center,
          up, up, n0a, n1a, sholdx, sholdx);
    }
  }

  if (numEdges == 2) {
    for(int i = 0; i < ringSize/2-1; i ++) {
      int i1 = (i+1) % ringSize;
      vec3 r0 = ring[i1];
      vec3 r1 = ring[i];
      vec3 r2 = ring[ringSize-i1-1];
      vec3 r3 = ring[ringSize-i-1];
      vec3 upn0 = up + norms[i1] * roadMound;
      vec3 upn1 = up + norms[i] * roadMound;
      vec3 upn2 = up + norms[ringSize-i1-1] * roadMound;
      vec3 upn3 = up + norms[ringSize-i-1] * roadMound;
      makeQuad(mesh, r0-center, r1-center, r2-center, r3-center,
          upn0, upn1, upn2, upn3, node_tx, node_tx);
    }
  }

  return ring;
}

bool makeSupportPillar(item ndx, Mesh* mesh, Mesh* simpleMesh,
    vec3 center, vec3 offset) {

  vec3 realLoc = center+offset;
  vec3 onLand = pointOnLand(realLoc);
  onLand.z -= 4;
  float z = realLoc.z - onLand.z - 1;
  if (onLand.z < -4) return false;
  if (z < 0) return true;

  vector<item> collisions = getGraphCollisions(box(vec2(realLoc), 125));
  for (int i = 0; i < collisions.size(); i++) {
    item cNdx = collisions[i];
    if (cNdx == ndx) continue;
    vec3 intersection = nearestPointOnLine(realLoc, getLine(cNdx));
    if (intersection.z < realLoc.z) {
      float size = cNdx < 0 ? getNode(cNdx)->intersectionSize :
        edgeWidth(cNdx);
      if (distance2DSqrd(intersection, realLoc) < size*size) {
        return false;
      }
    }
  }

  vec3 tex = node_x;
  tex.z = 0;
  makeCylinder(mesh, onLand-offset, z, 2.f, 24, tex);
  if (simpleMesh != 0) {
    makeCylinder(mesh, onLand-offset, z, 2.f, 6, tex);
  }

  return true;
}

void renderNode(item ndx) {
  if (!c(CGraphVisible)) return;
  if (!isRenderEnabled()) { return; }

  Node* node = getNode(ndx);
  if (!(node->flags & _graphExists)) {
    return;
  }

  if (node->entity == 0) {
    node->entity = addEntity(RoadShader);
    node->signEntity = addEntity(SignShader);
  }

  vec3 center = node->center;
  bool isExpwy = node->config.type == ConfigTypeExpressway;
  bool isRail = node->config.type == ConfigTypeHeavyRail;
  bool isPed = node->config.type == ConfigTypePedestrian;
  bool isRoadway = !isRail && !isPed;
  int gameMode = getGameMode();
  vector<item> edges = getRenderableEdges(ndx);
  int numEdges = edges.size();
  if (gameMode == ModeBuildingDesigner) { return; }

  Entity* entity = getEntity(node->entity);
  item texture = isExpwy ? expresswayTexture :
    isRail ? railTexture : roadTexture;
  entity->texture = texture;
  placeEntity(node->entity, node->center, 0, 0);
  setCull(node->entity, node->intersectionSize, 10000);
  entity->simpleDistance = graphSimpleDistance * node->intersectionSize/10;

  createMeshForEntity(node->entity);
  createSimpleMeshForEntity(node->entity);
  createMeshForEntity(node->signEntity);

  Entity* tunnelEntity = 0;
  Mesh* tunnelMesh = 0;
  if (isNodeUnderground(ndx)) {
    if (node->tunnelEntity == 0) {
      node->tunnelEntity = addEntity(SignShader);
    }
    tunnelEntity = getEntity(node->tunnelEntity);
    tunnelEntity->texture = texture;
    setCull(node->tunnelEntity, node->intersectionSize, 10000);
    placeEntity(node->tunnelEntity, node->center, 0, 0);
    setEntityVisible(node->tunnelEntity, tunnelsVisible());
    createMeshForEntity(node->tunnelEntity);
    tunnelMesh = getMesh(tunnelEntity->mesh);
  }

  Mesh* mesh = getMesh(entity->mesh);
  Mesh* simpleMesh = getMesh(entity->simpleMesh);

  if (edges.size() <= 2 && !(node->flags & _graphComplete) &&
      (node->flags & _graphIsColliding)) {
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
      makeRailroad(mesh, block, j, node->center, isRail);
    }
  }

  if (abs(node->center.z-pointOnLand(node->center).z) > c(CRoadRise)*4) {
    makeSupportPillar(ndx, mesh, simpleMesh, vec3(0,0,0), node->center);
  }

  vector<vec3> ring = renderNodeCore(ndx, mesh, tunnelMesh, false);
  renderNodeCore(ndx, simpleMesh, 0, true);

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
  } else if (node->config.strategy == TrafficLightStrategy) {
    renderTrafficLights(ndx, ring);
  } else if (node->config.strategy == JointStrategy) {
    if (node->signEntity != 0) {
      setEntityVisible(node->signEntity, false);
    }
  }

  bool complete = node->flags & _graphComplete;
  bool legal = legalMessage(ndx) == NULL;
  setEntityVisible(node->entity, true);
  setEntityTransparent(node->entity, !complete);
  //setEntityHighlight(node->entity, !complete);
  //setEntityBlueHighlight(node->entity, !complete);
  setEntityRedHighlight(node->entity, !legal);
  setEntityRaise(node->entity, complete ? 0 : 4);
  setEntityTransparent(node->signEntity, !complete);
  //setEntityHighlight(node->signEntity, !complete);
  //setEntityBlueHighlight(node->signEntity, !complete);
  setEntityRedHighlight(node->signEntity, !legal);
  setEntityRaise(node->signEntity,  complete ? 0 : 4);
}

vec3 cableHeight(float i, float num) {
  float u = i/num;
  float z = 1-u;
  z = z*z;
  return vec3(0,0, z * 105);
}

void makeSuspensionCables(Mesh* mesh, vec3 tl, vec3 along, vec3 right,
    bool simple) {
  vec3 uright = normalize(right);
  vec3 ualong = normalize(along);
  float lngth = length(along);
  const float cableSpacing = 40;
  int num = lngth/cableSpacing;
  tl.z -= 1.5;

  for (float i = 1; i <= num+1; i++) {
    vec3 loc = tl + ualong*cableSpacing*i;
    vec3 prevLoc = tl + ualong*cableSpacing*(i-1);
    vec3 cableUp = cableHeight(i, num);
    vec3 prevCableUp = cableHeight(i-1, num);
    vec3 stayAlong = prevLoc + prevCableUp - loc - cableUp;
    if (i == 1) stayAlong = stayAlong*1.65f;

    //makeAngledCube(mesh, loc + vec3(0,0, -2.5) - uright*4.f + ualong*2.5f,
     //   ualong*-5.f, right + uright*8.f, vec3(0,0,2.5), true, bridge_xs);

    for (float j = 0; j < 2; j++) {
      vec3 sloc = loc + right*j + uright * (2.f*(j*2-1));
      if (cableUp.z > 2) {
        makeCylinder(mesh, sloc-vec3(0,0,1),
            cableUp, 1.5f, 6, suspension_x, false);
      }

      makeCylinder(mesh, sloc+cableUp+stayAlong, -stayAlong,
          4, 6, suspension_x, i == num+1);
    }
  }
}

void makeSuspensionCables(item ndx,
    Mesh* mesh, vec3 btl, vec3 along, vec3 right, bool simple) {

  Edge* edge = getEdge(ndx);
  vec3 uright = normalize(right);
  vec3 ualong = normalize(along);
  float width = length(right);
  float amount = length(along);

  makeIBeam(mesh, btl-up*2.5f-uright*3.f, right+uright*6.f,
      along, up*2.f, width+4, true, false, suspension_x);
  vec3 btld = btl-up*.5f;
  makeQuad(mesh, btld, btld+along, btl, btl+along,
    node_x, node_x);
  makeQuad(mesh, btld+right+along, btld+right, btl+right+along, btl+right,
    node_x, node_x);

  makeGuardrails(mesh, btl, along, right);
  makeGuardrails(mesh, btl+right+along, -along, -right);

  bool suspension[2];
  for (int i = 0; i < 2; i++) {
    Node* node = getNode(edge->ends[i]);
    suspension[i] = false;
    if (node->pillar != 0) {
      Pillar* p = getPillar(node->pillar);
      suspension[i] = p->flags & _pillarSuspension;

      if (suspension[i]) {
        vec3 buff = ualong * (node->intersectionSize*.1f);
        along += buff;
        if (i == 0) btl -= buff;
      }
    }
  }

  for (int i = 0; i < 2; i++) {
    if (!suspension[i]) continue;

    float mult = i*2-1;
    vec3 segAlong = along;
    if (suspension[!i]) segAlong = segAlong * .5f;

    makeSuspensionCables(mesh, btl+(along+right)*float(i),
        -segAlong*mult, -right*mult, simple);
  }
}

void makeBridgeSegment(Mesh* mesh, vec3 btl, vec3 along, vec3 right,
    bool forceTruss, bool simple, bool isSidewalk, bool includeIBeams) {
  vec3 uright = normalize(right);
  vec3 ualong = normalize(along);
  float width = length(right);
  float amount = length(along);

  if (simple) {
    vec3 btld = btl-up*2.5f;
    makeQuad(mesh, btld, btld+along, btl, btl+along,
      bridge_xs, bridge_xs);
    makeQuad(mesh, btld+right+along, btld+right, btl+right+along, btl+right,
      bridge_xs, bridge_xs);

  } else {
    if (includeIBeams) {
      makeIBeam(mesh, btl-up*2.5f+uright, right-uright*2.f, along, up*2.f,
        width-4, false, false, bridge_xs);
    }
    vec3 btld = btl-up*.5f;
    vec3 node_tx = isSidewalk ? sidewalk_xs : node_x;
    makeQuad(mesh, btld, btld+along, btl, btl+along,
      node_tx, node_tx);
    makeQuad(mesh, btld+right+along, btld+right, btl+right+along, btl+right,
      node_tx, node_tx);
  }

  makeGuardrails(mesh, btl, along, right);
  makeGuardrails(mesh, btl+right+along, -along, -right);

  // Make a Truss
  if (forceTruss) {// || amount > tileSize*12) {
    float triangleLength = 25.f;
    float beamWidth = 2.f;
    float beamHeight = 2.f;
    int numTriangles = amount/triangleLength;
    float beamAngle = pi_o/3.f;
    float innerWidth = 1.f;
    vec3 beamStep = ualong*triangleLength;
    vec3 beamAlong = (triangleLength+1) *
      (ualong*cos(beamAngle) + up*sin(beamAngle));
    vec3 beamBack = (triangleLength+1) *
      (-ualong*cos(beamAngle) + up*sin(beamAngle));
    vec3 beamUp = beamHeight *
      (-ualong*sin(beamAngle) + up*cos(beamAngle));
    vec3 beamUpBack = beamHeight *
      (ualong*sin(beamAngle) + up*cos(beamAngle));
    vec3 beamRight = uright*beamWidth;
    vec3 start = btl+.5f*(amount - numTriangles*triangleLength)*ualong
      - up*beamHeight;
    vec3 botBeamStart = start;
    vec3 topBeamStart = start+.5f*beamStep +
      up*sin(beamAngle)*triangleLength;
    vec3 topBeamStartAdj = topBeamStart - ualong*1.0f;
    vec3 botBeamAlong = beamStep*float(numTriangles);
    vec3 topBeamAlong = beamStep*float(numTriangles-1) + ualong*2.f;

    makeIBeam(mesh, botBeamStart+right+beamRight,
        up*beamHeight, botBeamAlong,
        -beamRight, innerWidth, true, true, bridge_xs);
    makeIBeam(mesh, botBeamStart,
        up*beamHeight, botBeamAlong,
        -beamRight, innerWidth, true, true, bridge_xs);
    makeIBeam(mesh, topBeamStartAdj+right+beamRight,
        up*beamHeight, topBeamAlong,
        -beamRight, innerWidth, true, true, bridge_xs);
    makeIBeam(mesh, topBeamStartAdj,
        up*beamHeight, topBeamAlong,
        -beamRight, innerWidth, true, true, bridge_xs);

    for (int i = 0; i < numTriangles; i++) {
      vec3 loc = start+float(i)*beamStep;
      vec3 locR0 = loc+right+beamRight;
      vec3 locL0 = loc;
      vec3 locR1 = loc+right+beamStep;
      vec3 locL1 = loc+beamStep-beamRight;
      makeIBeam(mesh, locR0, beamUp, beamAlong, -beamRight,
          innerWidth, true, true, bridge_xs);
      makeIBeam(mesh, locL0, beamUp, beamAlong, -beamRight,
          innerWidth, true, true, bridge_xs);
      makeIBeam(mesh, locR1, beamUpBack, beamBack, beamRight,
          innerWidth, true, true, bridge_xs);
      makeIBeam(mesh, locL1, beamUpBack, beamBack, beamRight,
          innerWidth, true, true, bridge_xs);

      makeIBeam(mesh, topBeamStart+float(i)*beamStep+ualong*beamWidth*.5f,
          -ualong*beamWidth, right, up*beamHeight,
          innerWidth*.5f, true, false, bridge_xs);
    }
  }
}

void makeExpresswaySign(Mesh* signMesh, vec3 end, vec3 ualong, vec3 unorm,
    float width) {

  //Signs
  vec3 poleForward = ualong * stopLightPoleWidth * .55f;
  vec3 sbloc = end - ualong*tileSize;
  vec3 sloc = sbloc + vec3(0,0,8) - poleForward +
    unorm*(width*.5f - c(CLaneWidth));
  vec3 sup = vec3(0,0,6);
  vec3 salong = unorm * c(CLaneWidth) * 3.f;
  vec3 sspacing = unorm * c(CLaneWidth) * 3.25f;

  //Poles
  vec3 palong = unorm * (width * .5f - stopLightPoleWidth);
  vec3 pup = vec3(0,0,13);
  vec3 pdown = vec3(0,0,2);
  vec3 ptup = vec3(0,0,4);
  vec3 pbup = vec3(0,0,9);

  //Lights
  vec3 lightAlong = unorm*.5f;
  vec3 lightRight = ualong*.5f;
  vec3 lightUp = vec3(0,0,.25f);
  vec3 lightOffset = - ualong - lightAlong*.5f - lightUp;

  //Light Structure
  vec3 lsAlong = unorm*.25f;
  vec3 lsRight = ualong*.25f;
  vec3 lsUp = vec3(0,0,1.5f);
  vec3 lsOffset = ualong*.5f - lsAlong*.5f - lightUp - poleForward;

  //Light carriage
  vec3 lcAlong = salong + sspacing + unorm*.25f;
  vec3 lcRight = ualong*.25f;
  vec3 lcUp = vec3(0,0,.25f);
  vec3 lcOffset = - ualong*.75f - lcUp - lightUp - sspacing*1.0f - unorm*.125f;

  //Super structure
  vec3 ssRight = ualong*.5f;
  vec3 ssUp = vec3(0,0,.5f);
  vec3 ssOffset = - sspacing*1.0f - unorm*.125f + vec3(0,0,1.25f) +
    ualong*.125f;
  vec3 ssSecond = vec3(0,0,3.5f);

  //Light Structure Underbar
  vec3 lsuRight = ualong*1.25f - poleForward;
  vec3 lsuUp = vec3(0,0,.25f);
  vec3 lsuOffset = - ualong*.5f - unorm*.125f - lightUp - lsuUp;

  //Textures
  vec3 xs = expwySignStart/spriteSheetSize;
  vec3 xe = expwySignEnd/spriteSheetSize;
  vec3 xbp = iconColorGray/spriteSheetSize;
  vec3 xls = iconLightStart/spriteSheetSize;
  vec3 xle = iconLightEnd/spriteSheetSize;

  for (int i = 0; i < 2; i++) {
    vec3 siloc = sloc - sspacing*float(i);
    //Sign
    makeQuad(signMesh, siloc+sup, siloc+sup+salong, siloc, siloc+salong,
        xs, xe);

    //Sign back
    makeQuad(signMesh, siloc+sup+salong, siloc+sup, siloc+salong, siloc,
        xbp, xbp);
    for (int j = 0; j < 4; j++) {
      //Light Structure
      vec3 lsLoc = siloc+salong*(j/3.f);
      makeAngledCube(signMesh, lsLoc + lsOffset,
          lsAlong, lsRight, lsUp, true, xbp);
      //Underbar
      makeAngledCube(signMesh, lsLoc + lsuOffset,
          lsAlong, lsuRight, lsuUp, true, xbp);
    }
    for (int j = 0; j < 2; j++) {
      vec3 lightLoc = siloc + salong*(j*0.5f+0.25f) + lightOffset;
      //Light
      makeAngledCube(signMesh, lightLoc,
        lightAlong, lightRight, lightUp, false, xbp);
      makeQuad(signMesh, lightLoc+lightUp,
          lightLoc+lightRight+lightUp,
          lightLoc+lightAlong+lightUp,
          lightLoc+lightAlong+lightRight+lightUp,
          xls, xle);
    }

    //Reverse direction of sign
    float swap = xe.x;
    xe.x = xs.x;
    xs.x = swap;
    //sup = vec3(0,0,9);
  }

  //Light carriage
  makeAngledCube(signMesh, sloc + lcOffset, lcAlong, lcRight, lcUp, true, xbp);

  //Pole
  makeCylinder(signMesh, sbloc + palong - pdown,
    pup+pdown, stopLightPoleWidth, 6, xbp);
  for (int i = 0; i < 2; i++) {
    //Superstructure
    makeAngledCube(signMesh, sloc + ssOffset + ssSecond*float(i),
        lcAlong, ssRight, ssUp, true, xbp);
  }
}

void makeTunnelEntrance(Mesh* mesh, Mesh* tunnelMesh,
    vec3 start, vec3 along, vec3 right, vec3 center, bool sides) {

  // Test to make sure it's really a tunnel
  vec3 realLoc0 = start+center;
  float landZ0 = pointOnLand(realLoc0).z;
  bool tooHigh0 = landZ0 < realLoc0.z;
  bool tooLow0 = landZ0 > realLoc0.z + 6;
  bool doTunnel = true;
  if (tooHigh0 || tooLow0) {
    vec3 realLoc1 = realLoc0 + along;
    float landZ1 = pointOnLand(realLoc1).z;
    bool tooHigh1 = landZ1 < realLoc1.z;
    bool tooLow1 = landZ1 > realLoc1.z + 6;
    if (tooHigh0 && tooHigh1) doTunnel = false;
    if (tooLow0 && tooLow1) doTunnel = false;
  }

  vec3 tup = vec3(0,0,7+c(CSidewalkRise));
  vec3 halfTup = tup*.5f;
  vec3 tdown = vec3(0,0,-2); //c(CSidewalkDown));
  vec3 wall = normalize(right)*1.f;
  vec3 halfRight = right*.5f;
  vec3 ualong = normalize(along);

  if (sides) {
    // Sides
    makeAngledCube(mesh, start + tdown - halfRight,
        along, wall, halfTup, true, node_x);
    makeAngledCube(mesh, start + tdown + halfRight - wall,
        along, wall, halfTup, true, node_x);
  }

  if (tunnelMesh != 0 && doTunnel) {
    // Sides
    makeAngledCube(tunnelMesh, start + tdown - halfRight + halfTup,
        along, wall, halfTup, false, node_x);
    makeAngledCube(tunnelMesh, start + tdown + halfRight - wall + halfTup,
        along, wall, halfTup, false, node_x);
    // Top
    makeAngledCube(tunnelMesh, start + tdown + tup - halfRight,
        along, right, vec3(0,0,1), true, node_x);

    // Black to hide land at entrance
    for (int i = 0; i < 2; i++) {
      float in = 1-i*2;
      vec3 t = start + tdown + along * float(i);
      vec3 b = t + ualong*(5.f*in);
      t += tup;
      vec3 tl = t - halfRight*in;
      vec3 tr = t + halfRight*in;
      vec3 bl = b - halfRight*in;
      vec3 br = b + halfRight*in;
      makeQuad(tunnelMesh, bl, br, tl, tr, black_x, black_x);
    }
  }
}

void makeTicketMachine(Mesh* mesh, vec3 center,
    vec3 ualong, vec3 unorm, vec3 up) {
  const vec3 bX = iconColorDarkBlue/spriteSheetSize;
  const vec3 wX = iconColorWhite/spriteSheetSize;
  makeAngledCube(mesh, center,
      -ualong*1.4f, unorm*1.f, up*2.f, true, bX);
  makeAngledCube(mesh, center-ualong*.2f+unorm*1.f+up*.2f,
      -ualong*1.0f, unorm*.1f, up*1.6f, true, wX);
}

void makeExitTurnstile(Mesh* mesh, vec3 center,
    vec3 along, vec3 right, vec3 up) {
  const vec3 bRight = right;
  const vec3 bX = iconColorDarkBlue/spriteSheetSize;
  const float tBWidth = 0.5f;
  const float tPLength = .7f;
  const float height = 2.f;
  const vec3 tAlong = bRight;
  const vec3 tAxis = up*height;
  const vec3 tX = iconColorGray/spriteSheetSize;
  const int num = 6;
  const vec3 tUp = tAxis/float(num);

  makeCylinder(mesh, center, tAxis, tBWidth, 12, tX);
  for (int i = 0; i < 3; i++) {
    float theta = 2*i*pi_o/3;
    vec3 dir = rotate(tAlong, theta, tAxis) * tPLength;
    for (float j = 0; j < num; j++) {
      makeCylinder(mesh, center + tUp*(j+.5f), dir, 0.2f, 6, tX);
    }
  }

  makeCylinder(mesh, center-right, tAxis, tBWidth, 12, tX);
  for (float x = -1; x < 2; x+=2) {
    vec3 axis = (bRight*.75f + along*x)*.75f;
    for (float j = 0; j < num; j++) {
      makeCylinder(mesh, center + tUp*j - right, axis, 0.2f, 6, tX);
    }
  }
}

void makeTurnstile(Mesh* mesh, vec3 center, vec3 along, vec3 right, vec3 up) {
  const vec3 bAlong = along*2.f;
  const vec3 bRight = right;
  const vec3 bUp = up;
  const vec3 bX = iconColorDarkBlue/spriteSheetSize;
  const float tBWidth = 0.65f;
  const float tPLength = .6f;
  const vec3 tAlong = bRight;
  const vec3 tAxis = normalize(-bRight - vec3(0,0,1.f));
  const vec3 tUp = bUp - up*tBWidth*.5f;
  const vec3 tStart = center+tUp+normalize(right)*.2f;
  const vec3 tX = iconColorGray/spriteSheetSize;
  makeAngledCube(mesh, center-bAlong*.5f, bAlong,
      bRight*.5f, bUp, true, bX);

  makeCylinder(mesh, tStart, tAxis*.25f, tBWidth, 6, tX);
  for (int i = 0; i < 3; i++) {
    float theta = 2*i*pi_o/3;
    vec3 dir = rotate(tAlong, theta, tAxis) * tPLength;
    makeCylinder(mesh, tStart, -dir, 0.1f, 6, tX);
  }
}

void makeTransitMap(Mesh* mesh, vec3 center,
    vec3 along, vec3 right, vec3 up) {
  const vec3 bAlong = along*2.25f;
  const vec3 bRight = right*.5f;
  const vec3 bUp = up*.25f;
  const vec3 bX = (iconZoneColor[6]+vec3(.5,.5,0))/spriteSheetSize;
  const vec3 sAlong = along*2.f;
  const vec3 sRight = right*.25f;
  const vec3 sUp = up*2.f;
  const vec3 sX = iconColorWhite/spriteSheetSize;
  makeAngledCube(mesh, center-bAlong*.5f-bRight*.5f, bAlong, bRight, bUp, true,
      bX);
  makeAngledCube(mesh, center-sAlong*.5f-sRight*.5f, sAlong, sRight, sUp, true,
      sX);
}

void renderEdge(item edgeIndex) {
  if (!c(CGraphVisible)) return;
  if (!isRenderEnabled()) { return; }

  Edge* edge = getEdge(edgeIndex);
  if (!(edge->flags & _graphExists)) {
    return;
  }

  if (edge->entity == 0) {
    edge->entity = addEntity(RoadShader);
    edge->wearEntity = addEntity(WearShader);
    edge->textEntity = addEntity(TextShader);
    edge->signEntity = addEntity(SignShader);
  }

  bool complete = (edge->flags & _graphComplete);
  bool showBody = complete && !(edge->flags & _graphCity);
  bool legal = legalMessage(edgeIndex) == NULL;
  bool oneWay = edge->config.flags & _configOneWay;
  bool isExpwy = edge->config.type == ConfigTypeExpressway;
  bool isRail = edge->config.type == ConfigTypeHeavyRail;
  bool isPed = edge->config.type == ConfigTypePedestrian;
  bool isRoadway = !isRail && !isPed;
  bool isRoad = edge->config.type == ConfigTypeRoad;
  bool isUnderground = (edge->config.flags & _configDontMoveEarth) &&
    isNodeUnderground(edge->ends[0]) && isNodeUnderground(edge->ends[1]);
  int numLanes = edge->config.numLanes;

  Node* node0 = getNode(edge->ends[0]);
  Node* node1 = getNode(edge->ends[1]);
  vec3 ends[] = { edge->line.start, edge->line.end };
  vec3 transverse = ends[0] - ends[1];
  float roadLength = length(transverse);
  vec3 center = ends[1] + transverse*0.5f;
  ends[0] -= center;
  ends[1] -= center;
  float width = edgeWidth(edgeIndex);
  item bridgeType = getBridgeType(edgeIndex);

  bool isDense = shouldDrawSidewalk(center);
  bool railDense = isRail && (isUnderground || shouldDrawRailDense(center));
  bool isSidewalk = bridgeType >= 0 && isRoad ||
    (isRail && (edge->config.flags & _configPlatform));
  vec3 norm(transverse.y, -transverse.x, 0);
  norm *= width / length(norm) / 2.f;
  float hand = trafficHandedness();
  vec3 hnorm = norm * hand;

  vec3 tr = ends[0] + norm;
  vec3 tl = ends[0] - norm;
  vec3 br = ends[1] + norm;
  vec3 bl = ends[1] - norm;


  Entity* entity = getEntity(edge->entity);
  item texture = isExpwy ? expresswayTexture :
    isRail ? railTexture :
    roadTexture;
  entity->texture = texture;
  setEntityIlluminated(edge->entity, true);
  entity->simpleDistance = graphSimpleDistance;

  Entity* wearEntity = getEntity(edge->wearEntity);
  wearEntity->texture = texture;

  createMeshForEntity(edge->entity);
  createSimpleMeshForEntity(edge->entity);
  createMeshForEntity(edge->wearEntity);
  createMeshForEntity(edge->textEntity);
  createMeshForEntity(edge->signEntity);
  createSimpleMeshForEntity(edge->signEntity);

  Entity* tunnelEntity = 0;
  Mesh* tunnelMesh = 0;
  if (isEdgeUnderground(edgeIndex)) {
    if (edge->tunnelEntity == 0) {
      edge->tunnelEntity = addEntity(SignShader);
    }
    tunnelEntity = getEntity(edge->tunnelEntity);
    tunnelEntity->texture = texture;
    placeEntity(edge->tunnelEntity, center, 0, 0);
    setCull(edge->tunnelEntity, roadLength*1.5f, 10000);
    createMeshForEntity(edge->tunnelEntity);
    tunnelMesh = getMesh(tunnelEntity->mesh);
  }

  Mesh* wearMesh = getMesh(wearEntity->mesh);

  placeEntity(edge->entity, center, 0, 0);
  placeEntity(edge->wearEntity, center, 0, 0);
  setCull(edge->entity, roadLength*1.5f, 10000*numLanes*numLanes);
  setCull(edge->wearEntity, roadLength*1.5f, 10000);

  vec3 xs, xe;
  vec3 sholdx = railDense ? node_x : sholder_xs;
  int n = numLanes-1;
  float txLength = length(transverse)/32;
  if (isRail) {
    float* tex = railDense ?
      &denseRailTex[0] : &undenseRailTex[0];
    xs = vec3(tex[0]/roadTextureSizeX, 0.0f, 1);
    xe = vec3(tex[numLanes*(1+!oneWay)]/roadTextureSizeX, txLength, 1);
  } else if (isPed) {
    xs = sidewalk_xs;
    xe = sidewalk_xs;
  } else if (oneWay) {
    xs = vec3(oneWayRoadTex[n  ]/roadTextureSizeX, 0, 1);
    xe = vec3((oneWayRoadTex[n+1]+1)/roadTextureSizeX, txLength, 1);

    if (hand < 0) {
      float temp = xs.x;
      xs.x = xe.x;
      xe.x = temp;
    }
  } else {
    xs = vec3(twoWayRoadTex[n  ]/roadTextureSizeX, 0, 1);
    xe = vec3((twoWayRoadTex[n+1]+1)/roadTextureSizeX, txLength, 1);
  }

  vec3 wxs = vec3(1.0f-(xe.x-xs.x), 0.0f, 1);
  vec3 wxe = vec3(1.0f, roadLength/32, 1);
  vec3 upr = up + norm*roadMound;
  vec3 upl = up - norm*roadMound;

  makeQuad(wearMesh, tr, tl, br, bl, upr, upl, upr, upl, wxs, wxe);

  vec3 right = tr-tl;
  vec3 uright = normalize(right);
  vector<BridgeSegment> bridgeSegments = getBridgeSegments(edge->line);
  vec3 unit = normalize(transverse);
  vec3 ualong = normalize(bl-tl);
  vec3 hualong = normalize(bl-tl);
  vec3 unorm = normalize(norm);
  vec3 hunorm = normalize(hnorm);
  vec3 median = edge->config.flags & _configMedian ?
    unorm*c(CLaneWidth) : vec3(0,0,0);
  vec3 sUp = vec3(0,0,c(CSidewalkRise));
  vec3 sNorm = unorm*c(CSidewalkWidth);
  item colorNdx = 22;

  if (edge->config.flags & _configPlatform) {
    vector<item> colors = suggestStationColors(edgeIndex);
    if (colors.size() > 0) {
      colorNdx = colors[0];
    }
    if (colorNdx < 0 || colorNdx >= numColors) {
      colorNdx = 0;
    }
  }

  for (int simple = 0; simple < 2; simple++) {

    vec3 cursor = tl;
    Mesh* mesh = getMesh(simple ? entity->simpleMesh : entity->mesh);

    if (isRail) {
      if (simple) {
        float* tex = railDense ?
          &denseRailTex[0] : &undenseRailTex[0];
        xs = vec3(tex[0]/roadTextureSizeX, 0.0f, 1);
        xe = vec3(tex[numLanes*(1+!oneWay)]/roadTextureSizeX, txLength, 1);

      } else {
        xs = railDense ? node_x : sholder_xs;
        xs.z = 1;
        xe = xs;
        cursor -= railRise;

        for (int i = 0; i < 2 && edge->laneBlocks[i] > 0; i ++) {
          LaneBlock* block = getLaneBlock(edge->laneBlocks[i]);
          for (int j = 0; j < block->numLanes; j ++) {
            makeRailroad(mesh, block, j, center, true);
          }
        }
      }

      makeQuad(mesh, tr-railRise, tl-railRise, br-railRise, bl-railRise,
          upr, upl, upr, upl, xs, xe);

    } else {
      makeQuad(mesh, tr, tl, br, bl, upr, upl, upr, upl, xs, xe);
    }

    if (bridgeType == 1) {
      makeSuspensionCables(edgeIndex, mesh, tl, bl-tl, right, simple);
      makeBridgeSegment(mesh, tl+sUp-sNorm, bl-tl, right+sNorm*2.f,
          false, simple, true, false);
      if (isSidewalk) {
        makeGuardrails(mesh, tl+sUp-sNorm, bl-tl, -right);
        makeGuardrails(mesh, bl+right+sUp+sNorm, tl-bl, right);
      }
    }

    for (int i=0; i < bridgeSegments.size(); i++) {
      BridgeSegment seg = bridgeSegments[i];
      vec3 btl = cursor;
      float amount = seg.length;
      vec3 bbl = cursor - unit*seg.length;
      cursor = bbl;
      vec3 btr = btl+right;
      vec3 bbr = bbl+right;
      vec3 sholderDown = vec3(0, 0, -c(CRoadRise)*3);
      vec3 es = ualong*4.f;
      vec3 esn = -es;
      if (i == 0) {
        esn = vec3(0,0,0);
      }

      if (i == bridgeSegments.size() - 1) {
        es = vec3(0,0,0);
      }

      bool isSegSidewalk = isSidewalk ||
            (isRoad && showBody && (seg.type != BSTLand ||
            shouldDrawSidewalk((bbr+btl+btr+bbl)/4.f + center)));

      if (seg.type == BSTBridge) {
        if (showBody) {
          bool anyPillar = false;
          const float pillarSpacing = 25;
          const float numPillars = ceil(amount/pillarSpacing);
          for (int i = 0; i < numPillars; i++) {
            anyPillar = makeSupportPillar(edgeIndex, mesh, 0,
                btl+right*.5f - unit*(pillarSpacing*i), center) || anyPillar;
          }

          if (bridgeType != 1 && !isSegSidewalk) {
            makeBridgeSegment(mesh, btl, bbl-btl, right,
              node0->pillar != 0  && node1->pillar != 0, simple, false, true);
          }
        }

      } else if (seg.type == BSTTunnel && showBody) {
        vec3 tright = right +
          unorm*(2.f + (isSegSidewalk ? 2.f*c(CSidewalkWidth) : 0.f));
        makeTunnelEntrance(mesh, tunnelMesh, (btl+btr)*.5f, bbl-btl,
            -tright, center, !(edge->config.flags & _configPlatform));
        //makeTunnelEntrance(mesh, tunnelMesh, ends[1], -ualong,
        //tright, center);
      }

      if (isSegSidewalk) {
        // Sidewalk

        vec3 bNorm = unorm * ((seg.type == BSTLand) ? c(CSidewalkBezel):0.f);
        vec3 sDown = vec3(0,0, seg.type == BSTLand ? c(CSidewalkDown):0.f);
        vec3 btrs = btr + sNorm;
        vec3 btls = btl - sNorm;
        vec3 bbrs = bbr + sNorm;
        vec3 bbls = bbl - sNorm;
        vec3 along = bbl-btl;

        if (seg.type == BSTBridge && bridgeType != 1) {
          makeBridgeSegment(mesh, btl+sUp-sNorm, along, right+sNorm*2.f,
              node0->pillar != 0  && node1->pillar != 0, simple, true, true);
          makeGuardrails(mesh, btl, along, right);
          makeGuardrails(mesh, btl+right+along, -along, -right);
        }

        // Sidewalk Surface
        makeQuad(mesh, btr+sUp, bbr+sUp, btrs+sUp, bbrs+sUp,
            sidewalk_xs, sidewalk_xs);
        makeQuad(mesh, bbl+sUp, btl+sUp, bbls+sUp, btls+sUp,
            sidewalk_xs, sidewalk_xs);

        if (!simple) {
          //Inner walls
          makeQuad(mesh, bbl+sDown, btl+sDown, bbl+sUp, btl+sUp,
              sidewalk_xs, sidewalk_xs);
          makeQuad(mesh, btr+sDown, bbr+sDown, btr+sUp, bbr+sUp,
              sidewalk_xs, sidewalk_xs);

          //Outer walls
          if (seg.type == BSTLand) {
            makeQuad(mesh, bbrs+sDown+bNorm, btrs+sDown+bNorm,
                bbrs+sUp, btrs+sUp, sholdx, sholdx);
            makeQuad(mesh, btls+sDown-bNorm, bbls+sDown-bNorm,
                btls+sUp, bbls+sUp, sholdx, sholdx);
          }

          //Ends
          makeQuad(mesh, bbr+sDown, bbrs+sDown+bNorm, bbr+sUp, bbrs+sUp,
              sidewalk_xs, sidewalk_xs);
          makeQuad(mesh, bbls+sDown-bNorm, bbl+sDown, bbls+sUp, bbl+sUp,
              sidewalk_xs, sidewalk_xs);
          makeQuad(mesh, btl+sDown, btls+sDown-bNorm, btl+sUp, btls+sUp,
              sidewalk_xs, sidewalk_xs);
          makeQuad(mesh, btrs+sDown+bNorm, btr+sDown, btrs+sUp, btr+sUp,
              sidewalk_xs, sidewalk_xs);
        }

        // Render train station platform awning
        if (edge->config.flags & _configPlatform) {
          vec3 awnUp0 = vec3(0,0,3);
          vec3 awnUp1 = vec3(0,0,4);
          int col = colorNdx / 4;
          int row = colorNdx % 4;
          vec3 awnColor = vec3((col*2+1)/roadTextureSizeX,
                1 + ((row-4)*2+1)/roadTextureSizeY, 0);

          if (seg.type == BSTTunnel) {
            // Station Walls
            makeAngledCube(mesh, bbrs+unorm, -unorm,
                btr-bbr, awnUp1, true, awnColor);
            makeAngledCube(mesh, btls-unorm, unorm,
                bbl-btl, awnUp1, true, awnColor);
          } else {
            //Awning
            makeAngledCube(mesh, bbrs+awnUp0, awnUp1-awnUp0-sNorm,
                btr-bbr, vec3(0,0,0.5), true, awnColor);
            makeAngledCube(mesh, btls+awnUp0, awnUp1-awnUp0+sNorm,
                bbl-btl, vec3(0,0,0.5), true, awnColor);
          }

          if (!simple && seg.type != BSTTunnel) {
            // Support Poles
            float polePad = 1;
            float poleSegLength = seg.length-polePad*2;
            int numPoles = poleSegLength/15;
            if (numPoles < 2) numPoles = 2;
            vec3 poleSpacing = unit*(poleSegLength/numPoles);
            for (int p = 0; p <= numPoles; p++) {
              for (int k = 0; k < 4; k++) {
                if (k == 1 || k == 2) continue;
                vec3 loc = k == 0 ? bbrs :
                  k == 1 ? bbr :
                  k == 2 ? bbl : bbls;
                loc += unorm * (k%2 == 0 ? -1.f : 1.f);
                loc += poleSpacing*float(p);
                loc += unit*polePad;
                float z = (k == 0 || k == 3) ? awnUp0.z : awnUp1.z;
                makeCylinder(mesh, loc, z, 0.5, 12, xGray, xGray);
              }
            }
          }
        }

      } else if (seg.type == BSTLand && showBody) { // Sholder
        vec3 btrs = pointOnLand(btr + norm + esn + center)
          + sholderDown - center;
        vec3 btls = pointOnLand(btl - norm + esn + center) +
          sholderDown - center;
        vec3 bbrs = pointOnLand(bbr + norm + es + center) +
          sholderDown - center;
        vec3 bbls = pointOnLand(bbl - norm + es + center) +
          sholderDown - center;

        //if (!isUnderground) {
          vec3 n2 = -cross(btrs-btr, bbr-btr);
          makeQuad(mesh, btr, bbr, btrs, bbrs,
              up, up, n2, n2, sholdx, sholdx);
          vec3 n3 = -cross(bbls-bbl, btl-bbl);
          makeQuad(mesh, bbl, btl, bbls, btls,
              up, up, n3, n3, sholdx, sholdx);
        //}

        if (!simple) {
          vec3 n0 = cross(bbrs-bbr, bbl-bbr);
          makeQuad(mesh, bbr, bbl, bbrs, bbls,
              up, up, n0, n0, sholdx, sholdx);
          vec3 n1 = cross(btls-btl, btr-btl);
          makeQuad(mesh, btl, btr, btls, btrs,
              up, up, n1, n1, sholdx, sholdx);
        }
      }

      // Median strip
      if (edge->config.flags & _configMedian) {
        vec3 center0 = btl+unorm*width*.5f;
        vec3 center1 = bbl+unorm*width*.5f;
        vec3 mnorm = unorm*c(CLaneWidth);
        vec3 mwalong = ualong*0.5f;
        vec3 mwnorm = unorm*0.5f;
        vec3 mtr = center0 + mnorm;
        vec3 mtl = center0 - mnorm;
        vec3 mbr = center1 + mnorm;
        vec3 mbl = center1 - mnorm;
        vec3 gup = vec3(0,0,0.5);
        vec3 mwup = vec3(0,0,0.75);
        vec3 up = vec3(0,0,1);
        vec3 btrans = btl - bbl;

        if (seg.type != BSTLand) {
          if (simple) {
            makeQuad(mesh, mtr+mwup, mtl+mwup, mbr+mwup, mbl+mwup,
                guard_xs, guard_xs);

          } else {
            makeAngledCube(mesh, mtl, mnorm*2.f, -btrans, mwup,
                true, guard_xs);
          }

        } else {
          if (!simple) {
            makeAngledCube(mesh, mtl, mwnorm, -btrans, mwup, true, guard_xs);
            makeAngledCube(mesh, mbr, -mwnorm, btrans, mwup, true, guard_xs);
            makeAngledCube(mesh, mtl, mnorm*2.f, mwalong, mwup,
                true, guard_xs);
            makeAngledCube(mesh, mbr, -mnorm*2.f, -mwalong,
                mwup, true, guard_xs);
            makeQuad(mesh, mtr+gup, mtl+gup, mbr+gup,
                mbl+gup, grass_xs, grass_xs);
          }

          /*
          const float treeSpacing = 4;
          int numTrees = seg.length / treeSpacing;
          float treeOffset = (seg.length/treeSpacing - numTrees + 1) * .5f;
          const float treeProb = 0.5;
          item randomSeed = edgeIndex * 100000;

          for (int t = 0; t < numTrees; t++) {
            if (randFloat(&randomSeed) > treeProb) continue;
            float zs = pow(randFloat(&randomSeed),2)*2+4;
            vec3 treeLoc = center0 + (t+treeOffset) * treeSpacing * ualong +
              up*zs*.5f;

            if (simple) {
              makeSimpleTree(mesh, treeLoc, zs*1.5f, zs, bridge_xs);
            } else {
              makeTree(mesh, treeLoc, zs*1.5f, zs, bridge_xs, trunk_xs);
            }
          }
          */
        }
      }
    }
  }

  bufferMesh(entity->mesh);
  bufferMesh(entity->simpleMesh);
  bufferMesh(wearEntity->mesh);
  if (tunnelEntity != 0) {
    bufferMesh(tunnelEntity->mesh);
  }

  Entity* signEntity = getEntity(edge->signEntity);
  signEntity->texture = iconTexture;
  placeEntity(edge->signEntity, center, 0, 0);
  setCull(edge->signEntity, roadLength*1.5f, 2000);
  signEntity->simpleDistance = graphSimpleDistance;

  Entity* textEntity = getEntity(edge->textEntity);
  textEntity->texture = textTexture;
  placeEntity(edge->textEntity, center, 0, 0);
  float fnumLanes = edge->config.numLanes;
  setCull(edge->textEntity, roadLength*1.5f, 1000*numLanes);

  Mesh* textMesh = getMeshForEntity(edge->textEntity);
  Mesh* signMesh = getMeshForEntity(edge->signEntity);
  Mesh* signMeshSimple = getMesh(signEntity->simpleMesh);

  // Make one-way signs
  if (oneWay) {
    if (node0->config.strategy != JointStrategy) {
      makeSign(signMesh, br, -transverse,
        iconDoNotEnterSign, iconDoNotEnterSignBack);
      makeSign(signMesh, bl, -transverse,
        iconDoNotEnterSign, iconDoNotEnterSignBack);
    }

    if (node1->config.strategy != JointStrategy) {
      makeSign(signMesh, tr, -hnorm,
        iconOneWaySignLeft, iconOneWaySignBack);
      makeSign(signMesh, tl, hnorm,
        iconOneWaySignRight, iconOneWaySignBack);
    }

    if (roadLength > 50) {
      Line iconLine = iconToSpritesheet(iconTurn[2], 0.f);
      float iconX = iconLine.end.x - iconLine.start.x;
      iconLine.start.x += iconX*.25f;
      iconLine.end.x -= iconX*.25f;
      vec3 ialong = ualong*width;
      for (int x = 0; x < 2; x ++) {
        vec3 start = br - ialong*2.f - ualong*25.f;
        vec3 across = tl - tr;
        makeQuad(x == 0 ? signMesh : signMeshSimple, start, start + across,
          start + ialong*2.f, start + across + ialong*2.f,
          iconLine.end, iconLine.start);
      }
    }
  }

  // Make speed limit signs
  if (isRoadway && length(transverse) > 111) {
    item speedLimit = edge->config.speedLimit;
    vec3 ico = useMetric() ?
      iconSpeedLimitKmph[speedLimit] : iconSpeedLimitMph[speedLimit];
    makeSign(signMesh, tr-transverse/4.f, transverse,
      ico, iconSpeedLimitSignBack);
    if (!oneWay) {
      makeSign(signMesh, bl+transverse/4.f, -transverse,
        ico, iconSpeedLimitSignBack);
    }
  }

  float padding = 0.5;
  float laneWidth = c(CLaneWidth);
  vec3 textAlong = transverse*(laneWidth-padding*2)/roadLength;
  vec3 paddingAlong = transverse*laneWidth*2.f/roadLength;
  vec3 paddingNorm = unorm*padding +
    (edge->config.flags & _configMedian ? unorm*laneWidth*0.25f : vec3(0,0,0));
  vec3 speedAlong = transverse*laneWidth/roadLength;
  vec3 speedNorm = unorm*(laneWidth-padding*2);
  vec3 laneNorm = unorm*laneWidth;
  vec3 oneWayNorm = norm - unorm*c(CSholderWidth);
  float speed = speedLimits[edge->config.speedLimit];
  int speedInt = useMetric() ? int(speed*c(CMsToKmph)) : int(speed*c(CMsToMph));
  char* speedStr = sprintf_o("%d", speedInt);
  if (length(transverse) < 20 || edge->config.speedLimit == 0) {
    free(speedStr);
    speedStr = 0;
  }

  // Make turn markings
  if (isRoadway) {
    for (int i = 0; i < 2 && edge->laneBlocks[i] > 0; i ++) {
      LaneBlock* block = getLaneBlock(edge->laneBlocks[i]);
      for (int j = 0; j < block->numLanes; j ++) {
        Lane* lane = &block->lanes[j];
        vec3 lualong = normalize(lane->ends[0]-lane->ends[1]);
        vec3 luacross = vec3(lualong.y, -lualong.x, 0);
        vec3 loc = lane->ends[1] - luacross*laneWidth*.5f + lualong*2.f -
          center;
        //loc += i ? median*luacross*.5f : -median*luacross*.5f;
        vec3 turnAcross = luacross*laneWidth;
        vec3 turnDown = lualong*laneWidth*2.f;

        int turnIconNdx = 0;
        for (int k = 0; k < lane->drains.size(); k ++) {
          turnIconNdx |= getLaneBlock(lane->drains[k])->flags;
        }
        turnIconNdx = (turnIconNdx & _laneTurnMask) >> _laneTurnShift;
        if (turnIconNdx == 0) continue;
        vec3 icon = iconTurn[turnIconNdx];
        Line iconLine = iconToSpritesheet(icon, 0.f);

        if (numLanes > 1) {
          makeQuad(signMesh, loc, loc + turnAcross,
            loc + turnDown, loc + turnAcross + turnDown,
            iconLine.start, iconLine.end);
        }

        // Speed Limit
        if (speedStr != 0) {
          loc = lane->ends[0] - luacross*laneWidth*.5f - lualong*2.f - center;
          renderString(textMesh, speedStr, loc, textAlong, paddingNorm);
          /*
          vec3 speedDown = ualong*length(speedNorm);
          if (oneWay) {
            renderString(textMesh, speedStr, ends[0] - oneWayNorm - median -
              speedAlong + laneNorm * float(j) + paddingNorm, speedNorm,
              -speedDown);
          } else {
            renderString(textMesh, speedStr, ends[0] + median*.75f -
              speedAlong + laneNorm * float(j) + paddingNorm, speedNorm,
              -speedDown);
            renderString(textMesh, speedStr, ends[1] - median*.75f +
              speedAlong - laneNorm * float(j) - paddingNorm, -speedNorm,
              speedDown);
          }
          free(speedStr);
          */
        }
      }
    }
  }

  if (speedStr != 0) {
    free(speedStr);
    speedStr = 0;
  }

  //Stop Line
  for (int i = 0; i < (oneWay ? 1 : 2) && isRoadway; i ++) {
    item nodeNdx = edge->ends[!i];
    Node* node = getNode(nodeNdx);
    vector<item> edges = getRenderableEdges(nodeNdx);
    if (node->config.type == ConfigTypeExpressway || edges.size() <= 2) {
      continue;
    }
    float multiplier = i*2-1;
    multiplier *= hand;
    vec3 sloc = i ? ends[0] : ends[1];
    sloc -= median*multiplier;
    float swidth = numLanes*laneWidth;
    if (oneWay) {
      sloc -= swidth*.5f*unorm;
    }
    vec3 sacross = -unorm * swidth * multiplier;
    vec3 sdown = ualong * multiplier;
    makeQuad(signMesh, sloc, sloc+sacross, sloc+sdown, sloc+sacross+sdown,
      iconColorWhite/spriteSheetSize, iconColorWhite/spriteSheetSize);
    makeQuad(signMeshSimple,
      sloc, sloc+sacross, sloc+sdown, sloc+sacross+sdown,
      iconColorWhite/spriteSheetSize, iconColorWhite/spriteSheetSize);
  }

  // Expressway Signs
  for (int i = 0; i < 2 && isRoadway; i++) {
    if (oneWay && i == 0) continue;

    Node* node = getNode(edge->ends[i]);
    vector<item> edges = getRenderableEdges(edge->ends[i]);
    if (edges.size() <= 2) {
      continue;
    }
    bool hasExpwy = isExpwy;
    if (!hasExpwy) {
      for (int i = 0; i < edges.size(); i++) {
        if (getEdge(edges[i])->config.type == ConfigTypeExpressway) {
          hasExpwy = true;
          break;
        }
      }
    }

    if (hasExpwy) {
      float multiplier = i*2 - 1;
      vec3 ualongFlat = hualong;
      ualongFlat.z = 0;
      ualongFlat = normalize(ualongFlat);
      makeExpresswaySign(signMesh, ends[i], ualongFlat*multiplier,
          hunorm*multiplier, width);
      makeExpresswaySign(signMeshSimple, ends[i], ualongFlat*multiplier,
          hunorm*multiplier, width);
      setEntityIlluminated(edge->signEntity, true);
      setCull(edge->signEntity, roadLength*1.5f, 2500);
    }
  }

  // Platform
  const float signSpacing = 25;
  vec3 signAlong = ualong * signSpacing;
  vec3 aUp = normalize(cross(ualong, unorm));
  if (isRail && isSidewalk) { // station platform
    vec3 stl = tl - unorm*c(CSidewalkWidth);
    vec3 sbr = br + unorm*c(CSidewalkWidth);
    for (int i = 0; i < roadLength/signSpacing-1; i++) {
      vec3 loc0 = stl+signAlong*(i+.5f);
      vec3 loc1 = sbr-signAlong*(i+.5f);
      if (i % 2 == 0) {
        makeTicketMachine(signMesh, loc0, ualong, unorm, aUp);
        makeTicketMachine(signMesh, loc1, -ualong, -unorm, aUp);
      } else {
        makeTransitMap(signMesh, stl + signAlong*(i+.5f),
            ualong, -unorm, aUp);
        makeTransitMap(signMesh, sbr - signAlong*(i+.5f),
            -ualong, unorm, aUp);
      }
    }

    makeTransitMap(signMesh, stl-transverse - ualong, ualong, -unorm, aUp);
    makeTransitMap(signMesh, sbr+transverse + ualong, -ualong, unorm, aUp);
  }

  // Turnstiles
  if (isPed && (edge->config.flags & _configToll)) {
    vec3 turnstileSpace = -unorm*c(CLaneWidth)*.5f;
    vec3 turnstileLoc = (br+tr)*.5f + turnstileSpace*.5f;
    for (int i = 0; i < edge->config.numLanes; i++) {
      makeTurnstile(signMesh, turnstileLoc+turnstileSpace*float(i),
          -ualong, -turnstileSpace, aUp);
    }

    const vec3 bX = iconColorDarkBlue/spriteSheetSize;
    //const vec3 gX = iconColorDarkGray/spriteSheetSize;
    const vec3 gX = (iconZoneColor[6]+vec3(.5,.5,0))/spriteSheetSize;
    vec3 remains = hnorm*2.f +
      turnstileSpace*(edge->config.numLanes+.0f);
    //remains *= -1;
    //vec3 remains = hnorm + turnstileSpace*(edge->config.numLanes+0.f);
    vec3 boothUp = aUp*3.f;
    vec3 boothWUp = aUp*1.f;
    vec3 boothRight = remains*.25f;
    vec3 boothAlong = -ualong*2.f;
    vec3 boothWX = -ualong*.2f;
    vec3 boothWY = unorm*.2f;
    vec3 boothCenter = (bl+tl)*.5f + remains*.75f - boothWX*.5f-boothWY*.5f;
    vec3 exitCenter = (bl+tl)*.5f + remains*.3f;
    vec3 exitAlong = length(remains)*.25f*ualong;
    vec3 sunshStart = (bl+tl)*.5f - boothAlong - boothWX - boothWY;

    // Sunsheild
    if (!isUnderground) {
      makeAngledCube(signMesh, sunshStart + boothUp,
          boothAlong*2.f+boothWX*2.f, norm*2.f+boothWY*2.f,
          aUp*.25f, true, bX);
    }

    makeExitTurnstile(signMesh, exitCenter, exitAlong, remains*.25f, aUp);

    for (float x = -1; x < 2; x+=2) {
      // Station Booth
      for (float y = -1; y < 2; y+=2) {
        makeAngledCube(signMesh, boothCenter+x*boothAlong+y*boothRight,
            boothWX, boothWY, boothUp, isUnderground, gX);
      }
      makeAngledCube(signMesh, boothCenter+x*boothAlong-boothRight,
          boothWX, boothRight*2.f, boothWUp, true, gX);
      makeAngledCube(signMesh, boothCenter+x*boothRight-boothAlong,
          boothAlong*2.f, boothWY, boothWUp, true, gX);

      // Sunsheild Support Wall
      makeAngledCube(signMesh,
          sunshStart + (x+1)*norm + boothWX + boothWY*.5f,
          boothAlong*2.f, boothWY, boothUp, isUnderground, gX);

      // Ticket Machines
      vec3 ticketLoc = boothCenter - (x+4)*1.f*ualong;
      makeTicketMachine(signMesh, ticketLoc, ualong, unorm, aUp);

      // Maps
      makeTransitMap(signMesh, boothCenter + (x-.4f)*ualong*6.f,
          ualong, -unorm, aUp);
      if (roadLength > 40) {
        makeTransitMap(signMesh, boothCenter + x*transverse*.4f,
            ualong, -unorm, aUp);
      }

      // Barrier Walls
      if (!isUnderground) {
        makeAngledCube(signMesh, bl+norm*(x+1)-boothWY*.5f,
            transverse, boothWY, aUp*1.5f, true, gX);
      }
    }
  }

  bufferMesh(signEntity->mesh);
  bufferMesh(signEntity->simpleMesh);

  // Make name
  if (showBody && length(transverse) > 50 &&
      edge->name != 0 && edge->name[0] != 0 && isRoad) {
    vec3 padding = paddingNorm*fnumLanes + median*.5f;

    for (int i = 0; i < 2; i++) {
      item blockNdx = edge->laneBlocks[i];
      if (blockNdx == 0) continue;
      LaneBlock* block = getLaneBlock(blockNdx);
      if (block->numLanes == 0) continue;
      Lane* lane = &block->lanes[0];
      vec3 loc = lane->ends[i];// + padding;
      /*
      if (i == 0) {
        renderString(textMesh, edge->name, loc, textAlong * fnumLanes);
      } else {
        renderString(textMesh, edge->name, loc, textAlong * fnumLanes);
      }
      */
    }
    if (isLeftHandTraffic()) {
      padding *= -1;
    }
    if (oneWay) {
      renderString(textMesh, edge->name,
        ends[0] - oneWayNorm + padding - paddingAlong,
        -textAlong * fnumLanes);
    } else {
      renderString(textMesh, edge->name,
        ends[0] + padding - paddingAlong,
        -textAlong * fnumLanes);
      renderString(textMesh, edge->name,
        ends[1] - padding + paddingAlong,
        textAlong * fnumLanes);
    }
  }

  // Speed markings
  /*
  if (isRoadway && length(transverse) > 20 && edge->config.speedLimit > 0) {
    for (int i=0; i < numLanes; i++) {
      float speed = speedLimits[edge->config.speedLimit];
      int speedInt = useMetric() ?
        int(speed*c(CMsToKmph)) : int(speed*c(CMsToMph));
      char* speedStr = sprintf_o("%d", speedInt);
      vec3 speedDown = ualong*length(speedNorm);
      if (oneWay) {
        renderString(textMesh, speedStr, ends[0] - oneWayNorm - median -
          speedAlong + laneNorm * float(i) + paddingNorm, speedNorm,
          -speedDown);
      } else {
        renderString(textMesh, speedStr, ends[0] + median*.75f -
          speedAlong + laneNorm * float(i) + paddingNorm, speedNorm,
          -speedDown);
        renderString(textMesh, speedStr, ends[1] - median*.75f +
          speedAlong - laneNorm * float(i) - paddingNorm, -speedNorm,
          speedDown);
      }
      free(speedStr);
    }
  }
  */

  bufferMesh(textEntity->mesh);
  setWear(edgeIndex, edge->wear);

  if (isRail && isSidewalk) { // station platform
    entity->flags |= _entityTransit;
  } else {
    entity->flags &= ~_entityTransit;
  }

  setEntityVisible(edge->wearEntity, false); //complete);
  setEntityRaise(edge->wearEntity, 2);
  setEntityRedHighlight(edge->wearEntity, false);
  setEntityBlueHighlight(edge->wearEntity, false);

  setEntityTransparent(edge->entity, !complete);
  //setEntityHighlight(edge->entity, !complete);
  //setEntityBlueHighlight(edge->entity, !complete);
  setEntityRaise(edge->entity, 0); //complete ? 0 : 4);
  setEntityRedHighlight(edge->entity, !legal);
  setEntityBringToFront(edge->entity, !complete);

  setEntityTransparent(edge->signEntity, !complete);
  //setEntityHighlight(edge->signEntity, !complete);
  //setEntityBlueHighlight(edge->signEntity, !complete);
  setEntityRaise(edge->signEntity, 1); //complete ? 1 : 5);
  setEntityRedHighlight(edge->signEntity, !legal);
  signEntity->flags |= _entityTraffic;

  setEntityTransparent(edge->textEntity, !complete);
  //setEntityHighlight(edge->textEntity, !complete);
  //setEntityBlueHighlight(edge->textEntity, !complete);
  setEntityRaise(edge->textEntity, 1); //complete ? 2 : 6);
  setEntityRedHighlight(edge->textEntity, !legal);

  if (edge->tunnelEntity != 0) {
    setEntityTransparent(edge->tunnelEntity, !complete);
    setEntityVisible(edge->tunnelEntity, tunnelsVisible());
    //setEntityHighlight(edge->textEntity, !complete);
    //setEntityBlueHighlight(edge->textEntity, !complete);
    setEntityRaise(edge->tunnelEntity, 0); //complete ? 2 : 6);
    setEntityRedHighlight(edge->tunnelEntity, !legal);
  }
}

void setElementHighlight(item ndx, bool highlight) {
  if (ndx < 0) {
    Node* node = getNode(ndx);
    if (node->entity != 0) {
      setEntityHighlight(node->entity, highlight);
      setEntityHighlight(node->signEntity, highlight);
    }
    if (node->pillar != 0) {
      setPillarHighlight(node->pillar, highlight);
    }

  } else if (ndx > 0) {
    Edge* edge = getEdge(ndx);
    if (edge->entity != 0) {
      setEntityHighlight(edge->entity, highlight);
      setEntityHighlight(edge->wearEntity, highlight);
      setEntityHighlight(edge->textEntity, highlight);
      setEntityHighlight(edge->signEntity, highlight);
    }
  }
}

void renderStopDisc(Mesh* m, item ndx, vec3 offset) {
  Stop* stop = getStop(ndx);
  GraphLocation gl = stop->graphLoc;
  gl.lane = getLaneIndex(gl.lane, 0);
  Lane* lane = getLane(gl);
  vec3 loc = getLocation(gl);
  gl.dap += 0.1;
  vec3 unorm = uzNormal(getLocation(gl)-loc);
  loc += unorm * (c(CTransitLineWidth)*-1.25f) * trafficHandedness();
  loc -= offset;
  vec3 bloc = loc;
  vec3 tloc = loc;
  bloc.z += 22;
  tloc.z += 20;

  set<int> colorSet;
  vector<item> lines = stop->lines;
  for (int i = 0; i < lines.size(); i++) {
    TransitLine* line = getTransitLine(lines[i]);
    int color = line->color;
    if (color == 255) {
      TransitSystem* system = getTransitSystem(line->system);
      color = system->color[0];
    }
    colorSet.insert(color);
  }

  vector<int> colors(colorSet.begin(), colorSet.end());
  if (colors.size() == 0) colors.push_back(22);
  int colorS = colors.size();
  int segPerColor = ceil(24.f/colorS);
  int segments = segPerColor * colorS;
  const float segFactor = pi_o*2/segments;

  for (int i = 0; i < segments; i++) {
    vec3 color = getColorInPalette(colors[i/segPerColor]);
    float ang0 = i*segFactor;
    float ang1 = (i+1)*segFactor;
    float c0 = cos(ang0);
    float s0 = sin(ang0);
    float c1 = cos(ang1);
    float s1 = sin(ang1);
    vec3 tl = tloc + vec3(c0,s0,0)*(c(CTransitLineWidth)*1.5f);
    vec3 tr = tloc + vec3(c1,s1,0)*(c(CTransitLineWidth)*1.5f);
    vec3 bl = bloc + vec3(c0,s0,0)*(c(CTransitLineWidth)*1.f);
    vec3 br = bloc + vec3(c1,s1,0)*(c(CTransitLineWidth)*1.f);
    vec3 nl = normalize(.5f*(tl+bl) - tloc);
    vec3 nr = normalize(.5f*(tr+br) - tloc);
    vec3 wd = vec3(0,0,0);

    makeQuad(m, tl, tr, bl, br, nl, nr, nl, nr, color, color);
    makeTriangle(m, tl+wd, tr+wd, tloc+wd, colorWhite);
  }
}

void renderLane(Mesh* mesh, item laneNdx, float start, float end,
    float width, item laneOffset, vec3 offset, vec3 tx) {
  Lane* lane = getLane(laneNdx);

  if (end == -1) {
    end = lane->length;
  }
  float fraction = (end-start) / lane->length;
  float basis = start / lane->length;

  const int numCurveSegments = 12;
  vec3 norm0 = uzNormal(lane->spline.normal[0]);
  vec3 norm1 = -uzNormal(lane->spline.normal[1]);
  vec3* locs = (vec3*) alloca(sizeof(vec3)*(numCurveSegments+1)*2);
  for (int j = 0; j <= numCurveSegments; j++) {
    float theta = fraction*float(j)/numCurveSegments + basis;
    vec3 loc = interpolateSpline(lane->spline, theta) - offset;
    vec3 n = normalize(lerp(norm0, norm1, theta)) * width;
    if (laneOffset != 0) loc += n*float(laneOffset)*trafficHandedness();
    locs[j*2+0] = loc - n;
    locs[j*2+1] = loc + n;
  }

  for (int j = 0; j < numCurveSegments*2; j+=2) {
    makeQuad(mesh, locs[j+0], locs[j+1], locs[j+2], locs[j+3],
        tx, tx);
  }
}

void renderLane(Mesh* mesh, item laneNdx, float start, float end) {
  renderLane(mesh, laneNdx, start, end,
      c(CLaneWidth)*.45f, 0, vec3(0,0,0), colorWhite);
}

void renderLane(Mesh* mesh, item laneNdx) {
  renderLane(mesh, laneNdx, 0, -1);
}

void renderLaneBlock(Mesh* mesh, item ndx) {
  if (!isRenderEnabled()) { return; }

  LaneBlock* block = getLaneBlock(ndx);
  if (!(block->flags & _laneExists)) return;

  for(int i = 0; i < block->numLanes; i ++) {
    renderLane(mesh, ndx+i);
  }
}

/*
void setLaneBlockColor(item ndx) {
  LaneBlock* block = getLaneBlock(ndx);
  if (block->entity == 0) return;
  Entity* entity = getEntity(block->entity);
  bool open = block->flags & _laneOpen;
  bool active = block->flags & _laneActive;
  bool isNode = block->graphElements[1] < 0;
  setEntityRaise(block->entity, 6+active);
  entity->flags &= ~(255 << 24);
  int colorIndex = !open ? 1 : !isNode ? 0 : active ? 2 : 3;
  entity->flags |= (colorIndex << 24);
}
*/

#include "graph/render.cpp"

