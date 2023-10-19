#include "achievement.hpp"

#include "feature.hpp"
#include "game.hpp"

#include "../amenity.hpp"
#include "../building/building.hpp"
#include "../building/design.hpp"
#include "../console/conDisplay.hpp"
#include "../economy.hpp"
#include "../error.hpp"
#include "../game/game.hpp"
#include "../graph.hpp"
#include "../graph/transit.hpp"
#include "../money.hpp"
#include "../option.hpp"
#include "../parts/messageBoard.hpp"
#include "../person.hpp"
#include "../plan.hpp"
#include "../platform/lua.hpp"
#include "../pool.hpp"
#include "../renum.hpp"
#include "../string_proxy.hpp"
#include "../time.hpp"
#include "../tools/road.hpp"
#include "../weather.hpp"
#include "../zone.hpp"

#include "spdlog/spdlog.h"
#include <string.h>
#include <boost/dynamic_bitset.hpp>

boost::dynamic_bitset<> achievementAcquired;
static item currentAchievement = 0;
static item achievementBuilding = 0;
static item numUnlocked = 0;
static float achTime = 0;

RenumTable achRenum;
Pool<Achievement> achievements;
unordered_map<string, item> achievementsByCode;
unordered_map<item, item> amenityForAchievement;

item getNumAchievements() { return achievements.size(); }
Achievement getAchievement(item ndx) { return *achievements.get(ndx); }

bool isAchievementAcquired(item ndx) {
  return achievementAcquired[ndx];
}

void setAchievementAcquired(item ndx, bool val) {
  if (val) {
    achievementAcquired[ndx] = true;
  } else {
    achievementAcquired[ndx] = false;
  }
}

int addAchievement(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  char* code = luaFieldString(L, "code");
  if (code == 0) {
    handleError("Must provide code for achievement");
    return 0;
  }
  item ndx = achievementsByCode[code];
  if (ndx == 0) ndx = achievements.create();
  achievementsByCode[code] = ndx;

  Achievement* ach = achievements.get(ndx);
  ach->code = code;
  ach->name = luaFieldString(L, "name");
  ach->text = luaFieldString(L, "text");
  ach->effect = luaFieldString(L, "effect");
  ach->hint = luaFieldString(L, "hint");
  ach->effectText = strdup_s("");

  int conditionType = luaFieldType(L, "condition");
  if (conditionType == 0) {
    handleError("Must provide condition function for achievement %s", code);
  } else if (conditionType == LUA_TSTRING) {
    ach->conditionText = luaFieldString(L, "condition");
    ach->conditionFunc = -1;
  } else {
    ach->conditionFunc = luaFieldFunction(L, "condition");
    ach->conditionText = luaFieldString(L, "associate_building");
  }

  if (ach->name == 0) {
    handleError("Must provide name for achievement %s", code);
  }
  if (ach->text == 0) {
    handleError("Must provide text for achievement %s", code);
  }
  if (ach->effect == 0) {
    handleError("Must provide effect for achievement %s", code);
  }

  pushRenum(&achRenum, ndx, ach->code);
  return 0;
}

