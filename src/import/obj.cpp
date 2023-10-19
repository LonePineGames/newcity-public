#include "obj.hpp"
#include "OBJ_Loader.hpp"

#include "../draw/camera.hpp"
#include "../platform/lookup.hpp"

#include <cstring>

enum {
  OBJ_LOOKUP_FAILED = -1
};

static std::vector<ObjModel> objStore;
static item meshNdxTemp;
static item entNdxTemp;


//-----------------------------------------------------------------------------
// ObjStore functions
//-----------------------------------------------------------------------------

bool clearObjStore() {
  if(objStore.size() == 0) {
    return false;
  }

  std::vector<ObjModel>().swap(objStore);
  return true;
}

std::vector<ObjModel> getObjStore() {
  return objStore;
}

int getObjIndex(std::string name) {
  if(objStore.size() == 0) {
    consolePrintLine("No .obj files in obj store");
    return OBJ_LOOKUP_FAILED;
  }

  int index = OBJ_LOOKUP_FAILED;
  const char* ncst = name.c_str();
  for(int i = 0; i < objStore.size(); i++) {
    if(strcmp(ncst, objStore[i].name.c_str()) == 0) {
      index = i;
      break;
    }
  }

  return index;
}


//-----------------------------------------------------------------------------
// Functions for converting from objl:: specific types into ObjModel
//-----------------------------------------------------------------------------

ObjVertex convertVertex(objl::Vertex vert) {
  return ObjVertex(
    // pos
    vec3(vert.Position.X,
      vert.Position.Y,
      vert.Position.Z),
    // norm
    vec3(vert.Normal.X,
      vert.Normal.Y,
      vert.Normal.Z),
    // uv
    vec2(vert.TextureCoordinate.X,
      vert.TextureCoordinate.Y));
}

std::vector<ObjVertex> convertVertices(std::vector<objl::Vertex> verts) {
  std::vector<ObjVertex> objVerts;
  for(int i = 0; i < verts.size(); i++) {
    objVerts.push_back(convertVertex(verts[i]));
  }
  return objVerts;
}

ObjMaterial convertMaterial(objl::Material mat) {
  ObjMaterial newMat;
  newMat.name = mat.name;
  newMat.Ka = vec3(mat.Ka.X,mat.Ka.Y,mat.Ka.Z);
  newMat.Kd = vec3(mat.Kd.X,mat.Kd.Y,mat.Kd.Z);
  newMat.Ks = vec3(mat.Ks.X,mat.Ks.Y,mat.Ks.Z);
  newMat.Ns = mat.Ns;
  newMat.Ni = mat.Ni;
  newMat.d = mat.d;
  newMat.illum = mat.illum;
  newMat.map_Ka = mat.map_Ka;
  newMat.map_Kd = mat.map_Kd;
  newMat.map_Ks = mat.map_Ks;
  newMat.map_Ns = mat.map_Ns;
  newMat.map_d = mat.map_d;
  newMat.map_bump = mat.map_bump;
  return newMat;
}

std::vector<ObjMaterial> convertMaterials(std::vector<objl::Material> mats) {
  std::vector<ObjMaterial> objMats;
  for(int i = 0; i < mats.size(); i++) {
    objMats.push_back(convertMaterial(mats[i]));
  }
  return objMats;
}

std::vector<ObjMesh> convertMeshes(std::vector<objl::Mesh> meshes) {
  std::vector<ObjMesh> objMeshes;
  for(int i = 0; i < meshes.size(); i++) {
    objMeshes.push_back(ObjMesh(
      meshes[i].MeshName,
      convertVertices(meshes[i].Vertices),
      meshes[i].Indices,
      convertMaterial(meshes[i].MeshMaterial)
    ));
  }
  return objMeshes;
}


//-----------------------------------------------------------------------------
// API for interacting with .obj files
//-----------------------------------------------------------------------------

