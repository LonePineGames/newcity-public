#pragma once

#include "../console/conDisplay.hpp"
#include "../console/conInput.hpp"
#include "../draw/entity.hpp"
#include "../draw/mesh.hpp"
#include "../draw/shader.hpp"
#include "../draw/texture.hpp"
#include "../platform/file.hpp"

#include "../item.hpp"
#include "../renderUtils.hpp"
#include "../string.hpp"
#include "../string_proxy.hpp"

// Taken from OBJ_Loader.hpp and adapted for our purposes
struct ObjMaterial
{
  ObjMaterial()
  {
    name;
    Ns = 0.0f;
    Ni = 0.0f;
    d = 0.0f;
    illum = 0.0f;
  }

  // Material Name
  std::string name;
  // Ambient Color
  vec3 Ka;
  // Diffuse Color
  vec3 Kd;
  // Specular Color
  vec3 Ks;
  // Specular Exponent
  float Ns;
  // Optical Density
  float Ni;
  // Dissolve
  float d;
  // Illumination
  int illum;
  // Ambient Texture Map
  std::string map_Ka;
  // Diffuse Texture Map
  std::string map_Kd;
  // Specular Texture Map
  std::string map_Ks;
  // Specular Hightlight Map
  std::string map_Ns;
  // Alpha Texture Map
  std::string map_d;
  // Bump Map
  std::string map_bump;
};

// Modeled after objl::Vertex, but with our types
struct ObjVertex {
  vec3 pos;
  vec3 norm;
  vec2 uv;

  ObjVertex(vec3 p,vec3 n,vec2 u) {
    pos = p;
    norm = n;
    uv = u;
  }
};

// Represents a mesh, including a unique mesh name, vertices, and indices
struct ObjMesh {
  std::string name;
  std::vector<ObjVertex> vertices;
  std::vector<unsigned int> indices;
  ObjMaterial material;

  ObjMesh(std::string n,
    std::vector<ObjVertex> v,
    std::vector<unsigned int> i,
    ObjMaterial mat = ObjMaterial()) {
    name = n;
    vertices = v;
    indices = i;
    material = mat;
  }
};

// Represents an .obj model
struct ObjModel {
  std::string name;
  std::vector<ObjMesh> submeshes;
  std::vector<ObjVertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<ObjMaterial> materials;
  item meshNdx;
  item entNdx;

  ObjModel(std::string n,
    std::vector<ObjMesh> meshNew,
    std::vector<ObjVertex> vNew,
    std::vector<unsigned int> iNew,
    std::vector<ObjMaterial> matNew,
    item mn = -1,
    item en = -1) {
    name = n;
    submeshes = meshNew;
    vertices = vNew;
    indices = iNew;
    materials = matNew;
    meshNdx = mn;
    entNdx = en;
  }

  // Currently only checks to see if something loaded
  // TODO: Update with logic to verify the loaded data
  bool isValid() {
    return submeshes.size() > 0
      && vertices.size() > 0
      && indices.size() > 0
      && materials.size() > 0;
  }

  bool isRendered() {
    return meshNdx != -1;
  }

  bool hasEntity() {
    return entNdx != -1;
  }
};

bool clearObjStore();
bool loadObjFile(std::string name);
bool tryPrintObjData(std::string name);
bool renderObj(ObjModel* obj, Shader shader, int tex, vec3 pos, int size);
bool renderObj(std::string name);
bool tryUnloadObj(std::string name, bool rmvEnt = true);
void importObj(const char* name, item meshNdx);
void importObj(const char* name, item meshNdx, bool buffer);
