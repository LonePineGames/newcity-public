#include "culler.hpp"

#include "entity.hpp"
#include "shader.hpp"

#include "../cup.hpp"
#include "../game/game.hpp"
#include "../thread.hpp"

#include "spdlog/spdlog.h"
#include <boost/dynamic_bitset.hpp>
#include <boost/container/set.hpp>

typedef boost::container::set<item> itemset;
typedef itemset::iterator itemsetIter;
static itemset toUpdateInCuller;
float lastMeshQuality = 0;
item numEntities = 0;
item update = 0;
Cull nextCull;
atomic<CullerState> cullerState(CullerWaiting);
atomic<bool> shouldCullerContinue;

Cup<uint32_t> eFlags;
Cup<uint32_t> eDataFlags;
Cup<uint8_t> eShader;
Cup<item> eTexture;
Cup<float> eSize;
Cup<float> eMaxDist;
Cup<float> eMaxDistMQ;
Cup<float> eSimpleDist;
Cup<float> eSimpleDistMQ;
Cup<float> eLoc;
Cup<float> eMatrix;
Cup<item> eMesh;
Cup<item> eSimpleMesh;
Cup<item> eSelectedMesh;
boost::dynamic_bitset<> eActive;
boost::dynamic_bitset<> eHard;
boost::dynamic_bitset<> eUpdate;

void resetCuller_c() {
  toUpdateInCuller.clear();
  lastMeshQuality = 0;
  numEntities = 0;

  eFlags.clear();
  eDataFlags.clear();
  eShader.clear();
  eTexture.clear();
  eSize.clear();
  eMaxDist.clear();
  eMaxDistMQ.clear();
  eSimpleDist.clear();
  eSimpleDistMQ.clear();
  eLoc.clear();
  eMatrix.clear();
  eMesh.clear();
  eSimpleMesh.clear();
  eSelectedMesh.clear();
  eActive.clear();
  eHard.clear();
}

void resizeCuller_c() {
  item size = entitiesSize() + 1;
  numEntities = size;
  if (eFlags.size() <= size) {
    size = (size+1)*2;
    eFlags.resize(size);
    eDataFlags.resize(size);
    eShader.resize(size);
    eTexture.resize(size);
    eSize.resize(size);
    eMaxDist.resize(size);
    eMaxDistMQ.resize(size);
    eSimpleDist.resize(size);
    eSimpleDistMQ.resize(size);
    eLoc.resize(size*3);
    eMatrix.resize(size*16);
    eMesh.resize(size);
    eSimpleMesh.resize(size);
    eSelectedMesh.resize(size);
    eActive.resize(size);
    eHard.resize(size);
  }
  if (eUpdate.size() < size) {
    eUpdate.resize((size+1)*2);
  }
//  SPDLOG_INFO("resizeCuller_c numEntities {} size {}",
 //     numEntities, size);
}

void updateEntityInCuller_g(item ndx) {
  if (numEntities <= ndx) {
    resizeCuller_c();
  }

  Entity* e = getEntity(ndx);
  eFlags.set(ndx, e->flags);
  eDataFlags.set(ndx, e->dataFlags);
  eShader.set(ndx, e->shader);
  eTexture.set(ndx, e->texture);
  eSize.set(ndx, e->entitySize);
  eMaxDist.set(ndx, e->maxCameraDistance);
  eSimpleDist.set(ndx, e->simpleDistance);
  eMesh.set(ndx, e->mesh);
  eSimpleMesh.set(ndx, e->simpleMesh);
  eSelectedMesh.set(ndx, 0);

  bool hard = e->flags & _entityHardTransitions;
  eHard[ndx] = hard;
  float mq = hard ? 1 : getMeshQuality();
  float maxDist = e->maxCameraDistance * mq;
  float simpleDist = e->simpleDistance * mq;
  eMaxDistMQ.set(ndx, maxDist*maxDist);
  eSimpleDistMQ.set(ndx, simpleDist*simpleDist);

//  SPDLOG_INFO("memcpy_s eLoc.size() {} ndx {}", eLoc.size(), ndx);
  memcpy_s(eLoc.get(ndx*3), &e->location, 3*sizeof(float));
  memcpy_s(eMatrix.get(ndx*16), &e->matrix, 16*sizeof(float));

  eActive[ndx] = (e->flags & _entityExists) &&
    (e->flags & _entityVisible) &&
    (e->mesh > 0 || e->simpleMesh > 0);
}

