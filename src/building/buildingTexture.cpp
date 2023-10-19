#include "buildingTexture.hpp"

#include "building.hpp"
#include "design.hpp"
#include "renderBuilding.hpp"

#include "../cup.hpp"
#include "../draw/entity.hpp"
#include "../draw/texture.hpp"
#include "../game/game.hpp"
#include "../platform/file.hpp"
#include "../platform/lookup.hpp"
#include "../string_proxy.hpp"
#include "../time.hpp"
#include "../zone.hpp"

#include "spdlog/spdlog.h"
#include "string.h"
#include <unordered_map>

enum BTexType {
  BTexUnknown, BTexAlbedo, BTexIllumination, BTexNormal,
  numBTexTypes
};

struct BuildingTexture {
  uint32_t flags = 0;
  char* file = 0;
  char* filepath = 0;
  char* illumGroup = 0;
  char* normGroup = 0;
  uint8_t type = BTexUnknown;
  uint8_t minValue = 0;
  uint8_t maxValue = 255;
  uint8_t minDensity = 0;
  uint8_t maxDensity = 255;
  float minHeight = -FLT_MAX;
  float maxHeight = FLT_MAX;
  uint8_t illumValue = 0;
  item minYear = INT_MIN;
  item maxYear = INT_MAX;
  item textureNdx = buildingTexture;
  vector<item> illumTextures;
  vector<item> illumTextureID;
  item normalTex = 0;
  item glID = 0;
  vector<uint8_t> zones;
  vector<char*> designName;
};

const uint32_t _buildingTextureUniversity = 1 << 1;
const uint32_t _buildingTextureInvalid = 1 << 2;

Cup<BuildingTexture> buildingTextures;
vector<item> buildingTexTable[numZoneTypes];
vector<item> buildingsToPaint;
vector<vector<item>> matchingTextures;
static unordered_map<std::string, vector<item>> illumGroups;
static unordered_map<std::string, item> normalGroups;
static float lightLevel = 2;
static bool hasUniDesigns = false;

void resetBuildingTextures() {
  for (int i = 0; i < buildingTextures.size(); i++) {
    BuildingTexture* tex = buildingTextures.get(i);
    if (tex->file != 0) free(tex->file);
    if (tex->filepath != 0) free(tex->filepath);
    if (tex->illumGroup != 0) free(tex->illumGroup);
    if (tex->normGroup != 0) free(tex->normGroup);
    for (int j = 0; j < tex->designName.size(); j++) {
      if (tex->designName[j] != 0) free(tex->designName[j]);
    }
    tex->designName.clear();

    vector<item> swap;
    tex->illumTextures.swap(swap);
  }

  for (int i = 0; i < numZoneTypes; i++) {
    buildingTexTable[i].clear();
  }

  hasUniDesigns = false;
  buildingTextures.clear();
  buildingsToPaint.clear();
  illumGroups.clear();
  normalGroups.clear();
  matchingTextures.clear();
  lightLevel = 2;
}

item numBuildingTextures() {
  return buildingTextures.size();
}

void softResetBuildingTextures() {
  buildingsToPaint.clear();
  matchingTextures.clear();
  lightLevel = 2;
}

item getDrawTextureForBuildingTexture(item ndx) {
  BuildingTexture* tex = buildingTextures.get(ndx);
  return tex->textureNdx;
}

bool isBuildingTextureIllumination(item ndx) {
  BuildingTexture* tex = buildingTextures.get(ndx);
  return tex->type == BTexIllumination;
}

void setBuildingTextureLightLevel(float l) {
  lightLevel = l;
}

float getBuildingTextureLightLevel() {
  return lightLevel;
}

