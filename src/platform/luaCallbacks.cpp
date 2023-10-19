#include "../amenity.hpp"
#include "../business.hpp"
#include "../building/design.hpp"
#include "../draw/texture.hpp"
#include "../draw/image.hpp"
#include "../economy.hpp"
#include "../game/feature.hpp"
#include "../land.hpp"
#include "../money.hpp"
#include "../name.hpp"
#include "../person.hpp"
#include "../time.hpp"
#include "../util.hpp"

int luaCB_inflate(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) {
    lua_pushnumber(L, 0);
    return 1;
  }

  float in = luaL_checknumber(L, 1);
  lua_pushnumber(L, in*getInflation());
  return 1;
}

int luaCB_formatInt(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) {
    lua_pushstring(L, "Error: Missing Argument to formatInt");
  }

  float amount = luaL_checknumber(L, 1);
  const char* neg = amount < -0.1 ? "-" : "";
  amount = abs(amount);
  char* result;
  if (amount >= 1000000000) { // 1B
    result = sprintf_o("%s%.2fB", neg, amount/1000000000.f);
  } else if (amount >= 100000000) { // 100M
    result = sprintf_o("%s%.0fM", neg, amount/1000000.f);
  } else if (amount >= 10000000) { // 10M
    result = sprintf_o("%s%.1fM", neg, amount/1000000.f);
  } else if (amount >= 1000000) { // 1M
    result = sprintf_o("%s%.2fM", neg, amount/1000000.f);
  } else if (amount >= 10000) { // 10K
    result = sprintf_o("%s%.0fK", neg, amount/1000.f);
  } else if (amount >= 1000) {
    result = sprintf_o("%s%d,%03d", neg, int(amount/1000), int(amount)%1000);
  } else {
    result = sprintf_o("%s%d", neg, int(amount));
  }

  lua_pushstring_free(L, result);
  return 1;
}

int luaCB_formatFloat(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) {
    lua_pushstring(L, "Error: Missing Argument to formatInt");
  }

  float val = luaL_checknumber(L, 1);
  lua_pushstring_free(L, formatFloat(val));
  return 1;
}

int luaCB_formatPercent(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) {
    lua_pushstring(L, "Error: Missing Argument to formatInt");
  }

  float val = luaL_checknumber(L, 1);
  char* res;
  if (abs(val) >= 100) {
    char* temp = formatFloat(val);
    res = sprintf_o("%sx", temp);
    free(temp);
  } else if (abs(val) > 0.03) {
    res = sprintf_o("%d%%", int(val*100));
  } else {
    char* temp = formatFloat(val*100);
    res = sprintf_o("%s%%", temp);
    free(temp);
  }

  lua_pushstring_free(L, res);
  return 1;
}

int luaCB_formatMoney(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  float in = luaL_checknumber(L, 1);
  lua_pushstring_free(L, printMoneyString(in));
  return 1;
}

int luaCB_budgetLine(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) {
    handleError("budgetLine() requires one parameter");
    lua_pushnumber(L, 0);
    return 1;
  }

  item budgetLine = luaL_checknumber(L, 1);
  if (budgetLine < 0 || budgetLine >= numBudgetLines) {
    handleError("budgetLine() first paramter must be a BudgetLine");
    lua_pushnumber(L, 0);
    return 1;
  }

  item budget = numArgs >= 2 ? luaL_checknumber(L, 2) : 0;
  if (budget > 2) {
    handleError("budgetLine() second paramter out of bounds");
    budget = 2;
  }

  if (budget < -getNumHistoricalBudgets()) {
    lua_pushnumber(L, 0);
    return 1;
  }

  lua_pushnumber(L, getBudget(budget).line[budgetLine]);
  return 1;
}

int luaCB_budgetControl(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) {
    handleError("budgetControl() requires one parameter");
    lua_pushnumber(L, 0);
    return 1;
  }

  item budgetLine = luaL_checknumber(L, 1);
  if (budgetLine < 0 || budgetLine >= numBudgetLines) {
    handleError("budgetControl() first paramter must be a BudgetLine");
    lua_pushnumber(L, 0);
    return 1;
  }

  float result = 0;
  if (budgetLine >= PropertyTax && budgetLine <= FinesAndFeesIncome) {
    result = getTaxRate((BudgetLine)budgetLine);
  } else if (budgetLine >= EducationExpenses && budgetLine <= UniversityExpenses) {
    result = getBudgetControl((BudgetLine)budgetLine);
  } else if (budgetLine == LineOfCredit) {
    result = getLoanRepaymentTime();
  }

  lua_pushnumber(L, result);
  return 1;
}

