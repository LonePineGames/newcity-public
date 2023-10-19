#pragma once

#include "../platform/lua.hpp"

const uint32_t _meshImportComplete = 1 << 1;
const uint32_t _meshImportDontBuffer = 1 << 2;
const uint32_t _meshImportBillboard = 1 << 3;
const uint32_t _meshImportSimpleBillboard = 1 << 4;

struct MeshImport {
  uint32_t flags = 0;
  item mesh = 0;
  item simpleMesh = 0;
  item texture = 0;
  char* objFile = 0;
  char* objFileSimple = 0;
  char* textureFile = 0;
  char* illumTextureFile = 0;
};

MeshImport* getMeshImport(item ndx);
void assignMeshImport(item entityNdx, item meshImportNdx);
void loadMeshImport(item ndx);
item readMeshImportData(lua_State* L, bool buffer);
void swapMeshImports();
void swapMeshImports(bool force);
void resetMeshImports();

