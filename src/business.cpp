#include "business.hpp"

#include "board.hpp"
#include "building/design.hpp"
#include "economy.hpp"
#include "game/game.hpp"
#include "land.hpp"
#include "name.hpp"
#include "person.hpp"
#include "pool.hpp"
#include "route/broker.hpp"
#include "selection.hpp"
#include "tanker.hpp"
#include "time.hpp"
#include "util.hpp"
#include "zone.hpp"

#include "parts/messageBoard.hpp"

#include <set>

Pool<Business>* businesses = Pool<Business>::newPool(2000);
//set<item> openRetail;
//set<item> jobOpenings[numEducationLevels+1];
Tanker openRetail;
Tanker jobOpenings[numEducationLevels+1];

const char* businessTypeName[] = {
  "Retail", "Office", "Farm", "Factory", "Institution"
};

const char* getBusinessTypeName(BusinessType type) {
  return businessTypeName[type];
}

Business* getBusiness(item businessNdx) {
  return businesses->get(businessNdx);
}

BusinessType getBusinessType(item ndx) {
  int flags = getBusiness(ndx)->flags;
  int t = (flags & _businessTypeMask) >> _businessTypeShift;
  return (BusinessType)t;
}

item getBusinessEcon(item ndx) {
  Business* bz = getBusiness(ndx);
  Building* bd = getBuilding(bz->building);
  item econ = bd->econ;
  if (econ == 0) {
    return nationalEconNdx();
  } else {
    return econ;
  }
}

/*
void postPosition(item businessNdx, Position p) {
  insertIntoTanker(&(jobOpenings[p.minEducation]), businessNdx);
  Supply supplies = (Supply)(SuppliesNoEduJob + p.minEducation);
  Building* building = getBuilding(getBusiness(businessNdx)->building);
  supplyTableSuggest_g(building->graphLoc.lane, supplies, businessNdx);
}
*/

void repostPositions(item businessNdx) {
  Business* b = getBusiness(businessNdx);
  bool keepPosting[numEducationLevels+1];
  for (int i = 0; i < numEducationLevels; i++) {
    keepPosting[i] = false;
  }

  int numPositions = b->positions.size();
  for (int k = 0; k < numPositions; k++) {
    Position p = b->positions[k];
    if (p.employee == 0) {
      keepPosting[p.minEducation] = true;
    }
  }

  Building* building = getBuilding(b->building);
  for (int i = 0; i < numEducationLevels; i++) {
    Supply supplies = (Supply)(SuppliesNoEduJob + i);
    if (keepPosting[i]) {
      insertIntoTanker(&(jobOpenings[i]), businessNdx);
      supplyTableSuggest_g(building->graphLoc.lane, supplies, businessNdx);
    } else {
      removeFromTanker(&(jobOpenings[i]), businessNdx);
      supplyTableErase_g(building->graphLoc.lane, supplies, businessNdx);
    }
  }
}

item getRandomPosition(EducationLevel edu) {
  for (int i = edu; i >= 0; i--) {
    item result = randomInTanker(&(jobOpenings[i]));
    //item result = randInSet(&jobOpenings[i], businesses->size());
    if (result == 0) continue;
    return result;
    //int num = jobOpenings[i].size();
    //if (num == 0) continue;
    //auto it = std::begin(jobOpenings[i]);
    //std::advance(it, randItem(num)); //NOTE: O(n)
    //return *it;
  }
  return 0;
}

void adjustBizStats(item ndx, float mult) {
  Business* b = getBusiness(ndx);
  BusinessType type = getBusinessType(ndx);
  item econ = getBusinessEcon(ndx);
  adjustStat(econ, NumBusinesses, mult);

  if (type == Retail) {
    adjustStat(econ, NumRetailBiz, mult);
  } else if (type == Office) {
    adjustStat(econ, NumOfficeBiz, mult);
  } else if (type == Factory) {
    adjustStat(econ, NumIndustrialBiz, mult);
  } else if (type == Farm) {
    adjustStat(econ, NumAgriculturalBiz, mult);
  } else if (type == Institution) {
    // No-op
  }

  for (int i = 0; i < b->positions.size(); i++) {
    Position p = b->positions[i];
    EducationLevel edu = p.minEducation;
    adjustStat(econ, NumPositions, mult);
    adjustStat(econ, (Statistic)(NumNoEduPositions+edu), mult);
    if (p.employee == 0) {
      adjustStat(econ, NumOpenPositions, mult);
      adjustStat(econ, (Statistic)(NumOpenNoEduPositions+edu), mult);
    }
  }
}

void adjustBizOpenStats(item ndx, float mult) {
  BusinessType type = getBusinessType(ndx);
  item econ = getBusinessEcon(ndx);

  if (type == Retail) {
    adjustStat(econ, NumRetailOpen, mult);
  } else if (type == Institution) {
    adjustStat(econ, NumGovBuildingsOpen, mult);
  } else if (type == Factory) {
    adjustStat(econ, NumIndustrialOpen, mult);
  } else if (type == Farm) {
    adjustStat(econ, NumAgriculturalOpen, mult);
  }
}

item positionsForBizTypeAndEduLevel(item bizType, item eduLevel) {
  if (bizType >= Institution || bizType < 0) return 0;
  return c((IntConstant)(CRetailNoEduPositions+bizType*4+eduLevel));
}

