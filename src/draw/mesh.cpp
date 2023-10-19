#include "mesh.hpp"

#include "../pool.hpp"
#include "../thread.hpp"
#include "../util.hpp"

#include "buffer.hpp"

#include "spdlog/spdlog.h"
#include <algorithm>

enum MeshCommandType {
  AddMesh, RemoveMesh, BufferMesh, ResetMeshes,
  numMeshCommandTypes
};

struct MeshCommand {
  MeshCommandType type;
  item mesh;
  int flags;
  int numTriangles;
  void* data;
};

Pool<Mesh>* meshes = Pool<Mesh>::newPool(2000);
vector<BufferedMesh> bufferedMeshes;
vector<MeshCommand>* meshCommandsWrite = new vector<MeshCommand>(100);
vector<MeshCommand>* meshCommandsRead = new vector<MeshCommand>(100);
vector<GLuint> vaosToDelete;
mutex meshMutex;

static const int stride = 9;
const int minBufferCapacity = 10;
const float bufferGrowthRate = 1.618;
static int numVAOs = 0;
static int numVBOs = 0;
static int numABOs = 0;

item addMesh() {
  item ndx = meshes->create();
  Mesh* mesh = getMesh(ndx);

  mesh->triangles = 0;
  mesh->numTriangles = 0;
  mesh->bufferCapacity = 0;
  mesh->flags = _meshExists;

  MeshCommand c;
  c.type = AddMesh;
  c.mesh = ndx;
  c.flags = mesh->flags;
  unique_lock<mutex> lock(meshMutex);
  meshCommandsWrite->push_back(c);

  return ndx;
}

void removeMesh(item ndx) {
  if (ndx <= 0 || ndx > meshes->size()) {
    SPDLOG_WARN("WARNING: removeMesh on invalid ndx");
    return;
  }
  Mesh* mesh = getMesh(ndx);
  if (!(mesh->flags & _meshExists)) {
    SPDLOG_WARN("WARNING: Double removeMesh attempt");
    // Early return to prevent undefined behavior
    return;
  }

  mesh->flags = 0;
  mesh->numTriangles = 0;
  mesh->bufferCapacity = 0;
  if (mesh->triangles) {
    free(mesh->triangles);
  }
  mesh->triangles = 0;
  meshes->free(ndx);

  MeshCommand c;
  c.type = RemoveMesh;
  c.mesh = ndx;
  c.flags = 0;
  unique_lock<mutex> lock(meshMutex);
  meshCommandsWrite->push_back(c);
}

Mesh* getMesh(item ndx) {
  return meshes->get(ndx);
}

void bufferMesh(item ndx) {
  Mesh* mesh = getMesh(ndx);
  if (mesh == 0 || !(mesh->flags & _meshExists)) {
    SPDLOG_WARN("bufferMesh after removeMesh");
    return;
  }
  int numTriangles = mesh->numTriangles;
  int bufferLength = numTriangles * 3 * 3 * 3;
  GLfloat* vertexBuffer = (GLfloat*) &mesh->triangles[0];

  /*
  GLfloat* vertexBuffer = (GLfloat*) calloc(bufferLength, sizeof(GLfloat));
  memcpy_s(vertexBuffer, (GLfloat*) mesh->triangles,
      bufferLength * sizeof(GLfloat));
  free(mesh->triangles);
  mesh->triangles = 0;
  */

  mesh->triangles = 0;
  mesh->numTriangles = 0;
  mesh->bufferCapacity = 0;

  MeshCommand c;
  c.type = BufferMesh;
  c.mesh = ndx;
  c.flags = mesh->flags;
  c.numTriangles = numTriangles;
  c.data = vertexBuffer;
  unique_lock<mutex> lock(meshMutex);
  meshCommandsWrite->push_back(c);
}

