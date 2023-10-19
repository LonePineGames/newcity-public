#include "design.hpp"

#include "designOrganizer.hpp"
#include "designPackage.hpp"
#include "renderBuilding.hpp"

#include "../business.hpp"
#include "../economy.hpp"
#include "../draw/entity.hpp"
#include "../game/feature.hpp"
#include "../game/game.hpp"
#include "../game/version.hpp"
#include "../graph.hpp"
#include "../error.hpp"
#include "../land.hpp"
#include "../parts/designOrganizerPanel.hpp"
#include "../parts/mainMenu.hpp"
#include "../parts/toolbar.hpp"
#include "../platform/file.hpp"
#include "../platform/lookup.hpp"
#include "../platform/mod.hpp"
#include "../selection.hpp"
#include "../string_proxy.hpp"
#include "../util.hpp"
#include "../vehicle/wanderer.hpp"
#include "../zone.hpp"

#include "spdlog/spdlog.h"
#include <unordered_map>

#include <experimental/filesystem>
using namespace std::experimental::filesystem;

vector<Design> designs;
vector<Design> designerUndoHistory;
vector<item> designerUndoHistoryNdxs;
unordered_map<string, vector<item>> designsByNameMap;
item undoHistoryNdx = 0;
bool structuresVisible = true;

const float landValueMultiplier = 1;

const Effect oldEffectOrder[] = {
  Nature, ValueEffect, DensityEffect, Law,
  Order, EducationEffect, Health, Environmentalism, Prestige,
  Community, Technology, ProsperityEffect, ProsperityEffect,
  Order, Order, Order
};

const char* roofTypeNames[] = {
  "Gable",
  "Hip",
  "Flat",
  "Barrel",
  "Slant",
  "Gambrel",
};

const char* getRoofTypeName(item roofType) {
  if (roofType < 0) roofType = -roofType-1;
  return roofTypeNames[roofType];
}

void copyDesign(Design* dest, Design* source, bool copyStrings);

item getSelectedDesignNdx() {
  if (getGameMode() == ModeDesignOrganizer) {
    return getSelection();
  } else if (getGameMode() == ModeBuildingDesigner) {
    return 1;
  } else if (isFixBuildingMode() && getSelectionType() == SelectionBuilding) {
    Building* b = getBuilding(getSelection());
    return b->design;
  }

  return 0;
}

Design* getSelectedDesign() {
  if (getSelectedDesignNdx() == 0) {
    handleError("getSelectedDesign() with no design selected");
    TRIP;
  }
  return getDesign(getSelectedDesignNdx());
}

void shiftDesignerUndoHistory(item amount) {
  if (designerUndoHistory.size() == 0) {
    undoHistoryNdx = 0;
    return;
  }

  item nextNdx = undoHistoryNdx + amount;
  item undoS = designerUndoHistory.size();
  if(nextNdx < 0) nextNdx = 0;
  if(nextNdx >= undoS) nextNdx = undoS-1;

  if (nextNdx != undoHistoryNdx) {
    undoHistoryNdx = nextNdx;
    Design* d = getSelectedDesign();
    copyDesign(d, &designerUndoHistory[undoHistoryNdx], true);
    designerUndoHistoryNdxs[undoHistoryNdx] = getSelectedDesignNdx();

    if (getSelectionType() == SelectionDeco &&
        d->decos.size() <= getSelection()) {
      clearSelection();
    } else if (getSelectionType() == SelectionStructure &&
        d->structures.size() <= getSelection()) {
      clearSelection();
    }

    designRender();
    readdSpawnPoints(getSelectedDesignNdx());
  }
}

void pushDesignerUndoHistory() {
  item undoS = designerUndoHistory.size();
  if (undoHistoryNdx < undoS) {
    designerUndoHistory.erase(designerUndoHistory.begin()+undoHistoryNdx+1, designerUndoHistory.end());
    designerUndoHistoryNdxs.erase(designerUndoHistoryNdxs.begin()+undoHistoryNdx+1, designerUndoHistoryNdxs.end());
  }

  Design d;
  designerUndoHistory.push_back(d);
  designerUndoHistoryNdxs.push_back(getSelectedDesignNdx());

  undoS = designerUndoHistory.size();
  undoHistoryNdx ++;
  if(undoHistoryNdx < 0) undoHistoryNdx = 0;
  if(undoHistoryNdx >= undoS) undoHistoryNdx = undoS-1;

  copyDesign(&designerUndoHistory[undoHistoryNdx], getDesign(getSelectedDesignNdx()), true);
  readdSpawnPoints(getSelectedDesignNdx());

  if (getGameMode() == ModeDesignOrganizer) {
    updateOrganizerBuilding(getSelectedDesignNdx());
    saveDesign(getSelectedDesignNdx());
  }
}

bool hasDesignerUndoHistory() {
  item undoS = designerUndoHistory.size();
  return undoS > 0 && undoHistoryNdx > 0;
}