void rebuildPositions(item ndx) {
  Business* b = getBusiness(ndx);
  BusinessType type = getBusinessType(ndx);
  Building* building = getBuilding(b->building);

  // Collect all current employees
  vector<item> employeesAgg;
  vector<item> employees[numEducationLevels];
  for (int i = 0; i < b->positions.size(); i++) {
    Position p = b->positions[i];
    if (p.employee != 0) {
      employeesAgg.push_back(p.employee);
    }
  }

  // Sort them by education
  for (int i = 0; i < employeesAgg.size(); i++) {
    item pNdx = employeesAgg[i];
    employees[getEducationForPerson(pNdx)].push_back(pNdx);
  }

  b->positions.clear();

  // Determine number of positions by education level
  int eduNum[numEducationLevels] = {0};

  if (type == Retail) {
    eduNum[NoEducation] = c(CRetailNoEduPositions);
    eduNum[HSDiploma] = c(CRetailHSPositions);
    eduNum[BachelorsDegree] = c(CRetailBclPositions);
    eduNum[Doctorate] = c(CRetailPhdPositions);

  } else if (type == Office) {
    eduNum[NoEducation] = c(COfficeNoEduPositions);
    eduNum[HSDiploma] = c(COfficeHSPositions);
    eduNum[BachelorsDegree] = c(COfficeBclPositions);
    eduNum[Doctorate] = c(COfficePhdPositions);

  } else if (type == Farm) {
    eduNum[NoEducation] = c(CFarmNoEduPositions);
    eduNum[HSDiploma] = c(CFarmHSPositions);
    eduNum[BachelorsDegree] = c(CFarmBclPositions);
    eduNum[Doctorate] = c(CFarmPhdPositions);

  } else if (type == Factory) {
    eduNum[NoEducation] = c(CFactoryNoEduPositions);
    eduNum[HSDiploma] = c(CFactoryHSPositions);
    eduNum[BachelorsDegree] = c(CFactoryBclPositions);
    eduNum[Doctorate] = c(CFactoryPhdPositions);

  } else if (type == Institution) {
    Design* d = getDesign(building->design);

    if (d->category == ParksCategory) {
      eduNum[NoEducation] = c(CParkNoEduPositions);
      eduNum[HSDiploma] = c(CParkHSPositions);
      eduNum[BachelorsDegree] = c(CParkBclPositions);
      eduNum[Doctorate] = c(CParkPhdPositions);

    } else if (d->category == EducationCategory ||
        d->category == UniversityCategory) {
      eduNum[NoEducation] = c(CEducationNoEduPositions);
      eduNum[HSDiploma] = c(CEducationHSPositions);
      eduNum[BachelorsDegree] = c(CEducationBclPositions);
      eduNum[Doctorate] = d->category == UniversityCategory ?
        d->numBusinesses[3] : c(CEducationPhdPositions);

    } else if (d->category == CommunityCategory) {
      eduNum[NoEducation] = c(CServicesNoEduPositions);
      eduNum[HSDiploma] = c(CServicesHSPositions);
      eduNum[BachelorsDegree] = c(CServicesBclPositions);
      eduNum[Doctorate] = c(CServicesPhdPositions);
    }

  } else {
    handleError("Unknown Business Type");
  }

  // Create positions
  for (int t = numEducationLevels-1; t >= 0; t--) {
    bool anyOpen = false;
    for (int i = 0; i < eduNum[t]; i++) {
      Position p;
      p.employee = 0;
      p.minEducation = (EducationLevel)t;

      // See if there is a current employee for the role
      for (int a = numEducationLevels-1; a >= t; a--) {
        int numT = employees[a].size();
        if (numT > 0) {
          p.employee = employees[a][numT-1];
          employees[a].erase(employees[a].begin()+numT-1);
          break;
        }
      }

      // If no one filled it, post it
      if (p.employee == 0) {
        anyOpen = true;
      }

      b->positions.push_back(p);
    }

    /*
    Supply supplies = (Supply)(SuppliesNoEduJob + t);
    if (anyOpen) {
      insertIntoTanker(&(jobOpenings[t]), ndx);
      supplyTableSuggest_g(building->graphLoc.lane, supplies, ndx);
    } else {
      removeFromTanker(&(jobOpenings[t]), ndx);
      supplyTableErase_g(building->graphLoc.lane, supplies, ndx);
    }
    */
  }

  // Lay off anyone left
  for (int t = 0; t < numEducationLevels; t++) {
    for (int i = 0; i < employees[t].size(); i++) {
      item emp = employees[t][i];
      quitJob(emp);
      findAnyJob(emp);
    }
  }

  repostPositions(ndx);
}

bool hasHotelBusinesses(item buildingNdx) {
  Building* building = getBuilding(buildingNdx);
  for (int i = 0; i < building->businesses.size(); i++) {
    Business* biz = getBusiness(building->businesses[i]);
    if (biz->flags & _businessIsHotel) {
      return true;
    }
  }
  return false;
}