int luaCB_setBudgetControl(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 2) {
    handleError("setBudgetControl() requires two parameters");
    return 0;
  }

  item budgetLine = luaL_checknumber(L, 1);
  if (budgetLine < 0 || budgetLine >= numBudgetLines) {
    handleError("setBudgetControl() first paramter must be a BudgetLine");
    return 0;
  }

  float val = luaL_checknumber(L, 2);
  if (budgetLine >= PropertyTax && budgetLine <= FinesAndFeesIncome) {
    setTaxRate((BudgetLine)budgetLine, val);
  } else if (budgetLine >= EducationExpenses && budgetLine <= UniversityExpenses) {
    setBudgetControl((BudgetLine)budgetLine, val);
  } else if (budgetLine == LineOfCredit) {
    setLoanRepaymentTime(val);
  }

  return 0;
}

int luaCB_canBuy(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) {
    handleError("canBuy() requires one parameter");
    lua_pushboolean(L, false);
    return 1;
  }

  float amount = luaL_checknumber(L, 1);
  lua_pushboolean(L, canBuy(RoadBuildExpenses, amount));
  return 1;
}

int luaCB_transaction(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 2) {
    handleError("transaction() requires two parameters");
    lua_pushboolean(L, false);
    return 1;
  }

  item budgetLine = luaL_checknumber(L, 1);
  if (budgetLine < 0 || budgetLine >= numBudgetLines) {
    handleError("transaction() first paramter must be a BudgetLine");
    lua_pushboolean(L, false);
    return 1;
  }

  float amount = luaL_checknumber(L, 2);
  lua_pushboolean(L, transaction((BudgetLine) budgetLine, amount));
  return 1;
}

int luaCB_formatDuration(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  float in = luaL_checknumber(L, 1);
  lua_pushstring_free(L, printDurationString(in));
  return 1;
}

int luaCB_formatDurationNice(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  float in = luaL_checknumber(L, 1);
  lua_pushstring_free(L, printNiceDurationString(in));
  return 1;
}

int luaCB_formatDateTime(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  float in = luaL_checknumber(L, 1);
  lua_pushstring_free(L, printDateTimeString(in));
  return 1;
}

int luaCB_formatDate(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  float in = luaL_checknumber(L, 1);
  lua_pushstring_free(L, printDateString(in));
  return 1;
}

int luaCB_formatTime(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  float in = luaL_checknumber(L, 1);
  lua_pushstring_free(L, printTimeString(in));
  return 1;
}

int luaCB_formatSeason(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  float in = luaL_checknumber(L, 1);
  lua_pushstring_free(L, printSeasonString(in));
  return 1;
}

int luaCB_formatYear(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  float time = luaL_checknumber(L, 1);
  int year = time/oneYear + c(CStartYear);
  lua_pushstring_free(L, sprintf_o("%d", year));
  return 1;
}

int luaCB_format2DigitYear(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  float time = luaL_checknumber(L, 1);
  int year = time/oneYear + c(CStartYear);
  lua_pushstring_free(L, sprintf_o("%d", year%100));
  return 1;
}

int luaCB_featureEnabled(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  item feature = luaL_checknumber(L, 1);
  lua_pushboolean(L, isFeatureEnabled(feature));
  return 1;
}

int luaCB_setFeatureEnabled(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  item feature = luaL_checknumber(L, 1);
  bool val = lua_toboolean(L, 2);
  setFeatureEnabled(feature, val);
  return 0;
}

int luaCB_amenityEnabled(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  const char* param = luaL_checkstring(L, 1);
  vector<item> dNdxs = getDesignsByName(param);
  bool enabled = false;
  for (int i = 0; i < dNdxs.size(); i++) {
    if (getGovBuildingAvailable(dNdxs[i])) {
      enabled = true;
      break;
    }
  }
  lua_pushboolean(L, enabled);
  return 1;
}

