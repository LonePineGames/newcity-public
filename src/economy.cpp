#include "economy.hpp"

#include "amenity.hpp"
#include "board.hpp"
#include "building/building.hpp"
#include "building/design.hpp"
#include "business.hpp"
#include "draw/buffer.hpp"
#include "draw/entity.hpp"
#include "game/game.hpp"
#include "graph.hpp"
#include "heatmap.hpp"
#include "land.hpp"
#include "money.hpp"
#include "newspaper/newspaper.hpp"
#include "option.hpp"
#include "person.hpp"
#include "platform/event.hpp"
#include "pool.hpp"
#include "route/router.hpp"
#include "time.hpp"
#include "util.hpp"
#include "vehicle/vehicle.hpp"
#include "weather.hpp"
#include "zone.hpp"

#include "spdlog/spdlog.h"

Pool<Econ> econs;
static bool statIsRateCounter[numStatistics];
static const int versionStatistics = 44;
static const float defaultTimeStep = oneHour;
static const double economicTick = 1./24./60.;
const double simulationStart = 5*4;

static float targetUnemp = 0.04;
static double econDurToGo = 0;
static double economicWavePhase = 0.25;
static double economicDeterminant = 0;
static double nationalUnemp = 0.04;
static double nationalInterestRate = 0.04;
static double nationalStockIndex = 1000;
static double nationalStockTrajectory = 1;
static double inflationRate = 0.04;
static double inflation = defaultInflation;
double zoneDemSoft[numZoneTypes];

static item ourCityEcon = 0;
static item nationalEcon = 0;

static bool statsReset = true;

static const char* statisticCode[numStatistics] = {
  #define STAT(N) "Stat" #N,
  #include "statisticsEnum.hpp"
  #undef STAT
};

static const vector<Statistic> statCategoryTable[numStatisticCategories] = {
  {
    // Population
    Population, PopulationGrowth, NumFamilies,
    FamiliesMovingIn, FamiliesMovingOut, TotalMoves,
    PeopleSick, Deaths, LifeExpectancy,
    NumHomeless, EmergencyHousing, EmergencyHousingBalance
  },

  {
    // Education
    NumNoEdu, NumHSEdu, NumBclEdu, NumPhdEdu,
    NoEduPercent, HSEduPercent, BclEduPercent, PhdEduPercent,
    NumCollegeStudents, MaxCollegeStudents, NumCollegeBunks, NumEmptyBunks,
    NumDorms, NumColleges, NumProfessors, NumStudentsMovingIn,
    NumStudentsDroppingOut, NumStudentsGraduating,
  },

  {
    // Employment
    NumWorkers, NumEmployed, NumUnemployed,
    JobInterviews, NumHired, NumQuitOrFired,
    UnemploymentRate, NationalUnemploymentRate,
    TargetUnemploymentRate,

    NumNoEduWorkers, NumHSEduWorkers, NumBclEduWorkers, NumPhdEduWorkers,

    NumNoEduUnemployed, NumHSEduUnemployed, NumBclEduUnemployed,
    NumPhdEduUnemployed,

    NoEduUnemployment, HSEduUnemployment, BclEduUnemployment,
    PhdEduUnemployment,

    NumPositions,
    NumNoEduPositions, NumHSPositions, NumBclPositions, NumPhDPositions,

    NumOpenPositions,
    NumOpenNoEduPositions, NumOpenHSPositions, NumOpenBclPositions,
    NumOpenPhDPositions,
  },

  {
    // Buildings
    Skyscrapers,
    ResidentialDemand, RetailDemand, FarmDemand, FactoryDemand, OfficeDemand,
    ResidentialZoneDemand, RetailZoneDemand, FarmZoneDemand, FactoryZoneDemand, 
    OfficeZoneDemand,
    NumBuildings, BuildingsConstructed,
    NumResidentialBuildings, NumRetailBuildings,
    NumFarmBuildings, NumGovBuildings,
    NumOfficeBuildings, NumFactoryBuildings, NumMixedUseBuildings,

    NumAbandoned, AbandonedResidentialBuildings, AbandonedRetailBuildings,
    AbandonedFarmBuildings, AbandonedOfficeBuildings, AbandonedFactoryBuildings,
    AbandonedMixedUseBuildings,

    // Homes and Tenancies
    NumHomes, NumSingleFam, NumMultiFam, PercentMultiFam, EmptyHomes,
    NumShops, EmptyShops, NumOffices, EmptyOffices, PercentOffices,
    NumFarms, EmptyFarms, NumFactories, EmptyFactories, PercentFactories,
  },

  {
    // Businesses
    NumBusinesses, BusinessesCreated, BusinessesClosed,
    NumRetailBiz, NumRetailOpen, NumOfficeBiz, NumOfficeOpen,
    NumAgriculturalBiz, NumAgriculturalOpen,
    NumIndustrialBiz, NumIndustrialOpen,
    PercentRetail,
    RetailGrowth, FarmGrowth, FactoryGrowth, OfficeGrowth,
  },

  {
    // Tourism
    NumTouristsNow, NumTouristsArriving, NumTouristsLeaving,
    NumHotelRooms, NumEmptyHotelRooms,
    Memories, TouristRating, TouristTransactions,
  },

  {
    // Government
    AssetsStat,
    NumGovBuildings, NumGovBuildingsEnabled, NumGovBuildingsOpen,
    NumParks, ParkLots, NumParksEnabled, NumSchools, NumSchoolsEnabled,
    NumCommServices, NumCommServicesEnabled,
    NumUniBuildings, NumUniBuildingsEnabled,
    NumColleges, NumHealthcareProviders,
  },

  {
    // Activities
    TripsHome, WorkCommutes, JobInterviews, SocialVisits,
    StoreTrips, SchoolCommutes, ParkTrips, CommunityEngagements,
    DoctorsVisits, FreightShipments,
  },

  {
    // Transportation
    NumRoadSegments, NumIntersections, PavementMSq, RoadSystemValue,
    NumVehicles, TotalTrips, LongTrips, IntercityVisitors, IntercityTrips,
    VehicleTrips, TransitTrips, TimeSpentTraveling, AverageTripTime,
  },

  {
    // Macros
    CityCenters,
    PollutionStat, ValueStat, DensityStat, CrimeStat, EducationStat,
    ProsperityStat, CommunityStat, HealthStat,
    AggregatePropertyValue, RetailTransactions, TouristTransactions,
    InflationRate, InflationIndex,
    NationalInterestRate, CityBondRate, NationalStockIndex, FuelPrice,
    UnemploymentRate, NationalUnemploymentRate, TargetUnemploymentRate,
    NationalStockTrajectory, EconomicWavePhase, EconomicDeterminant,
  },

  {
    // Budget
    PropertyTaxRateStat, SalesTaxRateStat, FinesAndFeesRateStat,
    PropertyTaxStat, SalesTaxStat, FinesAndFeesStat,
    FuelTaxIncomeStat, AmenityIncomeStat, TransitIncomeStat, AssetSalesIncomeStat,
    RoadBuildExpensesStat, ExpwyBuildExpensesStat, RailBuildExpensesStat,
    PillarBuildExpensesStat, EminentDomainExpensesStat, RepairExpensesStat,
    BuildingBuildExpensesStat, MiscDiscExpensesStat,
    EducationExpensesStat, RecreationExpensesStat, ServicesExpensesStat,
    UniversityExpensesStat, TransitExpensesStat,
    TotalIncomeStat, TotalExpensesStat, TotalDiscretionaryStat,
    TotalMandatoryStat, TotalEarningsStat, AssetsStat, LineOfCreditStat,
    LoanInterestStat, CashflowStat, BudgetBalanceStat,
    CashAvailableStat,
  },

  {
    // Weather
    WeatherTempStat, WeatherPressureStat, WeatherCloudsStat,
    WeatherPercipitationStat, WeatherSnowPackStat,
    WeatherDroughtStat, WeatherWindSpeedStat,
  },

  {
    // Debug
    PeopleProcessed, Teleports,
    NumLaneBlocks, RouterQueueSize, RoutesBrokered, RouteRequests,
    RouteCacheHits, RouteCacheMisses, RouteCacheHitRate,
    RouteCacheErrors,
    FailedDistanceTests, RoutesInvalidated, RouteCacheSize,
    RoutesDiscarded, RoutesProcessed, RoutesProcessedPerSecond,
    RouterThreadsActive,
    FailedRoutes, VehicleRoutes, PersonRoutes, TransitRoutes,
    TransitVehicleRoutes,
    HomeRoutes, WorkRoutes, InterviewRoutes,
    StoreRoutes, FreightRoutes, DoctorRoutes, OtherRoutes,

    VehiclesSettling, VehiclesFree,
    NumTravelGroups, TravelGroupsSettling, TravelGroupsFree,
    Entities, EntitiesDrawn, CPUTime, FPS
  },
};