bool loadObjFile(std::string name) {
  if(name.length() == 0) {
    return false;
  }

  vector<string> files = lookupDirectory("models",".obj",0);

  if(files.size() == 0) {
    consolePrintError(consoleGetError(ConStatus::DIR_READ_ERROR),"/models/");
    return false;
  }

  int fileIndex = -1;
  const char* ncst = name.c_str();
  for(int i = 0; i < files.size(); i++) {
    if(strcmp(ncst,files[i].c_str()) == 0) {
      fileIndex = i;
      break;
    }
  }

  if(fileIndex == -1) {
    consolePrintError(consoleGetError(ConStatus::FILE_READ_ERROR),
      "/models/"+name);
    return false;
  }

  objl::Loader loadObj;
  std::string loadPath = "./models/" + name + ".obj";

  if(!loadObj.LoadFile(loadPath)) {
    consolePrintError(consoleGetError(ConStatus::OBJ_LOAD_ERROR), loadPath);
    return false;
  }

  std::string msg = "Successfully loaded ";
  msg.append(name);
  consolePrintLine(msg);

  ObjModel tempObj = ObjModel(name, 
    convertMeshes(loadObj.LoadedMeshes), 
    convertVertices(loadObj.LoadedVertices),
    loadObj.LoadedIndices, 
    convertMaterials(loadObj.LoadedMaterials));

  objStore.push_back(tempObj);
  return true;
}

bool tryPrintObjData(std::string name) {
  if(objStore.size() == 0) {
    consolePrintLine("No .obj files in obj store");
    return false;
  }

  int index = getObjIndex(name);

  if(index == OBJ_LOOKUP_FAILED) {
    consolePrintLine("Could not find " + name + " in obj store");
    return false;
  }

  std::vector<ObjVertex> verts = objStore.data()[index].vertices;
  if(verts.size() == 0) {
    consolePrintLine("No verts in stored .obj");
    return false;
  }

  consolePrintLine(sprintf_o("Number of vertices in %s: %d",name.c_str(),verts.size()));
  consolePrintLine("Vertex info:");

  for(int v = 0; v < verts.size(); v++) {
    std::string pos = sprintf_o("(%d: Pos) %0.3f,%0.3f,%0.3f",
      v,
      verts[v].pos.x,
      verts[v].pos.y,
      verts[v].pos.z);
    consolePrintLine(pos);
    std::string norm = sprintf_o("(%d: Norm) %0.3f,%0.3f,%0.3f",
      v,
      verts[v].norm.x,
      verts[v].norm.y,
      verts[v].norm.z);
    consolePrintLine(norm);
    std::string uv = sprintf_o("(%d: Uv) %0.3f,%0.3f",
      v,
      verts[v].uv.x,
      verts[v].uv.y);
    consolePrintLine(uv);
  }

  std::vector<unsigned int> indices = objStore.data()[index].indices;
  if(indices.size() == 0) {
    consolePrintLine("No indices in stored .obj");
    return false;
  }

  consolePrintLine(sprintf_o("Number of indices in %s: %d",name.c_str(),indices.size()));

  for(int i = 0; i < indices.size(); i++) {
    std::string ind = sprintf_o("%d",indices[i]);
    if(((i+1)%3)==0) {
      consolePrintLine(ind);
    } else {
      ind += ", ";
      consolePrint(ind);
    }
  }

  return true;
}

void importObj(const char* name, item meshNdx, bool buffer) {
  if (name == NULL || streql(name, "(null)")) return;
  objl::Loader loadObj;
  string filename = lookupFile(name, 0);
  if(!loadObj.LoadFile(filename.c_str())) {
    SPDLOG_WARN("Could not load mesh {}", filename);
    return;
  }

  ObjModel obj = ObjModel(filename,
    convertMeshes(loadObj.LoadedMeshes),
    convertVertices(loadObj.LoadedVertices),
    loadObj.LoadedIndices,
    convertMaterials(loadObj.LoadedMaterials));

  Mesh* mesh = getMesh(meshNdx);
  if(mesh == 0) return;

  std::vector<ObjVertex> verts = obj.vertices;
  std::vector<unsigned int> indices = obj.indices;
  int numTris = indices.size()/3;

  reserveMesh(meshNdx, numTris);

  for(int i = 0; i < numTris; i++) {
    Triangle tri;

    for(int j = 0; j < 3; j++) {
      RenderPoint p;
      ObjVertex vert = verts[indices[i*3+j]];
      p.point = vec3(vert.pos.x, vert.pos.z, vert.pos.y); // reverse y and z
      p.normal = vec3(vert.norm.x, vert.norm.z, vert.norm.y); // " "
      p.texture = vec3(vert.uv.x, 1-vert.uv.y, 0.0f); // y is inverted
      tri.points[2-j] = p; // reverse winding for backface culling
    }

    pushTriangle(mesh, tri);
  }

  if (buffer) {
    bufferMesh(meshNdx);
  }
}

