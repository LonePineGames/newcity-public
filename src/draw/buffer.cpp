#include "buffer.hpp"

#ifdef WIN32
#include <Windows.h>
#include <exception>
#endif

#include "camera.hpp"
#include "culler.hpp"
#include "cull.hpp"
#include "entity.hpp"
#include "texture.hpp"

#include "../renderLand.hpp"
#include "../game/game.hpp"

#include "spdlog/spdlog.h"
#include <boost/unordered_map.hpp>

struct DBShaderPass {
  boost::unordered_map<uint32_t, item> commandMap;
  Cup<DrawCommand> commands;
};

struct DrawBuffer {
  DBShaderPass passes[numShaders];
};

struct ABO {
  GLuint aboID;
  int numInstances;
  int capacity;
  bool isBuffered;
};

struct TreeData {
  Cup<vec4>* data;
  item entity;
};

Cup<UngroupedDrawCommand> ungroupedDrawCommands;

static const GLsizei dataSize = sizeof(DrawData);
static const GLsizei vec4Size = sizeof(vec4);
static const int baseAttrib = 3;
static const int stride = 9;
static const int numABOFrames = 3;
static vector<ABO> abos[numABOFrames+1];
static vector<ABO> treeABOs;
static unordered_map<item, item> treeABOmap;
static int currentABOFrame = 0;
static int currentABO = 0;
static vector<TreeData> treeDataToWriteFront, treeDataToWriteBack;
static int entitiesDrawn = 0;

DrawBuffer drawBuffers[2];
static atomic<bool> frontBuffer(false);

void initDrawBuffers() {
  //frontBuffer = (DrawBuffer*) calloc(1, sizeof(DrawBuffer));
  //backBuffer = (DrawBuffer*) calloc(1, sizeof(DrawBuffer));
  resetDrawBuffers();
}

void swapDrawBuffers() {
  frontBuffer = !frontBuffer;
  treeDataToWriteBack.swap(treeDataToWriteFront);
}

void clearDrawBuffer(DrawBuffer* b) {
  /*
  for (int s = 0; s < numShaders; s++) {
    std::vector<std::vector<DrawCommand>> meshList = b->instancedTable[s];
    for(auto &mesh : meshList) {
      for(auto &tex : mesh) {
        tex.data.empty();
      }
    }
    b->instancedTable[s] = meshList;

    std::vector<DrawCommand> singleList = b->singleTable[s];
    for(auto &dc : singleList) {
      dc.data.empty();
    }
    b->singleTable[s] = singleList;
  }

  b->refreshDrawBuffer();
  */

  for (int s = 0; s < numShaders; s++) {
    DBShaderPass* pass = &b->passes[s];
    for (int i = 0; i < pass->commands.size(); i++) {
      pass->commands.get(i)->data.clear();
    }
    pass->commandMap.clear();
    pass->commands.empty();
  }
}

void resetDrawBuffers() {
  for (int i = 0; i < numABOFrames+1; i++) {
    abos[i].clear();
  }

  treeABOs.clear();
  treeABOmap.clear();
  currentABOFrame = 0;
  currentABO = 0;

  for (int i = 0; i < treeDataToWriteFront.size(); i++) {
    TreeData data = treeDataToWriteFront[i];
    data.data->clear();
    free(data.data);
  }
  treeDataToWriteFront.clear();

  for (int i = 0; i < treeDataToWriteBack.size(); i++) {
    TreeData data = treeDataToWriteBack[i];
    data.data->clear();
    free(data.data);
  }
  treeDataToWriteBack.clear();

  clearDrawBuffer(&drawBuffers[0]);
  clearDrawBuffer(&drawBuffers[1]);
  ungroupedDrawCommands.clear();
}

DrawCommand* nextDrawCommand(DrawBuffer* b, Shader s, item entity,
    item mesh, item texture) {
  DBShaderPass* pass = &b->passes[s];
  uint32_t key = (texture << 24) + mesh;
  pass->commandMap[key] = pass->commands.size();
  DrawCommand* result = pass->commands.next();
  result->entity = entity;
  result->mesh = mesh;
  result->texture = texture;
  result->aboID = 0;
  result->numInstances = -1;
  return result;
}

