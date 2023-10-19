#include "entity.hpp"

#include "../cpool.hpp"
#include "../util.hpp"

#include "buffer.hpp"
#include "culler.hpp"
#include "texture.hpp"

#include "spdlog/spdlog.h"
#include <boost/dynamic_bitset.hpp>

CPool<Entity>* entities = CPool<Entity>::newPool(200000);
boost::dynamic_bitset<> entityActive_g;
boost::dynamic_bitset<> entityCulled_g;

bool renderEnabled = true;
vec3 nextGuide = vec3(0,0,1000000000000.);
float meshQuality = 1;

void entityPrintDebugInfo() {
  entityPrintDebugInfo(-1);
}

void entityPrintDebugInfo(int32_t ndx) {
  SPDLOG_INFO("Entity Debug: entities.size() - {}, entities.count() - {}", entities->size(), entities->count());

  if (ndx >= 0) {
    std::string info = "Entity at ndx " + std::to_string(ndx) + ": ";
    bool valid = true;

    if (ndx < entities->size()) {
      info += "Within Size";
    } else {
      info += "Out-of-bounds Size";
      valid = false;
    }

    if (ndx < entities->count()) {
      info += ", Within Count";
    } else {
      info += ", Out-of-bounds Count";
      valid = false;
    }

    if (valid) {
      Entity* e = entities->get(ndx);

      if (e == 0) {
        info += ", Entity Ptr Null";
        valid = false;
      } else {
        info += ", Entity Ptr Valid";
      }

      if (valid) {
        info += ", Matrix Dump: ";
        for (int i = 0; i < 16; i++) {
          if (i % 4 == 0) {
            info += "\n";
          }
          info += std::to_string(e->matrix[i]);
          if (i < 15) {
            info += ", ";
          }
        }
      }
    }

    SPDLOG_INFO(info);
  }
}

void resetEntities() {
  for (int i = 1; i <= entities->size(); i++) {
    if (getEntity(i)->flags & _entityExists) {
      removeEntity(i);
    }
  }
  entities->clear();
  entityActive_g.clear();
  nextGuide = vec3(0,0,1000000000000.);
  if (meshQuality <= 0.01) {
    meshQuality = 1;
  }

  //waitForCullerState_g(CullerWaiting);
  resetCuller_c();
  //setCullerState_g(CullerReset);
  //waitForCullerState_g(CullerWaiting);
}

void setRenderEnabled(bool enabled) {
  renderEnabled = enabled;
}

bool isRenderEnabled() {
  return renderEnabled;
}

void stopGuide() {
  nextGuide = vec3(0,0,1000000000000.);
}

void setGuide(vec3 guide, float size) {
  guide.z = size * size;
  nextGuide = guide;
}

void setGuideUniform(GLuint programID) {
  glUniform3f(glGetUniformLocation(programID, "guide"),
      nextGuide.x, nextGuide.y, nextGuide.z);
}

item addEntity(Shader shader) {
  item ndx = entities->create();
  Entity* entity = getEntity(ndx);
  entity->flags = _entityExists | _entityVisible;
  entity->dataFlags = 0;
  entity->texture = 0;
  entity->mesh = 0;
  entity->simpleMesh = 0;
  entity->shader = shader;
  memcpy_s(entity->matrix, &mat4(1.0f)[0][0], 16*sizeof(float));
  entity->entitySize = 100000;
  entity->maxCameraDistance = 100000;
  entity->simpleDistance = 0;
  entity->location = vec3(0,0,0);
  entity->rotScale = vec3(0,0,1);
  setEntityActive_g(ndx);

  return ndx;
}

void removeEntityAndMesh(item ndx) {
  if (ndx <= 0 || ndx > entitiesSize()) return;
  Entity* entity = getEntity(ndx);
  if (entity->mesh > 0) {
    removeMesh(entity->mesh);
  }
  if (entity->simpleMesh > 0) {
    removeMesh(entity->simpleMesh);
  }
  removeEntity(ndx);
}

void removeEntity(item ndx) {
  if (ndx <= 0 || ndx > entities->size()) return;
  Entity* entity = getEntity(ndx);
  if (entity == 0) return;
  if (!(entity->flags & _entityExists)) {
    SPDLOG_ERROR("Double removeEntity");
    logStacktrace();
    return;
  }
  entity->flags = 0;
  entity->dataFlags = 0;
  entity->mesh = 0;
  entity->simpleMesh = 0;
  setEntityActive_g(ndx);
  entities->free(ndx);
  markEntityDirty_g(ndx);
}

Entity* getEntity(item ndx) {
  return entities->get(ndx);
}