item addBusiness(item buildingNdx, BusinessType type) {
  Building* building = getBuilding(buildingNdx);
  Design* design = getDesign(building->design);
  item businessNdx = businesses->create();
  Business* business = getBusiness(businessNdx);
  business->flags = _businessExists;
  business->positions.clear();
  business->building = buildingNdx;
  building->businesses.push_back(businessNdx);

  float time = getCurrentDateTime();
  business->lastCustomerTime = time;
  business->lastFreightTime = time - c(CFreightInDays) + 2;
  business->foundedTime = time;

  if (type == Institution) {
    business->name = strdup_s(getDesign(building->design)->displayName);
  } else if (type == Retail && (design->flags & _designIsHotel) &&
      !hasHotelBusinesses(buildingNdx)) {
    business->flags |= _businessIsHotel;
    business->name = randomName(HotelName);
    if (design->zone != GovernmentZone) {
      if (building->name != 0) free(building->name);
      building->name = strdup_s(business->name);
    }
  } else if (building->zone == GovernmentZone && type == Retail) {
    business->name = randomName(GovRetailName);
  } else if (type == Retail) {
    business->name = randomName(RetailName);
  } else if (type == Office) {
    business->name = randomName(OfficeName);
  } else if (type == Farm) {
    business->name = randomName(FarmName);
  } else if (type == Factory) {
    business->name = randomName(IndustrialName);
  }

  business->flags |= type << _businessTypeShift;

  rebuildPositions(businessNdx);
  adjustStat(getBusinessEcon(businessNdx), BusinessesCreated, 1);
  adjustBizStats(businessNdx, 1);
  return businessNdx;
}

void removeBusiness(item businessNdx) {
  Business* business = getBusiness(businessNdx);
  if (!business->flags & _businessExists) {
    return;
  }

  Building* building = getBuilding(getBusiness(businessNdx)->building);

  for (int i = business->positions.size()-1; i >= 0; i--) {
    Position p = business->positions[i];
    if (p.employee != 0) {
      item emp = p.employee;
      quitJob(emp);
      findAnyJob(emp);
    }
  }

  for (int i = 0; i < numEducationLevels; i++) {
    removeFromTanker(&(jobOpenings[i]), businessNdx);
    Supply supplies = (Supply)(SuppliesNoEduJob + i);
    supplyTableErase_g(building->graphLoc.lane, supplies, businessNdx);
  }

  removeFromTanker(&openRetail, businessNdx);
  supplyTableErase_g(building->graphLoc.lane, SuppliesRetail, businessNdx);
  supplyTableErase_g(building->graphLoc.lane, SuppliesAmenity, businessNdx);
  supplyTableErase_g(building->graphLoc.lane, SuppliesTourism, businessNdx);
  adjustBizStats(businessNdx, -1);
  if (business->flags & _businessOpen) {
    adjustBizOpenStats(businessNdx, -1);
  }

  adjustStat(getBusinessEcon(businessNdx), BusinessesClosed, 1);
  removeBusinessFromTenancy(business->building, businessNdx);
  deselect(SelectionBusiness, businessNdx);
  removeMessageByObject(BusinessMessage, businessNdx);
  //boardClean(JobsNoEdu, businessNdx);
  //boardClean(JobsHSEdu, businessNdx);
  vector<Position> swap;
  business->positions.swap(swap);
  business->flags = 0;
  free(business->name);
  business->name = 0;
  businesses->free(businessNdx);
}

item getRandomRetail() {
  return randomInTanker(&openRetail);
  //item result = randInSet(&openRetail, businesses->size());
  //return result;

  /*
  int size = openRetail.size();
  if (size == 0) {
    return 0;
  }
  item ndx = rand()%size;
  return retail[ndx];
  */
}

item getRandomBusiness() {
  int size = businesses->size();
  if (size == 0) return 0;
  return randItem(size)+1;
}

/*
float retailRatio() {
  int count = businesses->count();
  if (count > 0) {
    return getStatistic(NumRetailBiz)*1.0f/businesses->count();
  } else {
    return 0.5f;
  }
}
*/

item numBusinesses(item econ) {
  return getStatistic(econ, NumBusinesses);
}

item sizeBusinesses() {
  return businesses->size();
}

bool longTimeSinceFreight(item ndx) {
  Business* b = getBusiness(ndx);
  float time = getCurrentDateTime();
  BusinessType type = getBusinessType(ndx);
  return type != Office && type != Institution &&
      time - b->lastFreightTime > c(CFreightInDays);
}

bool longTimeSinceCustomer(item ndx) {
  Business* b = getBusiness(ndx);
  float time = getCurrentDateTime();
  float timeSinceCustomer = time - b->lastCustomerTime;
  BusinessType type = getBusinessType(ndx);
  return (type == Retail && timeSinceCustomer > c(CCustomerIssue)) ||
    ((type==Farm || type==Factory) && timeSinceCustomer > c(CFreightOutIssue));
}

IssueIcon getBusinessIssue(item ndx) {
  Business* b = getBusiness(ndx);
  float time = getCurrentDateTime();
  BusinessType type = getBusinessType(ndx);
  if (longTimeSinceCustomer(ndx)) {
    return NoCustomers;
  } else if (type != Office && type != Institution &&
      time - b->lastFreightTime > c(CFreightInIssue)) {
    return NeedsFreight;
  }

  int numUnfilled = 0;
  for (int i = 0; i < b->positions.size(); i++) {
    if (b->positions[i].employee == 0) {
      numUnfilled ++;
    }
  }

  return numUnfilled >= c(CUnfilledJobsIssue) ? NeedsWorkers : NoIssue;
}