bool hasDesignerRedoHistory() {
  item undoS = designerUndoHistory.size();
  return undoS-1 > undoHistoryNdx;
}

item addDesign() {
  item ndx = designs.size() + 1;
  {
    Design d;
    designs.push_back(d);
  }

  Design* design = getDesign(ndx);
  design->name = 0;
  design->zone = ResidentialZone;
  design->size = vec3(1,1,0);
  design->numFamilies = 1;
  for (int i = 0; i < numBusinessTypes; i++) {
    design->numBusinesses[i] = 0;
  }
  design->minYear = 1850;
  design->maxYear = 9999;
  design->numBuildings = 0;
  design->minLandValue = 0;
  design->minDensity = 0;
  design->lowTide = 0;
  design->highTide = 0;
  design->mesh = 0;
  design->simpleMesh = 0;
  design->decoMesh = 0;
  design->simpleDecoMesh = 0;
  design->structures.clear();
  design->decos.clear();
  return ndx;
}

Design* getDesign(item ndx) {
  return &designs[ndx-1];
}

float defaultRoofSlopeForType(item type) {
  if (type < 0) type = -type-1;
  switch (type) {
    case GableRoof:  return 1.f/3.f;
    case HipRoof:    return 1.f/3.f;
    case FlatRoof:   return 1;
    case BarrelRoof: return 1.f/3.f;
    case SlantRoof:  return 0.2f;
    case GambrelRoof:  return 1.f/2.f;
    default: return 1.f/3.f;
  }
}

bool isStructuresVisible() {
  return structuresVisible;
}

void toggleStructuresVisible() {
  structuresVisible = !structuresVisible;
}

vec3 getStructureHandleLocation(item ndx, item handleType) {
  Design* d = getDesign(getSelectedDesignNdx());
  Building* b = getBuilding(getSelectedDesignNdx());
  if (ndx < 0 || ndx >= d->structures.size()) return vec3(0,0,0);
  Structure* s = &d->structures[ndx];
  vec3 loc = b->location + s->location;

  if (handleType == LocationHandle) {
    return loc;

  } else {
    vec3 size = s->size;
    float cangle = cos(s->angle);
    float sangle = sin(s->angle);
    vec3 right = vec3(-cangle, sangle, 0) * size.x * .5f;
    vec3 along = vec3(sangle, cangle, 0) * size.y;

    if (handleType == AngleHandle) {
      return loc + along;

    } else if (handleType == SizeHandle) {
      return loc + right + vec3(0,0,size.z);

    } else if (handleType == SelectionPointHandle) {
      //float slopePart = s->roofType == FlatRoof ? 0 : size.x*.5f*s->roofSlope;
      return loc + along*.5f + vec3(0,0,size.z);

    } else {
      handleError("Bad Handle Type");
      return loc;
    }
  }
}

vec3 getDecoHandleLocation(item ndx, item handleType) {
  Design* d = getDesign(getSelectedDesignNdx());
  Building* b = getBuilding(getSelectedDesignNdx());
  if (ndx < 0 || ndx >= d->decos.size()) return vec3(0,0,0);
  Deco* deco = &d->decos[ndx];
  vec3 loc = b->location + deco->location;

  if (handleType == LocationHandle) {
    return loc;

  } else if (handleType == SelectionPointHandle) {
    return loc + vec3(0,0, deco->scale);

  } else if (handleType == AngleHandle) {
    float cangle = cos(deco->yaw);
    float sangle = sin(deco->yaw);
    vec3 ualong = vec3(cangle, sangle, 0);
    vec3 along = ualong * deco->scale * 10.f;
    return loc - along;

  } else {
    handleError("Bad Handle Type");
    return loc;
  }
}

float landValueDistance(Design* d, float landValue) {
  return d->zone == FactoryZone || d->zone == FarmZone ? 0 :
     abs(d->minLandValue - landValue);
}