void importObj(const char* name, item meshNdx) {
  importObj(name, meshNdx, true);
}

bool renderObj(ObjModel* obj, Shader shader, int tex, vec3 pos, int size){
  meshNdxTemp = addMesh();
  Mesh* mesh = getMesh(meshNdxTemp);

  if(mesh == 0) {
    consolePrintError(consoleGetError(ConStatus::NULL_PTR),"Mesh ptr in renderObj");
    return false;
  }

  std::vector<ObjVertex> verts = obj->vertices;
  std::vector<unsigned int> indices = obj->indices;
  
  int numTris = indices.size()/3;
  consolePrintLine(sprintf_o("%s: %d", "numTris", numTris));

  reserveMesh(meshNdxTemp, numTris);

  RenderPoint p0, p1, p2;
  Triangle tri;
  for(int i = 0; i < numTris; i++) {
    
    p0.point = verts[indices[(i*3)]].pos;
    p0.normal = verts[indices[(i*3)]].norm;
    p0.texture = vec3(verts[indices[(i*3)]].uv, 0.0f);
    
    p1.point = verts[indices[(i*3)+1]].pos;
    p1.normal = verts[indices[(i*3)+1]].norm;
    p1.texture = vec3(verts[indices[(i*3)+1]].uv, 0.0f);

    p2.point = verts[indices[(i*3)+2]].pos;
    p2.normal = verts[indices[(i*3)+2]].norm;
    p2.texture = vec3(verts[indices[(i*3)+2]].uv, 0.0f);

    tri.points[0] = p0;
    tri.points[1] = p1;
    tri.points[2] = p2;

    pushTriangle(mesh, tri);
  }

  bufferMesh(meshNdxTemp);

  entNdxTemp = addEntity(shader);
  Entity* ent = getEntity(entNdxTemp);
  ent->mesh = meshNdxTemp;
  ent->simpleMesh = meshNdxTemp;
  ent->texture = tex;
  Camera cam = getMainCamera();
  vec3 dir = normalize(cam.direction);
  vec3 loc = vec3(cam.target); // + dir * float(20. - cam.distance);
  loc.z += 10;
  placeEntity(entNdxTemp, loc, 0.0f, pi_o*.5f, size);

  obj->meshNdx = meshNdxTemp;
  obj->entNdx = entNdxTemp;

  consolePrintLine(sprintf_o("%s", "Successfully rendered .obj"));
  return true;
}

bool renderObj(std::string name) {
  int index = getObjIndex(name);

  if(index == OBJ_LOOKUP_FAILED) {
    consolePrintLine("Could not find " + name + " in obj store");
    return false;
  }

  return renderObj(&objStore[index], RoadShader, grayTexture,
      vec3(0,0,-20), 100);
}

bool tryUnloadObj(std::string name, bool rmvEnt) {
  int index = getObjIndex(name);

  if(index == OBJ_LOOKUP_FAILED) {
    consolePrintLine("Could not find " + name + " in obj store");
    return false;
  }

  ObjModel* obj = &objStore[index];

  if(!obj->isRendered()) {
    consolePrintLine("Obj " + obj->name + " is not rendered; can't unload");
    return false;
  }

  removeMesh(obj->meshNdx);
  if(obj->hasEntity() && rmvEnt) {
    removeEntity(obj->entNdx);
  }

  return true;
}