char* getBusinessDescriptor(item ndx) {
  Business* b = getBusiness(ndx);
  IssueIcon ico = getBusinessIssue(ndx);
  const char* comment = "";
  if (ico == NeedsWorkers) {
    comment = " (Needs Workers)";
  } else if (ico == NoCustomers) {
    comment = " (No Customers)";
  } else if (ico == NeedsFreight) {
    comment = " (Needs Freight)";
  } else if (b->flags & _businessOpen) {
    comment = " - Open";
  } else {
    comment = " - Closed";
  }

  return sprintf_o("%s%s", b->name, comment);
}

void updateBusiness(item ndx, float duration, vec3 loc) {
  Business* b = getBusiness(ndx);
  if (!(b->flags & _businessExists)) return;

  Building* building = getBuilding(b->building);
  float time = getCurrentDateTime();
  float prosperityPenalty = 0;
  float communityPenalty = 0.0f;
  float stress = 0;
  //bool forceClosed = false;
  BusinessType type = getBusinessType(ndx);
  item econ = getBusinessEcon(ndx);

  if (longTimeSinceFreight(ndx)) {
    prosperityPenalty += c(CNoFreightProsperity);
    stress += c(CNoFreightStress);
    supplyTableSuggest_g(building->graphLoc.lane, SuppliesFreightNeed, ndx);
  }

  vector<item> onTheJob;
  for (int i = 0; i < b->positions.size(); i++) {
    Position pos = b->positions[i];
    item pNdx = pos.employee;
    if (pNdx == 0) {
      prosperityPenalty += c(CUnfilledPositionProsperity) / b->positions.size();
      stress += c(CUnfilledPositionStress) / b->positions.size();
      supplyTableSuggest_g(building->graphLoc.lane,
          (Supply)(SuppliesNoEduJob + pos.minEducation), ndx);

    } else {
      Person* p = getPerson(pNdx);
      if (p->activity == WorkActivity && p->location == b->building &&
          !(p->flags & _personTraveling)) {
        onTheJob.push_back(pNdx);
      }
    }
  }

  bool open = onTheJob.size() > 0;
  bool isRetail = getBusinessType(ndx) == Retail;
  if (open && (!(b->flags & _businessOpen))) {
    adjustBizOpenStats(ndx, 1);
    b->flags |= _businessOpen;
  } else if (!open && (b->flags & _businessOpen)) {
    adjustBizOpenStats(ndx, -1);
    b->flags &= ~_businessOpen;
  }

  if (isRetail) {
    if (open) {
      insertIntoTanker(&openRetail, ndx);
      supplyTableSuggest_g(building->graphLoc.lane, SuppliesRetail, ndx);
    } else {
      removeFromTanker(&openRetail, ndx);
      supplyTableErase_g(building->graphLoc.lane, SuppliesRetail, ndx);
    }
  }

  if (type == Institution) {
    if (open) {
      b->lastCustomerTime = time;
      supplyTableSuggest_g(building->graphLoc.lane, SuppliesAmenity, ndx);
    } else {
      supplyTableErase_g(building->graphLoc.lane, SuppliesAmenity, ndx);
    }
  }

  if (open) {
    Design* design = getDesign(building->design);
    if (design->effect[Tourism] > 0 || design->effect[Prestige] > 0) {
      supplyTableSuggest_g(building->graphLoc.lane, SuppliesTourism, ndx);
    }
  } else {
    supplyTableErase_g(building->graphLoc.lane, SuppliesTourism, ndx);
  }

  bool noCustomers = longTimeSinceCustomer(ndx);
  if (noCustomers) {
    prosperityPenalty += c(CNoCustomerProsperity);
    stress += c(CNoCustomersStress);
  }

  float timeOfDay = getCurrentTime();
  if ((open || noCustomers) &&
      onTheJob.size() < b->positions.size()*c(COnTheJobTarget)
      && timeOfDay < c(CBusinessCloseHour)*oneHour
      && timeOfDay > c(CBusinessOpenHour)*oneHour) {
    b->flags |= _businessComeToWork;
  } else {
    b->flags &= ~_businessComeToWork;
  }

  if(isRetail) {
    communityPenalty += c(CCommunityRetailBusinessEffect);
  }

  heatMapAdd(Prosperity, loc,
      (c(CBusinessProsperity) - prosperityPenalty) * duration);
  heatMapAdd(CommunityHM, loc,
      communityPenalty * duration);

  if (type == Office) {
    heatMapAdd(Density, loc, duration * c(COfficeDensity));
  } else if (type == Factory) {
    heatMapAdd(Density, loc, duration * c(CFactoryDensity));
  }

  if (c(CEnableBusinessClosing)) {
    if (type != Institution && numPeople(econ) > 1500) {
      float unemploymentDiff = unemploymentRate(econ) -
        targetUnemploymentRate(econ);
      float prosperity = heatMapGet(Prosperity, loc);
      stress += c(CBusinessStressBase);
      stress += c(CBusinessProsperityStress) * prosperity;
      stress += c(CBusinessUnemploymentStress) * unemploymentDiff;
      if (stress * randFloat() > c(CBusinessStressThreshold)) {
        removeBusiness(ndx);
        return;
      }
    }
  }

  repostPositions(ndx);
}

Statistic getBizTypeDemandStat(BusinessType i) {
   return i == Farm ? FarmDemand :
      i == Retail ? RetailDemand :
      i == Factory ? FactoryDemand :
      i == Office ? OfficeDemand : (Statistic)-1;
}

