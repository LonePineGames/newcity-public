#include "amenity.hpp"

#include "board.hpp"
#include "building/building.hpp"
#include "building/design.hpp"
#include "building/renderBuilding.hpp"
#include "business.hpp"
#include "collisionTable.hpp"
#include "cup.hpp"
#include "economy.hpp"
#include "game/game.hpp"
#include "icons.hpp"
#include "lot.hpp"
#include "person.hpp"
#include "plan.hpp"
#include "string_proxy.hpp"
#include "time.hpp"
#include "util.hpp"
#include "zone.hpp"

#include "parts/leftPanel.hpp"

#include "spdlog/spdlog.h"

const char* buildingCategoryString[numBuildingCategories+1] = {
  "Education", "Recreation", "Community", "University", "General"
};

BudgetLine buildingCategoryBudgetLine[numBuildingCategories] = {
  EducationExpenses, RecreationExpenses, ServicesExpenses, UniversityExpenses
};

const char* effectString[numEffects] = {
  "Nature", "Value", "Density", "Law", "Education",
  "Prosperity", "Community", "Technology", "Business", "Tourism",
  "Prestige", "Environment", "Order", "Health",
};

static const vec3 effectIcon[numEffects] = {
  iconBuildingCategory[1], iconHeatmap[1], iconHeatmap[2], iconHeatmap[3],
  iconHeatmap[4], iconHeatmap[5], iconFamily, iconTech,
  iconZoneMono[OfficeZone], iconTouristFemale, iconBusiness, iconTree,
  iconZoneMono[GovernmentZone], iconHealth,
};

float effectValue[numEffects];
Cup<item> governmentBuildings;
static vector<unsigned char> govBuildingAvail;
static vector<int> govBuildingsPlaced;
static Cup<item> healthcareProviders;
static item categoryCount[numBuildingCategories+1] = {0,0,0,0,0};
item universityQuad = 0;

item getRandomGovernmentBuilding() {
  int numB = governmentBuildings.size();
  if (numB == 0) {
    return 0;
  } else {
    int num = randItem(numB);
    return governmentBuildings[num];
  }
}

item getRandomHealthcare() {
  int numB = healthcareProviders.size();
  if (numB == 0) {
    return 0;
  } else {
    int num = randItem(numB);
    return healthcareProviders[num];
  }
}

void updateCategoryCount(item ndx, int val) {
  categoryCount[getDesign(ndx)->category] += val;
  categoryCount[numBuildingCategories] += val;
}

item getCategoryCount(item category) {
  return categoryCount[category];
}

int getGovBuildingsPlaced(item design) {
  return govBuildingsPlaced.size() <= design ? 0 : govBuildingsPlaced[design];
}

bool getGovBuildingAvailable(item ndx) {
  if (govBuildingAvail.size() <= ndx/8) {
    return false;
  } else {
    return govBuildingAvail[ndx/8] & 1 << ndx%8;
  }
}

void setGovBuildingAvailable(item ndx, bool val) {
  if (ndx/8 >= govBuildingAvail.size()) {
    govBuildingAvail.resize(ndx/8+1, 0);
  }
  if (val) {
    govBuildingAvail[ndx/8] |= 1 << ndx%8;
    updateCategoryCount(ndx, 1);
  } else {
    govBuildingAvail[ndx/8] &= ~(1 << ndx%8);
    updateCategoryCount(ndx, -1);
  }
}

void setAllGovernmentBuildingsAvailable() {
  for(int d = 1; d <= sizeDesigns(); d++) {
    Design* des = getDesign(d);
    if(des->zone != GovernmentZone) continue;
    setGovBuildingAvailable(d,true);
  }
}

const char* getBuildingCategoryString(item category) {
  return buildingCategoryString[category];
}

const char* getEffectString(item effect) {
  return effectString[effect];
}

const vec3 getEffectIcon(item effect) {
  return effectIcon[effect];
}

float getEduLevelHardLimit(item edu) {
  return getStatistic(ourCityEconNdx(), Population) * c((FloatConstant)(CMaxHSEdu + edu - 1));
}

float getEduLevelLimit(item edu) {
  float result = c((FloatConstant)(CMaxHSEduPerEffect+edu-1)) *
    effectValue[EducationEffect] * getEffectMultiplier(Technology) +
    getStatistic(ourCityEconNdx(), NumColleges) *
    c((FloatConstant)(CMaxHSEduPerCollege+edu-1));
  result = clamp(result, 0.f, getEduLevelHardLimit(edu));
  return result;
}

float getEffectMultiplier(item effect, item value) {
  if (effect == Order) {
    return pow(c(COrderLawBasis), value);
  } else if (effect == Technology) {
    return pow(c(CTechEduBasis), value);
  } else if (effect == BusinessEffect) {
    return pow(c(CBusinessUnempBasis), value);
  } else if (effect == Prestige) {
    return clamp(c(CMaxResidentialDensity) + c(CPrestigeMaxResDensity)*value,
        0.f, 1.f);
  } else if (effect == Environmentalism) {
    return pow(c(CEnvironmentalismPollutionBasis), value);
  } else if (effect == Community) {
    return pow(c(CCommunityCrimeBasis), value);
  } else if (effect == Tourism) {
    return c(CTouristsPerPoint) * value;
  } else if (effect == Education) {
    return getEduLevelLimit(BachelorsDegree);
  } else {
    return 1;
  }
}