bool testCondition(item ndx) {
  if (ndx <= 0 || ndx > achievements.size()) return false;
  Achievement* ach = achievements.get(ndx);
  achievementBuilding = 0;

  if (ach->conditionFunc > 0) {
    bool result = callLuaBooleanFunction(ach->conditionFunc, ach->code);
    if (result && ach->conditionText != 0) {
      if (streql(ach->conditionText, "skyscraper")) {
        achievementBuilding = getNewestSkyscraper();
      } else {
        achievementBuilding = newestBuildingForDesignKeyword(std::string(ach->conditionText));
      }
    }
    return result;
  }

  char* conditions = strdup_s(ach->conditionText);
  char* condptr = NULL;
  char* condition = strtok_r(conditions, ",", &condptr);
  bool met = true;
  item econ = ourCityEconNdx();

  while (met && condition != NULL) {
    char* dup = strdup_s(condition);
    char* typeptr = NULL;
    char* type = strtok_r(dup, ":", &typeptr);
    char* param = strtok_r(NULL, ":", &typeptr);
    int paramVal = param ? strtol(param, NULL, 10) : -1;

    if (streql(type, "population")) {
      if (paramVal > numPeople(ourCityEconNdx())) met = false;
    } else if (streql(type, "workers")) {
      if (paramVal > getStatistic(econ, NumWorkers)) met = false;
    } else if (streql(type, "unemployed")) {
      if (paramVal > getStatistic(econ, NumUnemployed)) met = false;
    } else if (streql(type, "employed")) {
      if (paramVal > getStatistic(econ, NumEmployed)) met = false;
    } else if (streql(type, "unemp_rate_below")) {
      if (paramVal < unemploymentRate(econ)*100) met = false;
    } else if (streql(type, "sick")) {
      if (paramVal > getStatistic(econ, PeopleSick)) met = false;
    } else if (streql(type, "tax_lock")) {
      if (!c(CEnableTaxLock)) met = false;

    } else if (streql(type, "loc_below")) {
      if (paramVal*1000*getInflation() <= getCredit()) met = false;
    } else if (streql(type, "loc_above")) {
      if (paramVal*1000*getInflation() > getCredit()) met = false;
    } else if (streql(type, "earnings_below")) {
      if (paramVal*1000*getInflation() <= getEarnings()) met = false;
    } else if (streql(type, "earnings_above")) {
      if (paramVal*1000*getInflation() > getEarnings()) met = false;

    } else if (streql(type, "year")) {
      if (paramVal > getBaseYear()) met = false;
    } else if (streql(type, "education")) {
      if (paramVal > getEffectValue(EducationEffect)) met = false;
    } else if (streql(type, "tech")) {
      if (paramVal > getEffectValue(Technology)) met = false;
    } else if (streql(type, "hsedu")) {
      if (paramVal > getStatistic(econ, HSEduPercent)*100) met = false;
    } else if (streql(type, "bcledu")) {
      if (paramVal > getStatistic(econ, BclEduPercent)*100) met = false;
    } else if (streql(type, "phdedu")) {
      if (paramVal > getStatistic(econ, PhdEduPercent)*100) met = false;

    } else if (streql(type, "colleges")) {
      int num = getStatistic(econ, NumColleges);
      if (num < paramVal) met = false;

    } else if (streql(type, "bunks")) {
      int bunks = getStatistic(econ, NumCollegeBunks);
      int students = getStatistic(econ, MaxCollegeStudents);
      int num = bunks - students;
      //SPDLOG_INFO("bunks {}", num);
      if (num < paramVal) met = false;

    } else if (streql(type, "nature")) {
      if (paramVal > getEffectValue(Nature)) met = false;
    } else if (streql(type, "law")) {
      if (paramVal > getEffectValue(Law)) met = false;
    } else if (streql(type, "wear")) {
      if (getMaxWear() < .5f) met = false;
    } else if (streql(type, "roads")) {
      if (numCompleteEdges() < paramVal) met = false;
    } else if (streql(type, "temp_above")) {
      if (getWeather().temp < paramVal) met = false;
    } else if (streql(type, "temp_below")) {
      if (getWeather().temp > paramVal) met = false;

    } else if (streql(type, "false")) {
      met = false;
    } else if (streql(type, "true")) {
      // pass

    } else if (streql(type, "building")) {
      char* paramDup = strdup_s(param);
      char* paramPtr = NULL;
      char* test = strtok_r(paramDup, ".", &paramPtr);
      item zone = 0;
      item xSize = 0;
      while (met && test != NULL) {

        if (test[0] == 'r') {
          zone = ResidentialZone;
        } else if (test[0] == 'm') {
          zone = MixedUseZone;
        } else if (test[0] == 'c') {
          zone = RetailZone;
        } else if (test[0] == 'o') {
          zone = OfficeZone;
        } else if (test[0] == 'a') {
          zone = FarmZone;
        } else if (test[0] == 'i') {
          zone = FactoryZone;

        } else if (test[0] == 'd') {
          float subParamVal = strtol(&test[1], NULL, 10);
          float val = getMaxDensity(zone, xSize)*10+.1;
          achievementBuilding = getMaxDensityBuilding(zone, xSize);
          if (val < subParamVal) met = false;

        } else if (test[0] == 'v') {
          float subParamVal = strtol(&test[1], NULL, 10);
          float val = getMaxLandValues(zone, xSize)*10+.1;
          achievementBuilding = getMaxLandValuesBuilding(zone, xSize);
          if (val < subParamVal) met = false;

        } else if (test[0] == 'x') {
          float subParamVal = strtol(&test[1], NULL, 10);
          xSize = subParamVal;

        } else if (test[0] == 'z') {
          float subParamVal = strtol(&test[1], NULL, 10);
          float val = getMaxFloors(zone, xSize);
          achievementBuilding = getMaxFloorsBuilding(zone, xSize);
          if (val < subParamVal) met = false;
        }

        test = strtok_r(NULL, ".", &paramPtr);
      }
      free(paramDup);

    } else if (streql(type, "amenity_built")) {
      vector<item> dNdxs = getDesignsByName(param);
      bool matched = false;
      for (int i = 0; i < dNdxs.size(); i++) {
        if (getGovBuildingsPlaced(dNdxs[i]) > 0) {
          matched = true;
          break;
        }
      }
      if (!matched) met = false;

    } else {
      SPDLOG_INFO("missing condition:{} param:{} in achievement code:{}",
          type, param, ach->code);
      met = false;
    }

    condition = strtok_r(NULL, ",", &condptr);
    free(dup);
  }

  free(conditions);
  return met;
}

