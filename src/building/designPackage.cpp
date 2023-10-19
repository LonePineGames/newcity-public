#include "designPackage.hpp"

#include "building.hpp"
#include "design.hpp"
#include "buildingTexture.hpp"
#include "renderBuilding.hpp"

#include "../draw/texture.hpp"
#include "../draw/entity.hpp"
#include "../game/game.hpp"
#include "../parts/mainMenu.hpp"
#include "../pool.hpp"
#include "../platform/file.hpp"
#include "../platform/lookup.hpp"
#include "../string_proxy.hpp"
#include "../util.hpp"

#include "spdlog/spdlog.h"

struct DesignPackage {
  Cup<item> albedoTextures;
  Cup<item> illumTextures;
};

Cup<DesignPackage> designPackages;
vector<item> packageTexturesAlbedo;
vector<item> packageTexturesIllum;

void resetDesignPackages() {
  for (int i = 0; i < designPackages.size(); i++) {
    DesignPackage* package = designPackages.get(i);
    package->albedoTextures.clear();
    package->illumTextures.clear();
  }
  designPackages.clear();
  packageTexturesAlbedo.clear();
  packageTexturesIllum.clear();
}

void linkDesignPackage(item sourceNdx, item destNdx) {
  item packageS = designPackages.size();
  item max = sourceNdx > destNdx ? sourceNdx : destNdx;
  if (max >= packageS) designPackages.resize(max+1);

  DesignPackage* source = designPackages.get(sourceNdx);
  DesignPackage* dest = designPackages.get(destNdx);
  source->albedoTextures.concat(&dest->albedoTextures);
  source->illumTextures.concat(&dest->illumTextures);
  dest->albedoTextures.copy(&source->albedoTextures);
  dest->illumTextures.copy(&source->illumTextures);
}

void convertDesignToPackage(item designNdx) {
  DesignPackage* package = designPackages.get(designNdx);
  package->albedoTextures.clear();
  package->illumTextures.clear();
}

bool removeEnd(char* txt, const char* end) {
  if (!endsWith(txt, end)) return false;

  int txtLngth = strlength(txt);
  int endLngth = strlength(end);
  txt[txtLngth - endLngth] = '\0';
  return true;
}

string suggestTextureFilename(const char* designName, const char* textureFile, bool isIllum) {
  /*
  char* textureFileEnd = textureFile;
  item strLength = strlength(textureFile);
  for (int i = strLength-1; i >= 0; i--) {
    if (textureFile[i] == '/') {
      textureFileEnd = &textureFile[i+1];
      break;
    }
  }

  removeEnd(textureFileEnd, ".png");
  removeEnd(textureFileEnd, ".albedo");
  removeEnd(textureFileEnd, ".illum");
  */

  string filePathStart = "designs/";
  filePathStart = filePathStart + designName + "/"; // + textureFileEnd + ".";
  string filePathEnd = isIllum ? ".illum" : ".albedo";
  filePathEnd = filePathEnd + ".png";

  for (int i = 0; ; i++) {
    char* file = sprintf_o("%s%02d%s", filePathStart.c_str(), i, filePathEnd.c_str());

    string canon = lookupFile(file, _lookupForceMod);
    if (!fileExists(canon)) {
      return lookupSave(file);
    }

    free(file);
  }
}

void addTextureToPackage(item designNdx, item texture, bool isIllum) {
  if (designNdx >= designPackages.size()) designPackages.resize(designNdx+1);
  DesignPackage* package = designPackages.get(designNdx);
  Texture* tex = getTexture(texture);
  Design* design = getDesign(designNdx);
  if (tex->filename == 0) return;
  if (design->name == 0) return;

  string suggestion = suggestTextureFilename(design->name != 0 ? design->name : "", tex->filename, isIllum);
  string source = tex->filename;
  copyFile(source, suggestion);

  item newTexture = getTexture(suggestion.c_str());
  if (!isIllum) {
    package->albedoTextures.push_back(newTexture);
  } else {
    package->illumTextures.push_back(newTexture);
  }
}

void deleteTextureInPackage(item designNdx, item texture) {
  if (designNdx >= designPackages.size()) return;
  DesignPackage* package = designPackages.get(designNdx);

  package->albedoTextures.removeByValue(texture);
  package->illumTextures.removeByValue(texture);

  Texture* tex = getTexture(texture);
  string file = tex->filename;
  if (stringContainsCaseInsensitive(file, "design/")) {
    deleteFile(file);
  }
}

void assignBuildingTexture(item buildingNdx) {
  Building* b = getBuilding(buildingNdx);
  Entity* e = getEntity(b->entity);
  if (designPackages.size() > b->design) {
    DesignPackage* package = designPackages.get(b->design);

    if (package->albedoTextures.size() > 0) {
      bool designer = getGameMode() == ModeBuildingDesigner;
      item texI = designer ? b->color : buildingNdx;
      texI = texI % package->albedoTextures.size();

      item albedoNdx = package->albedoTextures[texI];
      e->texture = albedoNdx;

      item illumS = package->illumTextures.size();
      if (illumS > 0) {
        float targetIllum = getTargetIllumLevel(buildingNdx);
        targetIllum = round(clamp(targetIllum, 0.f, float(illumS-1)));

        item illumNdx = package->illumTextures[targetIllum];
        Texture* albedoTex = getTexture(albedoNdx);
        Texture* illumTex = getTexture(illumNdx);
        e->texture = makeTexture(albedoTex->textureID, illumTex->textureID, albedoTex->filename, illumTex->filename);
      }

      return;
    }
  }

  // fallback to legacy building textures system
  e->texture = getBuildingTexture(buildingNdx);
}