item getRandomDesign(item zone, item econ, float density, float landValue) {
  density = round(density*10)/10.f + .01f;
  landValue = round(landValue*10)/10.f + .01f;

  int i = 0;
  float maxLVDDistFactor = 1.f/designs.size();
  while(i < designs.size()*2) {
    i++;
    item ndx = randItem(designs.size()) + 1;
    Design* d = getDesign(ndx);
    if (!(d->flags & _designEnabled)) continue;
    if (d->flags & _designDisableSpawning) continue;
    item year = getCurrentYear();
    if (d->minYear > year) continue;
    if (d->maxYear < year) continue;
    if (d->minDensity > density) continue;
    if (d->minLandValue > landValue) continue;

    float lvdDistance = abs(d->minDensity - density) +
      landValueDistance(d, landValue);
    float maxLVDDist = maxLVDDistFactor*i + 0.1;
    if (lvdDistance > maxLVDDist) continue;

    if (zone == MixedUseZone) {
      if (d->zone == GovernmentZone || d->zone == ParkZone ||
          d->zone == FarmZone || d->zone == FactoryZone) continue;
      float demand = zoneDemand(econ, d->zone);
      if (demand < randFloat()) continue;
    } else if (d->zone != zone) continue;

    // Don't build hotels if there's already empty hotel rooms,
    // or if there's no tourism
    if ((d->flags & _designIsHotel) && (getEffectValue(Tourism) <= 0 ||
        getStatistic(econ, NumEmptyHotelRooms) > d->numFamilies)) {
      continue;
    }

    bool bad = false;
    item numBiz = 0;
    for (int j = 0; j < numBusinessTypes; j++) {
      numBiz += d->numBusinesses[j];
      //SPDLOG_INFO("getRandomDesign {} biz:{} econ:{} getBusinessTypeDemand:{}",
        //zoneName[zone], i, econ, getBusinessTypeDemand(econ, (BusinessType)j));
      if ((d->flags & _designIsHotel) && j == Retail) {
        // no-op

        // Prefer building hotels
      } else if (j == Retail && !(d->flags & _designIsHotel) &&
          getStatistic(econ, NumEmptyHotelRooms) == 0 &&
          randFloat() > 0.25) {
        bad = true;
        break;

      } else if ((d->numBusinesses[j] > 0 ||
            (d->zone == FarmZone && j == Farm)) &&
          getBusinessTypeDemand(econ, (BusinessType)j) < 0.1) {
        bad = true;
        break;
      }
    }
    if (bad) continue;
    if (d->numFamilies == 0 && numBiz == 0 && randFloat() > 0.25)
      continue; // Avoid empty lots

    return ndx;
  }
  return 0;
}

vector<item> getDesignsByName(std::string& name) {
  vector<item> results = designsByNameMap[name]; // see if the result is cached

  if (results.size() == 0) {
    // find designs that match name
    for (int i = 1; i <= designs.size(); i++) {
      Design* d = getDesign(i);
      if (stringContainsCaseInsensitive(d->name, name)) {
        results.push_back(i);
      }
    }

    if (results.size() == 0) {
      results.push_back(0); // Push a null value so the result gets cached
    }

    designsByNameMap[name] = results; // cache result
  }

  if (results.size() == 1 && results[0] == 0) {
    results.clear(); // Clean up null value
  }

  return results;
}

vector<item> getDesignsByName(const char* name) {
  std::string nameStr = name;
  return getDesignsByName(nameStr);
}

item numBuildingsForDesignKeyword(std::string& name) {
  item result = 0;
  vector<item> matchingDesigns = getDesignsByName(name);
  for (int i = 0; i < matchingDesigns.size(); i++) {
    item dNdx = matchingDesigns[i];
    if (dNdx <= 0 || dNdx > sizeDesigns()) continue;

    Design* d = getDesign(dNdx);
    result += d->numBuildings;
  }
  return result;
}

item newestBuildingForDesignKeyword(const std::string& name) {
  item result = 0;
  float resultBuilt = -FLT_MAX;
  vector<item> matchingDesigns = getDesignsByName(name.c_str());

  for (int i = sizeBuildings(); i > 0; i--) {
    Building* b = getBuilding(i);
    if (!(b->flags & _buildingExists) || !(b->flags & _buildingComplete) || (b->flags & _buildingCity)) continue;
    if (resultBuilt > b->builtTime) continue; // older the current best result

    bool match = false;
    for (int k = 0; k < matchingDesigns.size(); k++) {
      if (matchingDesigns[k] == b->design) {
        match = true;
        break;
      }
    }

    if (match) {
      result = i;
      resultBuilt = b->builtTime;
    }
  }

  return result;
}

bool unlockGovDesignsByName(const char* name) {
  vector<item> dNdxs = getDesignsByName(name);
  int64_t numItems = dNdxs.size();

  if(numItems == 0) {
    SPDLOG_INFO("No amenity designs found for \"{}\"", name);
    return false;
  }

  bool found = false;
  for(int i = 0; i < numItems; i++) {
    Design* d = getDesign(dNdxs[i]);
    if(d == 0) {
      SPDLOG_ERROR("Fetched null design for {}: index {}", name, i);
      continue;
    }

    if(d->zone == GovernmentZone && !getGovBuildingAvailable(dNdxs[i])) {
      SPDLOG_INFO("Enabling amenity {}", d->displayName);
      setGovBuildingAvailable(dNdxs[i], true);
      found = true;
    }
  }

  // If we found an amenity, enable the building tool feature
  // if it's not already enabled
  if(found && !isFeatureEnabled(Feature::FBuildingTool)) {
    setFeatureEnabled(Feature::FBuildingTool, true);
  }

  return found;
}

int getNumBusinesses(item ndx) {
  Design* design = getDesign(ndx);
  int numBiz = 0;
  if (isDesignEducation(ndx)) {
    numBiz += design->numBusinesses[CSStorefronts];
    numBiz += design->numBusinesses[Institution];
  } else {
    for (int i = 0; i < numBusinessTypes; i++) {
      numBiz += design->numBusinesses[i];
    }
  }
  return numBiz;
}

