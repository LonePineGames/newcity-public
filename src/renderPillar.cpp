#include "renderPillar.hpp"

#include "color.hpp"
#include "draw/entity.hpp"
#include "draw/texture.hpp"
#include "graph.hpp"
#include "land.hpp"
#include "pillar.hpp"
#include "renderUtils.hpp"
#include "util.hpp"

#include "spdlog/spdlog.h"

//Pillar render
float pillarSizeX = 2;
float pillarSizeY = 5;
float suspensionZ = 115;
float crossbarZ = 10;

void renderPillar(item ndx) {
  if (!isRenderEnabled()) { return; }

  Pillar* pillar = getPillar(ndx);
  if (!(pillar->flags & _pillarExists)) {
    return;
  }
  if (pillar->entity == 0) {
    pillar->entity = addEntity(PaletteShader);
  }

  Entity* entity = getEntity(pillar->entity);
  entity->texture = paletteTexture;
  setEntityTransparent(pillar->entity, !(pillar->flags & _pillarComplete));

  createMeshForEntity(pillar->entity);
  Mesh* mesh = getMeshForEntity(pillar->entity);

  bool suspension = pillar->flags & _pillarSuspension;
  vec3 pillarXs = suspension ? colorRed : colorGray;
  vec3 ploc = pillar->location;
  vec3 loc = vec3(0, 0, pointOnLand(ploc).z - 20 - ploc.z);
  float widest = suspension ? 20 : pillarSizeX;
  float py = suspension ? 20 : pillarSizeY;
  float px = pillarSizeX*2.f;

  if (pillar->node) {
    float longest = 0;
    Node* node = getNode(pillar->node);
    int numEdges = node->edges.size();
    vec3 dir[2];

    for (int i = 0; i < 2 && i < numEdges; i ++) {
      Edge* e = getEdge(node->edges[i]);

      Line l = getLine(node->edges[i]);
      vec3 trans = l.end-l.start;
      float len = length(trans);
      trans.z = 0;
      dir[i] = normalize(trans);
      //atan2(trans.x, trans.y);
      if (len > longest) {
        longest = len;
      }

      float width = edgeWidth(node->edges[i]);
      if (width > widest) {
        widest = width;
      }
    }

    if (suspension) {
      py = std::max(node->intersectionSize*.5f, py);
      px = widest*1.5f + pillarSizeX;

      for (int i = 0; i < 2; i++) {
        float x = i*2-1;
        x *= widest*.75f - pillarSizeX*.5f;
        makeCube(mesh,
            vec3(x,0,-c(CRoadRise)),
            vec3(pillarSizeX*2.f, py, suspensionZ+c(CRoadRise)),
            pillarXs, true, false);
      }

      for (int i = 0; i < 4; i++) {
        float z = (i+1) * suspensionZ * .25f;
        float cby = i == 3 ? py : py*.5f;
        makeCube(mesh, vec3(0,0, z - crossbarZ),
            vec3(px, cby, crossbarZ),
            pillarXs, true, false);
      }

    } else {
      px = widest;
    }

    if (dot(dir[0], dir[1]) < 0) dir[1] = -dir[1];
    vec3 combDir = dir[0] + dir[1];
    float angle = atan2(combDir.x, combDir.y);
    placeEntity(pillar->entity, pillar->location, -angle, 0);

  } else {
    placeEntity(pillar->entity, ploc, 0, 0);
  }

  setCull(pillar->entity, 400, 100000);
  makeCube(mesh, loc,
    vec3(px, py, - loc.z - c(CRoadRise)),
    pillarXs, true, false);
  makeCube(mesh, loc,
    vec3(px-2, py+4, - loc.z - c(CRoadRise)),
    pillarXs, true, false);

  bufferMesh(entity->mesh);
}