void applyEffects(item ndx, bool blink) {
  numUnlocked++;

  Achievement* ach = achievements.get(ndx);
  char* effects = strdup_s(ach->effect);
  char* effptr = NULL;
  char* effect = strtok_r(effects, ",", &effptr);
  char* effectString = strdup_s("");

  while (effect != NULL) {
    // SPDLOG_INFO("applyEffects {}", effect);
    if (effect[0] == ':') {
      if (effect[1] == 'w') {
        winGame(achievementBuilding);

      } else if (effect[1] == 't') {
        if (effect[2] == 'l') {
          if(c(CEnableTaxLock)) {
            int tax = strtol(&effect[3], NULL, 10);
            lockTax(tax);
          } else {
            SPDLOG_INFO("Not locking taxes, tax lock disabled");
          }
        }
      }

    } else {
      bool found = false;
      for (int i = 0; i < numFeatures; i++) {
        if (streql(getFeatureCode(i), effect)) {
          found = true;
          if (i >= FPropertyTax && i <= FFinesAndFees) {
            enableTax(i-FPropertyTax+1);
          }
          if (i == FFuelTax) {
            enableTax(FuelTaxIncome);
          }

          //if (isFeatureEnabled(i)) break;
          if (i == FRoadCut && !c(CEnableCutTool)) break;
          setFeatureEnabled(i, true);
          //if (blink) {
            //setFeatureBlink(i, true);
          //}

          if (i == FRail || i == FBus) {
            initTransit();
          }

          /*
          if (tutorialMode()) {
            char* prevEffectString = effectString;
            effectString = sprintf_o("%s%s\n",
              prevEffectString, getFeatureDescriptor(i));
            free(prevEffectString);
          }
          */

          break;
        }
      }

      if (!found) {
        vector<item> dNdxs = getDesignsByName(effect);
        for (int i = 0; i < dNdxs.size(); i++) {
          item designNdx = dNdxs[i];
          Design* d = getDesign(designNdx);
          if (d->zone == GovernmentZone &&
              d->minYear <= getCurrentYear() &&
              d->maxYear >= getCurrentYear() &&
              (d->flags & _designEnabled)) {
              // && !getGovBuildingAvailable(designNdx])) {
            // We may have already unlocked it, but we still want to add the 
            // effect text, so removing the !getGovBuildingAvailable test
            setGovBuildingAvailable(designNdx, true);

            char* prevEffectString = effectString;
            effectString = sprintf_o("%sNew Amenity: %s\n",
              prevEffectString, d->displayName);
            free(prevEffectString);

            if (amenityForAchievement[ndx] == 0) {
              amenityForAchievement[ndx] = designNdx;
            }
          }
        }

        if (dNdxs.size() == 0) {
          SPDLOG_INFO("Feature or Amenity not found: {}", effect);
        } else if (!isFeatureEnabled(FBuildingTool)) {
          setFeatureEnabled(FBuildingTool, true);

          char* prevEffectString = effectString;
          effectString = sprintf_o("%s%s\n",
            prevEffectString, getFeatureDescriptor(FBuildingTool));
          free(prevEffectString);
        }
      }
    }
    effect = strtok_r(NULL, ",", &effptr);
  }

  if (ach->hint != 0 && strlen(ach->hint) > 0) {
    char* prevEffectString = effectString;
    effectString = sprintf_o("%s\n \n%s",
      prevEffectString, ach->hint);
    free(prevEffectString);
  }

  ach->effectText = effectString;
}