float getEffectMultiplier(item effect) {
  return getEffectMultiplier(effect, effectValue[effect]);
}

char* getMacroEffectDescriptor(item effect) {
  float value = effectValue[effect];
  if (value <= 0 && effect != Prestige) return strdup_s("");

  if (effect == Nature) {
    return strdup_s("Reduced Pollution");

  } else if (effect == ValueEffect) {
    return strdup_s("Enhanced Property Value (and more tax revenue)");

  } else if (effect == DensityEffect) {
    return strdup_s("Accelerated Growth");

  } else if (effect == Law) {
    return strdup_s("Reduced Crime");

  } else if (effect == EducationEffect) {
    char* hsStr;
    int num = getEduLevelLimit(HSDiploma);
    int max = getEduLevelHardLimit(HSDiploma);

    if (max <= num) {
      hsStr = sprintf_o("HS Diploma Limit: At Cap");
    } else if (num > 1000000) {
      hsStr = sprintf_o("HS Diploma Limit: %dM", num/1000000);
    } else if (num > 1000) {
      hsStr = sprintf_o("HS Diploma Limit: %dk", num/1000);
    } else {
      hsStr = sprintf_o("HS Diploma Limit: %d", num);
    }

    char* bclStr;
    num = getEduLevelLimit(BachelorsDegree);
    max = getEduLevelHardLimit(BachelorsDegree);

    if (max <= num) {
      bclStr = sprintf_o("Bachelor's Degree Limit: At Cap");
    } else if (num > 1000000) {
      bclStr = sprintf_o("Bachelor's Degree Limit: %dM", num/1000000);
    } else if (num > 1000) {
      bclStr = sprintf_o("Bachelor's Degree Limit: %dk", num/1000);
    } else if (num > 0) {
      bclStr = sprintf_o("Bachelor's Degree Limit: %d", num);
    }

    char* phdStr;
    num = getEduLevelLimit(Doctorate);
    max = getEduLevelHardLimit(Doctorate);

    if (max <= num) {
      phdStr = sprintf_o("Doctorate Limit: At Cap");
    } else if (num > 1000000) {
      phdStr = sprintf_o("Doctorate Limit: %dM", num/1000000);
    } else if (num > 1000) {
      phdStr = sprintf_o("Doctorate Limit: %dk", num/1000);
    } else if (num > 0) {
      phdStr = sprintf_o("Doctorate Limit: %d", num);
    }

    char* result = sprintf_o("%s\n%s\n%s", hsStr, bclStr, phdStr);
    free(hsStr);
    free(bclStr);
    free(phdStr);
    return result;

  } else if (effect == ProsperityEffect) {
    return strdup_s("Prosperity Intensified");

  } else if (effect == Community) {
    int num = -(getEffectMultiplier(Community, value)-1)*100;
    return sprintf_o("Crime Reduced by %d%%", num);

  } else if (effect == Technology) {
    int num = (getEffectMultiplier(Technology, value))*100;
    return sprintf_o("Education is %d%% as Effective", num);

  } else if (effect == BusinessEffect) {
    int num = -(getEffectMultiplier(BusinessEffect, value)-1)*100;
    int extraBiz = value * c(CBizPerBizPoint);
    return sprintf_o("Unemployment Reduced by %d%%\n%d extra Office Businesses", num, extraBiz);

  } else if (effect == Tourism) {
    int num = getEffectMultiplier(Tourism, value);
    float touristRating = getStatistic(ourCityEconNdx(), TouristRating);
    const char* starAppend = touristRating - int(touristRating) >= 0.5 ?
      " and a half" : "";
    char* stayTime = printNiceDurationString(
        getTouristTypicalStay(ourCityEconNdx()));

    char* numStr = 0;
    if (num >= 1000000) {
      numStr = sprintf_o("%d,%03d,%03d", num/1000000, (num/1000)%1000, num%1000);
    } else if (num >= 1000) {
      numStr = sprintf_o("%d,%03d", num/1000, num%1000);
    } else {
      numStr = sprintf_o("%d", num);
    }

    char* result = sprintf_o("%s tourists per year.\n"
        "With our %d%s star rating, a typical tourist's stay is %s.",
        numStr, int(touristRating), starAppend, stayTime);
    free(numStr);
    free(stayTime);
    return result;

  } else if (effect == Prestige) {
    float val = getEffectMultiplier(Prestige, value)*10;
    int num = val;
    num = clamp(num, 0, 10);

    if (num < 10) {
      float gap = float(num + 1) - val;
      int needed = ceil(gap/(c(CPrestigeMaxResDensity)*10));
      return sprintf_o("Increases Tourist Rating.\n"
        "Max Residential Density: %d\n"
        "(+%d needed for next level.)", num, needed);

    } else {
      return sprintf_o("Increases Tourist Rating.\n"
        "Max Residential Density: %d", num);
    }

  } else if (effect == Environmentalism) {
    int num = -(getEffectMultiplier(Environmentalism, value)-1)*100;
    return sprintf_o("Pollution Reduced by %d%%", num);

  } else if (effect == Order) {
    int num = (getEffectMultiplier(Order, value))*100;
    return sprintf_o("Law is %d%% as Effective", num);

  } else if (effect == Health) {
    return strdup_s("Sick Healed");

  } else {
    return strdup_s("Bad Effect");
  }

  return strdup_s("");
}