Statistic getBizTypeGrowthStat(BusinessType i) {
   return i == Farm ? FarmGrowth :
      i == Retail ? RetailGrowth :
      i == Factory ? FactoryGrowth :
      i == Office ? OfficeGrowth : (Statistic)-1;
}

float getBusinessTypeDemand(item econ, BusinessType type) {
  return getStatistic(econ, getBizTypeDemandStat(type));
}

float getBusinessTypeGrowth(item econ, BusinessType type) {
  return getStatistic(econ, getBizTypeGrowthStat(type));
}

void updateBusinessTypeDemand(item econ) {
  item pop = numPeople(econ);
  item totalOpen = getStatistic(econ, NumOpenPositions);
  bool logBizGrowthData = econ == ourCityEconNdx() &&
    c(CLogBusinessGrowthData) && (randFloat() < 0.05);
  if (logBizGrowthData) {
    SPDLOG_INFO("Business Growth Data for econ{}:{}",
        econ, getEcon(econ)->name);
  }

  for (int i = 0; i < numBusinessTypes; i++) {
    if (i == Institution) continue;

    if (logBizGrowthData) {
      SPDLOG_INFO("Business Growth Data: {}", businessTypeName[i]);
    }

    float result = 1 + getStatistic(econ, ProsperityStat);

    if (logBizGrowthData) {
      SPDLOG_INFO("{}: {} prosperity {}", businessTypeName[i], result,
          getStatistic(econ, ProsperityStat));
    }

    // Effect of Retail Economy
    if (i == Retail) {
      int numRetailBiz = getStatistic(econ, NumRetailBiz);
      if (numRetailBiz >= 10) {
        float retailEconomy = getStatisticAvg(econ, RetailTransactions,
            oneYear);
        float incomePerBiz = retailEconomy / numRetailBiz;
        float incomeTarget = c(CRetailEconomyPerBiz) * getInflation();
        float economyEffect = incomePerBiz / incomeTarget;
        economyEffect = clamp(economyEffect, 0.1f, 2.f);
        result *= economyEffect;

        if (logBizGrowthData) {
          SPDLOG_INFO("econ:{} {}:{} retailEconomy:{} incomePerBiz:{}"
              " economyEffect:{} result:{}",
              econ, i, businessTypeName[i], retailEconomy, incomePerBiz,
              economyEffect, result);
        }
      }
    }

    // Effect of Unfilled Positions
    for (int j = 0; j < numEducationLevels; j++) {
      if (positionsForBizTypeAndEduLevel(i, j) <= 0) continue;

      float openings = getStatistic(econ,
          (Statistic)(NumOpenNoEduPositions+j));
      float jobs = getStatistic(econ, (Statistic)(NumNoEduPositions+j));
      float openPercent = openings/nonZero(jobs);

      float maxOpen = c(CMaxOpenJobs);
      maxOpen += getEffectValue(BusinessEffect) * c(CBizPerBizPoint) * getStatistic(econ, (Statistic)(COfficeNoEduPositions+j));
      float jobsEffect = 1 - clamp(openPercent / maxOpen, 0.f, 1.f);
      if (totalOpen < 1000) {
        jobsEffect = clamp(jobsEffect, 1-totalOpen/1000.f, 1.f);
      }
      result *= jobsEffect;
      result = clamp(result, 0.f, 1.f);
      if (logBizGrowthData) {
        SPDLOG_INFO("econ:{} {}:{} edu:{} openJobs:{:.2f}% effect:{:.2f}%",
          econ, i, businessTypeName[i], j, openPercent*100, jobsEffect*100);
      }
    }

    if (i == Factory && pop < 1000) result = 0;
    if (i == Office && pop < 10000) result = 0;
    if (i == Farm && result < 0.01) result = 0.01;

    if (logBizGrowthData) {
      SPDLOG_INFO("econ:{} {}:{} growth => {}",
          econ, i, businessTypeName[i], result);
    }
    setStat(econ, getBizTypeGrowthStat((BusinessType)i), result);

    // Effect of empty buildings
    int tenancies = 0;
    int empties = 0;
    float emptyTarget = 0;

    if (i == Retail) {
      empties = getStatistic(econ, EmptyShops);
      tenancies += getStatistic(econ, NumShops);
      emptyTarget = c(CTargetEmptyCommercial);

    } else if (i == Office) {
      empties = getStatistic(econ, EmptyOffices);
      tenancies += getStatistic(econ, NumOffices);
      emptyTarget = c(CTargetEmptyCommercial);

    } else if (i == Farm) {
      empties = getStatistic(econ, EmptyFarms);
      tenancies += getStatistic(econ, NumFarms);
      emptyTarget = c(CTargetEmptyIndustrial);

    } else if (i == Factory) {
      empties = getStatistic(econ, EmptyFactories);
      tenancies += getStatistic(econ, NumFactories);
      emptyTarget = c(CTargetEmptyIndustrial);
    }

    float target = 2+emptyTarget*tenancies*2;
    result *= clamp((target - empties)/nonZero(target), 0.f, 1.f);
    if (logBizGrowthData) {
      SPDLOG_INFO("{}: {} empties {} {} {} {}", businessTypeName[i], result,
          emptyTarget, tenancies, target, empties);
      SPDLOG_INFO("econ:{} {}:{} demand => {}",
          econ, i, businessTypeName[i], result);
    }
    setStat(econ, getBizTypeDemandStat((BusinessType)i), result);
  }
}