void updateAchievements(double duration) {
  if (getGameMode() != ModeGame) return;

  achTime += duration;
  const float timePerAch = 1.0f/achievements.size();
  while (achTime > 1+timePerAch) {
    achTime -= timePerAch;
    currentAchievement = currentAchievement % achievements.size() + 1;

    if (isAchievementAcquired(currentAchievement)) {
      continue;
    }

    Achievement* ach = achievements.get(currentAchievement);
    bool isUni = c(CAllowUniversityAtStart) &&
      streql(ach->effect, "_quad_uni");
    if (testCondition(currentAchievement) || isUni) {

      char* dtStr = printDateTimeString(getCurrentDateTime());
      SPDLOG_INFO("Achievement Acquired: {} on {} w/ {}pop",
          ach->name, dtStr,
          numPeople(ourCityEconNdx()));
      free(dtStr);

      setAchievementAcquired(currentAchievement, true);
      applyEffects(currentAchievement, true);

      addMessage(AchievementMessage, currentAchievement, achievementBuilding);

      // If a building gives you an achievement, mark it as historical so the message dialog does not link to another building.
      if (achievementBuilding <= 0) return;

      Building* buildAch = getBuilding(achievementBuilding);

      if (buildAch == 0) return;

      buildAch->flags |= _buildingHistorical;
    }
  }
}

const char* getAchievementName(item ndx) {
  return achievements.get(ndx)->name;
}

const char* getAchievementText(item ndx) {
  return achievements.get(ndx)->text;
}

void writeAchievements(FileBuffer* file) {
  renumWrite(&achRenum, file);

  item achievementAcquiredSize = achievements.size()/8+1;
  char* achBytes = (char*) calloc(achievementAcquiredSize, 1);
  for (int i = 0; i <= achievements.size(); i++) {
    if (achievementAcquired[i]) {
      achBytes[i/8] |= 1 << (i%8);
    }
  }

  fwrite_int(file, achievementAcquiredSize);
  fwrite(achBytes, sizeof(char), achievementAcquiredSize, file);
  free(achBytes);
}