void markEntityDirty_g(item ndx) {
  if (eUpdate.size() <= ndx) {
    eUpdate.resize((ndx+1)*2);
  }
  eUpdate[ndx] = true;
  //toUpdateInCuller.insert(ndx);
}

void updateAllEntitiesInCuller_c() {
  item num = numEntities;
  if (num <= entitiesSize()) {
    resizeCuller_c();
    num = numEntities;
  }

  for (int i = 1; i < num; i++) {
    updateEntityInCuller_g(i);
  }
}

void updateEntityCuller_g() {
  int numUpdates = 0; //numEntities; //toUpdateInCuller.size();
  float startTime = glfwGetTime();

  item num = numEntities;
  if (num <= entitiesSize()) {
    resizeCuller_c();
    num = numEntities;
  }

  for (int i = 1; i < num; i++) {
    if (eUpdate[i]) {
      eUpdate[i] = false;
      updateEntityInCuller_g(i);
      numUpdates ++;
    }
  }

  //updateAllEntitiesInCuller_c();

  //for (int i = 0; i < toUpdateInCuller.size(); i++) {
    //updateEntityInCuller_g(toUpdateInCuller[i]);
  //}
  //toUpdateInCuller.clear();

  //for (itemsetIter it = toUpdateInCuller.begin();
      //it != toUpdateInCuller.end(); it++) {
    //updateEntityInCuller_g(*it);
  //}
  //toUpdateInCuller.clear();

  if (c(CLogUpdateTime)) {
    float endTime = glfwGetTime();
    SPDLOG_INFO("updateEntityCuller_g: {} updates took {:.2f}ms",
        numUpdates, (endTime - startTime)*1000);
  }
}

void selectMeshes_c(Cull cull) {
  float startTime = glfwGetTime();
  item num = numEntities;
  boost::dynamic_bitset<> eShow = eActive;
  boost::dynamic_bitset<> eTest = eActive;
  boost::dynamic_bitset<> eSimple;
  eSimple.resize(num);

  boost::dynamic_bitset<> shaderPass;
  boost::dynamic_bitset<> shaderFail;
  shaderPass.resize(numShaders);
  shaderFail.resize(numShaders);

  bool map = (cull.flags & _cullIsSatMap);
  bool capture = (cull.flags & _cullIsCapture);
  for (int s = 0; s < numShaders; s++) {
    if ((map && !isInMap((Shader)s)) ||
      (capture && !isInCapture((Shader)s))) {
      shaderFail[s] = true;
    } else if (s >= RainShader || s == SkyboxShader) {
      shaderPass[s] = true;
    }
  }

  if (map) {
    for (int i = 1; i < num; i++) {
      uint8_t s = eShader[i];
      bool show = !shaderFail[s] && eShow[i];
      eSelectedMesh.set(i, show ? eSimpleMesh[i] : 0);
    }
    float endTime = glfwGetTime();
    if (c(CLogUpdateTime)) {
      SPDLOG_INFO("Culler time (map) {:.2f}ms", (endTime-startTime)*1000);
    }
    return;
  }

  // Update some computed values
  float meshQuality = getMeshQuality();
  if (meshQuality != lastMeshQuality) {
    for (int i = 1; i < num; i++) {
      float mq = eHard[i] ? 1 : meshQuality;
      float maxDist = eMaxDist[i] * mq;
      float simpleDist = eSimpleDist[i] * mq;
      eMaxDistMQ.set(i, maxDist*maxDist);
      eSimpleDistMQ.set(i, simpleDist*simpleDist);
    }
    lastMeshQuality = meshQuality;
  }

  // Compute camera distance and do basic tests
  for (int i = 1; i < num; i++) {
    if (!eShow[i]) continue;
    float* loc = eLoc.get(i*3);
    float dx = loc[0] - cull.x;
    float dy = loc[1] - cull.y;
    float dz = loc[2] - cull.z;
    float camDist = dx*dx + dy*dy + dz*dz;
    float es = eSize[i];
    uint8_t s = eShader[i];

    if (shaderFail[s]) {
      eShow[i] = false;
      eTest[i] = false;

    } else if (shaderPass[s]) {
      eTest[i] = false;

    } else if (camDist < es*es && !eHard[i]) {
      eTest[i] = false;

    } else if (camDist > eMaxDistMQ[i]) {
      eShow[i] = false;
      eTest[i] = false;

    } else if (camDist > eSimpleDistMQ[i]) {
      eSimple[i] = true;
    }
  }

  // Compute projection and cull
  for (int i = 1; i < num; i++) {
    if (!eTest[i]) continue;
    float* loc = eLoc.get(i*3);
    vec4 vloc(loc[0], loc[1], loc[2], 1);
    vec4 viewSpace = cull.viewProjection * vloc;
    float w = viewSpace.w;
    float projX = viewSpace.x/w;
    float projY = viewSpace.y/w;
    float buffer = 1 + eSize[i]*cull.fovBias/w;
    bool hide = projX > buffer || projX < -buffer ||
      projY > buffer || projY < -buffer;
    if (hide) eShow[i] = false;
  }

  // Select Mesh
  for (int i = 1; i < num; i++) {
    item mesh = 0;
    if (eSimple[i]) mesh = eSimpleMesh[i];
    if (mesh == 0) mesh = eMesh[i];
    if (!eShow[i]) mesh = 0;
    eSelectedMesh.set(i, mesh);
  }

  float endTime = glfwGetTime();
  if (c(CLogUpdateTime)) {
    SPDLOG_INFO("Culler time {:.2f}ms", (endTime-startTime)*1000);
  }
}

