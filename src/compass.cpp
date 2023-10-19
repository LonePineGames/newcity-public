#include "compass.hpp"

#include "color.hpp"
#include "draw/camera.hpp"
#include "draw/entity.hpp"
#include "draw/texture.hpp"
#include "item.hpp"
#include "renderUtils.hpp"
#include "util.hpp"
#include "weather.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <stdio.h>

item compassEntity = 0;
item windVaneEntity = 0;

bool compassVisible = false;

void renderCompass() {
  if (compassEntity != 0) {
    removeEntityAndMesh(compassEntity);
  }
  compassEntity = addEntity(PaletteShader);
  Entity* entity = getEntity(compassEntity);
  entity->texture = paletteTexture;
  setEntityBringToFront(compassEntity, true);
  setEntityTransparent(compassEntity, true);
  setEntityHighlight(compassEntity, true);
  setEntityVisible(compassEntity, compassVisible);
  createMeshForEntity(compassEntity);
  Mesh* mesh = getMeshForEntity(compassEntity);

  makeCone(mesh, vec3(24,0,60),
    vec3(-48,0,0), 30, colorWhite, true);
  makeCylinder(mesh, vec3(0,0,60), vec3(0,0,25), 1, 24, colorWhite);
  bufferMesh(entity->mesh);

  if (windVaneEntity != 0) {
    removeEntityAndMesh(windVaneEntity);
  }
  windVaneEntity = addEntity(PaletteShader);
  Entity* wvEntity = getEntity(windVaneEntity);
  wvEntity->texture = paletteTexture;
  setEntityBringToFront(windVaneEntity, true);
  setEntityTransparent(windVaneEntity, true);
  setEntityHighlight(windVaneEntity, true);
  setEntityVisible(windVaneEntity, compassVisible);
  createMeshForEntity(windVaneEntity);
  Mesh* wvMesh = getMeshForEntity(windVaneEntity);

  vec3 wvLoc = vec3(0,0,80);
  vec3 wvUp = vec3(0,0,5);
  vec3 wvAlong = vec3(0,20,0);
  makeTriangle(wvMesh, wvLoc+wvUp, wvLoc-wvUp, wvLoc+wvAlong, colorWhite);
  makeTriangle(wvMesh, wvLoc-wvUp, wvLoc+wvUp, wvLoc+wvAlong, colorWhite);
  bufferMesh(wvEntity->mesh);
}

void updateCompass() {
  if (compassEntity == 0) {
    renderCompass();
  }

  float dist = getCameraDistance();
  vec3 loc = getCameraTarget();
  float angle = getHorizontalCameraAngle();
  float xoff = 1/cos(getVerticalCameraAngle()) - 0.15;
  loc += rotate(vec3(xoff, -getAspectRatio()*0.9, 0)*dist,
      angle, vec3(0,0,1));
  float scal = dist/500;

  placeEntity(compassEntity, loc, 0, 0, scal);

  Weather w = getWeather();
  float windAngle = atan2(w.wind.x, w.wind.y);
  windAngle += randFloat()*0.1-0.05;
  placeEntity(windVaneEntity, loc, windAngle, 0, scal);
}

void setCompassVisible(bool v) {
  compassVisible = v;
  if (compassEntity != 0) {
    setEntityVisible(compassEntity, v);
    setEntityVisible(windVaneEntity, v);
  }
}

void resetCompass() {
  compassEntity = 0;
  windVaneEntity = 0;
}