DrawCommand* getDrawCommand(DrawBuffer* b, Shader s, item entity,
    item mesh, item texture) {
  if (!isShaderInstanced(s)) {
    return nextDrawCommand(b, s, entity, mesh, texture);
  }

  DBShaderPass* pass = &b->passes[s];
  uint32_t key = (texture << 24) + mesh;
  item ndx = pass->commandMap[key];
  if (ndx == 0) {
    return nextDrawCommand(b, s, entity, mesh, texture);
  } else {
    return pass->commands.get(ndx);
  }
}

inline void collectEntity(item ndx, Cull cull, DrawBuffer* b) {
  Entity* e = getEntity(ndx);
  Shader s = e->shader;

  #ifdef COUNT_ENTITIES
    entitiesPerShader[s] ++;
  #endif

  bool map = (cull.flags & _cullIsSatMap);
  bool capture = (cull.flags & _cullIsSatMap);
  if (map && !isInMap(s)) return;
  if (capture && !isInCapture(s)) return;

  item mesh = map ? e->simpleMesh : capture ? e->mesh : getEntityMesh(e, cull);
  if (!map && !capture) setEntityCulled_g(ndx, mesh == 0);
  if (mesh == 0) return;
  if (mesh > 1000000) {
    SPDLOG_WARN("bad mesh {} {}", mesh, e->shader);
    return;
  }

  DrawCommand* command = getDrawCommand(b, s, ndx, mesh, e->texture);
  DrawData* data = command->data.next();
  #ifdef WIN32
  __try {
  #endif
    memcpy_s(data->matrix, e->matrix, 16*sizeof(float));
    data->flags = e->flags;
    data->dataFlags = e->dataFlags;
    entitiesDrawn ++;
    #ifdef COUNT_ENTITIES
    visEntPerShader[s] ++;
    #endif
  #ifdef WIN32
  }
  __except (EXCEPTION_EXECUTE_HANDLER) {
    SPDLOG_ERROR("Windows Structured Exception in collectEntity for ndx {}", ndx);
    entityPrintDebugInfo(ndx);
  }
  #endif

  /*
  UngroupedDrawCommand* command = ungroupedDrawCommands.next();
  command->mesh = mesh;
  command->entity = ndx;
  command->texture = e->texture;
  command->shader = s;
  DrawData* data = &command->data; //.next();
  memcpy_s(data->matrix, e->matrix, 16*sizeof(float));
  data->flags = e->flags;
  data->dataFlags = e->dataFlags;
  */
}

void startEntityCulling_g() {
  if (c(CCullingAlgo) == 0) {
    collectDrawBuffer();
    return;
  }

  Cull cull = getCull_g();
  waitForCullerState_g(CullerWaiting);
  updateEntityCuller_g();
  nextCullerCull_g(cull);
  setCullerState_g(CullerReady);

  if (c(CCullingAlgo) < 2) {
    selectMeshes_c(cull);
    setCullerState_g(CullerWaiting);
    collectEntities_g();
  }
}

void collectEntities_g() {
  if (c(CCullingAlgo) == 0) return;

  if (c(CCullingAlgo) >= 3) {
    //SPDLOG_INFO("waiting for culler");
    float waitStart = glfwGetTime();
    waitForCullerState_g(CullerWaiting);
    float waitEnd = glfwGetTime();
    //SPDLOG_INFO("waited for culler {:.2f}ms",
        //(waitEnd-waitStart)*1000);
    return;
  }

  float prepStart = glfwGetTime();
  DrawBuffer* b = &drawBuffers[frontBuffer];
  clearDrawBuffer(b);
  int numEntities = entitiesSize();
  ungroupedDrawCommands.empty();
  UngroupedDrawCommand* nextCommand = ungroupedDrawCommands.next();
  float prepEnd = glfwGetTime();
  bool doSetEntityCulled = getPerspective_g() == MainPerspective;

  waitForCullerState_g(CullerWaiting);
  float waitEnd = glfwGetTime();
  for (int i=1; i <= numEntities; i++) {
    //if (!isEntityActive_g(i)) continue;
    bool found = cullEntity_c(i, nextCommand);
    if (doSetEntityCulled) setEntityCulled_g(i, !found);
    if (found) nextCommand = ungroupedDrawCommands.next();
  }
  float cullEnd = glfwGetTime();

  /*
  entitiesDrawn = ungroupedDrawCommands.size();
  for (int i = 0; i < ungroupedDrawCommands.size(); i++) {
    UngroupedDrawCommand* ungpd = ungroupedDrawCommands.get(i);
    DrawCommand* command = getDrawCommand(b, (Shader)ungpd->shader,
        ungpd->entity, ungpd->mesh, ungpd->texture);
    memcpy_s(command->data.next(), &ungpd->data, sizeof(DrawData));
  }
  */
  float groupEnd = glfwGetTime();

  if (c(CLogUpdateTime)) {
    SPDLOG_INFO("prep {:.2f}ms wait {:.2f}ms cull {:.2f}ms group {:.2f}ms",
        (prepEnd-prepStart)*1000,
        (waitEnd-prepEnd)*1000,
        (cullEnd-waitEnd)*1000,
        (groupEnd-cullEnd)*1000);
  }
}