static const char* statCategoryNameTable[numStatistics] = {
  "Population", "Education", "Workforce", "Buildings", "Businesses", "Tourism",
  "Government", "Activities", "Transportation", "Macros", "Budget", "Weather",
  "Debug",
};

static const char* statNameTable[numStatistics] = {
  "Population", "Population Growth (hourly)", "Homeless People",
  "Households",
  "Move Ins (hourly)", "Move Outs (hourly)", "Total Moves (hourly)",
  "Employed", "Unemployed", "Open Positions", "Unemployment Rate",

  "High School Diplomas", "Open Positions for HS Grads",
  "HS Grad Unemployment Rate", "College Diplomas",
  "Open Positions for College Grads", "College Grad Unemployment Rate",
  "Positions Requiring No Education",

  "National Unemployment Rate", "Target Unemployment Rate",

  "Residential Demand", "Commercial Demand", "Industrial Demand",
  "Buildings", "Buildings Constructed",
  "Residential Buildings", "Retail Buildings",
  "Farm Buildings", "Amenities",
  "Office Buildings", "Factory Buildings",
  "Mixed Use Buildings",
  "Unused",

  "Total Homes", "Single Family Homes", "Multi-Family Homes",
  "Percent Multi-Family", "Empty Homes",
  "Storefronts", "Offices", "Percent Offices", "Empty Storefronts",
  "Farms", "Factories", "Industrialization", "Unused Farms",

  "Businesses", "Business Foundings (hourly)", "Business Closures (hourly)",
  "Retail Businesses",
  "Retail Businesses Open Now",
  "Office Businesses",
  "Office Businesses Open Now",
  "Agricultural Businesses",
  "Agricultural Businesses Open Now",
  "Industrial Businesses",
  "Industrial Businesses Open Now",
  "Percent Retail",

  "Amenities Enabled", "Parks", "Parks Open",
  "Eductational Institutions", "Eductational Institutions Open",
  "Community Services", "Community Services Open",
  "University Buildings", "University Buildings Open",

  "Road Segments", "Intersections and Junctions",
  "Square Feet of Pavement", "Value of Road System ($)",
  "Vehicles on the Road", "Trips Taken", "Extremely Long Trips",
  "Trips Home", "Work Commutes",
  "Social Visits", "Store Trips", "Freight Shipments", "School Commutes",
  "Park Visits", "Use of Community Services", "Visitors",
  "Trips to Other Cities",

  "Route Requests", "Route Cache Hits", "Route Cache Misses",
  "Route Cache Hit Rate",
  "Routes Processed", "Failed Routes", "Vehicle Routes", "Person Routes",
  "Home Routes", "Work Routes", "Store Routes", "Freight Routes",
  "Other Routes", "Failed Distance Tests", "Routes Invalidated",
  "Router Cache Size",

  "Pollution", "Land Value", "Density", "Crime", "Education", "Prosperity",
  "Aggregate Property Value ($)",
  "Retail Transactions ($/hour)",
  "Inflation Rate", "Price Index",
  "National Interest Rate", "City Bond Rate",
  "National Stock Market Index", "Stock Market Trajectory",
  "Economic Wave", "Economic Determinant",

  "Property Tax Rate", "Sales Tax Rate", "Fine and Fees Rate",
  "Property Tax Income ($/hour)", "Sales Tax Income ($/hour)",
  "Fines and Fees Income ($/hour)",
  "Gas Tax Income ($/hour)", "Amenity Income ($/hour)",
  "Transit Income ($/hour)",
  "Asset Sales ($/hour)", "Road Construction Costs ($/hour)",
  "Expwy Construction Costs ($/hour)",
  "Rail Construction Costs ($/hour)", "Pillar Construction Costs ($/hour)",
  "Eminent Domain Costs ($/hour)", "Repair Costs ($/hour)",
  "Building Construction Costs ($/hour)", "Misc Costs ($/hour)",
  "Education Spending ($/hour)", "Parks Spending ($/hour)",
  "Community Services Spending ($/hour)", "University Spending ($/hour)",
  "Transit Operations ($/hour)", "Total Income ($/hour)",
  "Total Expenses ($/hour)", "Total Discretionary Costs ($/hour)",
  "Total Mandatory Spending ($/hour)",
  "Total Earnings ($/hour)", "Government Assets ($)",
  "Line of Credit ($)", "Loan Interest ($/hour)", "Cashflow ($/hour)",
  "Balance ($)", "Cash to Spend ($)",

  "Temperature (F)", "Barometric Pressure", "Cloud Cover", "Percipition",
  "Snow Pack", "Drought", "Wind Speed (m/s)",

  "Router Queue", "Lane Blocks", "Teleports", "Trips by Private Car",
  "Failed Time Tests", "Positions Filled", "Positions Vacated",
  "People Processed per Hour", "Amenities Open Now",
  "Job Interviews", "Interview Routes",

  "People with No Education",
  "People with a HS Diploma",
  "People with a Bachelor's Degree",
  "People with a Doctorate",

  "Percent with No Education",
  "Percent with a HS Diploma",
  "Percent with a Bachelor's Degree",
  "Percent with a Doctorate",

  "Number of Workers",
  "Number of Workers with No Education",
  "Number of Workers with a HS Diploma",
  "Number of Workers with a Bachelor's Degree",
  "Number of Workers with a Doctorate",

  "Unemployed with No Education",
  "Unemployed with a HS Diploma",
  "Unemployed with a Bachelor's Degree",
  "Unemployed with a Doctorate",

  "Unemployment Rate with No Education",
  "Unemployment Rate with a HS Diploma",
  "Unemployment Rate with a Bachelor's Degree",
  "Unemployment Rate with a Doctorate",

  "Total Job Positions",
  "Jobs Requiring No Education",
  "Jobs Requiring a HS Diploma",
  "Jobs Requiring a Bachelor's Degree",
  "Jobs Requiring a Doctorate",

  "Open Jobs Requiring No Education",
  "Open Jobs Requiring a HS Diploma",
  "Open Jobs Requiring a Bachelor's Degree",
  "Open Jobs Requiring a Doctorate",

  "Empty Offices",
  "Empty Factories",

  "Entities", "Entities Drawn Per Frame",

  "College Students", "Maximum College Students",
  "Bunks for College Students", "Empty Bunks", "College Dormitories", 
  "Colleges", "Professorships",
  "Students Moving In", "Students Dropping Out", "Students Graduating",

  "Retail Demand", "Farm Demand", "Factory Demand", "Office Demand",
  "Retail Growth", "Farm Growth", "Factory Growth", "Office Growth",

  "Abandoned Buildings",
  "Abandoned Residential Buildings",
  "Abandoned Retail Buildings",
  "Abandoned Farm Buildings",
  "Abandoned Amenities",
  "Abandoned Office Buildings",
  "Abandoned Factory Buildings",
  "Abandoned Mixed Use Buildings",

  "Routes Discarded",
  "Transit Routes",
  "Fuel Price ($/Gallon)",
  "Trips by Transit",

  "Residential Zone Demand", "Retail Zone Demand",
  "Farm Zone Demand", "Government Zone Demand",
  "Office Zone Demand", "Factory Zone Demand", "Mixed Use Zone Demand", 
  "Park Zone Demand",

  "Park Lots",
  "Route Cache Errors",
  "Routes Brokered",

  "CPU Time", "Routes Processed Per Second",
  "Vehicles Settling", "Vehicles Free",
  "Travel Groups","Travel Groups Settling", "Travel Groups Free",
  "Frames per Second",

  "Doctor's Visits", "Doctor Routes", "People Sick", "Healthcare Providers",
  "Community", "Health",
  "Router Threads Active",
  "Transit Vehicle Routes",
  "Time Spent Traveling",
  "Recorded Trips",
  "Average Travel Time",

  "Deaths",
  "Death Years",
  "Average Age at Death",

  "Tourists Here Now",
  "Tourist Rating",
  "Hotel Rooms",
  "Empty Hotel Rooms",
  "Tourists Arriving per Hour",
  "Tourists Leaving per Hour",
  "Memories",
  "Tourist Retail Transactions ($/hour)",

  "Emergency Housing",
  "Emergency Housing Available/Needed",

  "Skyscrapers",
  "City Centers",
};