char* getEffectDescriptor(item effect, int value, int flags) {
  if (value == 0) return strdup_s("No Effect");
  if (effect == Nature) {
    if (value >= 0) {
      return strdup_s("Reduces Pollution in the Area");
    } else {
      return strdup_s("Pollutes the Area");
    }

  } else if (effect == ValueEffect) {
    if (value >= 0) {
      return strdup_s("Enhances Value in the Area");
    } else {
      return strdup_s("Reduces Value in the Area");
    }

  } else if (effect == DensityEffect) {
    if (value >= 0) {
      return strdup_s("Drives Growth in the Area");
    } else {
      return strdup_s("Impedes Growth in the Area");
    }

  } else if (effect == Law) {
    if (value >= 0) {
      return strdup_s("Reduces Crime in the Area");
    } else {
      return strdup_s("Increases Crime in the Area");
    }

  } else if (effect == EducationEffect) {
    int max = getEduLevelLimit(HSDiploma);
    int hardMax = getEduLevelHardLimit(HSDiploma);
    if (max >= hardMax || getGameMode() != ModeGame) {
      return strdup_s("HS Diploma Limit: Capped by population");
    }

    int num = c(CMaxHSEduPerEffect) * value * getEffectMultiplier(Technology);
    if (abs(num) > 1000000) {
      return sprintf_o("HS Diploma Limit: %+dM", num/1000000);
    } else if (abs(num) > 1000) {
      return sprintf_o("HS Diploma Limit: %+dk", num/1000);
    } else {
      return sprintf_o("HS Diploma Limit: %+d", num);
    }

  } else if (effect == ProsperityEffect) {
    if (value >= 0) {
      return strdup_s("Drives Prosperity in the Area");
    } else {
      return strdup_s("Impedes Prosperity in the Area");
    }

  } else if (effect == Community) {
    float num = -(getEffectMultiplier(Community, value)-1)*100;
    if (num < 0) {
      return sprintf_o("Damages Community in the Area\nCrime Increased by +%1.1f%% City-Wide", -num);
    } else {
      return sprintf_o("Builds Community in the Area\nCrime Reduced by +%1.1f%% City-Wide", num);
    }

  } else if (effect == Technology) {
    float num = (getEffectMultiplier(Technology, value)-1)*100;
    if (num >= 0) {
      return sprintf_o("Education is +%1.1f%% More Effective City-Wide", num);
    } else {
      return sprintf_o("Education is -%1.1f%% Less Effective City-Wide", -num);
    }

  } else if (effect == BusinessEffect) {
    float num = -(getEffectMultiplier(BusinessEffect, value)-1)*100;
    int extraBiz = value * c(CBizPerBizPoint);
    if (num < 0) {
      return sprintf_o("Unemployment Increased by +%1.1f%% City-Wide\n%d fewer Office Businesses", -num, extraBiz);
    } else {
      return sprintf_o("Unemployment Reduced by +%1.1f%% City-Wide\n%d extra Office Businesses", num, extraBiz);
    }

  } else if (effect == Tourism) {
    int num = getEffectMultiplier(Tourism, value);
    return sprintf_o("%+d Tourists Per Year City-Wide", num);

  } else if (effect == Prestige) {
    float num = c(CPrestigeMaxResDensity)*value*10;
    return sprintf_o("Max Residential Density: %+1.1f City-Wide", num);

  } else if (effect == Environmentalism) {
    float num = -(getEffectMultiplier(Environmentalism, value)-1)*100;
    if (num < 0) {
      return sprintf_o("Pollution Increased by +%1.1f%% City-Wide", -num);
    } else {
      return sprintf_o("Pollution Reduced by +%1.1f%% City-Wide", num);
    }

  } else if (effect == Order) {
    float num = (getEffectMultiplier(Order, value)-1)*100;
    if (num < 0) {
      return sprintf_o("Law is -%1.1f%% Less Effective City-Wide", -num);
    } else {
      return sprintf_o("Law is +%1.1f%% More Effective City-Wide", num);
    }

  } else if (effect == Health) {
    if (value >= 0) {
      return strdup_s("Improves Health in the Area");
    } else {
      return strdup_s("Sickens People in the Area");
    }

  } else {
    return strdup_s("Bad Effect");
  }
}

char* getEffectDescriptor(item effect) {
  return getEffectDescriptor(effect, effectValue[effect],
    _designEnabled | _designSingleton | _designIsOfficial |
    _designProvidesHSDiploma | _designProvidesBclDegree | _designProvidesPhd);
}

float getEffectValue(item effect) {
  float result = effectValue[effect];
  return result;
}