bool cullEntity_c(item ndx, UngroupedDrawCommand* command) {
  if (ndx >= numEntities) return false;

  item mesh = eSelectedMesh[ndx];
  if (mesh == 0) return false;
  if (mesh > 1000000 || mesh < 0) {
    SPDLOG_WARN("bad mesh {}", mesh, eShader[ndx]);
    return false;
  }

  command->mesh = mesh;
  command->entity = ndx;
  command->texture = eTexture[ndx];
  command->shader = eShader[ndx];
  DrawData* data = &command->data;
  memcpy_s(data->matrix, eMatrix.get(ndx*16), 16*sizeof(float));
  data->flags = eFlags[ndx];
  data->dataFlags = eDataFlags[ndx];
  return true;
}

Cull getCull_g() {
  Cull cull;
  Camera cam = getCurrentCamera_g();
  bool map = getPerspective_g() == SatMapPerspective;
  bool capture = getPerspective_g() == CapturePerspective;
  cull.flags = map ? _cullIsSatMap : capture ? _cullIsCapture : 0;
  cull.x = cam.position.x;
  cull.y = cam.position.y;
  cull.z = cam.position.z;
  cull.fovBias = getFOVBias();
  cull.viewProjection = cam.viewProjection;
  return cull;
}

void waitForCullerState_g(CullerState state) {
  while(state != cullerState) {
    sleepMicroseconds(100);
  }
}

void setCullerState_g(CullerState state) {
  cullerState = state;
}

void nextCullerCull_g(Cull cull) {
  nextCull = cull;
}

void cullerLoop_c() {
  shouldCullerContinue = true;
  while (shouldCullerContinue) {
    if (isGameLoading()) {
      // wait
    } else if (cullerState == CullerReady) {
      cullerState = CullerRunning;
      if (c(CCullingAlgo) >= 3) {
        collectDrawBuffer();
      } else {
        selectMeshes_c(nextCull);
      }
      cullerState = CullerWaiting;
    //} else if (cullerState = CullerReset) {
      //resetCuller_c();
      //cullerState = CullerWaiting;
    }
    sleepMicroseconds(100);
  }
}

void killCuller() {
  shouldCullerContinue = false;
}