bool isDesignEducation(item ndx) {
  Design* design = getDesign(ndx);
  return design->zone == GovernmentZone &&
    (design->category == EducationCategory ||
    design->category == UniversityCategory);
}

item getDesignNumHomes(item ndx) {
  Design* d = getDesign(ndx);
  int numHomes = d->numFamilies;
  return numHomes;
}

bool isDesignAquatic(item ndx) {
  Design* d = getDesign(ndx);
  return d->flags & _designAquatic;
}

float getDesignZ(item ndx) {
  Design* d = getDesign(ndx);
  return d->size.z;
}

money getDesignValue(item designNdx, item econ, float lv, float density) {
  Design* d = getDesign(designNdx);
  float lvd = 1 + abs(d->minDensity - density) +
      landValueDistance(d, lv);
  return getDesignValue(designNdx) / lvd * (.5f + zoneDemand(econ, d->zone));
}

float designCommonness(item designNdx) {
  Design* d = getDesign(designNdx);
  float buildingsInZone = getStatistic(ourCityEconNdx(),
      (Statistic)(NumResidentialBuildings + d->zone -1));
  float commonness = d->numBuildings / buildingsInZone;
  return commonness;
}

money getDesignValue(item designNdx) {
  Design* d = getDesign(designNdx);

  if (d->zone == GovernmentZone) {
    return abs(d->cost * c(CAmenityCostMult));
  }

  float densityFactor = d->zone == FarmZone || d->zone == FactoryZone ?
    c(CDensityMultiplierIndustrial) : c(CDensityMultiplier);
  densityFactor *= d->minDensity;
  densityFactor += 1;
  float landValueFactor = (1 + d->minLandValue * c(CLandValueMultiplier));

  float commonness = designCommonness(designNdx);
  float commonnessFactor = 1-commonness/c(CMaxDesignCommonness);
  commonnessFactor = clamp(commonnessFactor, 0.f, 1.f);
  if (d->numBuildings < 20) commonnessFactor = 1;

  float validDesignFactor = bool(d->flags & _designEnabled) &&
    !bool(d->flags & _designDisableSpawning) ? 1.0 : 0.5;

  int numBiz = getNumBusinesses(designNdx);
  float buildingValue = (d->numFamilies + 1);
  if (d->flags & _designIsHotel) {
    buildingValue *= c(CHotelRoomBuildingValue);
  } else {
    buildingValue *= c(CHomeBuildingValue);
  }
  buildingValue += numBiz * c(CBusinessBuildingValue);
  if (isDesignAquatic(designNdx)) buildingValue *= c(CAquaticBuildingValue);
  buildingValue *= c(CBuildingValue);
  float landValue = d->size.x * d->size.y * c(CLandValue);
  float value = buildingValue + landValue;
  value *= densityFactor;
  value *= landValueFactor;
  value *= commonnessFactor;
  value *= validDesignFactor;
  return value;
}

void resizeDesign(Design* design) {
  design->size = maxPoint(design);
}

void resizeDesign(item ndx) {
  resizeDesign(getDesign(ndx));
}

item sizeDesigns() {
  return designs.size();
}

void resetDesigns() {
  for (int i = 1; i <= designs.size(); i++) {
    Design* d = getDesign(i);
    if (d->name != 0) free(d->name);
    if (d->displayName != 0) free(d->displayName);
  }
  designs.clear();
  resetDesignRender();
  designerUndoHistory.clear();
  designerUndoHistoryNdxs.clear();
  designsByNameMap.clear();
  undoHistoryNdx = 0;
  structuresVisible = true;
}

void readDesignDirectory(
    void (*callback)(FileBuffer* file, int version, const char* name)
) {

  /*
  vector<char*> files;
  if (c(CDisableDefaultBuildingSet)) {
    files = readDirectoryModAndWorkshopOnly(designDirectory(), designExtension());
    free(dir);
  } else {
    files = readDirectoryAll(designDirectory(), designExtension());
  }

  for (int i = 0; i < files.size(); i++) {
    char* name = files[i];
    */

  uint32_t lookupFlags = 0;
  if (c(CDisableDefaultBuildingSet)) lookupFlags |= _lookupExcludeBase;
  vector<string> files = lookupDesigns(lookupFlags);
  //vector<string> files = lookupDirectory(
      //designDirectory(), designExtension(), lookupFlags);

  for (int i = 0; i < files.size(); i++) {
    string name = files[i];
    bool package = endsWith(name.c_str(), "/");
    if (package) name = name.substr(0, name.length()-1); // remove end slash

    if (streql(name.c_str(), "autosave")) continue;

    FILE* fileHandle;
    string filePath = designDirectory();
    if (package) {
      filePath = filePath + name + "/design.design";
    } else {
      filePath = filePath + name + designExtension();
    }
    string filename = lookupFile(filePath, lookupFlags);
    string fixedFilename = fixDesignerPath(filename);
    FileBuffer buf = readFromFile(fixedFilename.c_str());
    FileBuffer* file = &buf;

    char header[4];
    fread(&header, sizeof(char), 4, file);
    int version = fread_int(file);
    if (version < 0) {
      SPDLOG_INFO("Corrupt design {} {}", name, version);
      break;
    }
    if (version > saveVersion) {
      SPDLOG_INFO("Design {} too new ({}), ignoring", name, version);
      break;
    }
    if (version >= 46) {
      decompressBuffer(file);
    }

    file->version = version;
    if (version >= 51) {
      file->patchVersion = fread_int(file);
    }

    int flags = defaultFileFlags;
    if (version >= 50) {
      flags = fread_int(file);
      char* modpack = fread_string(file);
      free(modpack);
    }

    if (version >= 45) {
      unsigned char* extraData =
        (unsigned char*) calloc(1024, sizeof(unsigned char));
      fwrite_data(file, extraData, 1024);
      free(extraData);
    }

    if (version >= 13) {
      fread_item(file, version); //Game Mode
    }
    if (version >= 14) {
      free(fread_string(file));
    }

    readDecosRenum(file);
    callback(file, version, name.c_str());
    freeBuffer(file);
  }

  if (designs.size() < 5) {
    handleError("Designs didn't load.");
  }
}