void updateBusinessTypeDemand_legacy(item econ) {
  bool logBizGrowthData = econ == ourCityEconNdx() &&
    c(CLogBusinessGrowthData) && (randFloat() < 0.05);

  if (logBizGrowthData) {
    SPDLOG_INFO("Business Growth Data for econ{}:{}",
        econ, getEcon(econ)->name);
  }

  for (int i = 0; i < numBusinessTypes; i++) {
    if (i == Institution) continue;

    if (logBizGrowthData) {
      SPDLOG_INFO("Business Growth Data: {}", businessTypeName[i]);
    }

    // Early Game Special Rules
    //if (numPeople(econ) < 1480) {
      //if (i == Office || i == Factory) {
        //businessTypeDemand[i] = 0;
        //continue;
      //}
    //}

    // Effect of Unemployment and Education
    EducationLevel edu = i == Office ? BachelorsDegree :
      i == Farm ? NoEducation : HSDiploma;
    //float unemp = getStatistic(edu == NoEducation ? UnemploymentRate :
          //(Statistic)(NoEduUnemployment+edu));
    float unemp = unemploymentRate(econ, edu);
    float targetUnemp = targetUnemploymentRate(econ);
    float unemploymentDiff = unemp - targetUnemp;
    float bias = c((FloatConstant)(CUnemploymentRetailBias+i));
    float result = (unemploymentDiff+bias) * c(CBusinessUnemploymentFactor);
    if (logBizGrowthData) {
      SPDLOG_INFO("{}: result:{} unemp/edu:{} unemploymentDiff:{}"
          " bias:{} target:{}",
          businessTypeName[i], result,
          unemp, unemploymentDiff, bias, targetUnemp);
    }

    result *= 1 + getStatistic(econ, ProsperityStat);

    if (logBizGrowthData) {
      SPDLOG_INFO("{}: {} prosperity {}", businessTypeName[i], result,
          getStatistic(econ, ProsperityStat));
    }

    // Effect of Retail Economy
    if (i == Retail) {
      int numRetailBiz = getStatistic(econ, NumRetailBiz);
      if (numRetailBiz >= 10) {
        float retailEconomy = getStatisticAvg(econ, RetailTransactions,
            oneYear);
        float incomePerBiz = retailEconomy / numRetailBiz;
        float incomeTarget = c(CRetailEconomyPerBiz) * getInflation();
        float economyEffect = incomePerBiz / incomeTarget;
        economyEffect = clamp(economyEffect, 0.f, 2.f);
        result *= economyEffect;

        if (logBizGrowthData) {
          SPDLOG_INFO("econ:{} {}:{} retailEconomy:{} incomePerBiz:{}"
              " economyEffect:{} result:{}",
              econ, i, businessTypeName[i], retailEconomy, incomePerBiz,
              economyEffect, result);
        }
      }
    }

    /*
    // Effect of Retail Ratio
    int numBiz = getStatistic(econ, NumBusinesses);
    float retailRatio = numBiz == 0 ? 0.5 : getStatistic(econ, NumRetailBiz) /
      nonZero(getStatistic(econ, NumBusinesses));
    float popFactor = clamp(getStatistic(econ, Population)/200000.f, 0.f, 1.f);
    float targetRatio = mix(c(CRetailRatio0), c(CRetailRatio200), popFactor);

    if (logBizGrowthData) {
      SPDLOG_INFO("econ:{} {}:{} retailRatio:{} => {}",
          econ, i, businessTypeName[i], retailRatio, result);
    }

    if (i == Retail) {
      result *= 1-c(CSalesTaxRetailEffect)*getTaxRate(SalesTax);

    } else {
      retailRatio = 1-retailRatio;
      targetRatio = 1-targetRatio;
    }

    float ratioEffect = clamp(1.5f - retailRatio/targetRatio, 0.0f, 1.5f);
    if (numPeople(econ) < 1480 && i != Retail) {
      ratioEffect = clamp(ratioEffect, 0.2f, 1.5f);
    }
    result *= ratioEffect;
    if (logBizGrowthData) {
      SPDLOG_INFO("{}: {} retail ratio {} {} {} {}", businessTypeName[i],
        result, retailRatio, popFactor, targetRatio, ratioEffect);
    }
    */

    // Effect of Unfilled Positions
    float openings = 0;
    float jobs = 0;
    for (int j = edu; j < numEducationLevels; j++) {
      openings += getStatistic(econ, (Statistic)(NumOpenNoEduPositions+j));
      jobs += getStatistic(econ, (Statistic)(NumNoEduPositions+j));
    }
    float openPercent = openings/nonZero(jobs);
    float jobsEffect = 1 - clamp(openPercent / c(CMaxOpenJobs), 0.f, 1.f);
    //if (numPeople(econ) < 980 && i == Farm) {
      //jobsEffect = mix(1.f, jobsEffect, numPeople(econ)/980.f);
    //}
    result *= jobsEffect;
    result = clamp(result, 0.f, 1.f);

    //if ((i == Factory || i == Office) && numPeople(econ) < 1000) {
      //result = 0;
    //}

    if (logBizGrowthData) {
      SPDLOG_INFO("econ:{} {}:{} growth => {}",
          econ, i, businessTypeName[i], result);
    }
    setStat(econ, getBizTypeGrowthStat((BusinessType)i), result);

    // Effect of empty buildings
    int tenancies = 0;
    int empties = 0;
    float emptyTarget = 0;

    if (i == Retail) {
      empties = getStatistic(econ, EmptyShops);
      tenancies += getStatistic(econ, NumShops);
      emptyTarget = c(CTargetEmptyCommercial);

    } else if (i == Office) {
      empties = getStatistic(econ, EmptyOffices);
      tenancies += getStatistic(econ, NumOffices);
      emptyTarget = c(CTargetEmptyCommercial);

    } else if (i == Farm) {
      empties = getStatistic(econ, EmptyFarms);
      tenancies += getStatistic(econ, NumFarms);
      emptyTarget = c(CTargetEmptyIndustrial);

    } else if (i == Factory) {
      empties = getStatistic(econ, EmptyFactories);
      tenancies += getStatistic(econ, NumFactories);
      emptyTarget = c(CTargetEmptyIndustrial);
    }

    float target = 2+emptyTarget*tenancies*2;
    result *= clamp((target - empties)/nonZero(target), 0.f, 1.f);
    if (logBizGrowthData) {
      SPDLOG_INFO("{}: {} empties {} {} {} {}", businessTypeName[i], result,
          emptyTarget, tenancies, target, empties);
      SPDLOG_INFO("econ:{} {}:{} demand => {}",
          econ, i, businessTypeName[i], result);
    }
    setStat(econ, getBizTypeDemandStat((BusinessType)i), result);
  }
}