float getHeatMapAdjustment(HeatMapIndex ndx) {
  if (ndx == Pollution) {
    return getEffectMultiplier(Environmentalism);
  } else if (ndx == Crime) {
    return getEffectMultiplier(Community);
  } else {
    return 1.0f;
  }
}

float getAdjustedDensity(item zone, vec3 loc) {
  float unadjustedDensity = heatMapGet(Density, loc);
  if (zone == FactoryZone) {
    //float edu = heatMapGet(Education, loc);
    //float adjEdu = clamp(edu*2.0f, 0.f, unemploymentRate(HSDiploma)*20);
    //adjEdu = clamp(adjEdu, 0.f, 1.f);
    return clamp(unadjustedDensity*2.5f, 0.f, 1.f);
  } else if (zone == ResidentialZone || zone == MixedUseZone) {
    //float adjDensity = clamp(unadjustedDensity, 0.f,
        //effectValue[Prestige]/25.f);
    return clamp(unadjustedDensity, 0.f, getEffectMultiplier(Prestige));
  } else {
    return unadjustedDensity;
  }
}

float getAdjustedLandValue(item zone, vec3 loc) {
  float unadjustedValue = heatMapGet(Value, loc);
  return unadjustedValue;
}

void adjustAmenityStats(item bNdx, float mult) {
  Building* b = getBuilding(bNdx);
  if (!(b->flags & _buildingExists)) return;
  if (!(b->flags & _buildingComplete)) return;
  Design* d = getDesign(b->design);
  item e = b->econ;

  if (isDesignEducation(b->design)) {
    adjustStat(e, MaxCollegeStudents, d->numBusinesses[CSStudents] * mult);
    adjustStat(e, NumColleges, d->numBusinesses[CSColleges]*mult);
    adjustStat(e, NumProfessors, d->numBusinesses[CSProfessors]*mult);
    adjustStat(e, NumCollegeBunks, getDesignNumHomes(b->design)*mult);
    if (d->numFamilies > 0) {
      adjustStat(e, NumDorms, 1*mult);
    }

  } else if (!(d->flags & _designIsHotel)) {
    adjustStat(e, EmergencyHousing, getDesignNumHomes(b->design)*mult);
  }

  if (d->flags & _designProvidesHealthCare) {
    adjustStat(e, NumHealthcareProviders, 1*mult);
  }
}

void rebuildAmenityStats() {
  resetStat(MaxCollegeStudents);
  resetStat(NumCollegeBunks);
  resetStat(NumEmptyBunks);
  resetStat(NumDorms);
  resetStat(NumColleges);
  resetStat(NumProfessors);
  resetStat(NumHealthcareProviders);
  resetStat(EmergencyHousing);

  for (int i = 0; i < governmentBuildings.size(); i++) {
    Building* b = getBuilding(governmentBuildings[i]);
    if(!(b->flags & _buildingEnabled) || (b->flags & _buildingAbandoned)) continue;
    adjustAmenityStats(governmentBuildings[i], 1);
  }
}

void addBuildingEffect(item designNdx) {
  Design* d = getDesign(designNdx);
  BudgetLine budgetLine = buildingCategoryBudgetLine[d->category];
  float controlEffect = getBudgetControlEffect(budgetLine);
  for (int i=0; i < numEffects; i++) {
    effectValue[i] += d->effect[i] * controlEffect;
  }
}

void subtractBuildingEffect(item designNdx) {
  Design* d = getDesign(designNdx);
  for (int i=0; i < numEffects; i++) {
    effectValue[i] -= d->effect[i];
  }
}

void updateBuildingPlans() {
  for (int i = 0; i < governmentBuildings.size(); i++) {
    item ndx = governmentBuildings[i];
    Building* b = getBuilding(ndx);
    if (b->plan) {
      updatePlanCost(b->plan);
    }
  }
}

void handleUniqueGovernmentBuilding(item ndx, Design* d) {
  item cityEcon = ourCityEconNdx();

  // University
  if(endsWith(d->name, "_uni")) {
    if(endsWith(d->name, "_quad_uni")) {
      universityQuad = ndx;
      unlockGovDesignsByName("_dorms_uni");
    } else if(endsWith(d->name, "_dorms_uni") &&
        getStatistic(ourCityEconNdx(), Statistic::NumCollegeBunks) >= 1000) {
      unlockGovDesignsByName("_school_uni");
    } else if(endsWith(d->name, "_school_uni")) {
      unlockGovDesignsByName("_am_uni");
    } else if(endsWith(d->name, "_am_uni")) {
      unlockGovDesignsByName("_mschool_uni");
    }
  }
}

void addGovernmentBuilding(item ndx) {
  Building* b = getBuilding(ndx);
  if (!(b->flags & _buildingExists)) return;
  item dNdx = b->design;
  Design* d = getDesign(dNdx);
  governmentBuildings.push_back(ndx);
  if (b->flags & _buildingComplete) {
    if (dNdx >= govBuildingsPlaced.size()) {
      govBuildingsPlaced.resize(dNdx+1, 0);
    }
    govBuildingsPlaced[dNdx] ++;

    adjustAmenityStats(ndx, 1);
    handleUniqueGovernmentBuilding(ndx, d);
  }
  if (d->flags & _designProvidesHealthCare) {
    healthcareProviders.push_back(ndx);
  }

  setAmenityHighlight(ndx);
}