void addDesignCallback(FileBuffer* file, int version, const char* name) {
  addDesign();
  readDesign(file, version, designs.size(), name);
}

void initDesigns() {
  readDesignDirectory(addDesignCallback);

  for (int i = 1; i <= designs.size(); i++) {
    Design* d = getDesign(i);
    if ((d->flags & _designEnabled) && (d->flags & _designAlwaysAvailable)) {
      setGovBuildingAvailable(i, true);
      setFeatureEnabled(FBuildingTool, true);
    }
  }
}

void writeDesign(FileBuffer* file, item ndx, const char* name) {
  Design* design = getDesign(ndx);
  if (design == NULL) {
    // No-op, null ptr
    return;
  }

  if (name != 0) {
    design->name = strdup_s(name);
  }

  if (design->name == 0) {
    fwrite_string(file, "autosave");
  } else {
    fwrite_string(file, design->name);
  }

  fwrite_char(file, design->zone);
  fwrite_int(file, design->flags);
  fwrite_item(file, design->minYear);
  fwrite_item(file, design->maxYear);

  fwrite_item(file, design->numFamilies);
  for (int i = 0; i < numBusinessTypes; i++) {
    fwrite_item(file, design->numBusinesses[i]);
  }

  fwrite_float(file, design->minLandValue);
  fwrite_float(file, design->minDensity);
  fwrite_vec3 (file, design->size);
  fwrite_float(file, design->lowTide);
  fwrite_float(file, design->highTide);

  fwrite_item(file, design->structures.size());
  for (int i = 0; i < design->structures.size(); i++) {
    fwrite_item(file, design->structures[i].roofType);
    fwrite_float(file, design->structures[i].angle);
    fwrite_float(file, design->structures[i].roofSlope);
    fwrite_vec3(file, design->structures[i].location);
    fwrite_vec3(file, design->structures[i].size);
  }

  fwrite_item(file, design->decos.size());
  for (int i = 0; i < design->decos.size(); i++) {
    fwrite_item(file, design->decos[i].decoType);
    fwrite_vec3(file, design->decos[i].location);
    fwrite_float(file, design->decos[i].yaw);
    fwrite_float(file, design->decos[i].scale);
  }

  fwrite_money(file, design->cost);
  fwrite_money(file, design->maintenance);
  fwrite_item(file, design->category);
  fwrite_string(file, design->displayName);
  fwrite_item(file, numEffects);
  for (int j = 0; j < numEffects; j++) {
    fwrite_float(file, design->effect[j]);
  }
}

void setDesignHasStatue(Design* design) {
  bool hasStatue = false;
  for (int i = 0; i < design->decos.size(); i++) {
    if (design->decos[i].decoType == statueDecoType()) {
      hasStatue = true;
      break;
    }
  }

  if (hasStatue) {
    design->flags |= _designHasStatue;
  } else {
    design->flags &= ~_designHasStatue;
  }
}