const char* timePeriodNames[numTimePeriods] = {
  "ALL", "20Y", "5Y", "1Y", "1D"
};

Econ* getEcon(item ndx) {
  return econs.get(ndx);
}

item getEcon(vec3 loc) {
  return ourCityEconNdx();
}

item sizeEcons() {
  return econs.size();
}

const char* getStatisticCode(item stat) {
  return statisticCode[stat];
}

item getStatisticForCode(const char* code) {
  for (int i = 0; i < numStatistics; i++) {
    if (streql(statisticCode[i], code)) {
      return i;
    }
  }
  return -1;
}

item addEcon(EconType type, vec3 location, char* name, item parent) {
  item ndx = econs.create();
  Econ* econ = getEcon(ndx);
  econ->flags = _econExists;
  econ->location = location;
  econ->multiplier = 1;
  econ->name = name;
  econ->type = type;
  econ->parent = parent;

  if (type == NationalEcon) {
    econ->flags |= _econAggregate;
  }

  for (int i = 0; i < numStatistics; i++) {
    TimeSeries* series = &(econ->statistics[i]);
    series->hasData = false;
    series->values.clear();
    series->values.reserve(5000);
    series->startDate = 0;
    series->timeStep = defaultTimeStep;
    econ->presentValues[i] = 0;
  }

  return ndx;
}

void removeEcon(item ndx) {
  Econ* econ = getEcon(ndx);
  if (econ->name != 0) free(econ->name);
  econ->name = 0;
  econ->flags = 0;

  for (int i = 0; i < numStatistics; i++) {
    TimeSeries* series = &(econ->statistics[i]);
    series->hasData = false;
    series->values.clear();
    series->values.reserve(5000);
    series->startDate = 0;
    series->timeStep = defaultTimeStep;
    econ->presentValues[i] = 0;
  }
}

const char* getEconName(item ndx) {
  if (ndx == ourCityEconNdx()) return getCityName();
  return getEcon(ndx)->name;
}

const char* timePeriodName(int tp) {
  return timePeriodNames[tp];
}

float nationalUnemploymentRate() {
  return nationalUnemp;
}

float targetUnemploymentRate(item econ) {
  if (econ == ourCityEconNdx()) {
    return targetUnemp;
  } else {
    return nationalUnemp;
  }
}

float zoneDemand(item econ, item zone) {
  return getStatistic(econ,
      (Statistic)(ResidentialZoneDemand+zone-1));
}

float zoneDemandSoft(item zone) {
  return zoneDemSoft[zone];
}

float getInflationMultiplier() {
  float inf = getInflation();
  if (inf > 500) return 500;
  if (inf > 200) return 200;
  if (inf > 100) return 100;
  if (inf > 50) return 50;
  if (inf > 20) return 20;
  if (inf > 10) return 10;
  if (inf > 5) return 5;
  if (inf > 2) return 2;
  return 1;
}