void loadBuildingTextures() {
  buildingTextures.resize(1); // reserve zero for null

  vector<string> files = lookupDirectory("textures/buildings", ".png",
    c(CDisableDefaultBuildingTextures) ? _lookupExcludeBase : 0);
  for (int i = 0; i < files.size(); i ++) {
    BuildingTexture tex;
    tex.file = strdup_s(files[i].c_str());

    char* conditions = strdup_s(tex.file);
    char* condptr = NULL;
    char* condition = strtok_r(conditions, "_", &condptr);

    while (condition != NULL) {
      char* dup = strdup_s(condition);
      char* typeptr = NULL;
      char* type = strtok_r(dup, "-", &typeptr);
      char* param = strtok_r(NULL, "-", &typeptr);
      int paramVal = param ? strtol(param, NULL, 10) : -1;

      if (streql(type, "residential")) {
        tex.zones.push_back(ResidentialZone);
      } else if (streql(type, "retail")) {
        tex.zones.push_back(RetailZone);
      } else if (streql(type, "farm")) {
        tex.zones.push_back(FarmZone);
      } else if (streql(type, "government")) {
        tex.zones.push_back(GovernmentZone);
      } else if (streql(type, "mixeduse")) {
        tex.zones.push_back(MixedUseZone);
      } else if (streql(type, "office")) {
        tex.zones.push_back(OfficeZone);
      } else if (streql(type, "industrial")) {
        tex.zones.push_back(FactoryZone);

      } else if (streql(type, "university")) {
        tex.zones.push_back(GovernmentZone);
        tex.flags |= _buildingTextureUniversity;
        hasUniDesigns = true;

      } else if (streql(type, "minvalue")) {
        tex.minValue = paramVal;
      } else if (streql(type, "mindensity")) {
        tex.minDensity = paramVal;
      } else if (streql(type, "maxvalue")) {
        tex.maxValue = paramVal;
      } else if (streql(type, "maxdensity")) {
        tex.maxDensity = paramVal;
      } else if (streql(type, "minfloors")) {
        tex.minHeight = paramVal*4;
      } else if (streql(type, "maxfloors")) {
        tex.maxHeight = paramVal*4;
      } else if (streql(type, "minyear")) {
        tex.minYear = paramVal;
      } else if (streql(type, "maxyear")) {
        tex.maxYear = paramVal;

      } else if (streql(type, "illum")) {
        tex.illumGroup = strdup_s(param);
      } else if (streql(type, "norm")) {
        tex.normGroup = strdup_s(param);
      } else if (streql(type, "design")) {
        tex.designName.push_back(strdup_s(param));

      } else if (streql(type, "illumination")) {
        tex.type = BTexIllumination;
        tex.illumValue = paramVal;
      } else if (streql(type, "albedo")) {
        tex.type = BTexAlbedo;
      } else if (streql(type, "normal")) {
        tex.type = BTexNormal;
      }

      condition = strtok_r(NULL, "_", &condptr);
      free(dup);
    }

    free(conditions);

    if (tex.illumGroup && tex.normGroup) {
      char* temp = tex.normGroup;
      char* res = sprintf_o("%s_%s", tex.normGroup, tex.illumGroup);
      free(temp);
      tex.normGroup = res;
    }

    char* fn = sprintf_o("textures/buildings/%s.png", tex.file);
    tex.filepath = strdup_s(lookupFile(fn, 0).c_str());
    tex.glID = loadImage(tex.filepath, c(CBuildingTextureFiltering),
        GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE);
    tex.textureNdx = makeTexture(tex.glID, 0, fn, 0);
    free(fn);

    item ndx = buildingTextures.size();
    buildingTextures.push_back(tex);
    if (tex.type == BTexAlbedo) {
      if (tex.zones.size() == 0) {
        for (int k = 0; k < numZoneTypes; k++) {
          tex.zones.push_back(k);
        }
      }
      for (int k = 0; k < tex.zones.size(); k++) {
        buildingTexTable[tex.zones[k]].push_back(ndx);
      }

    } else if (tex.type == BTexIllumination) {
      vector<item> group;
      std::string str(tex.illumGroup);
      auto result = illumGroups.find(str);
      if (result != illumGroups.end()) group = result->second;

      group.resize(tex.illumValue+1);
      group[tex.illumValue] = ndx;
      illumGroups[str] = group;

    } else if (tex.type == BTexNormal) {
      normalGroups[tex.normGroup] = ndx;
    }
  }

  // Assign everything to albedos
  for (int i = 1; i < buildingTextures.size(); i++) {
    BuildingTexture* tex = buildingTextures.get(i);
    tex->illumTextures.clear();
    tex->illumTextureID.clear();
    if (tex->type != BTexAlbedo) continue;

    if (tex->illumGroup != 0) {
      tex->illumTextures = illumGroups[tex->illumGroup];
    }

    if (tex->normGroup != 0) {
      tex->normalTex = normalGroups[tex->normGroup];
    }
  }
}