bool readDesign(FileBuffer* file, int version, Design* design,
    const char* name) {

  if (design->name != 0) free(design->name);
  if (design->displayName != 0) {
    free(design->displayName);
    design->displayName = 0;
  }

  if (version >= 27) {
    design->name = fread_string(file);

  } else if (name != 0) {
    design->name = strdup_s(name);
  } else {
    design->name = strdup_s("");
  }

  // Remove end slash
  item length = strlength(design->name);
  if (length > 0 && design->name[length-1] == '/') {
    design->name[length-1] = '\0';
  }

  if (version >= 47) {
    design->zone = fread_char(file);
  } else {
    design->zone = fread_item(file, version);
  }

  if (version >= 34) {
    design->flags = fread_int(file);
  } else {
    design->flags = _designEnabled;
  }

  if (version >= 56) {
    design->minYear = fread_item(file);
    design->maxYear = fread_item(file);
    if (version < 58 && design->maxYear == 2100) {
      design->maxYear = 9999;
    }

  } else {
    design->minYear = 1850;
    design->maxYear = 9999;
  }
  design->numBuildings = 0;

  if (version >= 47) {
    if (version >= 58) {
      design->numFamilies = fread_item(file);
      for (int i = 0; i < numBusinessTypes; i++) {
        design->numBusinesses[i] = fread_item(file);
      }

    } else {
      design->numFamilies = fread_char(file);
      for (int i = 0; i < numBusinessTypes; i++) {
        design->numBusinesses[i] = fread_char(file);
      }
    }

    design->minLandValue = fread_float(file);
    design->minDensity = fread_float(file);

  } else {
    int numBiz = 0;
    if (version >= 19) {
      design->numFamilies = fread_item(file, version);
      numBiz = fread_item(file, version);
      design->minLandValue = fread_float(file);
      design->minDensity = fread_float(file);
    } else {
      design->numFamilies = design->zone == ResidentialZone;
      numBiz = design->zone != ResidentialZone;
      design->minLandValue = 0.0f;
      design->minDensity = 0.0f;
    }

    for (int i = 0; i < numBusinessTypes; i++) {
      design->numBusinesses[i] = 0;
    }

    if (design->zone == FarmZone) {
      if (design->minDensity > 0.05f) {
        design->numBusinesses[Factory] = numBiz;
      } else {
        design->numBusinesses[Farm] = numBiz;
      }
    } else if (design->zone == GovernmentZone) {
      // one government business
      design->numBusinesses[Retail] = std::max(numBiz-1, 0);
    } else {
      design->numBusinesses[Retail] = numBiz;
    }
  }

  if (version < 55) {
    design->size = vec3(fread_vec2(file), 0);
  } else {
    design->size = fread_vec3(file);
  }

  if (version >= 57) {
    design->lowTide = fread_float(file);
    design->highTide = fread_float(file);
  } else {
    design->lowTide = 0;
    design->highTide = 0;
  }

  int structures = fread_item(file, version);
  if (structures > 10000 || structures < 0) {
    handleError("Corrupt design %s - %s", design->name, name);
    return false;
  }
  design->structures.clear();
  design->structures.resize(structures);
  for (int i = 0; i < structures; i++) {
    Structure* s = &design->structures[i];
    s->roofType = fread_item(file, version);
    float yaw = fread_float(file);
    yaw = yaw - 2*pi_o * floor(yaw/(2*pi_o));
    s->angle = yaw;
    if (version > 55 || (version == 55 && file->patchVersion > 0)) {
      s->roofSlope = fread_float(file);
    } else {
      s->roofSlope = defaultRoofSlopeForType(s->roofType);
    }
    if (version < 55) {
      s->location = vec3(fread_vec2(file), 0);
    } else {
      s->location = fread_vec3(file);
    }
    s->size = fread_vec3(file);

    for (int side = 0; side < numStructureSides; side++) {
      s->sides[side] = 0;
    }
  }

  int decos = fread_item(file, version);
  if (decos > 10000) {
    handleError("Corrupt design");
    return false;
  }
  design->decos.clear();
  design->decos.resize(decos);

  for (int i = 0; i < decos; i++) {
    int type = fread_item(file, version);

    if (version < 26) {
      if (type > 4) {
        type += 7;
      } else if (type > 2) {
        type += 6;
      }
    }

    design->decos[i].decoType = renumDeco(type);
    if (version < 55) {
      design->decos[i].location = vec3(fread_vec2(file), 0);
      design->decos[i].yaw = 0;
      design->decos[i].scale = 1;
    } else {
      design->decos[i].location = fread_vec3(file);
      float yaw = fread_float(file);
      yaw = yaw - 2*pi_o * floor(yaw/(2*pi_o));
      design->decos[i].yaw = yaw;
      design->decos[i].scale = fread_float(file);
    }
  }

  if (version >= 33) {
    design->cost = fread_money(file, version);
    design->maintenance = fread_money(file, version);
    design->category = fread_item(file, version);
    design->displayName = fread_string(file);

    for (int j = 0; j < numEffects; j++) {
      design->effect[j] = 0;
    }
    int numEff = fread_item(file, version);
    for (int j = 0; j < numEff; j++) {
      if (version < 47) {
        design->effect[oldEffectOrder[j]] = fread_float(file);
      } else {
        design->effect[j] = fread_float(file);
      }
    }
  }

  if (design->zone == GovernmentZone && version < 27) {
    design->name = strdup_s("G2x3_school");
  }

  resizeDesign(design);
  return true;
}

void readDesign(FileBuffer* file, int version, item ndx, const char* name) {
  Design* design = getDesign(ndx);

  design->mesh = 0;
  design->simpleMesh = 0;
  design->decoMesh = 0;
  design->simpleDecoMesh = 0;

  readDesign(file, version, design, name);
}