void readAchievements(FileBuffer* file, int version) {
  if (version >= 56) {
    renumRead(&achRenum, file);
  } else {
    renumLegacyStart(&achRenum);
    #define RENUM(N) renumLegacy(&achRenum, #N);
    #include "achievements.v55.renum"
    #undef RENUM
  }

  /*
  int achSize = achievements.size();
  Achievement* achPtr = 0;
  SPDLOG_INFO("achievements.size(): {}", achSize);
  for(int i = 1; i <= achSize; i++) {
    achPtr = achievements.get(i);
    if(achPtr != 0) {
      SPDLOG_INFO("{}, achPtr->name: {}", i, achPtr->name);
    } else {
      SPDLOG_INFO("{}, null achPtr", i);
    }
  }
  */

  if (version >= 38) {
    achievementAcquired.resize(achievements.size()+1);
    item achievementAcquiredSize = fread_int(file);
    char* achBytes = (char*) calloc(achievementAcquiredSize, 1);
    fread(achBytes, sizeof(char), achievementAcquiredSize, file);
    for (int i = 1; i <= achievements.size() &&
        i/8 < achievementAcquiredSize; i++) {

      item achNum = renum(&achRenum, i);
      Achievement* ach = achievements.get(i);
      if (achNum <= 0 || achNum > achievements.size()) {
        // no-op
      } else if (achBytes[i/8] & (1 << (i%8))) {
        //SPDLOG_INFO("{}, Ach {} acquired ({})",
            //i, ach != 0 ? ach->name : "Unknown", ach->effect );
        achievementAcquired[achNum] = true;
      } else {
        //SPDLOG_INFO("{}, Ach {} locked ({})",
            //i, ach != 0 ? ach->name : "Unknown", ach->effect );
        achievementAcquired[achNum] = false;
      }
    }
    free(achBytes);
  }
}

void initAchievements() {
  achievementBuilding = 0;
  numUnlocked = 0;
  achievementAcquired.resize(achievements.size()+1);

  // Core unlocks for new cities 
  setFeatureEnabled(FPlay, true);
  setFeatureEnabled(FNewspaper, true);
  setFeatureEnabled(FNoteBudget, true);
  setFeatureEnabled(FPropertyTax, true);
  setFeatureEnabled(FSalesTax, true);
  setFeatureEnabled(FNoteChart, true);
  setFeatureEnabled(FNoteObject, true);

  /*
  if (wasFeatureUsed(FNoteChart)) {
    setFeatureEnabled(FNoteChart, true);
  }
  if (wasFeatureUsed(FNoteObject)) {
    setFeatureEnabled(FNoteObject, true);
  }
  */

  if (getGameMode() == ModeTest) {
    for (int i = 0; i < numFeatures; i++) {
      setFeatureEnabled(i, true);
    }

  } else {
    for (int i = 1; i <= achievements.size(); i++) {
      if (isAchievementAcquired(i)) {
        applyEffects(i, false);
      }
    }
  }

  // Unlock everything except amenities if gameAccelerated is true
  if (getGameAccelerated()) {
    for(int i = 0; i < numFeatures; i++) {
      if (i == FTransitSystems) continue;
      if (i == FBuildingTool) continue;
      setFeatureEnabled(i, true);
    }
  }

  initTransit();
}

void initAchievementsLua() {
  achievementsByCode.clear();
  achievements.clear();
  achievementAcquired.clear();
  amenityForAchievement.clear();
  renumClear(&achRenum);

  addLuaCallback("addAchievement", addAchievement);
}

void resetAchievements() {
  achievementAcquired.clear();
  achievementAcquired.resize(achievements.size());

  for (int i = 1; i <= achievements.size(); i++) {
    Achievement* ach = achievements.get(i);
    free(ach->code);
    free(ach->name);
    free(ach->text);
    free(ach->effect);
    free(ach->effectText);
    free(ach->hint);
  }
  achievements.clear();
}

item getAmenityForAchievement(item achNdx) {
  return amenityForAchievement[achNdx];
}

item numAchievementsUnlocked() {
  return numUnlocked;
}

