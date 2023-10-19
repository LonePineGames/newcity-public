#include "pillar.hpp"

#include "draw/entity.hpp"
#include "graph.hpp"
#include "land.hpp"
#include "plan.hpp"
#include "pool.hpp"
#include "renderGraph.hpp"
#include "renderPillar.hpp"
#include "util.hpp"

#include <algorithm>

const money pillarCostPerMeter = 4000;

Pool<Pillar>* pillars = Pool<Pillar>::newPool(200);

item sizePillars() {
  return pillars->size();
}

item addPillar(vec3 loc, bool suspension) {
  //loc = unitize(loc);
  item ndx = pillars->create();
  Pillar* pillar = getPillar(ndx);
  pillar->entity = 0;
  float landHeight = pointOnLand(loc).z;
  pillar->location = vec3(loc.x, loc.y,
    c(CPillarHeight) + std::max(0.f, landHeight));
  pillar->flags = _pillarExists;
  if (suspension) pillar->flags |= _pillarSuspension;
  pillar->node = 0;
  renderPillar(ndx);
  return ndx;
}

void completePillar(item ndx) {
  Pillar* pillar = getPillar(ndx);
  pillar->flags |= _pillarComplete;
  renderPillar(ndx);
  if (pillar->node) {
    renderNode(pillar->node);
  }
}

void removePillar(item ndx) {
  Pillar* pillar = getPillar(ndx);
  pillar->flags = 0;
  if (pillar->plan) {
    removePlan(pillar->plan);
  }
  pillars->free(ndx);
  removeEntityAndMesh(pillar->entity);

  if (pillar->node < 0) {
    Node* node = getNode(pillar->node);
    for (int i = node->edges.size()-1; i >=0; i --) {
      removeEdge(node->edges[i], true); // This kills the node
    }
  }
}

item intersectPillar(Line mouseLine) {
  item best = 0;
  float bestDist = c(CTileSize)*3;

  for (int i = 1; i <= pillars->size(); i ++) {
    Pillar* pillar = getPillar(i);
    if (!(pillar->flags & _pillarExists)) continue;

    vec3 pillarWaterline = pillar->location;
    pillarWaterline.z = 0;
    Line pillarLine = line(pillar->location, pillarWaterline);
    float dist = lineDistance(mouseLine, pillarLine);
    if (dist < bestDist) {
      best = i;
      bestDist = dist;
    }
  }

  return best;
}

money pillarCost(item ndx) {
  Pillar* pillar = getPillar(ndx);
  money z = -std::min(pointOnLand(pillar->location).z, 0.f);
  z = pow(z, c(CPillarCostExponent)) + c(CPillarHeight);
  z *= pillar->flags & _pillarSuspension ?
    c(CPillarCostSuspension) : c(CPillarCost);
  z *= getInflation();
  return z;
}

const char* pillarLegalMessage(item ndx) {
  return NULL;
}

void setPillarHighlight(item ndx, bool highlight) {
  Pillar* pillar = getPillar(ndx);
  if (!(pillar->flags & _pillarExists)) {
    return;
    //handleError("setPillarHighlight but pillar doesn't exist");
  }

  setEntityHighlight(pillar->entity, highlight);
}

void resetPillars() {
  pillars->clear();
}

void initPillarsEntities() {
  for (int i = 1; i < pillars->size(); i++) {
    Pillar* pillar = getPillar(i);
    if ((pillar->flags & _pillarExists) && pillar->entity == 0) {
      pillar->entity = addEntity(PaletteShader);
      createMeshForEntity(pillar->entity);
    }
  }
}

void renderPillars() {
  for (int i = 1; i < pillars->size(); i++) {
    renderPillar(i);
  }
}

Pillar* getPillar(item pillar) {
  return pillars->get(pillar);
}

void writePillar(FileBuffer* file, item ndx) {
  Pillar* pillar = getPillar(ndx);

  fwrite_vec3 (file, pillar->location);
  fwrite_item(file, pillar->node);
  fwrite_item(file, pillar->plan);
  fwrite_int  (file, pillar->flags);
}

void readPillar(FileBuffer* file, int version, item ndx) {
  Pillar* pillar = getPillar(ndx);

  pillar->location = fread_vec3 (file);
  pillar->node     = fread_item(file, version);
  pillar->plan     = fread_item(file, version);
  pillar->flags    = fread_int  (file);
  pillar->entity = 0;
}

void writePillars(FileBuffer* file) {
  pillars->write(file);

  for (int i=1; i <= pillars->size(); i++) {
    writePillar(file, i);
  }
}

void readPillars(FileBuffer* file, int version) {
  pillars->read(file, version);

  for (int i=1; i <= pillars->size(); i++) {
    readPillar(file, version, i);
  }
}