Mesh* getMeshForEntity(item ndx) {
  return getMesh(getEntity(ndx)->mesh);
}

void createMeshForEntity(item entityNdx) {
  Entity* entity = getEntity(entityNdx);
  int oldMesh = entity->mesh;
  if (entity->mesh <= 0) {
    entity->mesh = addMesh();
  }
  if (oldMesh < 0 || entity->mesh < 0) {
    handleError("Bad Mesh index");
  }
  setEntityActive_g(entityNdx);
}

void createSimpleMeshForEntity(item entityNdx) {
  Entity* entity = getEntity(entityNdx);
  int oldMesh = entity->simpleMesh;
  if (entity->simpleMesh <= 0) {
    entity->simpleMesh = addMesh();
  }
  if (oldMesh < 0 || entity->simpleMesh < 0) {
    handleError("Bad Mesh index");
  }
  setEntityActive_g(entityNdx);
}

void bufferMeshForEntity(item entityNdx) {
  Entity* e = getEntity(entityNdx);
  if (e->mesh != 0) {
    bufferMesh(e->mesh);
  }
  if (e->simpleMesh != 0) {
    bufferMesh(e->simpleMesh);
  }

  markEntityDirty_g(entityNdx);
}

bool isEntityActive_g(item ndx) {
  if (ndx >= entityActive_g.size()) return false;
  if (ndx < 0) return false;
  #ifdef WIN32
  __try {
  #endif
    return entityActive_g[ndx];
  #ifdef WIN32
  }
  __except (EXCEPTION_EXECUTE_HANDLER) {
    SPDLOG_ERROR("Exception in isEntityActive_g at ndx {}", ndx);
    return false;
  }
  #endif
}

void setEntityActive_g(item ndx) {
  if (entityActive_g.size() <= ndx) {
    entityActive_g.resize(ndx*2+1);
  }

  Entity* e = getEntity(ndx);
  bool active = (e->flags & _entityExists) &&
    (e->flags & _entityVisible) &&
    (e->mesh > 0 || e->simpleMesh > 0);
  entityActive_g[ndx] = active;
  markEntityDirty_g(ndx);
}

bool wasEntityCulled_g(item ndx) {
  if (entityCulled_g.size() <= ndx) return true;
  return entityCulled_g[ndx];
}

void setEntityCulled_g(item ndx, bool val) {
  if (entityCulled_g.size() <= ndx) entityCulled_g.resize(ndx*2+1);
  entityCulled_g[ndx] = val;
}

void setEntityVisible(item ndx, bool visible) {
  if (ndx <= 0) return;
  if (visible) {
    getEntity(ndx)->flags |= _entityVisible;
  } else {
    getEntity(ndx)->flags &= ~_entityVisible;
  }
  setEntityActive_g(ndx);
}

void setEntityIlluminated(item ndx, bool illuminate) {
  if (ndx <= 0) return;
  if (illuminate) {
    getEntity(ndx)->flags |= _entityIlluminate;
  } else {
    getEntity(ndx)->flags &= ~_entityIlluminate;
  }
  markEntityDirty_g(ndx);
}

void setEntityHighlight(item ndx, bool highlight) {
  if (ndx <= 0 || ndx > entitiesSize()) return;
  Entity* e = getEntity(ndx);
  bool was = e->flags & _entityHighlight;
  if (was == highlight) return;

  if (highlight) {
    e->flags |= _entityHighlight;
  } else {
    e->flags &= ~_entityHighlight;
  }
  markEntityDirty_g(ndx);
}

void setEntityRedHighlight(item ndx, bool highlight) {
  if (ndx <= 0) return;
  if (highlight) {
    getEntity(ndx)->flags |= _entityRedHighlight;
  } else {
    getEntity(ndx)->flags &= ~_entityRedHighlight;
  }
  markEntityDirty_g(ndx);
}

void setEntityBlueHighlight(item ndx, bool highlight) {
  if (ndx <= 0) return;
  if (highlight) {
    getEntity(ndx)->flags |= _entityBlueHighlight;
  } else {
    getEntity(ndx)->flags &= ~_entityBlueHighlight;
  }
  markEntityDirty_g(ndx);
}

void setEntityTransparent(item ndx, bool transparent) {
  if (ndx <= 0) return;
  if (transparent) {
    getEntity(ndx)->flags |= _entityTransparent;
  } else {
    getEntity(ndx)->flags &= ~_entityTransparent;
  }
  markEntityDirty_g(ndx);
}

