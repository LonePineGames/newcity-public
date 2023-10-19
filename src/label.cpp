#include "label.hpp"

#include "building/building.hpp"
#include "city.hpp"
#include "configuration.hpp"
#include "draw/entity.hpp"
#include "draw/texture.hpp"
#include "economy.hpp"
#include "graph.hpp"
#include "land.hpp"
#include "name.hpp"
#include "plan.hpp"
#include "pool.hpp"
#include "string.hpp"
#include "string_proxy.hpp"
#include "util.hpp"
#include "zone.hpp"

#include "spdlog/spdlog.h"

#include <algorithm>

Pool<Label>* labels = Pool<Label>::newPool(20);
static bool labelsVisible = true;

Label* getLabel(item ndx) {
  return labels->get(ndx);
}

float getLabelFontSize(item ndx) {
  Label* label = getLabel(ndx);
  int size = (label->flags & _labelSizeMask) >> _labelSizeShift;
  return 10 * pow(2, size * .5f);
}

bool areLabelsVisible() {
  return labelsVisible;
}

void setLabelsVisible(bool viz) {
  if (viz == labelsVisible) return;
  labelsVisible = viz;
  for (int i = 1; i <= labels->size(); i++) {
    Label* label = getLabel(i);
    if (!(label->flags & _labelExists)) continue;
    if (label->entity == 0) continue;
    setEntityVisible(label->entity, viz);
  }
  setCitiesVisible(viz);
}

void renderLabel(item ndx) {
  Label* label = getLabel(ndx);
  if (!(label->flags & _labelExists)) return;

  if (label->entity == 0) {
    label->entity = addEntity(WSUITextShader);
  }

  float fontSize = getLabelFontSize(ndx);
  Entity* textEntity = getEntity(label->entity);
  textEntity->texture = textTexture;
  textEntity->flags |= _entityHardTransitions;
  textEntity->simpleDistance = fontSize * 2 - 100;
  if (textEntity->simpleDistance < 0) textEntity->simpleDistance = 0;
  textEntity->maxCameraDistance = fontSize * 200;
  //setEntityBringToFront(label->entity, true);
  setEntityTransparent(label->entity, false);
  setEntityHighlight(label->entity, true);
  vec3 loc = pointOnLand(label->location);
  loc.z += fontSize + 50;
  placeEntity(label->entity, loc, 0.f, 0.f);
  createMeshForEntity(label->entity);
  createSimpleMeshForEntity(label->entity);
  Mesh* textMesh = getMesh(textEntity->simpleMesh);

  vec3 normal = vec3(0,-1,0);
  renderStringCentered(textMesh, label->text,
    vec3(0,-fontSize*.5f, 0), -zNormal(normal)*fontSize, -normal*fontSize);

  setEntityRedHighlight(label->entity, label->flags & _labelRed);
  setEntityBlueHighlight(label->entity, label->flags & _labelBlue);

  //renderStringCentered(getMesh(textEntity->mesh), label->text,
    //vec3(0,-fontSize*.5f, 0), zNormal(normal)*fontSize, normal*fontSize);
  //bufferMesh(textEntity->mesh);

  bufferMesh(textEntity->simpleMesh);
}

item addLabel(vec3 location, uint32_t flags) {
  item ndx = labels->create();
  Label* label = getLabel(ndx);
  label->flags = _labelExists | flags;
  label->text = strdup_s("");
  label->location = location;
  label->entity = 0;

  renderLabel(ndx);
  return ndx;
}

void removeLabel(item ndx) {
  Label* label = getLabel(ndx);
  free(label->text);
  label->flags = 0;
  if (label->entity != 0) {
    removeEntity(label->entity);
  }
  label->entity = 0;
  labels->free(ndx);
}

void setLabelHighlight(item ndx, bool highlight) {
  if (ndx == 0) return;

  Label* lab = getLabel(ndx);
  if (lab->entity == 0) return;
  setEntityHighlight(lab->entity, highlight);
}

item intersectLabel(Line mouseLine) {
  float bestDist = FLT_MAX;
  item best = 0;

  for (int i = 1; i <= labels->size(); i ++) {
    Label* label = getLabel(i);
    if (!(label->flags & _labelExists)) continue;

    float fontSize = getLabelFontSize(i);
    vec3 loc = label->location;
    loc.z += fontSize + 50;
    float dist = pointLineDistance(loc, mouseLine);

    if (dist < fontSize && dist < bestDist) {
      bestDist = dist;
      best = i;
    }
  }

  return best;
}

void resetLabels() {
  for (int i = 1; i <= labels->size(); i ++) {
    Label* label = getLabel(i);
    if (label->text) free(label->text);
  }
  labels->clear();
}

void initLabelsEntities() {
  for (int i = 1; i <= labels->size(); i ++) {
    Label* label = getLabel(i);
    if (label->flags & _labelExists && label->entity == 0) {
      label->entity = addEntity(WSUITextShader);
      createMeshForEntity(label->entity);
    }
  }
}

void renderLabels() {
  for (int i = 1; i <= labels->size(); i ++) {
    renderLabel(i);
  }
}

void readLabel(FileBuffer* file, item ndx) {
  Label* label = getLabel(ndx);

  label->flags = fread_uint32_t(file);
  label->text = fread_string(file);
  label->location = fread_vec3(file);
  label->entity = 0;
}

void writeLabel(FileBuffer* file, item ndx) {
  Label* label = getLabel(ndx);

  fwrite_uint32_t(file, label->flags);
  fwrite_string(file, label->text);
  fwrite_vec3(file, label->location);
}

void readLabels(FileBuffer* file) {
  labels->read(file, file->version);

  for (int i=1; i <= labels->size(); i++) {
    readLabel(file, i);
  }
}

void writeLabels(FileBuffer* file) {
  labels->write(file);

  for (int i=1; i <= labels->size(); i++) {
    writeLabel(file, i);
  }
}