void collectDrawBuffer() {
  DrawBuffer* b = &drawBuffers[frontBuffer];
  clearDrawBuffer(b);
  Camera cam = getCurrentCamera_g();
  ungroupedDrawCommands.empty();

  #ifdef COUNT_ENTITIES
    int entitiesPerShader[numShaders] = {0};
    int visEntPerShader[numShaders] = {0};
  #endif

  float cullStart = glfwGetTime();
  Cull cull = getCull_g();

  if (isGameLoading() || c(CCullingAlgo) == 0) {
    int numEntities = entitiesSize();
    for (int i=1; i <= numEntities; i++) {
      if (!isEntityActive_g(i)) continue;
      collectEntity(i, cull, b);
    }

  } else {
    selectMeshes_c(cull);

    bool doSetEntityCulled = getPerspective_g() == MainPerspective;
    int numEntities = entitiesSize();
    //UngroupedDrawCommand* nextCommand = ungroupedDrawCommands.next();
    for (int i=1; i <= numEntities; i++) {
      //if (!isEntityActive_g(i)) continue;
      UngroupedDrawCommand ungpd;
      bool found = cullEntity_c(i, &ungpd);
      if (doSetEntityCulled) setEntityCulled_g(i, !found);
      //if (found) nextCommand = ungroupedDrawCommands.next();
      if (found) {
        DrawCommand* command = getDrawCommand(b, (Shader)ungpd.shader,
            ungpd.entity, ungpd.mesh, ungpd.texture);
        memcpy_s(command->data.next(), &ungpd.data, sizeof(DrawData));
      }
    }
  }
  float cullEnd = glfwGetTime();

  /*
  entitiesDrawn = ungroupedDrawCommands.size();
  for (int i = 0; i < ungroupedDrawCommands.size(); i++) {
    UngroupedDrawCommand* ungpd = ungroupedDrawCommands.get(i);
    DrawCommand* command = getDrawCommand(b, (Shader)ungpd->shader,
        ungpd->entity, ungpd->mesh, ungpd->texture);
    memcpy_s(command->data.next(), &ungpd->data, sizeof(DrawData));
  }
  */

  float groupEnd = glfwGetTime();
  if (c(CLogUpdateTime) && !isGameLoading()) {
    SPDLOG_INFO("cull time: {:.2f}ms group time: {:.2f}ms",
        (cullEnd-cullStart)*1000, (groupEnd-cullEnd)*1000);
  }

  #ifdef COUNT_ENTITIES
    for (int s = 0; s < numShaders; s++) {
      SPDLOG_INFO("{}/{} entities in {} shader",
          visEntPerShader[s], entitiesPerShader[s], getShaderName((Shader)s));
    }
  #endif
}

void setupInstancedAttribs() {
  for (int i = 0; i < 4; i++) {
    glEnableVertexAttribArray(i+baseAttrib);
    glVertexAttribPointer(i+baseAttrib, 4,
        GL_FLOAT, GL_FALSE, dataSize, (void*)(i * vec4Size));
    glVertexAttribDivisor(i+baseAttrib, 1);
  }

  glEnableVertexAttribArray(4+baseAttrib);
  glVertexAttribIPointer(4+baseAttrib, 2, GL_UNSIGNED_INT,
      dataSize, (void*)(4 * vec4Size));
  glVertexAttribDivisor(4+baseAttrib, 1);
}

void setupTreeAttribs() {
  glEnableVertexAttribArray(0+baseAttrib);
  glVertexAttribPointer(0+baseAttrib, 4, GL_FLOAT, GL_FALSE,
      vec4Size, (void*)(0 * vec4Size));
  glVertexAttribDivisor(0+baseAttrib, 1);
}