const char* statName(item ndx) {
  if (useMetric()) {
    if (ndx == WeatherTempStat) {
      return "Temperature (C)";
    } else if (ndx == PavementMSq) {
      return "Square Meters of Pavement";
    } else if (ndx == FuelPrice) {
      return "Fuel Price ($/Liter)";
    }
  }
  return statNameTable[ndx];
}

const char* statCategoryName(item ndx) {
  return statCategoryNameTable[ndx];
}

vector<Statistic> statsCategory(item ndx) {
  return statCategoryTable[ndx];
}

void adjustStat(item econNdx, item ndx, float adj) {
  if (econNdx <= 0 || econNdx > econs.size()) {
    SPDLOG_WARN("bad econ {}/{}", econNdx, econs.size());
    return;
  }
  if (ndx >= numStatistics || ndx < 0) {
    SPDLOG_WARN("bad statistic {}/{}", ndx, numStatistics);
    return;
  }

  Econ* econ = getEcon(econNdx);
  econ->presentValues[ndx] += adj;

  if (econ->parent != 0) {
    adjustStat(econ->parent, ndx, adj);
  }
}

void setStat(item econNdx, item ndx, float adj) {
  getEcon(econNdx)->presentValues[ndx] = adj;
}

void resetStat(item ndx) {
  for (int k = 1; k <= econs.size(); k++) {
    getEcon(k)->presentValues[ndx] = 0;
  }
}

float getRatio(item econNdx, Statistic num, Statistic denom) {
  return getStatistic(econNdx, num) / nonZero(getStatistic(econNdx, denom));
}

float getRestrictedRatio(item econNdx, Statistic num, Statistic denom) {
  float denomNum = getStatistic(econNdx, denom);
  if (denomNum < 500) return 0;
  return getStatistic(econNdx, num) / nonZero(denomNum);
}

float getStatisticNow(item econNdx, Statistic stat) {
  if (statIsRateCounter[stat]) {
    Econ* econ = getEcon(econNdx);
    TimeSeries* sample = &econ->statistics[stat];
    item num = sample->values.size();
    if (num == 0) return 0;
    return sample->values[num-1];
  } else {
    return getStatistic(econNdx, stat);
  }
}

float getStatisticAt(item econNdx, Statistic stat, float time) {
  Econ* econ = getEcon(econNdx);
  TimeSeries* sample = &econ->statistics[stat];
  item numValues = sample->values.size();
  //SPDLOG_INFO("getStatisticAt {}@{}/{} numValues{} hasData{}",
      //getStatisticCode(stat), printDateTimeString(time), time,
      //numValues, sample->hasData);
  //SPDLOG_INFO("getStatisticAt startDate{} timeStep{}",
      //printDateTimeString(sample->startDate),
      //printDurationString(sample->timeStep));
  float endDate = sample->startDate + sample->timeStep * numValues;
  float targetSlice = (time - sample->startDate)/sample->timeStep;
  item targetNum = clamp(int(round(targetSlice)), 0, numValues);
  //SPDLOG_INFO("getStatisticAt endDate{} targetSlice{} targetNum{}",
      //endDate, targetSlice, targetNum);
  if (numValues == 0 || !sample->hasData) {
    return getStatisticNow(econNdx, stat);
  } else if (sample->startDate > time) {
    return sample->values[0];
  }

  //if (abs(endDate - time) > abs(getCurrentDateTime() - time)) {
    //return getStatisticNow(econNdx, stat);
  //}

  return sample->values[targetNum];
}

float getStatisticSum(item econNdx, Statistic stat, float time) {
  Econ* econ = getEcon(econNdx);
  TimeSeries* sample = &econ->statistics[stat];
  item numValues = sample->values.size();
  float endDate = sample->startDate + sample->timeStep * numValues;
  float targetSlice = (time - sample->startDate)/sample->timeStep;
  item targetNum = clamp(int(round(targetSlice)), 0, numValues);
  if (numValues == 0 || !sample->hasData || targetNum >= numValues) {
    return 0;
  } else if (sample->startDate > time) {
    targetNum = 0;
  }

  float result = 0;
  for (int i = targetNum; i < numValues; i++) {
    float val = sample->values[i];
    result += val;
  }
  return result;
}

float getStatisticAvg(item econNdx, Statistic stat, float time) {
  Econ* econ = getEcon(econNdx);
  TimeSeries* sample = &econ->statistics[stat];
  item numValues = sample->values.size();
  float endDate = sample->startDate + sample->timeStep * numValues;
  float startDate = endDate - time;
  float targetSlice = (startDate - sample->startDate)/sample->timeStep;
  item targetNum = clamp(int(round(targetSlice)), 0, numValues-1);
  /*
  SPDLOG_INFO("do getStatisticAvg({},{},{}) targetSlice {} endDate{} startDate{} sample->startDate{} sample->timeStep{}",
      econNdx, statisticCode[stat], time, targetSlice, endDate, startDate,
      sample->startDate, sample->timeStep);
      */
  if (numValues == 0 || !sample->hasData || targetNum >= numValues) {
    return 0;
  }

  float result = 0;
  for (int i = targetNum; i < numValues; i++) {
    float val = sample->values[i];
    result += val;
  }

  /*
  SPDLOG_INFO("total{} values{} avg{} numValues{} targetNum{}",
      result, numValues-targetNum,
      result / (numValues-targetNum),
      numValues, targetNum);
      */
  return result / (numValues - targetNum);
}

float getStatisticMin(item econNdx, Statistic stat, float time) {
  Econ* econ = getEcon(econNdx);
  TimeSeries* sample = &econ->statistics[stat];
  item numValues = sample->values.size();
  float endDate = sample->startDate + sample->timeStep * numValues;
  float targetSlice = (time - sample->startDate)/sample->timeStep;
  item targetNum = clamp(int(round(targetSlice)), 0, numValues);
  if (numValues == 0 || !sample->hasData || targetNum >= numValues) {
    return 0;
  } else if (sample->startDate > time) {
    targetNum = 0;
  }

  float result = sample->values[targetNum];
  for (int i = targetNum+1; i < numValues; i++) {
    float val = sample->values[i];
    if (val < result) {
      result = val;
    }
  }
  return result;
}

float getStatisticMax(item econNdx, Statistic stat, float time) {
  Econ* econ = getEcon(econNdx);
  TimeSeries* sample = &econ->statistics[stat];
  item numValues = sample->values.size();
  float endDate = sample->startDate + sample->timeStep * numValues;
  float targetSlice = (time - sample->startDate)/sample->timeStep;
  item targetNum = clamp(int(round(targetSlice)), 0, numValues);
  if (numValues == 0 || !sample->hasData || targetNum >= numValues) {
    return 0;
  } else if (sample->startDate > time) {
    targetNum = 0;
  }

  float result = sample->values[targetNum];
  for (int i = targetNum+1; i < numValues; i++) {
    float val = sample->values[i];
    if (val > result) {
      result = val;
    }
  }
  return result;
}

