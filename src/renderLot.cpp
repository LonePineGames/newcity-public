#include "draw/entity.hpp"
#include "draw/texture.hpp"
#include "heatmap.hpp"
#include "icons.hpp"
#include "land.hpp"
#include "lot.hpp"
#include "parts/toolbar.hpp"
#include "renderUtils.hpp"
#include "zone.hpp"

#include "spdlog/spdlog.h"

const int numLotMeshes = 11 * (numZoneTypes + 1);
static item lotMesh[numLotMeshes];

void makeRibbon(Mesh* mesh, vec3 p1, vec3 p2, vec3 up, vec3 xp) {
  makeQuad(mesh, p1, p2, p2+up, p1+up, xp, xp);
  makeQuad(mesh, p2, p1, p1+up, p2+up, xp, xp);
}

item getLotMesh(item zone, item density) {
  if (zone < 0 || zone >= numZoneTypes) return 0;
  item meshNum = zone*11 + density;
  item meshNdx = lotMesh[meshNum];
  if (meshNdx == 0) {
    meshNdx = addMesh();
    lotMesh[meshNum] = meshNdx;
    Mesh* mesh = getMesh(meshNdx);

    vec3 normal = vec3(0,1,0);
    vec3 back = tileSize*0.5f * normal;
    vec3 along = vec3(back.y, -back.x, 0)*0.5f;
    vec3 start = back+along;
    vec3 blockSize = vec3(tileSize*.5f, tileSize*.5f, 3);
    vec3 blockCenter = back*.5f;
    Line iconL = iconToSpritesheet(iconZoneColor[zone], 0.f);
    vec3 cx = .5f * (iconL.start + iconL.end);

    // Algorithm to make stacked bricks
    for (int i = 0; i < density; i++) {
      int l = i / 2;
      bool last = i == density - 1;
      vec3 bs = blockSize;
      vec3 off = vec3(0,0,0);
      vec3 loc = blockCenter;
      if (l % 2 == 0) {
        off.x = bs.x*.3f;
        bs.x *= .4f;
      } else {
        off.y = bs.y*.3f;
        bs.y *= .4f;
      }
      if (!last || i%2==1) {
        loc += off * ((i%2==0) ? -1.f : 1.f);
      }

      makeCube(mesh, loc, bs, cx, true, false);
      if (i % 2 == 1 || last) {
        blockCenter.z += 5;
      }
    }

    start.z = blockCenter.z + 5;
    makeIcon(mesh, start, -along*2.f, -back, iconZone[zone]);

    bufferMesh(meshNdx);
  }

  return meshNdx;
}

item renderVirtualLot(vec3 loc, vec3 normal, item zone, item density,
    bool highlight) {
  item entityNdx = addEntity(LotShader);
  Entity* entity = getEntity(entityNdx);
  entity->texture = iconTexture;
  entity->mesh = getLotMesh(zone, density);
  placeEntity(entityNdx, loc, atan2(normal.y, normal.x) - pi_o*.5f, 0);
  setEntityHighlight(entityNdx, highlight);
  setEntityVisible(entityNdx, highlight || areLotsVisible());
  setCull(entityNdx, tileSize*10, c(CLotVisibleDistance));
  return entityNdx;
}

void renderLotAs(item ndx, item zone, item density, bool highlight) {
  Lot* lot = getLot(ndx);
  if (lot->entity == 0) {
    lot->entity = addEntity(LotShader);
  }
  Entity* entity = getEntity(lot->entity);
  entity->texture = iconTexture;
  entity->mesh = getLotMesh(zone, density);
  placeEntity(lot->entity, lot->loc,
      atan2(lot->normal.y, lot->normal.x) - pi_o*.5f, 0);
  setEntityHighlight(lot->entity, highlight);
  bool viz = lot->zone != GovernmentZone && (
    highlight || (areLotsVisible() && lot->zone != ParkZone) ||
    (lot->zone == ParkZone && areParkLotsVisible()));
  setEntityVisible(lot->entity, viz);
  setCull(lot->entity, tileSize, c(CLotVisibleDistance));
}

void setLotHighlight(item ndx, bool highlight) {
  Lot* lot = getLot(ndx);
  if (lot->entity != 0) {
    setEntityHighlight(lot->entity, highlight);
  }
}

void renderLot(item ndx) {
  Lot* lot = getLot(ndx);

  if (isLotVisible(ndx)) {
    item density = isLotDensityVisible() && (lot->zone >= ResidentialZone && lot->zone <= MixedUseZone) ? getLotMaxDensity(ndx) : 0;
    renderLotAs(ndx, lot->zone, density, false);

  } else if (lot->entity != 0) {
    removeEntity(lot->entity);
    lot->entity = 0;
  }
}

void resetLotRender() {
  for (int i=0; i < numLotMeshes; i++) {
    lotMesh[i] = 0;
  }
}