bool doesTextureMatchDesign(item designNdx, item texNdx) {
  Design* d = getDesign(designNdx);
  bool designIsUni = d->category == UniversityCategory;
  float designZ = getDesignZ(designNdx);
  BuildingTexture* tex = buildingTextures.get(texNdx);
  bool texIsUni = tex->flags & _buildingTextureUniversity;
  if (hasUniDesigns && (texIsUni != designIsUni)) return false;

  if (tex->designName.size() > 0) {
    if (d->name == 0) return false;
    bool validDesignName = false;
    for (int j = 0; j < tex->designName.size(); j++) {
      if (strstr(d->name, tex->designName[j]) != 0) {
        validDesignName = true;
        break;
      }
    }
    if (!validDesignName) {
      return false;
    }
  }

  return float(tex->minValue) <= d->minLandValue*10+0.1 &&
      float(tex->maxValue) >= d->minLandValue*10-0.1 &&
      float(tex->minDensity) <= d->minDensity*10+0.1 &&
      float(tex->maxDensity) >= d->minDensity*10-0.1 &&
      tex->minHeight <= designZ &&
      tex->maxHeight >= designZ;
}

bool doesTextureMatchBuilding(item buildingNdx, item texNdx) {
  Building* b = getBuilding(buildingNdx);
  BuildingTexture* tex = buildingTextures.get(texNdx);
  if (tex->flags & _buildingTextureInvalid) return false;

  item buildingYear = b->builtTime / oneYear + c(CStartYear);
  if (tex->minYear > buildingYear) return false;
  if (tex->maxYear < buildingYear) return false;

  return true;
}

void findDesignTextures(item designNdx) {
  vector<item> allMatches;
  vector<item> nameMatches;
  Design* d = getDesign(designNdx);
  item zone = d->zone;
  int size = buildingTexTable[zone].size();

  for (int i = 0; i < size; i++) {
    item texNdx = buildingTexTable[zone][i];
    if (doesTextureMatchDesign(designNdx, texNdx)) {
      allMatches.push_back(texNdx);
      BuildingTexture* tex = buildingTextures.get(texNdx);
      if (tex->designName.size() > 0) {
        nameMatches.push_back(texNdx);
      }
    }
  }

  if (nameMatches.size() > 0) {
    allMatches = nameMatches;
  }

  //SPDLOG_INFO("{}/{} matching textures for design #{} {}",
      //allMatches.size(), size, designNdx, d->name);
  if (allMatches.size() == 0) {
    allMatches.push_back(-1);
  }

  if (matchingTextures.size() <= designNdx) {
    matchingTextures.resize(designNdx*2);
  }
  matchingTextures[designNdx] = allMatches;
}