float getStatisticMin(item econ, Statistic stat, float time);
float getStatisticMax(item econ, Statistic stat, float time);

float getStatistic(item econNdx, Statistic stat) {
  if (econNdx == 0) return 0;

  Econ* econ = getEcon(econNdx);

  if (stat >= PropertyTaxRateStat && stat <= FinesAndFeesRateStat) {
    return getTaxRate((BudgetLine)(stat - PropertyTaxRateStat + 1));

  } else if (stat >= PropertyTaxStat && stat <= BudgetBalanceStat) {
    int budgetLine = stat - PropertyTaxStat + 1;
    int flags = budgetLineFlags[budgetLine];
    if (flags & _budgetNotAggregate) {
      return getBudget(0).line[budgetLine];
    }
  }

  switch (stat) {
    //case Population: return numPeople();
    //case NumFamilies: return numFamilies();

    case UnemploymentRate: return getRestrictedRatio(econNdx,
                               NumUnemployed, NumWorkers);
    case NoEduPercent: return getRatio(econNdx, NumNoEdu, Population);
    case HSEduPercent: return getRatio(econNdx, NumHSEdu, Population);
    case BclEduPercent: return getRatio(econNdx, NumBclEdu, Population);
    case PhdEduPercent: return getRatio(econNdx, NumPhdEdu, Population);

    case NoEduUnemployment: return getRestrictedRatio(econNdx,
              NumNoEduUnemployed, NumNoEduWorkers);
    case HSEduUnemployment: return getRestrictedRatio(econNdx,
              NumHSEduUnemployed, NumHSEduWorkers);
    case BclEduUnemployment: return getRestrictedRatio(econNdx,
              NumBclEduUnemployed, NumBclEduWorkers);
    case PhdEduUnemployment: return getRestrictedRatio(econNdx,
              NumPhdEduUnemployed, NumPhdEduWorkers);

    //case NationalUnemploymentRate: return nationalUnemploymentRate();
    case TargetUnemploymentRate: return targetUnemploymentRate(econNdx);

                                   /*
    case ResidentialDemand: return zoneDemand(ResidentialZone);
    case RetailDemand: return getBusinessTypeDemand(Retail);
    case FarmDemand: return getBusinessTypeDemand(Farm);
    case FactoryDemand: return getBusinessTypeDemand(Factory);
    case OfficeDemand: return getBusinessTypeDemand(Office);

    case RetailGrowth: return getBusinessTypeGrowth(Retail);
    case FarmGrowth: return getBusinessTypeGrowth(Farm);
    case FactoryGrowth: return getBusinessTypeGrowth(Factory);
    case OfficeGrowth: return getBusinessTypeGrowth(Office);

    case NumBuildings: return numBuildings();
    */
    case PercentMultiFam: return getStatistic(econNdx, NumMultiFam) /
                          nonZero(getStatistic(econNdx, NumHomes));
    case EmptyHomes: return boardSize(econNdx, Homes);
    case NumEmptyHotelRooms: return boardSize(econNdx, HotelRooms);
    case PercentOffices: return getStatistic(econNdx, NumOffices) /
      nonZero(getStatistic(econNdx, NumShops) +
          getStatistic(econNdx, NumOffices));
    case EmptyShops: return boardSize(econNdx, RetailTenancies);
    case EmptyOffices: return boardSize(econNdx, OfficeTenancies);
    case PercentFactories: return getStatistic(econNdx, NumFactories) /
      nonZero(getStatistic(econNdx, NumFarms) +
          getStatistic(econNdx, NumFactories));
    case EmptyFarms: return boardSize(econNdx, FarmTenancies);
    case EmptyFactories: return boardSize(econNdx, FactoryTenancies);
    case NumEmptyBunks: return boardSize(econNdx, DormBunks);

    //case NumBusinesses: return numBusinesses();
    case PercentRetail: return getRatio(econNdx,
                              NumRetailBiz, NumBusinesses);

    case NumRoadSegments: return numCompleteEdges();
    case NumLaneBlocks: return numLaneBlocks();
    case NumVehicles: return countVehicles();
    case RouteCacheHitRate: return econ->presentValues[RouteCacheHits] /
      nonZero(econ->presentValues[RouteCacheHits] +
          econ->presentValues[RouteCacheMisses]);
    case RouteCacheSize: return routeCacheSize();

    case PollutionStat: return heatMapTotal(Pollution);
    case ValueStat: return heatMapTotal(Value);
    case DensityStat: return heatMapTotal(Density);
    case CrimeStat: return heatMapTotal(Crime);
    case EducationStat: return heatMapTotal(Education);
    case ProsperityStat: return heatMapTotal(Prosperity);
    case CommunityStat: return heatMapTotal(CommunityHM);
    case HealthStat: return heatMapTotal(HealthHM);

    //case AggregatePropertyValue: return getAggregatePropertyValue();
    //case InflationRate: return getInflationRate();
    //case InflationIndex: return getInflation() * 100 / defaultInflation;
    //case NationalInterestRate: return getNationalInterestRate();
    case CityBondRate: return getInterestRate();
    //case NationalStockIndex: return nationalStockIndex;
    //case NationalStockTrajectory: return nationalStockTrajectory;
    //case EconomicWavePhase: return economicWavePhase;
    //case EconomicDeterminant: return economicDeterminant;
    case CashAvailableStat: return getCredit();

    case WeatherTempStat: return getWeather().temp;
    case WeatherPressureStat: return mix(27.f, 32.f, getWeather().pressure);
    case WeatherCloudsStat: return getWeather().clouds;
    case WeatherPercipitationStat: return getWeather().percipitation;
    case WeatherSnowPackStat: return getWeather().snow;
    case WeatherDroughtStat: return 0; //getWeather().drought;
    case WeatherWindSpeedStat: return length(getWeather().wind);
    case Entities: return countEntities();
    case EntitiesDrawn: return numEntitiesDrawn();
    case RoutesProcessedPerSecond:
            return econ->presentValues[RoutesProcessed] /
              nonZero(econ->presentValues[CPUTime]);
    case AverageTripTime:
            return econ->presentValues[TimeSpentTraveling] /
              nonZero(econ->presentValues[RecordedTrips]);
    case LifeExpectancy: {
            float val = econ->presentValues[DeathYears] /
              nonZero(econ->presentValues[Deaths]);
            return val > 0 ? val : getStatisticNow(econNdx, LifeExpectancy);
          } break;

    case TouristRating: {
      float memoriesPerTourist = getStatisticNow(econNdx, Memories) /
        nonZero(getStatisticNow(econNdx, NumTouristsNow));
      double rating = memoriesPerTourist / c(CMemoriesForFiveStars) * 5;
      float lastVal = getStatisticAvg(econNdx, TouristRating, 1);
      rating = clamp(rating, -1., 6.);
      rating = mix(double(lastVal), rating, oneHour);
      rating = clamp(rating, 0., 5.);
      return rating;
    } break;

    case EmergencyHousingBalance: {
      return -emergencyHousingBalance(econNdx);
    } break;

    default: return econ->presentValues[stat];
  }
}

