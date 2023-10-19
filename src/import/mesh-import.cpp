#include "mesh-import.hpp"

#include "obj.hpp"

#include "../draw/entity.hpp"
#include "../draw/texture.hpp"
#include "../platform/lookup.hpp"
#include "../pool.hpp"

Pool<MeshImport> imports;
vector<item> meshImportsToLoad;

void resetMeshImports() {
  SPDLOG_INFO("resetMeshImports");
  for (int i = 1; i <= imports.size(); i++) {
    MeshImport* import = imports.get(i);
    if (import->objFile != 0) free(import->objFile);
    if (import->objFileSimple != 0) free(import->objFileSimple);
    if (import->textureFile != 0) free(import->textureFile);
    if (import->illumTextureFile != 0) free(import->illumTextureFile);
  }

  imports.clear();
  meshImportsToLoad.clear();
}

MeshImport* getMeshImport(item ndx) {
  return imports.get(ndx);
}

void loadMeshImport(item ndx) {
  if (isGameLoading() && !isGameWaitingForSwap()) return;

  MeshImport* import = imports.get(ndx);

  if (import == 0)
    return;

  if (import->flags & _meshImportComplete)
    return;

  if (import->mesh == 0) {
    import->mesh = addMesh();

    if (import->flags & _meshImportBillboard) {
      Mesh* mesh = getMesh(import->mesh);
      mesh->flags |= _entityBillboard;
    }
  }

  if (import->simpleMesh == 0 && import->objFileSimple != 0) {
    import->simpleMesh = addMesh();

    if (import->flags & _meshImportSimpleBillboard) {
      Mesh* mesh = getMesh(import->simpleMesh);
      mesh->flags |= _entityBillboard;
    }
  }

  if (import->objFile != 0) {
    importObj(import->objFile, import->mesh,
        !(import->flags & _meshImportDontBuffer));
  }

  if (import->objFileSimple != 0) {
    importObj(import->objFileSimple, import->simpleMesh,
        !(import->flags & _meshImportDontBuffer));
  }

  if (import->textureFile != 0) {
    import->texture = importTexture(import->textureFile,
        import->illumTextureFile);
  } else {
    import->texture = paletteTexture;
  }

  import->flags |= _meshImportComplete;
}

void swapMeshImports(bool force) {
  if (!force && isGameLoading() && !isGameWaitingForSwap()) return;

  for (int i = 0; i < meshImportsToLoad.size(); i++) {
    item ndx = meshImportsToLoad[i];
    loadMeshImport(ndx);
  }

  meshImportsToLoad.clear();
}

void swapMeshImports() {
  swapMeshImports(false);
}

void assignMeshImport(item entityNdx, item meshImportNdx) {
  if (entityNdx <= 0 || meshImportNdx <= 0) return;

  MeshImport* import = imports.get(meshImportNdx);
  //if (!(import->flags & _meshImportComplete)) {
    //loadMeshImport(meshImportNdx);
  //}

  Entity* entity = getEntity(entityNdx);
  entity->mesh = import->mesh;
  entity->simpleMesh = import->simpleMesh;
  entity->texture = import->texture;
}

char* convertFilenameInMod(char* in) {
  if (in == 0) return 0;
  string result = lookupFile(in, 0);
  free(in);
  return strdup_s(result.c_str());
}

item readMeshImportData(lua_State* L, bool buffer) {
  item ndx = imports.create();
  MeshImport* import = imports.get(ndx);
  import->flags = 0;
  if (!buffer) import->flags |= _meshImportDontBuffer;
  import->objFile = convertFilenameInMod(luaFieldString(L, "mesh"));
  import->objFileSimple = convertFilenameInMod(luaFieldString(L, "simpleMesh"));
  import->textureFile = convertFilenameInMod(luaFieldString(L, "texture"));
  import->illumTextureFile = convertFilenameInMod(
      luaFieldString(L, "illumination"));
  if(luaFieldBool(L, "billboard")) import->flags |= _meshImportBillboard;
  if(luaFieldBool(L, "simpleBillboard")) import->flags |= _meshImportSimpleBillboard;

  meshImportsToLoad.push_back(ndx);
  return ndx;
}