void reserveMesh(Mesh* mesh, item numTris) {
  if (!mesh->flags & _meshExists) {
    handleError("reserveMesh on non-existant mesh");
    return;
  }

  int targetCap = numTris + mesh->numTriangles;
  int bufferCap = mesh->bufferCapacity;
  if (bufferCap < minBufferCapacity) bufferCap = minBufferCapacity;
  while (bufferCap <= targetCap) bufferCap *= bufferGrowthRate;

  int bufferSize = bufferCap*stride*3*sizeof(GLfloat);
  if (!mesh->triangles) {
    mesh->triangles = (TriangleData*) malloc(bufferSize);
  } else if (bufferCap <= mesh->bufferCapacity) {
    return;
  } else {
    mesh->triangles = (TriangleData*) realloc(mesh->triangles, bufferSize);
  }

  mesh->bufferCapacity = bufferCap;

  /*
  while (!mesh->triangles ||
      mesh->bufferCapacity < mesh->numTriangles+numTris) {
    mesh->bufferCapacity = std::max(minBufferCapacity,
        int(ceil(mesh->bufferCapacity * bufferGrowthRate)));
    int bufferSize = mesh->bufferCapacity*stride*3*sizeof(GLfloat);
    if (!mesh->triangles) {
      mesh->triangles = (TriangleData*) malloc(bufferSize);
    } else {
      mesh->triangles = (TriangleData*) realloc(mesh->triangles, bufferSize);
    }
  }
  */
}

void reserveMesh(item meshNdx, item numTris) {
  Mesh* mesh = getMesh(meshNdx);
  reserveMesh(mesh, numTris);
}

int convertUV(float val) {
  return int(val * 32767); //SHRT_MAX / 64.f);
}

ivec3 convertUV(vec3 in) {
  return ivec3(
    convertUV(in.x),
    convertUV(in.y),
    in.z
  );
}

void pushTriangle(Mesh* mesh, Triangle triangle) {
  if (mesh == 0 || !(mesh->flags & _meshExists)) {
    SPDLOG_WARN("pushTriangle on non-existent mesh");
    return;
  }

  reserveMesh(mesh, 1);

  TriangleData data;
  for (int i = 0; i < 3; i++) {
    RenderPoint r = triangle.points[i];
    DrawPoint* d = &data.points[i];
    d->point = r.point;
    d->normal = r.normal;
    d->texture = convertUV(r.texture);
  }

  mesh->triangles[mesh->numTriangles] = data;
  mesh->numTriangles ++;
}

void insertMesh(Mesh* mesh, item sourceNdx, vec3 offset,
    float yaw, float pitch, vec3 scal) {
  if (mesh == 0 || !(mesh->flags & _meshExists) || sourceNdx <= 0) {
    SPDLOG_WARN("insertMesh on non-existent mesh");
    return;
  }

  Mesh* other = getMesh(sourceNdx);
  reserveMesh(mesh, other->numTriangles);

  mat4 matrix = mat4(1.0f);
  matrix = translate(matrix, offset);
  matrix = rotate(matrix, yaw, vec3(0, 0, 1));
  matrix = rotate(matrix, pitch, vec3(1, 0, 0));
  matrix = scale(matrix, scal);

  //SPDLOG_WARN("insertMesh {} {}tris @ ({},{},{})",
      //sourceNdx, other->numTriangles, offset.x, offset.y, offset.z);

  for (int t = 0; t < other->numTriangles; t++) {
    TriangleData data;
    TriangleData triangle = other->triangles[t];
    for (int i = 0; i < 3; i++) {
      DrawPoint r = triangle.points[i];
      DrawPoint* d = &data.points[i];
      vec4 p = vec4(r.point, 1);
      vec4 n = vec4(r.normal, 0);
      p = matrix * p;
      n = matrix * n;
      d->point = vec3(p);
      d->normal = vec3(n);
      d->texture = r.texture;
    }

    mesh->triangles[mesh->numTriangles] = data;
    mesh->numTriangles ++;
  }

  //memcpy_s(&mesh->triangles[mesh->numTriangles],
      //(float*) other->triangles,
      //other->numTriangles * sizeof(TriangleData));

  //mesh->numTriangles += other->numTriangles;
}

BufferedMesh* getBufferedMesh(item ndx) {
  return &bufferedMeshes[ndx];
}

