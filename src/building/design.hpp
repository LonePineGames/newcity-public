#pragma once

#include "deco.hpp"

#include "../amenity.hpp"
#include "../item.hpp"
#include "../serialize.hpp"

#include <string>

enum BuildingCategories {
  EducationCategory, ParksCategory, CommunityCategory, UniversityCategory
};

enum CollegeStats {
  CSStorefronts, CSColleges, CSStudents, CSProfessors,
  numCollegeStats
};

enum RoofTypes {
  GableRoof, HipRoof, FlatRoof, BarrelRoof, SlantRoof,
  GambrelRoof,
  numRoofTypes
};

enum HandleTypes {
  LocationHandle, AngleHandle, SizeHandle, SelectionPointHandle
};

enum StructureSides {
  Side0, Side1, Side2, Side3, SideRoof, SideUnderRoof,
  numStructureSides
};

struct Structure {
  item roofType;
  item sides[numStructureSides];
  float angle;
  float roofSlope;
  vec3 location;
  vec3 size;
};

const int _designEnabled              = 1 << 0;
const int _designSingleton            = 1 << 1;
const int _designIsOfficial           = 1 << 2;
const int _designProvidesHSDiploma    = 1 << 3;
const int _designProvidesBclDegree    = 1 << 4;
const int _designProvidesPhd          = 1 << 5;
const int _designProvidesHealthCare   = 1 << 6;
const int _designAlwaysAvailable      = 1 << 8;
const int _designNoEminentDomain      = 1 << 9;
const int _designAquatic              = 1 << 10;
const int _designHasStatue            = 1 << 11;
const int _designIsHotel              = 1 << 12;
const int _designDisableSpawning      = 1 << 13;

struct Design {
  int flags = _designEnabled;
  item numBuildings;
  item mesh;
  item decoMesh;
  item simpleMesh;
  item simpleDecoMesh;
  item minYear;
  item maxYear;

  float minLandValue;
  float minDensity;
  float lowTide;
  float highTide;

  vec3 size;
  vector<Structure> structures;
  vector<Deco> decos;
  char* name = 0;

  char zone;
  item numFamilies;
  item numBusinesses[5];

  //Government
  char category = 0;
  money cost = 0;
  money maintenance = 0;
  char* displayName = 0;
  char effect[numEffects] = {0};
};

item addDesign();
Design* getDesign(item ndx);
item getRandomDesign(item zone, float density, float landValue);
item getRandomDesign(item zone, item econ, float density, float landValue);
money getDesignValue(item designNdx);
money getDesignValue(item designNdx, item econ, float lv, float d);
int getNumBusinesses(item ndx);
float designCommonness(item designNdx);
void resizeDesign(item ndx);
item sizeDesigns();
vector<item> getDesignsByName(const char* name);
item numBuildingsForDesignKeyword(std::string& name);
item newestBuildingForDesignKeyword(const std::string& name);
bool unlockGovDesignsByName(const char* name);
bool isDesignEducation(item ndx);
bool isDesignAquatic(item ndx);
item getDesignNumHomes(item ndx);
float getDesignZ(item ndx);
float defaultRoofSlopeForType(item type);
const char* getRoofTypeName(item roofType);
vec3 getStructureHandleLocation(item ndx, item handleType);
vec3 getDecoHandleLocation(item ndx, item handleType);
bool isStructuresVisible();
void toggleStructuresVisible();
void setDesignHasStatue(Design* design);

void shiftDesignerUndoHistory(item amount);
void pushDesignerUndoHistory();
bool hasDesignerUndoHistory();
bool hasDesignerRedoHistory();
item getSelectedDesignNdx();
Design* getSelectedDesign();

void resetDesigns();
void initDesigns();
FileBuffer* writeDesignToFileBuffer(const char* name);
void saveDesign(item dNdx);
bool loadDesign(const char* filename, bool isAutosave);
bool readDesign(const char* filename, Design* design) ;
void readDesigns(FileBuffer* file, int version);
void readDesign(FileBuffer* file, int version, item ndx, const char* name);
void writeDesigns(FileBuffer* file);
void writeDesign(FileBuffer* file, item ndx, const char* name);
std::string fixDesignerPath(std::string filePath);