void removeGovernmentBuilding(item ndx) {
  Building* b = getBuilding(ndx);
  if ((b->flags & _buildingComplete) && (b->flags & _buildingEnabled)) {
    adjustAmenityStats(ndx, -1);
    subtractBuildingEffect(getBuilding(ndx)->design);
  }
  if ((b->flags & _buildingComplete) && b->design < govBuildingsPlaced.size()) {
    govBuildingsPlaced[b->design] --;
  }

  if (ndx == universityQuad) universityQuad = 0;

  governmentBuildings.removeByValue(ndx);
  healthcareProviders.removeByValue(ndx);
  updateBuildingPlans();
}

const char* governmentBuildingLegalMessage(item ndx) {
  Building* b = getBuilding(ndx);
  Design* d = getDesign(b->design);

  if (d->zone != GovernmentZone) return NULL;
  if (!getGovBuildingAvailable(b->design)) return "Not Unlocked";

  if (d->flags & _designSingleton) {
    for (int i = 0; i < governmentBuildings.size(); i++) {
      item ndx1 = governmentBuildings[i];
      if (ndx1 == ndx) {
        continue;
      }
      Building* b1 = getBuilding(ndx1);
      if (b1->design == b->design) {
        return "Limit One Per City";
      }
    }
  }

  if (d->minDensity > 0) {
    float density = heatMapGet(Density, b->location);
    if (density+.01f < d->minDensity) {
      return "Must be Placed in a Denser Area";
    }
  }

  if (d->minLandValue > 0) {
    float landValue = heatMapGet(Value, b->location);
    if (landValue < d->minLandValue) {
      return "Must be Placed in an Area with Higher Land Value";
    }
  }

  if (d->category == UniversityCategory && !endsWith(d->name, "_quad_uni")) {

    Building* quad = 0;
    if (universityQuad != 0) {
      quad = getBuilding(universityQuad);
      if (!(quad->flags & _buildingExists)) quad = 0;
      if (!(quad->flags & _buildingComplete)) quad = 0;
    }

    if (quad == 0) {
      return "Must Place University Quad First";
    }

    if (length(quad->location - b->location) > c(CUniversityMaxQuadDistance)) {
      return "Too Far From University Quad";
    }

    int numStudents = d->numBusinesses[CSStudents];
    if (numStudents > 0) {
      numStudents += getStatistic(b->econ, MaxCollegeStudents);
      int numBunks = getStatistic(b->econ, NumCollegeBunks);
      if (numStudents > numBunks) {
        return "Not Enough Dorms";
      }
    }
  }

  return NULL;
}

void setAmenityHighlight(item ndx) {
  if (ndx <= 0 || ndx > sizeBuildings()) return;
  Building* b = getBuilding(ndx);
  if (!(b->flags & _buildingExists)) return;

  item hm = getHeatMap();
  bool highlight = isHeatMapIntense() && hm >= 0;
  if (!highlight && getLeftPanel() == BudgetPanel) {
    highlight = true;
    hm = -1;
  }

  if (!(b->flags & _buildingComplete)) {
    setBuildingHighlight(ndx, true);
    setBuildingRedHighlight(ndx, false);
    return;
  }

  Design* d = getDesign(b->design);
  bool matches = highlight && (hm < 0 ||
    (hm == Pollution && d->effect[Nature] != 0) ||
    (hm == Education && d->effect[EducationEffect] != 0) ||
    (hm == Prosperity && d->effect[ProsperityEffect] != 0) ||
    (hm == Value && d->effect[ValueEffect] != 0) ||
    (hm == Density && d->effect[DensityEffect] != 0) ||
    (hm == CommunityHM && d->effect[Community] != 0) ||
    (hm == HealthHM && d->effect[Health] != 0) ||
    (hm == Crime && d->effect[Law] != 0));

  bool negative = matches && (
    (hm == Pollution && d->effect[Nature] < 0) ||
    (hm == Education && d->effect[EducationEffect] < 0) ||
    (hm == Prosperity && d->effect[ProsperityEffect] < 0) ||
    (hm == Value && d->effect[ValueEffect] < 0) ||
    (hm == Density && d->effect[DensityEffect] < 0) ||
    (hm == CommunityHM && d->effect[Community] < 0) ||
    (hm == HealthHM && d->effect[Health] < 0) ||
    (hm == Crime && d->effect[Law] < 0));

  if (matches) {
    if (!(b->flags & _buildingEnabled)) {
      setBuildingHighlight(ndx, false);
      setBuildingRedHighlight(ndx, false);
    } else {
      setBuildingHighlight(ndx, true);
      setBuildingRedHighlight(ndx, negative);
    }
  } else {
    setBuildingHighlight(ndx, false);
    setBuildingRedHighlight(ndx, false);
  }
}

void setAmenityHighlights() {
  for (int i = 0; i < governmentBuildings.size(); i++) {
    item ndx = governmentBuildings[i];
    setAmenityHighlight(ndx);
  }
}