int luaCB_amenitiesBuilt(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  const char* param = luaL_checkstring(L, 1);
  vector<item> dNdxs = getDesignsByName(param);
  item num = 0;
  for (int i = 0; i < dNdxs.size(); i++) {
    num += getGovBuildingsPlaced(dNdxs[i]);
  }
  lua_pushnumber(L, num);
  return 1;
}

int luaCB_amenityEffectScore(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  item param = luaL_checknumber(L, 1);
  float val = getEffectValue(param);
  lua_pushnumber(L, val);
  return 1;
}

int luaCB_amenityEffectMultiplier(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  item param = luaL_checknumber(L, 1);
  float val = getEffectMultiplier(param);
  lua_pushnumber(L, val);
  return 1;
}

int luaCB_amenityEffectString(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  item param = luaL_checknumber(L, 1);
  lua_pushstring(L, getEffectString(param));
  return 1;
}

int luaCB_amenityEffectDescription(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  item param = luaL_checknumber(L, 1);
  lua_pushstring_free(L, getEffectDescriptor(param));
  return 1;
}

int luaCB_stat(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  item econNdx = numArgs >= 2 ? luaL_checknumber(L, 2) : ourCityEconNdx();
  item stat = luaL_checknumber(L, 1);
  //SPDLOG_INFO("stat {} is {}", getStatisticCode(stat),
      //getStatisticNow(ourCityEconNdx(), (Statistic)stat));
  lua_pushnumber(L, getStatisticNow(econNdx, (Statistic)stat));
  return 1;
}

int luaCB_setStat(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 2) return 0;

  item econNdx = numArgs >= 3 ? luaL_checknumber(L, 2) : ourCityEconNdx();
  item stat = luaL_checknumber(L, 1);
  float value = luaL_checknumber(L, numArgs >= 3 ? 3 : 2);
  setStat(econNdx, stat, value);
  return 0;
}

int luaCB_statPast(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 2) return 0;

  item econNdx = numArgs >= 3 ? luaL_checknumber(L, 3) : ourCityEconNdx();
  item stat = luaL_checknumber(L, 1);
  float timeDiff = luaL_checknumber(L, 2);
  float prevVal = getStatisticAt(econNdx, (Statistic)stat,
      getCurrentDateTime() - timeDiff);
  lua_pushnumber(L, prevVal);
  return 1;
}

int luaCB_statGrowth(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 2) return 0;

  item econNdx = numArgs >= 3 ? luaL_checknumber(L, 3) : ourCityEconNdx();
  item stat = luaL_checknumber(L, 1);
  float timeDiff = luaL_checknumber(L, 2);
  float val = getStatisticNow(econNdx, (Statistic)stat);
  float prevVal = getStatisticAt(econNdx, (Statistic)stat,
      getCurrentDateTime() - timeDiff);
  float growth = (prevVal != 0) ? (val/prevVal - 1) :
    ((val == 0) ? 0 : ((val < 0) ? -1 : 1));
  lua_pushnumber(L, growth);
  return 1;
}

int luaCB_statDelta(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 2) return 0;

  item econNdx = numArgs >= 3 ? luaL_checknumber(L, 3) : ourCityEconNdx();
  item stat = luaL_checknumber(L, 1);
  float timeDiff = luaL_checknumber(L, 2);
  float val = getStatisticNow(econNdx, (Statistic)stat);
  float prevVal = getStatisticAt(econNdx, (Statistic)stat,
      getCurrentDateTime() - timeDiff);
  float delta = val - prevVal;
  //SPDLOG_INFO("statDelta {}@{}/{} {} - {} = {}", getStatisticCode(stat),
      //printDateTimeString(getCurrentDateTime()-timeDiff),
      //getCurrentDateTime()-timeDiff,
      //val, prevVal, delta);
  lua_pushnumber(L, delta);
  return 1;
}

int luaCB_statSum(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 2) return 0;

  item econNdx = numArgs >= 3 ? luaL_checknumber(L, 3) : ourCityEconNdx();
  item stat = luaL_checknumber(L, 1);
  float timeDiff = luaL_checknumber(L, 2);
  float val = getStatisticSum(econNdx, (Statistic)stat,
      getCurrentDateTime()-timeDiff);
  lua_pushnumber(L, val);
  return 1;
}