void setupVertexAttribs() {
  int strideSize = stride*sizeof(float);
  int offsetSize = sizeof(float)*3;

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  //vertices
  glVertexAttribPointer(
    0,                    // attribute
    3,                    // size
    GL_FLOAT,             // type
    GL_FALSE,             // normalized?
    strideSize,           // stride
    (void*)(offsetSize*0) // array buffer offset
  );

  //normals
  glVertexAttribPointer(
    1,                    // attribute
    3,                    // size
    GL_FLOAT,             // type
    GL_FALSE,             // normalized?
    strideSize,           // stride
    (void*)(offsetSize*1) // array buffer offset
  );

  //texture
  glVertexAttribPointer(
    2,                    // attribute
    3,                    // size
    GL_FLOAT,             // type
    GL_FALSE,             // normalized?
    strideSize,           // stride
    (void*)(offsetSize*2) // array buffer offset
  );
}

void setupVAO(BufferedMesh* mesh) {
  if (mesh->vaoID == 0) {
    glGenVertexArrays(1, &mesh->vaoID);
    vaoAdded();
    glBindVertexArray(mesh->vaoID);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexVBOID);
    setupVertexAttribs();

  } else {
    glBindVertexArray(mesh->vaoID);
  }
}

void bufferMesh(BufferedMesh* mesh) {
  if (mesh->buffer) {
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexVBOID);
    int bufferSize = mesh->numTriangles * sizeof(GLfloat) * 27;
    glBufferData(GL_ARRAY_BUFFER, bufferSize, mesh->buffer,
      mesh->flags & _meshStreamDraw ? GL_STREAM_DRAW :
      mesh->flags & _meshDynDraw ? GL_DYNAMIC_DRAW :
      GL_STATIC_DRAW);
    free(mesh->buffer);
    mesh->buffer = 0;
  }
}

uvec2 convertFlags(uint64_t in) {
  uint32_t x = in >> 32;
  uint32_t y = in;
  //SPDLOG_INFO("convertFlags {:#b} => {:#b}, {:#b}", in, x, y);
  return uvec2(x, y);
}

ABO* bindABO();
void draw(Shader s, DrawCommand* command) {
  int numInstances = command->numInstances >= 0 ?
    command->numInstances : command->data.size();
  bool isInstanced = isShaderInstanced(s);
  if (numInstances <= 0) return;
  if (isInstanced && command->aboID == 0) return;
  BufferedMesh* mesh = getBufferedMesh(command->mesh);
  if (mesh->numTriangles <= 0) return;
  if (!(mesh->flags & _meshExists)) {
    SPDLOG_WARN("Trying to render non-existing mesh {}, flags {}, vbo {}, "
      "numT {}, numI {}", command->mesh, mesh->flags, mesh->vertexVBOID,
      mesh->numTriangles, numInstances);
    return;
  }

  if (mesh->vertexVBOID <= 0) {
    if (mesh->buffer == 0) {
      return;
    } else {
      glGenBuffers(1, &mesh->vertexVBOID);
      vboAdded();
    }
  }

  setTexture(command->texture, s, command->mesh);
  setupVAO(mesh);
  bufferMesh(mesh);

  if (isInstanced) {
    glBindBuffer(GL_ARRAY_BUFFER, command->aboID);
    if (s == TreeShader || s == ShadowTreeShader) {
      DrawData* data = command->data.get(0);
      glUniformMatrix4fv(getUniformID(s, MUniform), 1, GL_FALSE,
          &data->matrix[0]);
      uvec2 flags = uvec2(data->dataFlags, data->flags);
      if (mesh->flags & _meshBillboard) flags.y |= _entityBillboard;
      glUniform2uiv(getUniformID(s, FlagsUniform), 1,
          (const GLuint*)  &flags);
      setupTreeAttribs();
    } else {
      setupInstancedAttribs();
    }
    glDrawArraysInstanced(GL_TRIANGLES, 0, mesh->numTriangles * 3,
        numInstances);

  } else {
    DrawData* data = command->data.get(0);
    for (int i = 0; i < numInstances; i++) {
      DrawData* dataI = &data[i];
      glUniformMatrix4fv(getUniformID(s, MUniform), 1, GL_FALSE,
          &dataI->matrix[0]);
      uvec2 flags = uvec2(dataI->dataFlags, dataI->flags);
      glUniform2uiv(getUniformID(s, FlagsUniform), 1,
          (const GLuint*) &flags);
      glDrawArrays(GL_TRIANGLES, 0, mesh->numTriangles * 3);
    }
  }
}