item getBuildingTexture(item ndx, bool isSwap) {
  Building* b = getBuilding(ndx);
  bool designer = getGameMode() == ModeBuildingDesigner;
  bool organizer = getGameMode() == ModeDesignOrganizer;
  if (matchingTextures.size() <= b->design ||
      matchingTextures[b->design].size() == 0) {
    findDesignTextures(b->design);
  }
  vector<item> matches = matchingTextures[b->design];

  item result = 0;
  item matchesS = matches.size();
  if (matchesS > 0) {
    // Find the albedo
    item color = designer ? b->color : ndx;
    item rnd = color;
    for (int i = 0; i < matchesS; i++) {
      item texNdx = matches[(rnd+i)%matchesS];
      if (texNdx < 0) continue;
      if (doesTextureMatchBuilding(ndx, texNdx)) {
        result = texNdx;
        break;
      }
    }
  }

  if (result == 0) return result;

  BuildingTexture* tex = buildingTextures.get(result);
  item numIllumTextures = tex->illumTextures.size();
  float targetIllum = getTargetIllumLevel(ndx);
  targetIllum = clamp(targetIllum, 0.f, float(numIllumTextures-1));

  //SPDLOG_INFO("{} illum{} numIllum{}",
      //ndx, targetIllum, numIllumTextures);

  if (targetIllum == 0 || numIllumTextures == 0) {
    // No Illumination
    if (tex->textureNdx == buildingTexture || tex->textureNdx == 0) {
      if (isSwap) {
        tex->textureNdx = makeTexture(tex->glID, 0, tex->filepath, 0);
      } else {
        buildingsToPaint.push_back(ndx);
      }
    }
    //SPDLOG_INFO("{} texNdx{} glID{}",
     //   ndx, tex->textureNdx, tex->glID);
    return tex->textureNdx;

  } else {
    // Find best illumination
    item best = 0;
    float bestDist = 10000;
    item bestNdx = 0;
    for (int i = 0; i < numIllumTextures; i++) {
      float dist = abs(i-targetIllum);
      if (dist < bestDist && tex->illumTextures[i] != 0) {
        best = i;
        bestDist = dist;
        bestNdx = tex->illumTextures[i];
      }
    }

    if (bestNdx == 0) return tex->textureNdx;

    //SPDLOG_INFO("{} target{} num{} best{} bestDist{} bestNdx{}",
        //ndx, targetIllum, numIllumTextures, best, bestDist, bestNdx);

    // Assign illumination texID
    int iTID_S = tex->illumTextureID.size();
    item texID = best >= iTID_S ? 0 : tex->illumTextureID[best];
    //SPDLOG_INFO("{} iTID_S{} best{} texID{}", ndx, iTID_S, best, texID);
    if (texID == 0) {
      if (isSwap) {
        BuildingTexture* illumTex = buildingTextures.get(bestNdx);
        texID = makeTexture(tex->glID, illumTex->glID, tex->filepath, illumTex->filepath);
        if (best >= iTID_S) tex->illumTextureID.resize(best+1, 0);
        tex->illumTextureID[best] = texID;
      } else {
        buildingsToPaint.push_back(ndx);
      }
    }

    return texID;
  }
}

item getBuildingTexture(item ndx) { return getBuildingTexture(ndx, false); }

item numMatchingTextures(item ndx) {
  Building* b = getBuilding(ndx);
  bool designer = getGameMode() == ModeBuildingDesigner;
  bool organizer = getGameMode() == ModeDesignOrganizer;
  if (matchingTextures.size() <= b->design ||
      matchingTextures[b->design].size() == 0) {
    findDesignTextures(b->design);
  }
  vector<item> matches = matchingTextures[b->design];

  item result = 0;
  item matchesS = matches.size();
  item numMatches = 0;
  for (int i = 0; i < matchesS; i++) {
    item texNdx = matches[i];
    if (texNdx < 0) continue;
    if (doesTextureMatchBuilding(ndx, texNdx)) {
      numMatches ++;
    }
  }
  return numMatches;
}

void swapBuildingTextures() {
  for (int i = 0; i < buildingsToPaint.size(); i++) {
    item ndx = buildingsToPaint[i];
    Building* b = getBuilding(ndx);
    if (b->entity == 0) continue;
    Entity* e = getEntity(b->entity);
    e->texture = getBuildingTexture(ndx, true);
  }

  buildingsToPaint.clear();
}

void deleteBuildingTexture(item buildingNdx, bool illum) {
  Building* b = getBuilding(buildingNdx);
  Entity* e = getEntity(b->entity);
  Texture* t = getTexture(e->texture);

  item glId = !illum ? t->textureID : t->illuminationTextureID;

  for (int i = 0; i < buildingTextures.size(); i++) {
    BuildingTexture* tex = buildingTextures.get(i);
    if (tex->glID == glId) {
      tex->flags |= _buildingTextureInvalid;
    }

    if (illum) {
      for (int k = 0; k < tex->illumTextureID.size(); k++) {
        if (tex->illumTextureID[k] == 0) continue;
        Texture* illumTex = getTexture(tex->illumTextureID[k]);
        if (illumTex->illuminationTextureID == glId) {
          tex->illumTextureID[k] = 0;
          tex->illumTextures[k] = 0;
        }
      }
    }
  }

  paintBuilding(buildingNdx);
}