float getAmenityThrow(item ndx) {
  Design* d = getDesign(ndx);
  float maxDist = c(CTileSize)*5;

  if (endsWith(d->name, "_quad_uni")) {
    float dist = c(CUniversityMaxQuadDistance);
    if (dist > maxDist) maxDist = dist;
  }

  if (d->effect[Nature] > 0) {
    float dist = c(CAmenityPollutionThrow);
    if (dist > maxDist) maxDist = dist;
  }

  if (d->effect[ValueEffect] > 0) {
    float dist = c(CAmenityValueThrow);
    if (dist > maxDist) maxDist = dist;
  }

  if (d->effect[Law] > 0) {
    float dist = c(CAmenityCrimeThrow);
    if (dist > maxDist) maxDist = dist;
  }

  if (d->effect[Health] > 0) {
    float dist = c(CHealthThrow);
    if (dist > maxDist) maxDist = dist;
  }

  if (d->effect[EducationEffect] > 0 || d->effect[Community] > 0) {
    float dist = c(CAmenityEduThrow);
    if (dist > maxDist) maxDist = dist;
  }

  return maxDist;
}

void updateGovernmentBuilding(item ndx, float duration) {
  if (getGameMode() == ModeBuildingDesigner) {
    return;
  }

  Building* b = getBuilding(ndx);
  Design* d = getDesign(b->design);
  float angle = randFloat(0, pi_o*2);
  float mag = randFloat(0, 1);
  vec3 thrw = vec3(sin(angle), cos(angle), 0.f) * mag;
  vec3 loc = getBuildingCenter(ndx);

  for (int i = 0; i < b->lots.size(); i++) {
    Lot* lot = getLot(b->lots[i]);
    lot->zone = GovernmentZone;
  }

  if ((b->flags & _buildingEnabled) && d->numBusinesses[Institution] > 0) {
    bool hasInstitution = false;
    for (int i = 0; i < b->businesses.size(); i++) {
      if (getBusinessType(b->businesses[i]) == Institution) {
        hasInstitution = true;
        break;
      }
    }

    if (!hasInstitution) {
      addBusiness(ndx, Institution);
    }
  }

  BuildingCategories designCategory = (BuildingCategories)d->category;
  BudgetLine budgetLine = buildingCategoryBudgetLine[designCategory];
  float controlEffect = getBudgetControlEffect(budgetLine);
  float maint = getMaintenance(ndx);
  bool enabled = (b->flags & _buildingEnabled) && (controlEffect > 0 || maint < 0);

  if (enabled) {
    float prosp = c(CAmenityProsperityBonus)*maint/1000000 +
      c(CAmenityProsperityEffect)*d->effect[ProsperityEffect]*controlEffect;
    heatMapAdd(Prosperity, loc, prosp*duration);

    float dens = c(CAmenityDensityEffect) * d->effect[DensityEffect];
    if (d->category != UniversityCategory) {
      dens += c(CAmenityDensityBonus)*maint/1000000;
    }
    if (dens != 0) {
      dens *= controlEffect;
      heatMapAdd(Density, loc, dens*duration);
    }

    if (d->effect[Nature] != 0) {
      heatMapAdd(Pollution, loc,
        c(CAmenityPollutionEffect) * (d->effect[Nature] * controlEffect) * duration);
      heatMapAdd(Pollution, loc+thrw*c(CAmenityPollutionThrow),
          c(CAmenityPollutionEffect) * (d->effect[Nature] * controlEffect) * duration);
    }

    if (d->effect[ValueEffect] != 0) {
      heatMapAdd(Value, loc,
        c(CAmenityValueEffect) * (d->effect[ValueEffect] * controlEffect) * duration);
      heatMapAdd(Value, loc+thrw*c(CAmenityValueThrow),
          c(CAmenityValueEffect) * (d->effect[ValueEffect] * controlEffect) * duration);
    }

    if (d->effect[Law] != 0) {
      float cEffect = c(CAmenityCrimeEffect) *
        (d->effect[Law] * getEffectMultiplier(Order) * controlEffect);
      heatMapAdd(Crime, loc, cEffect * duration);
      heatMapAdd(Crime, loc+thrw*c(CAmenityCrimeThrow), cEffect * duration);
    }

    if (d->effect[Health] != 0) {
      float hEffect = c(CAmenityHealthEffect) *
        (d->effect[Health] * controlEffect);
      heatMapAdd(HealthHM, loc, hEffect * duration);
      heatMapAdd(HealthHM, loc+thrw*c(CHealthThrow), hEffect * duration);
    }

    if(d->effect[Community] != 0) {
      float cEffect = c(CCommunityAmenityEffect) *
        (d->effect[Community] * controlEffect);
      heatMapAdd(CommunityHM, loc, cEffect * duration);
      heatMapAdd(CommunityHM, loc + thrw * c(CCommunityThrow),
        cEffect * duration);
    }

    bool doCommunity = randFloat() <
      duration * d->effect[Community]*c(CCommunityFreq);
    item epNdx=0, efNdx=0;
    bool eduTarget[4] = {false, false, false, false};
    float educationValue = d->effect[EducationEffect] * controlEffect *
      getEffectMultiplier(Technology);
    bool anyEdu = educationValue > 0;
    if (anyEdu) {
      anyEdu = false;
      for (int i = numEducationLevels; i > 0; i--) {
        //if (numPeople(ourCit) < 100) break;
        if (educationValue == 0) break;
        if (!(d->flags & (_designProvidesHSDiploma << (i-1)))) continue;
        int numEducated = getStatistic(b->econ, (Statistic)(NumNoEdu+i));
        int maxEducated = getEduLevelLimit(i);
        if (numEducated > maxEducated) continue;
        if (randFloat() > duration * educationValue*c(CEduFreq))
          continue;
         eduTarget[i] = true;
         anyEdu = true;
      }
    }

    if (anyEdu) {
      // Educate people randomly
      float numToEducate = duration * educationValue * c(CEduFreq);
      numToEducate = randRound(numToEducate);
      item numEducated = 0;
      // we'll try to educate numToEducate people

      // use up to three collision calls to find buildings in the area
      for (item i = 0; i < 3; i++) {
        Box rad = box(vec2(loc+thrw*c(CAmenityEduThrow)), tileSize*24.f);
        vector<item> collisions = getCollisions(BuildingCollisions, rad, ndx);
        item numColl = collisions.size();
        item collRnd = randItem(numColl);

        // look at up to 5 buildings in the collision
        for (item collNdx = 0; collNdx < numColl && collNdx < 5; collNdx++) {
          item ebNdx = collisions[(collRnd+collNdx)%numColl];
          if (ebNdx > 0) {
            Building* eb = getBuilding(ebNdx);

            // look at up to 5 families in this building
            item numFam = eb->families.size();
            item famRnd = randItem(numFam);
            for (item famNdx = 0; famNdx < numFam && famNdx < 5; famNdx++) {
              efNdx = eb->families[(famNdx+famRnd)%numFam];
              if (efNdx > 0) {
                Family* ef = getFamily(efNdx);

                // look at up to 5 members in this family
                item numMem = ef->members.size();
                item memRnd = randItem(numMem);
                for (item memNdx = 0; memNdx < numMem && memNdx < 5; memNdx++) {
                  item epNdx = ef->members[(memNdx+memRnd)%numMem];
                  if (epNdx > 0) {

                    // found a person, let's level them up
                    EducationLevel lvl = getEducationForPerson(epNdx);
                    item target = lvl+1;
                    if (eduTarget[target]) {
                      educatePerson(epNdx, (EducationLevel)target);
                      numEducated ++;
                      if (numEducated >= numToEducate) return; // done
                      break; // only educate one person at a time per family.
                    }
                  }
                }
              }
            }
          }
        }
      }

      //SPDLOG_INFO("educated: {}/{}", numEducated, int(numToEducate));
    }

    /*
    if (eduTarget > 0 || doCommunity) {
      Box rad = box(vec2(loc+thrw*c(CAmenityEduThrow)), tileSize*24.f);
      vector<item> collisions = getCollisions(BuildingCollisions, rad, ndx);
      item numColl = collisions.size();
      if (numColl > 0) {
        item ebNdx = collisions[randItem(numColl)];
        if (ebNdx > 0) {
          Building* eb = getBuilding(ebNdx);
          int numFam = eb->families.size();
          if (numFam > 0) {
            efNdx = eb->families[randItem(numFam)];
            if (efNdx > 0) {
              Family* ef = getFamily(efNdx);
              int numMem = ef->members.size();
              if (numMem > 0) {
                epNdx = ef->members[randItem(numMem)];
              }
            }
          }
        }
      }

      if (epNdx > 0) {
        if (eduTarget > 0) {
          educatePerson(epNdx, (EducationLevel)eduTarget);
        }

        if (doCommunity) {
          Family* f = getFamily(efNdx);
          if (!(f->flags & _familyIsTourists)) {
            float time = getCurrentDateTime();
            f->lastStoreTime = time;
            f->lastWorkTime = time;
          }
        }
      }
    }
    */

  } else {
    // Abandoned Building
    heatMapAdd(Crime, loc+thrw*1.f,
        10.f * duration);
  }
}