bool readDesign(const char* filename, Design* design) {
  string fixedFilename = fixDesignerPath(filename);
  FileBuffer buf = readFromFile(fixedFilename.c_str());
  FileBuffer* file = &buf;

  if (file->length < 4 || !readHeader(file)) {
    freeBuffer(file);
    return false;
  }

  //SPDLOG_INFO("readDesign {} version{} length{} cursor{}",
      //filename, file->version, file->length, file->cursor);

  readDecosRenum(file);
  readDesign(file, file->version, design, 0);
  freeBuffer(file);
  return true;
}

void writeDesigns(FileBuffer* file) {
  writeDecosRenum(file);
  fwrite_item(file, designs.size());
  for (int i = 1; i <= designs.size(); i++) {
    writeDesign(file, i, 0);
  }
}

void copyDesign(Design* dest, Design* source, bool copyStrings) {
  if (dest->name) free(dest->name);
  if (dest->displayName) free(dest->displayName);
  if (copyStrings) {
    if (source->name) dest->name = strdup_s(source->name);
    else dest->name = 0;
    if (source->displayName) dest->displayName = strdup_s(source->displayName);
    else dest->displayName = 0;
  } else {
    dest->name = source->name;
    dest->displayName = source->displayName;
  }

  dest->flags = source->flags | _designEnabled;
  dest->zone = source->zone;
  dest->numFamilies = source->numFamilies;

  for (int i = 0; i < numBusinessTypes; i++) {
    dest->numBusinesses[i] = source->numBusinesses[i];
  }

  dest->minYear = source->minYear;
  dest->maxYear = source->maxYear;
  dest->minLandValue = source->minLandValue;
  dest->minDensity = source->minDensity;
  dest->size = source->size;
  dest->lowTide = source->lowTide;
  dest->highTide = source->highTide;

  dest->structures.clear();
  for (int i = 0; i < source->structures.size(); i ++) {
    dest->structures.push_back(source->structures[i]);
  }
  dest->decos.clear();
  for (int i = 0; i < source->decos.size(); i ++) {
    dest->decos.push_back(source->decos[i]);
  }

  dest->cost = source->cost;
  dest->maintenance = source->maintenance;
  dest->category = source->category;

  for (int i = 0; i < numEffects; i++) {
    dest->effect[i] = source->effect[i];
  }
}

void copyDesign(Design* dest, Design* source) {
  copyDesign(dest, source, false);
}

void upgradeDesign(Design* design) {
  vector<item> packageLinks;
  item solution = 0;
  for (int i = 1; i <= designs.size(); i++) {
    Design* original = getDesign(i);

    if (design->name == 0 || original->name == 0) {
      return;
    }

    bool sameName = streql(design->name, original->name);
    /*
    if (startsWith(design->name, "G2x3_school")) {
      SPDLOG_INFO("upgradeDesign {} {} {} {} {} {}", i, design->name, sameName ? "==" : "!=", original->name, designs.size()+1);
      SPDLOG_INFO("{},{} => {},{} {}=>{}",
          original->size.x,
          original->size.y,
          design->size.x,
          design->size.y,
          int(original->zone),
          int(design->zone));
    }
    */
    if (!sameName) continue;

    if (original->size.x+tileSize >= design->size.x &&
        original->size.y+tileSize >= design->size.y &&
        original->zone == design->zone) {
      // It's a match, copy over
      copyDesign(original, design);
      solution = i;
      break;

    } else {
      packageLinks.push_back(i);
    }
  }

  if (solution == 0) {
    // No match, add it to the end
    addDesign();
    solution = designs.size();
    copyDesign(getDesign(solution), design);
  }

  for (int k = 0; k < packageLinks.size(); k++) {
    linkDesignPackage(solution, packageLinks[k]);
  }
}

void upgradeDesignCallback(FileBuffer* file, int version, const char* name) {
  Design design;
  if (readDesign(file, version, &design, name)) {
    upgradeDesign(&design);
  }
}

void upgradeDesigns() {
  readDesignDirectory(upgradeDesignCallback);
}

void readDesigns(FileBuffer* file, int version) {
  readDecosRenum(file);
  designs.clear();
  int size = fread_item(file, version);
  designs.resize(size);
  for (int i = 1; i <= size; i++) {
    readDesign(file, version, i, 0);
    getDesign(i)->flags &= ~_designEnabled; // Disable spawning new buildings
  }
  loadDesignPackageTextures();
  upgradeDesigns();

  for (int i = 1; i <= designs.size(); i++) {
    Design* d = getDesign(i);
    if ((d->flags & _designEnabled) && (d->flags & _designAlwaysAvailable)) {
      setGovBuildingAvailable(i, true);
      setFeatureEnabled(FBuildingTool, true);
    }
  }
}