void setEntityDesaturate(item ndx, bool desaturate) {
  if (ndx <= 0) return;
  if (desaturate) {
    getEntity(ndx)->flags |= _entityDesaturate;
  } else {
    getEntity(ndx)->flags &= ~_entityDesaturate;
  }
  markEntityDirty_g(ndx);
}

void setEntityBringToFront(item ndx, bool raise) {
  if (ndx <= 0) return;
  if (raise) {
    getEntity(ndx)->flags |= _entityBringToFront;
  } else {
    getEntity(ndx)->flags &= ~_entityBringToFront;
  }
  markEntityDirty_g(ndx);
}

void setEntityRaise(item ndx, int amount) {
  if (ndx <= 0) return;
  Entity* e = getEntity(ndx);
  e->flags &= ~_entityRaiseMask;
  e->flags |= (amount << _entityRaiseShift) & _entityRaiseMask;
  markEntityDirty_g(ndx);
}

item getEntityMesh(Entity* entity, Cull cull) {
  if(entity->shader >= RainShader || entity->shader == SkyboxShader) {
    return entity->mesh;
  }

  item simpleMesh = entity->simpleMesh;
  item mesh = entity->mesh;
  if (mesh <= 0) mesh = simpleMesh;
  if (simpleMesh <= 0) simpleMesh = mesh;
  if (mesh <= 0) return 0;

  vec3 eloc = entity->location;
  float cameraDistSqrd = distanceSqrd(vec3(cull.x, cull.y, cull.z), eloc);
  bool hard = (entity->flags & _entityHardTransitions);
  float es = entity->entitySize;
  if (!hard && cameraDistSqrd < es*es) return mesh;

  //item shader = entity->shader;
  //if ((shader == LandShader || shader == WaterShader) &&
      //cameraDistSqrd < 1000*1000) return entity->mesh;

  float mq = hard ? 1 : meshQuality;
  float maxDist = entity->maxCameraDistance * mq;
  if (cameraDistSqrd > maxDist*maxDist) return 0;
  vec4 viewSpace = cull.viewProjection * vec4(entity->location, 1.0);
  float w = viewSpace.w;
  viewSpace /= w;
  //if (randFloat() < 0.001) SPDLOG_INFO("({},{},{},{}), {}",
      //viewSpace.x, viewSpace.y, viewSpace.z, viewSpace.w, w);
  float buffer = 1 + es*cull.fovBias/w;
  //vec2 buffer;
  /*
  if (getFOV() > 0.f) {
    //float fovBias = clamp(6 - getFOV()*20, 0.5f, 4.f);
    //float fovBias = getBrightness() * 4.f;
    float fovBias = getFOVBias();
    buffer = vec2(1,1) * (1 + es*fovBias/w);
  } else {
    buffer = vec2(1,1) + cull.invProjSize*es*.75f;
  }
  */
  if (viewSpace.x < -buffer || viewSpace.x > buffer) return 0;
  if (viewSpace.y < -buffer || viewSpace.y > buffer) return 0;

  float simpleDist = entity->simpleDistance * mq;
  if (simpleMesh > 0 && cameraDistSqrd > simpleDist*simpleDist) {
    return simpleMesh;
  } else {
    return mesh;
  }
}

item getEntityMeshOld(Entity* entity, Camera c) {
  //if (!(entity->flags & _entityExists)) return 0;
  //if (!(entity->flags & _entityVisible)) return 0;
  if (entity->mesh <= 0) return 0;
  if(entity->shader >= RainShader || entity->shader == SkyboxShader) {
    return entity->mesh;
  }

  const vec3 eloc = entity->location;
  //if(!inFrustrum(c, eloc, entity->entitySize)) return 0;

  const float ex = eloc.x;
  const float ey = eloc.y;
  const float cx = c.target.x;
  const float cy = c.target.y;
  const float dx = ex-cx;
  const float dy = ey-cy;
  const bool hardTransitions = entity->flags & _entityHardTransitions;
  const float dist = pointRayDistanceSqrd(eloc, c.ray);
  const float es = entity->entitySize;
  const float cs = c.size;
  const float rd = c.resolutionDistance;
  const float rd2 = rd*rd + (hardTransitions ? 0 : dist);
  const float ds = es*es + cs*cs;
  const float ms = entity->maxCameraDistance;
  const float ms2 = ms*ms;
  const float sd = entity->simpleDistance;
  const float sd2 = sd*sd;
  const float distFactor = 1 - getFOV()*4;
  const float df = dist * distFactor;

  if (ms2 < rd2) return 0; // Culled for distance
  if (ds < df*.01f) return 0; // Probably way out of view

  //if (ds < df*1.f) {
    vec4 viewSpace = c.viewProjection * vec4(eloc, 1.0);
    float w = viewSpace.w;
    viewSpace /= w;
    if (randFloat() < 0.001) SPDLOG_INFO("({},{},{},{}), {}",
        viewSpace.x, viewSpace.y, viewSpace.z, viewSpace.w, w);
    //viewSpace.x /= c.window.x;
    //viewSpace.y /= c.window.y;
    const vec2 buffer = c.invProjSize*es + vec2(1,1);
    if (viewSpace.x < -buffer.x || viewSpace.x > buffer.x) return 0;
    if (viewSpace.y < -buffer.y || viewSpace.y > buffer.y) return 0;
    if (viewSpace.z > rd2) return 0;
    if (entity->simpleMesh != 0 && viewSpace.z > sd) return entity->simpleMesh;
  //}

  if (entity->simpleMesh != 0 && sd2 < rd2) {
    return entity->simpleMesh;
  } else {
    return entity->mesh;
  }
}