float convertStatistic(Statistic stat, float val) {
  if (stat == RoadSystemValue ||
      stat == AggregatePropertyValue ||
      stat == RetailTransactions ||
      (stat >= PropertyTaxStat && stat <= CashAvailableStat)) {
    return val * c(CMoneyMultiplier);
  } else if (useMetric()) {
    return val;
  } else {
    switch (stat) {
      //case RoadSystemValue: return val*getInflation();
      case PavementMSq: return val*9; // m^2 to feet
      case WeatherTempStat: return val*1.8 + 32; // C to F
      case FuelPrice: return val/0.264; // Liters to Gallons
      default: return val;
    }
  }
}

TimeSeries getTimeSeries(item econNdx, Statistic stat) {
  return getEcon(econNdx)->statistics[stat];
}

bool timeSeriesHasData(item econNdx, Statistic stat) {
  if (stat < 0 || stat >= numStatistics) return false;
  if (econNdx < 0 || econNdx > econs.size()) return false;
  Econ* econ = getEcon(econNdx);
  return econ->statistics[stat].hasData;
}

float getNationalInterestRate() {
  return nationalInterestRate;
}

float getEconomicDeterminant() {
  return economicDeterminant;
}

float moneyToTime(money amount) {
  if (c(CValueOfTime) == 0) return 0;
  float dollarPerHour = c(CValueOfTime)*getInflation();
  return amount / dollarPerHour * oneHour;
}

float getInflationRate() {
  return inflationRate;
}

money getInflation() {
  return inflation;
  /* double year = getCurrentDateTime()/oneYear;
  double factor = year/(inflationBaseYear-baseYear);
  return (1-factor)*0.09534 + factor; */
}

void updateEconomy(float duration) {
  double prosperity = heatMapTotal(Prosperity)*2;
  double time = getCurrentDateTime();
  double adjRate = duration*.2f;

  economicWavePhase += randFloat(-.9,1.1) * duration / (2*pi_o);
  economicWavePhase += randFloat(-.1,.1) * .2 / (2*pi_o);
  setStat(nationalEconNdx(), EconomicWavePhase, economicWavePhase);
  double econDet = sin(economicWavePhase * 2*pi_o) * .5;
  //econDet = (econDet + 1)*.5; // range [0, 1]
  econDet += randFloat(0.5, 1.5-prosperity*.5);
  econDet -= prosperity*adjRate*.0075;
  economicDeterminant = mix(economicDeterminant, econDet, adjRate*1.);
  economicDeterminant += randFloat(-1, 1)*adjRate*4.;
  economicDeterminant = clamp(economicDeterminant, 0., 1.);
  setStat(nationalEconNdx(), EconomicDeterminant, economicDeterminant);

  double unemp = mix(0.04, 0.20, pow(1-economicDeterminant, 2));
  unemp += randFloat(-0.02, 0.02);
  nationalUnemp = mix(nationalUnemp, unemp, adjRate*.2);
  nationalUnemp += randFloat(-1., 1.)*adjRate;
  nationalUnemp = clamp(nationalUnemp, 0., 0.3);
  setStat(nationalEconNdx(), NationalUnemploymentRate, nationalUnemp);

  double infRate = mix(0.00, 0.08, pow(economicDeterminant, 1.5));
  infRate += randFloat(-0.02, 0.02);
  inflationRate = mix(inflationRate, infRate, adjRate*.5);
  inflationRate += randFloat(-1, 1)*adjRate;
  inflation *= compound(inflationRate*randFloat(0.99,1.01), adjRate);
  setStat(nationalEconNdx(), InflationRate, inflationRate);
  setStat(nationalEconNdx(), InflationIndex, inflation * 100 /
      defaultInflation);

  double interest  = mix(0.01, 0.20, pow(1-economicDeterminant, 2.));
  interest += randFloat(-0.02, 0.02);
  nationalInterestRate = mix(nationalInterestRate, interest, adjRate);
  nationalInterestRate += randFloat(-1, 1)*adjRate;
  nationalInterestRate = clamp(nationalInterestRate, 0., 0.2);
  setStat(nationalEconNdx(), NationalInterestRate, nationalInterestRate);

  double traj = mix(-.8, 2., 1-pow(1-economicDeterminant, 4));
  traj += randFloat(-0.5, 0.5);
  traj = clamp(traj, 0.1, 2.0);
  nationalStockTrajectory = mix(nationalStockTrajectory, traj, adjRate*4);
  nationalStockIndex *= mix(1., nationalStockTrajectory, adjRate*.05);
  nationalStockIndex += nationalStockIndex * randFloat(-1,1.) * .0025;
  setStat(nationalEconNdx(), NationalStockIndex, nationalStockIndex);
  setStat(nationalEconNdx(), NationalStockTrajectory, nationalStockTrajectory);

  //Target Unemployment Rate
  float bias = mix(c(CTargetUnemploymentBias0), c(CTargetUnemploymentBias1M),
    clamp(numPeople(ourCityEconNdx())/1000000.f, 0.f, 1.f));
  float bizEffect = getEffectMultiplier(BusinessEffect);
  float adjustment = bias
    + (.5f-heatMapTotal(Prosperity)) * 0.05f;
  adjustment *= bizEffect;
  adjustment += getTaxRate(PropertyTax) * c(CPropertyTaxUnemploymentEffect)
    + getTaxRate(SalesTax) * c(CSalesTaxUnemploymentEffect)
    + getTaxRate(FinesAndFeesIncome) * c(CFinesAndFeesTaxUnemploymentEffect);
  float intrinsicRate = nationalUnemploymentRate()*2*getEffectMultiplier(BusinessEffect);
  float nextTarget = clamp(intrinsicRate + adjustment, 0.01f, 0.4f);
  if (statsReset) {
    targetUnemp = nextTarget;
  } else {
    targetUnemp = mix(targetUnemp, nextTarget, adjRate*20);
  }
  targetUnemp = clamp(targetUnemp, 0.f, 0.25f);

  // Fuel
  setStat(nationalEconNdx(), FuelPrice,
      mix(0.05 * inflation * (0.2 + inflationRate*10),
        nationalStockIndex * 0.00004, 0.5));

  setStat(ourCityEconNdx(), FPS, getFPS());

  statsReset = false;
}