void runMeshCommands() {
  for (int i = 0; i < meshCommandsRead->size(); i++) {
    MeshCommand* c = &(*meshCommandsRead)[i];

    if (c->type == AddMesh) {
      if (c->mesh >= bufferedMeshes.size()) {
        bufferedMeshes.resize(c->mesh+1);
      }
      BufferedMesh* mesh = &bufferedMeshes[c->mesh];
      mesh->flags = c->flags;
      if (mesh->vertexVBOID) {
        SPDLOG_WARN("vbo being overwritten");
      }
      mesh->vertexVBOID = 0;
      if (mesh->vaoID) {
        SPDLOG_WARN("vao being overwritten");
      }
      mesh->vaoID = 0;
      mesh->numTriangles = 0;
      if (mesh->buffer) {
        SPDLOG_WARN("unbuffered mesh being overwritten");
        free(mesh->buffer);
        mesh->buffer = 0;
      }

    } else if (c->type == RemoveMesh) {
      BufferedMesh* mesh = &bufferedMeshes[c->mesh];

      if (mesh->vaoID != 0) {
        //glDeleteBuffers(1, &mesh->vaoID);
        mesh->vaoID = 0;
        numVAOs--;
      }

      if (mesh->vertexVBOID != 0) {
        glDeleteBuffers(1, &mesh->vertexVBOID);
        mesh->vertexVBOID = 0;
        numVBOs--;
      }

      mesh->flags = 0;
      if (mesh->buffer) {
        free(mesh->buffer);
        mesh->buffer = 0;
      }

    } else if (c->type == BufferMesh) {
      BufferedMesh* mesh = &bufferedMeshes[c->mesh];
      mesh->numTriangles = c->numTriangles;
      mesh->flags = c->flags;
      if (mesh->buffer != 0) free(mesh->buffer);
      mesh->buffer = (GLfloat*) c->data;

      /*
      int dataSize = c->numTriangles * 27;
      GLfloat* vertexBuffer = (GLfloat*) calloc(dataSize, sizeof(GLfloat));
      memcpy_s(vertexBuffer, (GLfloat*) c->data,
          dataSize * sizeof(GLfloat));
      mesh->buffer = vertexBuffer;
      free(c->data);
      c->data = 0;
      */

      /*
      int dataSize = c->numTriangles * sizeof(GLfloat) * 27;
      if (mesh->vertexVBOID <= 0) {
        glGenBuffers(1, &mesh->vertexVBOID);
      }
      glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexVBOID);
      glBufferData(GL_ARRAY_BUFFER, dataSize, c->data, GL_STATIC_DRAW);
      free(c->data);
      */

    } else if (c->type == ResetMeshes) {
      for (int i = 1; i < bufferedMeshes.size(); i++) {
        BufferedMesh* mesh = getBufferedMesh(i);
        if (mesh->buffer) {
          free(mesh->buffer);
          mesh->buffer = 0;
        }
        if (mesh->vertexVBOID != 0) {
          glDeleteBuffers(1, &mesh->vertexVBOID);
          mesh->vertexVBOID = 0;
          numVBOs--;
        }
        if (mesh->vaoID != 0) {
          glDeleteBuffers(1, &mesh->vaoID);
          mesh->vaoID = 0;
          numVAOs--;
        }
      }
    }
  }

  meshCommandsRead->clear();
}

void vaoAdded() {
  numVAOs++;
}
void aboAdded() {
  numABOs++;
}
void vboAdded() {
  numVBOs++;
}

void swapMeshCommands() {
  unique_lock<mutex> lock(meshMutex);
  vector<MeshCommand>* swap = meshCommandsRead;
  meshCommandsRead = meshCommandsWrite;
  meshCommandsWrite = swap;
  //SPDLOG_INFO("{} meshes", meshes->count());
  //SPDLOG_INFO("{} vaos", numVAOs);
  //SPDLOG_INFO("{} vbos", numVBOs);
}

item meshesSize() {
  return meshes->size();
}

void resetMeshes() {
  for (int i = 1; i <= meshes->size(); i++) {
    if (getMesh(i)->flags & _meshExists) {
      removeMesh(i);
    }
  }

  MeshCommand c;
  c.type = ResetMeshes;
  meshCommandsWrite->push_back(c);
}