item numMatchingTexturesForBuilding(item buildingNdx) {
  Building* b = getBuilding(buildingNdx);
  if (designPackages.size() > b->design) {
    DesignPackage* package = designPackages.get(b->design);
    item texS = package->albedoTextures.size();
    if (texS > 0) {
      return texS;
    }
  }

  // fallback to legacy building textures system
  return numMatchingTextures(buildingNdx);
}

vector<item> getAllPackageTextures(bool illum) {
  return illum ? packageTexturesIllum : packageTexturesAlbedo;
}

void loadDesignPackageTextures(string packageName, item designNdx, uint32_t loadFlags) {
  vector<string> images = lookupDirectory("designs/" + packageName, ".png", loadFlags);
  if (designNdx >= designPackages.size()) designPackages.resize(designNdx+1);
  DesignPackage* package = designNdx <= 0 ? 0 : designPackages.get(designNdx);

  for (int k = 0; k < images.size(); k++) {
    bool albedo = endsWith(images[k].c_str(), ".albedo");
    bool illum = endsWith(images[k].c_str(), ".illum");
    if (!albedo && !illum) continue;

    string fullPath = "designs/" + packageName + "/" + images[k] + ".png";
    item tex = getTexture(fullPath.c_str());
    if (albedo) {
      packageTexturesAlbedo.push_back(tex);
    } else if (illum) {
      packageTexturesIllum.push_back(tex);
    }

    // add to package
    if (package != 0) {
      if (albedo) {
        package->albedoTextures.push_back(tex);
      } else if (illum) {
        package->illumTextures.push_back(tex);
      }
    }
  }

  if (package != 0) {
    //SPDLOG_INFO("loadDesignPackageTextures {} {}/{}", packageName, package->albedoTextures.size(), package->illumTextures.size());
  }
}

void loadDesignPackageTextures() {
  bool designer = getGameMode() == ModeBuildingDesigner;
  uint32_t loadFlags = designer ? getLoadMenuLookupFlags() : 0;
  if (designer) {
    Design* d = getSelectedDesign();
    loadDesignPackageTextures(d->name, getSelectedDesignNdx(), loadFlags);

  } else {
    vector<string> packages = lookupSubDirectories("designs", loadFlags);

    for (int i = 0; i < packages.size(); i++) {
      string packageName = packages[i];

      item designNdx = 0;
      for (int d = 1; d <= sizeDesigns(); d++) {
        Design* design = getDesign(d);
        if (design->name != 0 && streql(design->name, packageName.c_str())) {
          designNdx = d;
          break;
        }
      }
;
      //SPDLOG_INFO("loadDesignPackageTextures {} {} {}", packageName, designNdx, loadFlags);
      loadDesignPackageTextures(packageName, designNdx, loadFlags);
    }
  }
}

void cleanupLegacyDesignFiles() {
  Design* design = getSelectedDesign();

  // Null check both the design ptr and the name ptr
  if (design == 0 || design->name == 0) {
    return;
  }

  string path = "designs/";
  path = path + design->name + ".design";
  deleteFile(lookupFile(path, _lookupForceMod));
  deleteFile(lookupFile(path + ".png", _lookupForceMod));
}

void deleteDesign(item ndx) {
  if (ndx <= 0 || ndx > sizeDesigns()) return;
  Design* design = getDesign(ndx);
  if (design == 0 || design->name == 0) return;

  if (designPackages.size() > ndx) {
    DesignPackage* package = designPackages.get(ndx);
    package->albedoTextures.clear();
    package->illumTextures.clear();
  }

  string path = "designs/";
  path = path + design->name;

  while (true) {
    string file = lookupFile(path + ".design", _lookupForceMod);
    if (!fileExists(file)) break;
    deleteFile(file);
  }

  while (true) {
    string file = lookupFile(path + ".design.png", _lookupForceMod);
    if (!fileExists(file)) break;
    deleteFile(file);
  }

  while (true) {
    string file = lookupFile(path, _lookupForceMod);
    if (!fileExists(file)) break;
    deleteDir(file);
  }
}

void deleteDesignPackageTexture(item buildingNdx, bool illum) {
  Building* b = getBuilding(buildingNdx);
  int matches = 0;

  if (designPackages.size() > b->design) {
    DesignPackage* package = designPackages.get(b->design);
    Entity* e = getEntity(b->entity);
    Texture* tex = getTexture(e->texture);
    item glId = !illum ? tex->textureID : tex->illuminationTextureID;

    Cup<item>* list = !illum ? &package->albedoTextures : &package->illumTextures;
    for (int i = 0; i < list->size(); i++) {
      Texture* tex = getTexture(list->at(i));
      if (tex->textureID == glId) {
        list->remove(i);
        matches++;
        i --;
      }
    }
  }

  paintBuilding(buildingNdx);
}

bool designHasPackageTextures(item designNdx) {
  if (designPackages.size() > designNdx) {
    DesignPackage* package = designPackages.get(designNdx);
    return package->albedoTextures.size() > 0;
  }
  return false;
}

void designPackageResaved() {
  cleanupLegacyDesignFiles();

  item dNdx = getSelectedDesignNdx();
  DesignPackage* package = designPackages.get(dNdx);
  vector<item> albs = package->albedoTextures.toVector();
  vector<item> illums = package->illumTextures.toVector();
  package->albedoTextures.clear();
  package->illumTextures.clear();

  for (int isIllum = 0; isIllum < 2; isIllum++)  {
    vector<item> textures = isIllum ? illums : albs;
    for (int k = 0; k < textures.size(); k++) {
      addTextureToPackage(dNdx, textures[k], isIllum);
    }
  }

  assignBuildingTexture(getSelectedDesignNdx());
}