void updateBusinesses(double duration) {
  if (getGameMode() != ModeGame) return;

  for (int k = 1; k <= sizeEcons(); k++) {
    Econ* econ = getEcon(k);
    //SPDLOG_INFO("updateBusinesses({},{}) {} {}",
        //duration, k, econ->name, econ->flags);
    if (!(econ->flags & _econExists)) continue;
    if (econ->flags & _econAggregate) continue;

    updateBusinessTypeDemand(k);

    // Add Business
    for (int i = 0; i < numBusinessTypes; i++) {
      if (i == Institution) continue;

      Statistic stat = i == Farm ? FarmGrowth :
          i == Retail ? RetailGrowth :
          i == Factory ? FactoryGrowth :
          i == Office ? OfficeGrowth : (Statistic)-1;

      float demand = getStatistic(k, stat);
      //SPDLOG_INFO("addBusiness i:{}, demand:{}", i, demand);
      if (randFloat() < demand * duration * c(CBusinessSpawnRate)) {
        item board = i == Retail ? RetailTenancies :
                     i == Office ? OfficeTenancies :
                     i == Factory ? FactoryTenancies :
                        FarmTenancies;

        item building = boardTake(k, board);
        if (building > 0) {
          addBusiness(building, (BusinessType)i);
        }
      }
    }
  }
}

void employeeHired(item businessNdx, item personNdx) {
  Person* person = getPerson(personNdx);
  if (!(person->flags & _personExists)) {
    SPDLOG_WARN("employeeHired on non-existing person {}",
        personNdx);
    return;
  }

  if (person->employer == businessNdx) {
    return;
  } else if (person->employer != 0) {
    quitJob(personNdx);
  }

  Business* b = getBusiness(businessNdx);
  if (!(b->flags & _businessExists)) {
    SPDLOG_WARN("employeeHired on non-existing business {}",
        businessNdx);
    return;
  }

  int numPositions = b->positions.size();
  EducationLevel edu = getEducationForPerson(personNdx);
  EducationLevel bestPosEdu = (EducationLevel)-1;
  item bestPosition = -1;

  for (int k = 0; k < numPositions; k++) {
    Position p = b->positions[k];
    if (p.employee == 0 && p.minEducation <= edu &&
        p.minEducation > bestPosEdu) {
      bestPosition = k;
      bestPosEdu = p.minEducation;
    }
  }

  if (bestPosition >= 0) {
    b->positions[bestPosition].employee = personNdx;
    Person* p = getPerson(personNdx);
    adjustPersonStats(personNdx, -1);
    p->employer = businessNdx;
    adjustPersonStats(personNdx, 1);
    item econ = getBusinessEcon(businessNdx);
    adjustStat(econ, (Statistic)(NumOpenNoEduPositions+bestPosEdu), -1);
    adjustStat(econ, NumOpenPositions, -1);
  }

  repostPositions(businessNdx);
}

void employeeQuit(item personNdx) {
  Person* p = getPerson(personNdx);
  if (p->employer == 0) return;
  adjustPersonStats(personNdx, -1);
  item bussNdx = p->employer;
  p->employer = 0;
  adjustPersonStats(personNdx, 1);

  Business* b = getBusiness(bussNdx);
  if (!(b->flags & _businessExists)) return;

  item econ = getBusinessEcon(bussNdx);
  EducationLevel edu = getEducationForPerson(personNdx);
  adjustStat(econ, NumQuitOrFired, 1);

  for (int i = 0; i < b->positions.size(); i++) {
    Position pos = b->positions[i];
    if (pos.employee == personNdx) {
      pos.employee = 0;
      b->positions[i] = pos;
      //postPosition(bussNdx, pos);
      adjustStat(econ, (Statistic)(NumOpenNoEduPositions+pos.minEducation), 1);
      adjustStat(econ, NumOpenPositions, 1);
    }
  }

  repostPositions(bussNdx);
}

