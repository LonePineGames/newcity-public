#pragma once

#include "../main.hpp"
#include "../item.hpp"
#include "../line.hpp"

#include "camera.hpp"
#include "cull.hpp"
#include "mesh.hpp"
#include "shader.hpp"

const uint32_t _entityExists          = 1 << 0;
const uint32_t _entityVisible         = 1 << 1;
const uint32_t _entityIlluminate      = 1 << 2;
const uint32_t _entityHighlight       = 1 << 3;
const uint32_t _entityRedHighlight    = 1 << 4;
const uint32_t _entityBlueHighlight   = 1 << 5;
const uint32_t _entityTransparent     = 1 << 6;
const uint32_t _entityDesaturate      = 1 << 7;
const uint32_t _entityWind            = 1 << 8;
const uint32_t _entityBringToFront    = 1 << 9;
const uint32_t _entityNoHeatmap       = 1 << 10;
const uint32_t _entityHeatmapLimited  = 1 << 11;
const uint32_t _entityHardTransitions = 1 << 12;
const uint32_t _entityTransit         = 1 << 13;
const uint32_t _entityTraffic         = 1 << 14;
const uint32_t _entityBillboard       = 1 << 15;
const uint32_t _entityRaiseShift      = 16;
const uint32_t _entityRaiseMask       = 7 << _entityRaiseShift;

typedef struct {
  uint32_t flags;
  uint32_t dataFlags;
  Shader shader;
  item texture;
  float entitySize;
  float maxCameraDistance;
  float simpleDistance;
  vec3 location;
  vec3 rotScale;
  float matrix[16];
  item mesh;
  item simpleMesh;
} Entity;

void setRenderEnabled(bool enabled);
bool isRenderEnabled();

void entityPrintDebugInfo();
void entityPrintDebugInfo(int32_t ndx);

item addEntity(Shader shader);
Entity* getEntity(item ndx);
bool isEntityActive_g(item ndx);
void setEntityActive_g(item ndx);
bool wasEntityCulled_g(item ndx);
void setEntityCulled_g(item ndx, bool val);
void setEntityVisible(item ndx, bool visible);
void setEntityIlluminated(item ndx, bool visible);
void setEntityHighlight(item ndx, bool highlight);
void setEntityRedHighlight(item ndx, bool highlight);
void setEntityBlueHighlight(item ndx, bool highlight);
void setEntityDesaturate(item ndx, bool desaturate);
void setEntityTransparent(item ndx, bool transparent);
void setEntityBringToFront(item ndx, bool raise);
void setEntityRaise(item ndx, int amount);
void setEntityClip(item ndx, Line cl);
void removeEntity(item ndx);
void removeEntityAndMesh(item ndx);
void placeEntity(item ndx, vec3 location, float yaw, float pitch, float scale);
void placeEntity(item ndx, vec3 location, float yaw, float pitch);
void copyEntityPlacement(item fromNdx, item toNdx);
void setCull(item ndx, float entitySize, float maxCameraDistance);

item getEntityMesh(Entity* entity, Cull c);
void createMeshForEntity(item entityNdx);
void createSimpleMeshForEntity(item entityNdx);
Mesh* getMeshForEntity(item entityNdx);
void bufferMeshForEntity(item entityNdx);

void resetEntities();
item entitiesSize();
item countEntities();
void stopGuide();
void setGuide(vec3 guide, float size);
void setGuideUniform(GLuint programID);
float getMeshQuality();
void setMeshQuality(float quality);