int luaCB_statAvg(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 2) return 0;

  item econNdx = numArgs >= 3 ? luaL_checknumber(L, 3) : ourCityEconNdx();
  item stat = luaL_checknumber(L, 1);
  float timeDiff = luaL_checknumber(L, 2);
  float val = getStatisticAvg(econNdx, (Statistic)stat, timeDiff);
  lua_pushnumber(L, val);
  return 1;
}

int luaCB_statMin(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 2) return 0;

  item econNdx = numArgs >= 3 ? luaL_checknumber(L, 3) : ourCityEconNdx();
  item stat = luaL_checknumber(L, 1);
  float timeDiff = luaL_checknumber(L, 2);
  float val = getStatisticMin(econNdx, (Statistic)stat,
      getCurrentDateTime()-timeDiff);
  lua_pushnumber(L, val);
  return 1;
}

int luaCB_statMax(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 2) return 0;

  item econNdx = numArgs >= 3 ? luaL_checknumber(L, 3) : ourCityEconNdx();
  item stat = luaL_checknumber(L, 1);
  float timeDiff = luaL_checknumber(L, 2);
  float val = getStatisticMax(econNdx, (Statistic)stat,
      getCurrentDateTime()-timeDiff);
  lua_pushnumber(L, val);
  return 1;
}

int luaCB_log(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;
  SPDLOG_INFO("{}", lua_tostring(L, 1));
  return 0;
}

int luaCB_logWarn(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;
  SPDLOG_WARN("{}", lua_tostring(L, 1));
  return 0;
}

int luaCB_logError(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;
  SPDLOG_ERROR("{}", lua_tostring(L, 1));
  return 0;
}

int luaCB_handleError(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;
  handleError(lua_tostring(L, 1));
  return 0;
}

int luaCB_setGameSpeed(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;
  item speed = luaL_checknumber(L, 1);

  if (speed < 0 || speed > 7) {
    handleError("setGameSpeed called with invalid value");
    return 0;
  }

  setGameSpeed(speed);

  return 0;
}

int luaCB_now(lua_State* L) {
  lua_pushnumber(L, getCurrentDateTime());
  return 1;
}

int luaCB_timeNow(lua_State* L) {
  lua_pushnumber(L, getCurrentTime());
  return 1;
}

int luaCB_year(lua_State* L) {
  float time = 0;
  int numArgs = lua_gettop(L);
  if (numArgs < 1) {
    time = getCurrentDateTime();
  } else {
    time = luaL_checknumber(L, 1);
  }
  lua_pushnumber(L, time/oneYear + c(CStartYear));
  return 1;
}

int luaCB_getMapSize(lua_State* L) {
  lua_pushnumber(L, getMapSize());
  return 1;
}

int luaCB_getTerrainElevation(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  vec3 target;
  target.x = luaFieldNumber(L, "x");
  target.y = luaFieldNumber(L, "y");
  target.z = 0;

  lua_pushnumber(L, pointOnLand(target).z);
  return 1;
}

int luaCB_sampleBlueNoise(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  float x = luaFieldNumber(L, "x");
  float y = luaFieldNumber(L, "y");

  Image blueNoise = getBlueNoiseImage();
  vec4 result = samplePixel(blueNoise, x, y);

  lua_newtable(L);
  luaSetTableNumber(L, "x", result.x);
  luaSetTableNumber(L, "y", result.y);
  luaSetTableNumber(L, "z", result.z);
  luaSetTableNumber(L, "w", result.w);

  return 1;
}

int luaCB_cityName(lua_State* L) {
  lua_pushstring(L, getCityName());
  return 1;
}

int luaCB_randomCityName(lua_State* L) {
  lua_pushstring(L, randomName(NameTypes::CityName));
  return 1;
}

int luaCB_randomName(lua_State* L) {
  if (numPeople(ourCityEconNdx()) == 0) {
    lua_pushstring(L, "[no people]");
    return 1;
  }

  const char* name = 0;
  for (int k = 0; k < 100 && name == 0; k ++) {
    item ndx = randItem(sizePeople())+1;
    Person* person = getPerson(ndx);
    if (person->flags & _personExists) {
      name = personName(ndx);
    }
  }

  lua_pushstring(L, name);
  return 1;
}