void resetBusinesses() {
  for (int i = 1; i <= businesses->size(); i++) {
    Business* b = getBusiness(i);
    if (b->name) free(b->name);
    vector<Position> swap;
    b->positions.swap(swap);
  }

  businesses->clear();
  resetTanker(&openRetail);

  for (int i = 0; i < numEducationLevels+1; i++) {
    resetTanker(&(jobOpenings[i]));
  }
}

void rebuildBusinessStats() {
  resetStat(NumPositions);
  resetStat(NumNoEduPositions);
  resetStat(NumHSPositions);
  resetStat(NumBclPositions);
  resetStat(NumPhDPositions);
  resetStat(NumOpenPositions);
  resetStat(NumOpenNoEduPositions);
  resetStat(NumOpenHSPositions);
  resetStat(NumOpenBclPositions);
  resetStat(NumOpenPhDPositions);

  resetStat(NumBusinesses);
  resetStat(BusinessesCreated);
  resetStat(BusinessesClosed);
  resetStat(NumRetailBiz);
  resetStat(NumRetailOpen);
  resetStat(NumOfficeBiz);
  resetStat(NumOfficeOpen);
  resetStat(NumAgriculturalBiz);
  resetStat(NumAgriculturalOpen);
  resetStat(NumIndustrialBiz);
  resetStat(NumIndustrialOpen);
  resetStat(NumGovBuildingsOpen);

  for (int i = 1; i <= businesses->size(); i++) {
    Business* b = getBusiness(i);
    if (b->flags & _businessExists) {
      rebuildPositions(i);
      adjustBizStats(i, 1);
      if (b->flags & _businessOpen) {
        adjustBizOpenStats(i, 1);
      }
    }
  }
}

void writeBusiness(FileBuffer* file, item businessNdx) {
  Business* business = getBusiness(businessNdx);

  fwrite_int(file, business->flags);
  fwrite_item(file, business->building);
  fwrite_float(file, business->lastCustomerTime);
  fwrite_float(file, business->lastFreightTime);
  fwrite_string(file, business->name);
  fwrite_float(file, business->foundedTime);

  int num = business->positions.size();
  fwrite_item(file, num);
  for (int i = 0; i < num; i++) {
    Position p = business->positions[i];
    fwrite_item(file, p.employee);
    fwrite_char(file, p.minEducation);
  }
}

void readBusiness(FileBuffer* file, int version, item businessNdx) {
  Business* business = getBusiness(businessNdx);

  business->flags = fread_int(file);
  business->building = fread_item(file, version);
  if (version < 43) {
    float time = getCurrentDateTime();
    business->lastCustomerTime = time;
    business->lastFreightTime = time;
  } else {
    business->lastCustomerTime = fread_float(file);
    business->lastFreightTime = fread_float(file);
  }
  business->name = fread_string(file);

  if (version < 49) {
    business->foundedTime = getCurrentDateTime();
  } else {
    business->foundedTime = fread_float(file);
  }

  if (version < 47) {
    vector<item> employees;
    fread_item_vector(file, &employees, version);
    for (int i = 0; i < employees.size(); i++) {
      item personNdx = employees[i];
      if (personNdx == 0) continue;
      Position p;
      p.employee = personNdx;
      p.minEducation = getEducationForPerson(personNdx);
      business->positions.push_back(p);
    }

    Building* b = getBuilding(business->building);
    BusinessType type = Retail;
    if (b->zone == RetailZone || b->zone == ResidentialZone) {
      type = Retail;

    } else if (b->zone == FarmZone) {
      if (getDesign(b->design)->minDensity < 0.05) {
        type = Farm;
      } else {
        type = Factory;
      }

    } else if (b->zone == GovernmentZone) {
      if (business->flags & (1 << 2)) {
        type = Institution;
      } else {
        type = Retail;
      }
    }

    // Erase old type data, write new
    business->flags &= ~(3 << 1);
    business->flags |= type << _businessTypeShift;

  } else {
    int num = fread_item(file, version);
    for (int i = 0; i < num; i++) {
      Position p;
      p.employee = fread_item(file, version);
      p.minEducation = (EducationLevel)fread_char(file);
      business->positions.push_back(p);
    }
  }
}

void writeBusinesses(FileBuffer* file) {
  businesses->write(file);
  for (int i = 1; i <= businesses->size(); i++) {
    writeBusiness(file, i);
  }
  //fwrite_item_vector(file, &retail);
}

void readBusinesses(FileBuffer* file, int version) {
  businesses->read(file, version);
  for (int i = 1; i <= businesses->size(); i++) {
    readBusiness(file, version, i);
  }

  for (int i = 1; i <= businesses->size(); i++) {
    Business* b = getBusiness(i);
    if ((b->flags & _businessExists) &&
        (getBusinessType(i) == Retail) &&
        (b->flags & _businessOpen)) {
      insertIntoTanker(&openRetail, i);
      Building* building = getBuilding(b->building);
      supplyTableSuggest_g(building->graphLoc.lane, SuppliesRetail, i);
    }
  }

  if (version < 47) {
    vector<item> trash; // List of retail businesses
    fread_item_vector(file, &trash, version);
  }
}