money getMaintenance(item bNdx) {
  Building* b = getBuilding(bNdx);
  Design* d = getDesign(b->design);
  float density = heatMapGet(Density, b->location);
  float multiplier = c(CAmenityMaintMult) * (1 + density * 4);
  BudgetLine l = buildingCategoryBudgetLine[d->category];
  multiplier *= getBudgetControl(l);
  if (d->maintenance < 0) multiplier = c(CAmenityMaintMult);
  return d->maintenance * multiplier * getInflation();
}

BudgetLine getBudgetLineFromBuildingCategory(char category) {
  if(category < 0 || category >= numBuildingCategories) {
    return NullBudget;
  }

  return buildingCategoryBudgetLine[category];
}

void makeGovernmentBuildingPayments(Budget* budget, double duration) {
  for (int i = 0; i < governmentBuildings.size(); i++) {
    Building* building = getBuilding(governmentBuildings[i]);
    if (building->flags & _buildingEnabled) {
      Design* d = getDesign(building->design);
      BudgetLine l = buildingCategoryBudgetLine[d->category];
      float amount = -getMaintenance(governmentBuildings[i]);
      if (amount > 0) l = AmenityIncome;
      amount *= duration;
      transaction(budget, l, amount);
    }
  }
}

void setGovBuildingEnabled(item buildingNdx, bool enabled) {
  Building* b = getBuilding(buildingNdx);
  if(b == 0) return;
  Design* d = getDesign(b->design);
  item e = b->econ;
  if(d == 0) return;

  bool isEdu = isDesignEducation(b->design);

  if (enabled && !(b->flags & _buildingEnabled)) {
    b->flags |= _buildingEnabled;

    heatMapAdd(Prosperity, b->location, 200.f);
    adjustAmenityStats(buildingNdx, 1);
    adjustStat(e, NumGovBuildingsEnabled, 1);
    adjustStat(e, NumParksEnabled + d->category, 1);
    addBuildingEffect(b->design);
    int numHomes = getDesignNumHomes(b->design);
    boardPut(b->econ, isEdu ? DormBunks : Homes, buildingNdx, numHomes);
    
    if (d->flags & _designProvidesHealthCare) {
      healthcareProviders.push_back(buildingNdx);
    }

    for (int i = 0; i < d->numBusinesses[Institution]; i++) {
      addBusiness(buildingNdx, Institution);
    }

    if (isEdu) {
      boardPut(b->econ, RetailTenancies,
          buildingNdx, d->numBusinesses[CSStorefronts]);

    } else {
      for (int i = 0; i < numBusinessTypes; i++) {
        if (i == Institution) continue;
        boardPut(b->econ, RetailTenancies+i, buildingNdx, d->numBusinesses[i]);
      }
    }

  } else if (!enabled && b->flags & _buildingEnabled) {
    b->flags &= ~_buildingEnabled;

    heatMapAdd(Prosperity, b->location, -200.f);
    adjustAmenityStats(buildingNdx, -1);
    adjustStat(e, NumGovBuildingsEnabled, -1);
    adjustStat(e, NumParksEnabled + d->category, -1);
    subtractBuildingEffect(b->design);
    boardClean(b->econ, isEdu ? DormBunks : Homes, buildingNdx);
    
    if(d->flags & _designProvidesHealthCare) {
      healthcareProviders.removeByValue(buildingNdx);
    }

    for (int i=b->businesses.size()-1; i >= 0; i--) {
      removeBusiness(b->businesses[i]);
    }

    if(isEdu) {
      boardClean(b->econ, RetailTenancies, buildingNdx);
    } else {
      for(int i = 0; i < numBusinessTypes; i++) {
        if(i == Institution) continue;
        boardClean(b->econ, RetailTenancies+i, buildingNdx);
      }
    }

    vector<item> prevFamilies(
        b->families.begin(), b->families.end());
    for (int i=b->families.size()-1; i >= 0; i--) {
      evictFamily(b->families[i], false);
    }
    for (int i = 0; i < prevFamilies.size(); i++) {
      handleHome(prevFamilies[i]);
    }
  }

  setAmenityHighlight(buildingNdx);
}