int luaCB_randomMansName(lua_State* L) {
  if (numPeople(ourCityEconNdx()) == 0) {
    lua_pushstring(L, "[no people]");
    return 1;
  }

  const char* name = 0;
  for (int k = 0; k < 100 && name == 0; k ++) {
    item ndx = randItem(sizePeople())+1;
    Person* person = getPerson(ndx);
    if ((person->flags & _personExists) && !(person->flags & _personIsFemale)) {
      name = personName(ndx);
    }
  }

  lua_pushstring(L, name);
  return 1;
}

int luaCB_randomWomansName(lua_State* L) {
  if (numPeople(ourCityEconNdx()) == 0) {
    lua_pushstring(L, "[no people]");
    return 1;
  }

  const char* name = 0;
  for (int k = 0; k < 100 && name == 0; k ++) {
    item ndx = randItem(sizePeople())+1;
    Person* person = getPerson(ndx);
    if ((person->flags & _personExists) && (person->flags & _personIsFemale)) {
      name = personName(ndx);
    }
  }

  lua_pushstring(L, name);
  return 1;
}

int luaCB_randomStreetName(lua_State* L) {
  lua_pushstring(L, randomName(RoadName));
  return 1;
}

int luaCB_randomAddress(lua_State* L) {
  item num = randItem(1, 10000);
  const char* roadName = randomName(RoadName);
  const char* dirs[4] = {"N", "S", "E", "W"};
  const char* dir = dirs[randItem(0,3)];
  const char* roadTypes[4] = {"St", "Ave", "Blvd", "Ln"};
  const char* roadType = roadTypes[randItem(0,3)];
  char* result = sprintf_o("%d %s %s %s", num, dir, roadName, roadType);
  lua_pushstring_free(L, result);
  return 1;
}

int luaCB_randomPhoneNumber(lua_State* L) {
  char* result = sprintf_o("%03d-%04d", randItem(200,999), randItem(0,9999));
  lua_pushstring_free(L, result);
  return 1;
}

int luaCB_randomBusinessName(lua_State* L) {
  if (numBusinesses(ourCityEconNdx()) == 0) {
    lua_pushstring(L, "[no businesses]");
    return 1;
  }

  const char* name = 0;
  for (int k = 0; k < 100 && name == 0; k ++) {
    item ndx = randItem(sizeBusinesses())+1;
    Business* business = getBusiness(ndx);
    if (business->flags & _businessExists) {
      name = business->name;
    }
  }

  lua_pushstring(L, name);
  return 1;
}

int luaCB_randomTeam(lua_State* L) {
  lua_pushstring(L, randomName(SportsTeamName));
  return 1;
}

int luaCB_randomInt(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 2) return 0;

  int min = luaL_checknumber(L, 1);
  int max = luaL_checknumber(L, 2);
  lua_pushnumber(L, randItem(max-min)+min);
  return 1;
}

int luaCB_randomFloat(lua_State* L) {
  float min = 0;
  float max = 1;

  int numArgs = lua_gettop(L);
  if (numArgs >= 2) {
    min = luaL_checknumber(L, 1);
    max = luaL_checknumber(L, 2);
  }

  lua_pushnumber(L, randFloat(min, max));
  return 1;
}

int luaCB_selectIf(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 3) return 0;

  float condition = lua_toboolean(L, 1);
  const char* result1 = luaL_checkstring(L, 2);
  const char* result2 = luaL_checkstring(L, 3);
  lua_pushstring(L, condition != 0 ? result1 : result2);
  return 1;
}

int luaCB_selectRandom(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs < 1) return 0;

  vector<const char*> results;
  for (int i = 0; i < numArgs; i++) {
    results.push_back(luaL_checkstring(L, i+1));
  }
  lua_pushstring(L, results[randItem(numArgs)]);
  return 1;
}