void runDrawCommands(Shader shader, Shader group) {
  DrawBuffer* b = &drawBuffers[!frontBuffer];
  DBShaderPass* pass = &b->passes[group];

  for (int i = 0; i < pass->commands.size(); i++) {
    draw(shader, pass->commands.get(i));
  }
}

ABO* bindABO() {
  vector<ABO>* fabos = &abos[currentABOFrame];
  ABO* result;
  if (currentABO < fabos->size()) {
    result = &(*fabos)[currentABO];
    glBindBuffer(GL_ARRAY_BUFFER, result->aboID);

  } else {
    ABO t;
    fabos->push_back(t);
    result = &(*fabos)[currentABO];
    result->isBuffered = false;
    result->numInstances = -1;
    glGenBuffers(1, &result->aboID);
    glBindBuffer(GL_ARRAY_BUFFER, result->aboID);
  }

  currentABO ++;
  return result;
}

void bufferABO(DrawCommand* c) {
  int numInstances = c->data.size();
  DrawData* data = c->data.get(0);
  ABO* abo = bindABO();
  c->aboID = abo->aboID;
  int bufferSize = numInstances * dataSize;
  if (abo->isBuffered && abo->capacity >= numInstances) {
    glBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize, data);
    abo->numInstances = numInstances;
  } else {
    glBufferData(GL_ARRAY_BUFFER, bufferSize, data, GL_STREAM_DRAW);
    abo->numInstances = numInstances;
    abo->capacity = numInstances;
    abo->isBuffered = true;
  }
}

void writeTreeData(Cup<vec4>* data, item entity) {
  if (entity == 0) return;

  TreeData tdata;
  tdata.data = data;
  //SPDLOG_INFO("writeTreeData {}", data->size());
  tdata.entity = entity;
  treeDataToWriteFront.push_back(tdata);
}

void bufferTreeABO(TreeData data) {
  if (data.entity <= 0) return;

  item ndx = 0;
  auto result = treeABOmap.find(data.entity);
  if (result == treeABOmap.end()) {
    ABO n;
    n.isBuffered = false;
    n.numInstances = -1;
    glGenBuffers(1, &n.aboID);
    ndx = treeABOs.size();
    treeABOs.push_back(n);
    treeABOmap[data.entity] = ndx;
  } else {
    ndx = (*result).second;
  }
  ABO* abo = &treeABOs[ndx];
  glBindBuffer(GL_ARRAY_BUFFER, abo->aboID);

  vec4 fallback;
  bool valid = data.data != 0 && data.data->size() > 0;
  vec4* vec4Data = valid ? data.data->get(0) : &fallback;
  int numInstances = valid ? data.data->size() : 0;
  int bufferSize = numInstances * sizeof(vec4);
  if (abo->isBuffered && abo->numInstances > numInstances) {
    glBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize, vec4Data);
    abo->numInstances = numInstances;
  } else {
    glBufferData(GL_ARRAY_BUFFER, bufferSize, vec4Data, GL_DYNAMIC_DRAW);
    abo->numInstances = numInstances;
    abo->capacity = numInstances;
    abo->isBuffered = true;
  }

  data.data->clear();
}

void bufferTreeABOs() {
  for (int i = 0; i < treeDataToWriteBack.size(); i++) {
    TreeData data = treeDataToWriteBack[i];
    bufferTreeABO(data);
    //data.data->clear();
    //delete data.data;
  }
  treeDataToWriteBack.clear();
}

void bufferABOs() {
  currentABOFrame = (currentABOFrame+1)%numABOFrames;
  currentABO = 0;
  bufferTreeABOs();

  DrawBuffer* b = &drawBuffers[!frontBuffer];
  for (int s = 0; s < numShaders; s++) {
    if (isShaderInstanced((Shader)s)) {
      DBShaderPass* pass = &b->passes[s];
      for (int i = 0; i < pass->commands.size(); i++) {
        DrawCommand* command = pass->commands.get(i);
        if(s == TreeShader && getTreesVisible()) {
          auto result = treeABOmap.find(command->entity);
          if(result == treeABOmap.end()) {
            command->aboID = 0;
            command->numInstances = 0;
          } else {
            ABO abo = treeABOs[(*result).second];
            command->aboID = abo.aboID;
            command->numInstances = abo.numInstances;
          }
        } else {
          bufferABO(command);
        }
      }
    }
  }
}

int numEntitiesDrawn() {
  return entitiesDrawn;
}