bool loadDesign(const char* filename, bool isAutosave) {
  /*
  string filePath = saveDirectory();
  bool package = endsWith(filename, "/");
  if (package) {
    filePath = filePath + filename + "design.design";
    filename[strlength(filename)-1] = '\0'; // remove end slash
  } else {
    filePath = filePath + filename + fileExtension();
  }
  string fn = lookupFile(filePath, getLoadMenuLookupFlags());
  */

  string fn = filename;
  fn = fixDesignerPath(fn);

  resetAll();
  FileBuffer buf = readFromFile(fn.c_str());
  FileBuffer* file = &buf;

  startSegment(file, "Header");
  char header[4];
  fread(&header, sizeof(char), 4, file);
  int version = fread_int(file);
  SPDLOG_INFO("Reading file {} version {}", fn, version);

  if (version > saveVersion) {
    handleError("This design file (%s) is from\n"
        "a newer version (0.0.%d.x) of NewCity.",
        fn.c_str(), version);
    freeBuffer(file);
    return false;
  }

  if (version < 0) {
    handleError("This design file (%s) is corrupt.", fn.c_str());
    freeBuffer(file);
    return false;
  }

  if (version >= 46) {
    decompressBuffer(file);
  }

  file->version = version;
  if (version >= 51) {
    file->patchVersion = fread_int(file);
  }

  int flags = defaultFileFlags;

  if (version >= 50) {
    flags = fread_int(file);
    char* modpack = fread_string(file);
    SPDLOG_INFO("File was saved under modpack {}", modpack);
    free(modpack);
  }

  if (version >= 45) {
    unsigned char* extraData;
    int num = fread_data(file, &extraData);
    free(extraData);
  }

  addDesign();
  if (version >= 13) {
    fread_item(file, version); // Game Mode
  }
  if (version >= 14) {
    char* saveFilename = fread_string(file);
    setSaveFilename(saveFilename);
  }

  readDecosRenum(file);
  readDesign(file, version, 1, filename);

  SPDLOG_INFO("Reading {} successful", fn);
  freeBuffer(file);
  return true;
}

FileBuffer* writeDesignToFileBuffer(const char* name) {
  FileBuffer* file = makeFileBufferPtr();
  file->version = saveVersion;
  file->patchVersion = patchVersion;
  fputs("CITY", file);
  fwrite_int(file, saveVersion);
  fwrite_int(file, 0); // For uncompressed size
  fwrite_int(file, patchVersion);

  int flags = defaultFileFlags;

  fwrite_int(file, flags);
  fwrite_string(file, getMod());

  unsigned char* extraData =
    (unsigned char*) calloc(1024, sizeof(unsigned char));
  fwrite_data(file, extraData, 1024);
  free(extraData);

  fwrite_item(file, ModeBuildingDesigner);
  fwrite_string(file, getSaveFilename());
  writeDecosRenum(file);

  bool autosave = (name == 0 || streql(name, "autosave"));
  writeDesign(file, 1, autosave ? 0 : name);

  return file;
}

void saveDesign(item dNdx) {
  Design* d = getDesign(dNdx);

  FileBuffer* file = makeFileBufferPtr();
  file->version = saveVersion;
  file->patchVersion = patchVersion;
  fputs("CITY", file);
  fwrite_int(file, saveVersion);
  fwrite_int(file, 0); // For uncompressed size
  fwrite_int(file, patchVersion);

  int flags = defaultFileFlags;

  fwrite_int(file, flags);
  fwrite_string(file, getMod());

  unsigned char* extraData =
    (unsigned char*) calloc(1024, sizeof(unsigned char));
  fwrite_data(file, extraData, 1024);
  free(extraData);

  fwrite_item(file, ModeBuildingDesigner);
  fwrite_string(file, d->name);
  writeDecosRenum(file);

  writeDesign(file, dNdx, d->name);

  asyncCompressAndWrite(file, getSaveFilename(d->name), 0);
}

string fixDesignerPath(string filePath) {
  filePath = lookupFile(filePath, getLoadMenuLookupFlags());
  string pattern1 = "/design.design";
  string pattern2 = ".design";
  for (int attemptsLeft = 5; attemptsLeft > 0; attemptsLeft--) {
    if (fileExists(filePath) && !is_directory(filePath)) return filePath;
    if (streql(filePath.c_str(), "autosave/design")) {
      filePath = "designs/autosave/design.design";
    } else if (!stringContainsCaseInsensitive(filePath.c_str(), "designs/")) {
      filePath = "designs/" + filePath;
    } else if (is_directory(filePath)) {
      filePath = filePath + "/design.design";
    } else if (endsWith(filePath.c_str(), "/")) {
      filePath = filePath + "design.design";
    } else if (!endsWith(filePath.c_str(), ".design")) {
      filePath = filePath + ".design";
    } else if (endsWith(filePath.c_str(), pattern1.c_str())) {
      filePath = filePath.substr(0, filePath.length() - pattern1.length()) + pattern2;
    } else if (endsWith(filePath.c_str(), pattern2.c_str())) {
      filePath = filePath.substr(0, filePath.length() - pattern2.length()) + pattern1;
    }
    filePath = lookupFile(filePath, attemptsLeft > 3 ? getLoadMenuLookupFlags() : _lookupForceMod);
  }
  return filePath;
}