void initLuaGeneralCallbacks() {
  addLuaCallback("inflate", luaCB_inflate);
  addLuaCallback("formatMoney", luaCB_formatMoney);
  addLuaCallback("budgetLine", luaCB_budgetLine);
  addLuaCallback("budgetControl", luaCB_budgetControl);
  addLuaCallback("setBudgetControl", luaCB_setBudgetControl);
  addLuaCallback("canBuy", luaCB_canBuy);
  addLuaCallback("transaction", luaCB_transaction);

  addLuaCallback("featureEnabled", luaCB_featureEnabled);
  addLuaCallback("setFeatureEnabled", luaCB_setFeatureEnabled);

  addLuaCallback("amenitiesBuilt", luaCB_amenitiesBuilt);
  addLuaCallback("amenityEnabled", luaCB_amenityEnabled);
  addLuaCallback("amenityEffectScore", luaCB_amenityEffectScore);
  addLuaCallback("amenityEffectMultiplier", luaCB_amenityEffectMultiplier);
  addLuaCallback("amenityEffectDescription", luaCB_amenityEffectDescription);

  addLuaCallback("cityName", luaCB_cityName);
  addLuaCallback("randomCityName", luaCB_randomCityName);
  addLuaCallback("randomName", luaCB_randomName);
  addLuaCallback("randomMansName", luaCB_randomMansName);
  addLuaCallback("randomWomansName", luaCB_randomWomansName);
  addLuaCallback("randomBusinessName", luaCB_randomBusinessName);
  addLuaCallback("randomTeam", luaCB_randomTeam);
  addLuaCallback("randomStreetName", luaCB_randomStreetName);
  addLuaCallback("randomAddress", luaCB_randomAddress);
  addLuaCallback("randomPhoneNumber", luaCB_randomPhoneNumber);

  addLuaCallback("getMapSize", luaCB_getMapSize);
  addLuaCallback("getTerrainElevation", luaCB_getTerrainElevation);

  addLuaCallback("sampleBlueNoise", luaCB_sampleBlueNoise);

  addLuaCallback("selectIf", luaCB_selectIf);
  addLuaCallback("selectRandom", luaCB_selectRandom);
  addLuaCallback("randomInt", luaCB_randomInt);
  addLuaCallback("randomFloat", luaCB_randomFloat);
  addLuaCallback("formatInt", luaCB_formatInt);
  addLuaCallback("formatFloat", luaCB_formatFloat);
  addLuaCallback("formatPercent", luaCB_formatPercent);

  addLuaCallback("get", luaCB_stat);
  addLuaCallback("getPast", luaCB_statPast);
  addLuaCallback("delta", luaCB_statDelta);
  addLuaCallback("growth", luaCB_statGrowth);
  addLuaCallback("sum", luaCB_statSum);
  addLuaCallback("avg", luaCB_statAvg);
  addLuaCallback("min", luaCB_statMin);
  addLuaCallback("max", luaCB_statMax);
  addLuaCallback("set", luaCB_setStat);

  addLuaCallback("log", luaCB_log);
  addLuaCallback("warn", luaCB_logWarn);
  addLuaCallback("logError", luaCB_logError);
  addLuaCallback("handleError", luaCB_handleError);

  addLuaCallback("setGameSpeed", luaCB_setGameSpeed);
  addLuaCallback("now", luaCB_now);
  addLuaCallback("timeNow", luaCB_timeNow);
  addLuaCallback("year", luaCB_year);
  addLuaCallback("formatDuration", luaCB_formatDuration);
  addLuaCallback("formatDurationNice", luaCB_formatDurationNice);
  addLuaCallback("formatDateTime", luaCB_formatDateTime);
  addLuaCallback("formatDate", luaCB_formatDate);
  addLuaCallback("formatTime", luaCB_formatTime);
  addLuaCallback("formatSeason", luaCB_formatSeason);
  addLuaCallback("formatYear", luaCB_formatYear);
  addLuaCallback("format2DigitYear", luaCB_format2DigitYear);
  setLuaGlobal("OneYear", oneYear);
  setLuaGlobal("OneDay",  1);
  setLuaGlobal("OneHour", oneHour);
  setLuaGlobal("OneMinute", oneHour/60.f);

  for (int i = 0; i < numStatistics; i ++) {
    setLuaGlobal(getStatisticCode(i), i);
  }

  for (int i = 0; i < numFeatures; i ++) {
    setLuaGlobal(getFeatureCode(i), i);
  }
  setLuaGlobal("NumFeatures", numFeatures);

  for (int i = 0; i < numEffects; i ++) {
    string code = getEffectString(i);
    code = code + "Score";
    setLuaGlobal(code.c_str(), i);
  }

  for (int i = 0; i < numBudgetLines; i ++) {
    setLuaGlobal(getBudgetLineCode((BudgetLine)i), i);
  }
  setLuaGlobal("NumBudgetLines", numBudgetLines);

}