void copyEntityPlacement(item fromNdx, item toNdx) {
  Entity* from = getEntity(fromNdx);
  Entity* to = getEntity(toNdx);
  to->location = from->location;
  to->rotScale = from->rotScale;
  memcpy_s(to->matrix, from->matrix, 16*sizeof(float));
  markEntityDirty_g(toNdx);
}

void placeEntity(item ndx, vec3 location, float yaw, float pitch, float scal) {
  Entity* entity = getEntity(ndx);
  entity->location = location;
  entity->rotScale = vec3(yaw, pitch, scal);

  //memcpy_s(entity->matrix, &entity->location, 3*sizeof(float));
  //memcpy_s(&entity->matrix[3], &entity->rotScale, 3*sizeof(float));

  /* KEEP for reference
  mat4 matrix = mat4(1.0f);
  matrix = translate(matrix, location);
  matrix = rotate(matrix, yaw, vec3(0, 0, 1));
  matrix = rotate(matrix, pitch, vec3(1, 0, 0));
  matrix = scale(matrix, vec3(scal, scal, scal));
  memcpy_s(entity->matrix, &matrix[0][0], 16*sizeof(float));
  */

  const float cyaw = cos(yaw);
  const float syaw = sin(yaw);
  const float cpitch = cos(pitch);
  const float spitch = sin(pitch);
  const float x = location.x;
  const float y = location.y;
  const float z = location.z;

  entity->matrix[0*4 + 0] = cyaw*scal;
  entity->matrix[0*4 + 1] = syaw*scal;
  entity->matrix[0*4 + 2] = 0;
  entity->matrix[0*4 + 3] = 0;

  entity->matrix[1*4 + 0] = -syaw*cpitch*scal;
  entity->matrix[1*4 + 1] = cyaw*cpitch*scal;
  entity->matrix[1*4 + 2] = spitch*scal;
  entity->matrix[1*4 + 3] = 0;

  entity->matrix[2*4 + 0] = syaw*spitch*scal;
  entity->matrix[2*4 + 1] = -cyaw*spitch*scal;
  entity->matrix[2*4 + 2] = cpitch*scal;
  entity->matrix[2*4 + 3] = 0;

  entity->matrix[3*4 + 0] = x;
  entity->matrix[3*4 + 1] = y;
  entity->matrix[3*4 + 2] = z;
  entity->matrix[3*4 + 3] = 1;

  markEntityDirty_g(ndx);
}

void placeEntity(item ndx, vec3 location, float yaw, float pitch) {
  placeEntity(ndx, location, yaw, pitch, 1);
}

void setEntityClip(item ndx, Line cl) {
  Entity* entity = getEntity(ndx);
  for (int i = 0; i < 16; i++) {
    entity->matrix[i] = 0;
  }
  entity->matrix[0] = cl.start.x;
  entity->matrix[1] = cl.start.y;
  entity->matrix[2] = cl.end.x;
  entity->matrix[3] = cl.end.y;
  entity->matrix[4] = cl.end.z;
  markEntityDirty_g(ndx);
}

void setCull(item ndx, float entitySize, float maxDist) {
  Entity* entity = getEntity(ndx);
  entity->entitySize = entitySize;
  entity->maxCameraDistance = maxDist;
  markEntityDirty_g(ndx);
}

item countEntities() {
  return entities->count();
}

item entitiesSize() {
  return entities->size();
}

float getMeshQuality() {
  return meshQuality;
}

void setMeshQuality(float quality) {
  if (quality <= 0) {
    meshQuality = c(CMeshQuality);
  } else {
    meshQuality = quality;
  }
}