void updateZoneDemand(float duration) {
  for (int econ = 1; econ <= econs.size(); econ++) {
    item pop = numPeople(econ);
    float topDemand = 0;
    for (int i = 1; i < numZoneTypes; i++) {
      if (i == GovernmentZone) {
        setStat(econ, GovernmentZoneDemand, 0);
        continue;
      }

      if (i == ParkZone) {
        setStat(econ, ParkZoneDemand, 0);
        continue;
      }

      float result = 1.0;

      if (i == ResidentialZone || i == MixedUseZone) {
        int empties = getStatistic(econ, EmptyHomes);
        int tenancies = getStatistic(econ, NumHomes);
        float target = 2+c(CTargetEmptyHomes)*tenancies;
        result *= (target - empties)/nonZero(target);
        result *= 1-c(CPropertyTaxResidentialEffect)*getTaxRate(PropertyTax);

        float unemploymentDiff = unemploymentRate(econ) -
          targetUnemploymentRate(econ);
        result *= clamp(1 - unemploymentDiff/c(CMaxUnemploymentDiff),
            0.f, 1.f);

        if (i == MixedUseZone) {
          result = std::min(result, getBusinessTypeDemand(econ, Retail));
        }

      } else if (i == RetailZone) {
        result *= getBusinessTypeDemand(econ, Retail);

      } else if (i == OfficeZone) {
        result *= getBusinessTypeDemand(econ, Office);

      } else if (i == FarmZone) {
        result *= getBusinessTypeDemand(econ, Farm);

      } else if (i == FactoryZone) {
        result *= getBusinessTypeDemand(econ, Factory);
      }

      if (i == ResidentialZone && pop < 100) result = 1;
      if (i == FactoryZone && pop < 1000) result = 0;
      if (i == OfficeZone && pop < 10000) result = 0;
      if (i == MixedUseZone && pop < 20000) result = 0;

      //SPDLOG_INFO("zone demand {} {}=>{}",
          //getEcon(econ)->name, i, result);
      result = clamp(result*2.f, 0.f, 1.f);
      if (result > topDemand && i != MixedUseZone && i != FarmZone &&
          i != FactoryZone) topDemand = result;
      setStat(econ, ResidentialZoneDemand+i-1, result);
    }

    setStat(econ, MixedUseZoneDemand, topDemand);
  }

  for (int i = 1; i < numZoneTypes; i++) {
    float last = zoneDemSoft[i];
    float target = zoneDemand(ourCityEconNdx(), i);
    zoneDemSoft[i] = mix(last, target, duration*0.1f);
  }
}

void updateStatistics(double duration) {
  updateZoneDemand(duration);
  econDurToGo += duration / gameDayInRealSeconds;
  if (econDurToGo > economicTick) {
    econDurToGo -= economicTick;
    updateEconomy(economicTick);
    fireEvent(EventEconomyUpdate);
  }

  double time = getCurrentDateTime();
  TimeSeries* sample =
    &getEcon(ourCityEconNdx())->statistics[WeatherTempStat];
  float nextStatUpdate = sample->startDate +
    sample->timeStep * sample->values.size();
  //SPDLOG_INFO("econ time:{} nextStatUpdate:{}",
      //time, nextStatUpdate, sample->timeStep, sample->values.size());

  if (time >= nextStatUpdate || sample->values.size() == 0) {
    fireEvent(EventStatisticsUpdate);
    updateNewspaper();

    for (int k = 1; k <= econs.size(); k++) {
      Econ* econ = getEcon(k);
      for (int i = 0; i < numStatistics; i++) {
        TimeSeries* series = &econ->statistics[i];
        float val = getStatistic(k, (Statistic)i);

        if (i == RoadSystemValue) {
          val *= getInflation();
        }

        if (series->values.size() == 0) {
          if (val == 0) continue;
          series->startDate = getCurrentDateTime();
        }

        series->values.push_back(val);
        if (val != 0) {
          series->hasData = true;
        }
      }

      for (int i = 0; i < numStatistics; i++) {
        if (statIsRateCounter[i]) {
          econ->presentValues[i] = 0;
        }
      }
    }
  }
}

bool isStatDuration(Statistic stat) {
  return stat == AverageTripTime || stat == TimeSpentTraveling;
}

bool isStatRateCounter(Statistic stat) {
  return statIsRateCounter[stat];
}

void setRateCounters() {
  for (int i = 0; i < numStatistics; i++) {
    statIsRateCounter[i] = false;
  }

  for (int i = 0; i < numBudgetLines; i++) {
    int flag = budgetLineFlags[i];
    if (!(flag & _budgetNotAggregate)) {
      statIsRateCounter[PropertyTaxStat + i - 1] = true;
    }
  }

  for (int i = TotalTrips; i <= RoutesInvalidated; i++) {
    statIsRateCounter[i] = true;
  }

  statIsRateCounter[PopulationGrowth] = true;
  statIsRateCounter[FamiliesMovingIn] = true;
  statIsRateCounter[FamiliesMovingOut] = true;
  statIsRateCounter[TotalMoves] = true;
  statIsRateCounter[BuildingsConstructed] = true;
  statIsRateCounter[BusinessesCreated] = true;
  statIsRateCounter[BusinessesClosed] = true;
  statIsRateCounter[RetailTransactions] = true;
  statIsRateCounter[Teleports] = true;
  statIsRateCounter[VehicleTrips] = true;
  statIsRateCounter[FailedTimeTests] = true;
  statIsRateCounter[NumHired] = true;
  statIsRateCounter[NumQuitOrFired] = true;
  statIsRateCounter[PeopleProcessed] = true;
  statIsRateCounter[JobInterviews] = true;
  statIsRateCounter[InterviewRoutes] = true;
  statIsRateCounter[NumStudentsMovingIn] = true;
  statIsRateCounter[NumStudentsDroppingOut] = true;
  statIsRateCounter[NumStudentsGraduating] = true;
  statIsRateCounter[RoutesDiscarded] = true;
  statIsRateCounter[RoutesBrokered] = true;
  statIsRateCounter[TransitRoutes] = true;
  statIsRateCounter[TransitTrips] = true;
  statIsRateCounter[RouteCacheErrors] = true;
  statIsRateCounter[CPUTime] = true;
  statIsRateCounter[DoctorsVisits] = true;
  statIsRateCounter[DoctorRoutes] = true;
  statIsRateCounter[TransitVehicleRoutes] = true;
  statIsRateCounter[TimeSpentTraveling] = true;
  statIsRateCounter[RecordedTrips] = true;
  statIsRateCounter[AverageTripTime] = true;
  statIsRateCounter[Deaths] = true;
  statIsRateCounter[DeathYears] = true;
  statIsRateCounter[LifeExpectancy] = true;
  statIsRateCounter[NumTouristsArriving] = true;
  statIsRateCounter[NumTouristsLeaving] = true;
  statIsRateCounter[Memories] = true;
  statIsRateCounter[TouristTransactions] = true;
}