void sellGovBuilding(item buildingNdx) {
  Building* b = getBuilding(buildingNdx);
  Design* d = getDesign(b->design);
  updateBuildingValue(buildingNdx);
  float amount = getBuildingValue(buildingNdx);
  if (d->cost >= 0) amount *= c(CBuildingSalesFactor);
  transaction(d->cost < 0 ? EminentDomainExpenses : AssetSalesIncome, amount);
  removeBuilding(buildingNdx);
}

void resetEffectValueArr() {
  for(int i = 0; i < numEffects; i++) {
    effectValue[i] = 0;
  }
}

void resetGovernmentBuildings() {
  resetEffectValueArr();
  for (int i = 0; i <= numBuildingCategories; i++) {
    categoryCount[i] = 0;
  }
  govBuildingAvail.clear();
  governmentBuildings.clear();
  healthcareProviders.clear();
  govBuildingsPlaced.clear();
  universityQuad = 0;
}

void readGovernmentBuildings(FileBuffer* file, int version) {
  if (version < 35) {
    Cup<item>* dump = Cup<item>::newCup(100);
    for (int i = 0; i < 30; i++) {
      dump->read(file, version);
    }
    //free(dump);
  }

  for (int i = 1; i <= numBuildings(); i++) {
    Building* b = getBuilding(i);
    if ((b->flags & _buildingExists) && b->zone == GovernmentZone) {
      addGovernmentBuilding(i);
      if (b->flags & _buildingEnabled) {
        addBuildingEffect(b->design);
      }
    }
  }
}

void recalculateEffectsAllGovernmentBuildings() {
  resetEffectValueArr();
  for(int i = 0; i < governmentBuildings.size(); i++) {
    Building* b = getBuilding(governmentBuildings[i]);
    if (b->flags & _buildingEnabled) {
      addBuildingEffect(b->design);
    }
  }
}

void writeGovernmentBuildings(FileBuffer* file) {
}