void resetStatistics() {
  statsReset = true;
  targetUnemp = 0.01;
  econDurToGo = 0;
  economicWavePhase = 0.25;
  economicDeterminant = 1.0;
  nationalUnemp = 0.04;
  nationalInterestRate = 0.04;
  nationalStockIndex = 1000;
  nationalStockTrajectory = 1;
  inflationRate = 0.04;
  inflation = defaultInflation;
  ourCityEcon = 0;
  nationalEcon = 0;

  for (int k = 1; k <= econs.size(); k++) {
    Econ* econ = getEcon(k);

    for (int i = 0; i < numStatistics; i++) {
      TimeSeries* series = &econ->statistics[i];
      series->hasData = false;
      series->values.clear();
      vector<float> empty;
      series->values.swap(empty);
      series->startDate = 0;
      series->timeStep = defaultTimeStep;
      econ->presentValues[i] = 0;
    }
  }

  for (int i = 0; i < numZoneTypes; i++) {
    zoneDemSoft[i] = 0;
  }

  econs.clear();

  setRateCounters();
}

item ourCityEconNdx() { return ourCityEcon; }
item nationalEconNdx() { return nationalEcon; }

void initEcons() {
  float ms = getMapSize();
  nationalEcon = addEcon(NationalEcon, vec3(ms, ms, 0),
      strdup_s("National Economy"), 0);
  ourCityEcon = addEcon(OurCityEcon, vec3(ms, ms, 0),
      strdup_s("Local Economy"), nationalEcon);
}

void writeEcon(FileBuffer* file, item ndx) {
  Econ* econ = getEcon(ndx);

  fwrite_uint32_t(file, econ->flags);
  fwrite_uint8_t(file, econ->type);
  fwrite_item(file, econ->parent);
  fwrite_vec3(file, econ->location);
  fwrite_float(file, econ->multiplier);
  fwrite_string(file, econ->name);

  for (int i = 0; i < numStatistics; i++) {
    fwrite_float(file, econ->presentValues[i]);
    TimeSeries* series = &econ->statistics[i];
    fwrite_float(file, series->startDate);
    fwrite_float(file, series->timeStep);
    int num = series->values.size();
    fwrite_int(file, num);
    for (int j = 0; j < num; j++) {
      fwrite_float(file, series->values[j]);
    }
  }
}

void readEcon(FileBuffer* file, item ndx, int numStats) {
  Econ* econ = getEcon(ndx);

  if (file->version >= 52) {
    econ->flags = fread_uint32_t(file);
    econ->type = fread_uint8_t(file);
    econ->parent = fread_item(file);
    econ->location = fread_vec3(file);
    econ->multiplier = fread_float(file);
    econ->name = fread_string(file);
  }

  if (econ->type == OurCityEcon) ourCityEcon = ndx;
  if (econ->type == NationalEcon) nationalEcon = ndx;

  for (int i = 0; i < numStats; i++) {
    float presentVal = fread_float(file);
    float startDate = fread_float(file);
    float timeStep = fread_float(file);
    int num = fread_int(file);

    if (i < numStatistics) {
      econ->presentValues[i] = presentVal;
      TimeSeries* series = &econ->statistics[i];
      series->startDate = startDate;
      series->timeStep = timeStep;
      if (file->version < 54) series->timeStep = defaultTimeStep;
      series->values.clear();
      series->values.reserve(num*2);
      for (int j = 0; j < num; j++) {
        float val = fread_float(file);
        series->values.push_back(val);
        if (val != 0) {
          series->hasData = true;
        }
      }

    } else {
      for (int j = 0; j < num; j++) {
        fread_float(file);
      }
    }
  }

  for (int i = numStats; i < numStatistics; i++) {
    TimeSeries* series = &econ->statistics[i];
    series->values.clear();
    series->hasData = false;
    series->startDate = getCurrentDateTime();
    series->timeStep = defaultTimeStep;
  }

  // Fix a bug in the cashflow stat
  if (file->version < 55) {
    TimeSeries* cashflow = &econ->statistics[CashflowStat];
    for (int i = 0; i < cashflow->values.size(); i++) {
      cashflow->values[i] *= -1;
    }
  }
}

void writeStatistics(FileBuffer* file) {
  fwrite_double(file, econDurToGo);
  fwrite_double(file, economicWavePhase);
  fwrite_double(file, economicDeterminant);
  fwrite_double(file, nationalUnemp);
  fwrite_double(file, nationalInterestRate);
  fwrite_double(file, inflationRate);
  fwrite_double(file, inflation);
  fwrite_double(file, nationalStockIndex);
  fwrite_double(file, nationalStockTrajectory);

  fwrite_int(file, numStatistics);

  econs.write(file);
  for (int k = 1; k <= econs.size(); k++) {
    writeEcon(file, k);
  }
}

void readStatistics(FileBuffer* file, int version) {
  float time = getCurrentDateTime();
  if (version < versionStatistics) {
    double year = getCurrentYear();
    double factor = year/(2019-c(CStartYear));
    inflation = (1-factor)*0.09534 + factor;

  } else {
    econDurToGo = fread_double(file);
    economicWavePhase = fread_double(file);
    economicDeterminant = fread_double(file);
    nationalUnemp = fread_double(file);
    nationalInterestRate = fread_double(file);
    inflationRate = fread_double(file);
    inflation = fread_double(file);
    nationalStockIndex = fread_double(file);
    nationalStockTrajectory = fread_double(file);

    int numStats = fread_int(file);
    item numEcons = 1;
    if (version >= 52) {
      econs.read(file, version);
      for (int k = 1; k <= econs.size(); k++) {
        readEcon(file, k, numStats);
      }

    } else {
      //initEcons();
      readEcon(file, ourCityEconNdx(), numStats);
    }

  }
}

void rebuildStats() {
  rebuildPopulationStats();
  rebuildBuildingStats();
  rebuildBusinessStats();
  rebuildRoadStats();
  updateZoneDemand(0);
}

