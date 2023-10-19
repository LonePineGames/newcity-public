#pragma once

#include "serialize.hpp"

enum StatisticCategory {
  PopulationStats, EducationStats, WorkforceStats,
  BuildingsStats, BusinessStats, TouristStats, ServicesStats, ActivityStats,
  TransportationStats, MacrosStats, BudgetStats, WeatherStats,
  DebugStats,

  numStatisticCategories
};

enum Statistic {
  #define STAT(N) N,
  #include "statisticsEnum.hpp"
  #undef STAT
  numStatistics
};

/*
enum Statistic {
  Population, PopulationGrowth, NumHomeless, NumFamilies,
  FamiliesMovingIn, FamiliesMovingOut, TotalMoves,
  NumEmployed, NumUnemployed, NumOpenPositions, UnemploymentRate,

  // Formally Education/Employment Stats
  UnusedStat1, UnusedStat2, UnusedStat3, UnusedStat4, UnusedStat5, UnusedStat6,
  UnusedStat7,

  NationalUnemploymentRate, TargetUnemploymentRate,

  ResidentialDemand, //CommercialDemand, IndustrialDemand,
  UnusedStat10, UnusedStat11,
  NumBuildings, BuildingsConstructed,
  NumResidentialBuildings, NumRetailBuildings,
  NumFarmBuildings, NumGovBuildings,
  NumOfficeBuildings, NumFactoryBuildings, NumMixedUseBuildings, UnusedStat9,
  //NumAbandoned, AbandonedResidentialBuildings, AbandonedCommercialBuildings,
  //AbandonedIndustrialBuildings,
  NumHomes, NumSingleFam, NumMultiFam, PercentMultiFam, EmptyHomes,
  NumShops, NumOffices, PercentOffices, EmptyShops,
  NumFarms, NumFactories, PercentFactories, EmptyFarms,

  NumBusinesses, BusinessesCreated, BusinessesClosed,
  NumRetailBiz, NumRetailOpen, NumOfficeBiz, NumOfficeOpen,
  NumAgriculturalBiz, NumAgriculturalOpen,
  NumIndustrialBiz, NumIndustrialOpen,
  PercentRetail,

  NumGovBuildingsEnabled,
  NumParks, NumParksEnabled, NumSchools, NumSchoolsEnabled,
  NumCommServices, NumCommServicesEnabled,
  NumUniBuildings, NumUniBuildingsEnabled,

  NumRoadSegments, NumIntersections, PavementMSq, RoadSystemValue,
  NumVehicles, TotalTrips, LongTrips,
  TripsHome, WorkCommutes, SocialVisits,
  StoreTrips, FreightShipments, SchoolCommutes, ParkTrips,
  CommunityEngagements, IntercityVisitors, IntercityTrips,

  RouteRequests, RouteCacheHits, RouteCacheMisses, RouteCacheHitRate,
  RoutesProcessed, FailedRoutes, VehicleRoutes, PersonRoutes,
  HomeRoutes, WorkRoutes, StoreRoutes, FreightRoutes, OtherRoutes,
  FailedDistanceTests, RoutesInvalidated, RouteCacheSize,

  PollutionStat, ValueStat, DensityStat, CrimeStat, EducationStat,
  ProsperityStat,

  AggregatePropertyValue, RetailTransactions, InflationRate, InflationIndex,
  NationalInterestRate, CityBondRate, NationalStockIndex,
  NationalStockTrajectory, EconomicWavePhase, EconomicDeterminant,

  // Budgetary
  PropertyTaxRateStat, SalesTaxRateStat, FinesAndFeesRateStat,
  PropertyTaxStat, SalesTaxStat, FinesAndFeesStat,
  FuelTaxIncomeStat, PoliceIncomeStat, TransitIncomeStat, AssetSalesIncomeStat,
  RoadBuildExpensesStat, ExpwyBuildExpensesStat, RailBuildExpensesStat,
  PillarBuildExpensesStat, EminentDomainExpensesStat, RepairExpensesStat,
  BuildingBuildExpensesStat, MiscDiscExpensesStat,
  EducationExpensesStat, RecreationExpensesStat, ServicesExpensesStat,
  UniversityExpensesStat, TransitExpensesStat,
  TotalIncomeStat, TotalExpensesStat, TotalDiscretionaryStat,
  TotalMandatoryStat, TotalEarningsStat, AssetsStat, LineOfCreditStat,
  LoanInterestStat, CashflowStat, BudgetBalanceStat,
  CashAvailableStat,

  WeatherTempStat, WeatherPressureStat, WeatherCloudsStat,
  WeatherPercipitationStat, WeatherSnowPackStat, WeatherDroughtStat,
  WeatherWindSpeedStat,

  // After v44
  RouterQueueSize, NumLaneBlocks, Teleports, VehicleTrips, FailedTimeTests,
  NumHired, NumQuitOrFired, PeopleProcessed, NumGovBuildingsOpen,
  JobInterviews, InterviewRoutes,

  NumNoEdu, NumHSEdu, NumBclEdu, NumPhdEdu,

  NoEduPercent, HSEduPercent, BclEduPercent, PhdEduPercent,

  NumWorkers,
  NumNoEduWorkers, NumHSEduWorkers, NumBclEduWorkers, NumPhdEduWorkers,

  NumNoEduUnemployed, NumHSEduUnemployed, NumBclEduUnemployed,
  NumPhdEduUnemployed,

  NoEduUnemployment, HSEduUnemployment, BclEduUnemployment, PhdEduUnemployment,

  NumPositions,
  NumNoEduPositions, NumHSPositions, NumBclPositions, NumPhDPositions,

  NumOpenNoEduPositions, NumOpenHSPositions, NumOpenBclPositions,
  NumOpenPhDPositions,

  EmptyOffices, EmptyFactories,

  Entities, EntitiesDrawn,

  NumCollegeStudents, MaxCollegeStudents, NumCollegeBunks, NumEmptyBunks,
  NumDorms, NumColleges, NumProfessors, NumStudentsMovingIn,
  NumStudentsDroppingOut, NumStudentsGraduating,

  RetailDemand, FarmDemand, FactoryDemand, OfficeDemand,
  RetailGrowth, FarmGrowth, FactoryGrowth, OfficeGrowth,

  NumAbandoned, AbandonedResidentialBuildings, AbandonedRetailBuildings,
  AbandonedFarmBuildings, AbandonedGovernmentBuildings,
  AbandonedOfficeBuildings, AbandonedFactoryBuildings,
  AbandonedMixedUseBuildings,

  RoutesDiscarded, TransitRoutes, FuelPrice, TransitTrips,

  ResidentialZoneDemand, RetailZoneDemand,
  FarmZoneDemand, GovernmentZoneDemand,
  OfficeZoneDemand, FactoryZoneDemand, MixedUseZoneDemand, ParkZoneDemand,

  ParkLots,

  RouteCacheErrors, RoutesBrokered,
  CPUTime, RoutesProcessedPerSecond, VehiclesSettling, VehiclesFree,
  NumTravelGroups, TravelGroupsSettling, TravelGroupsFree,
  FPS,

  DoctorsVisits, DoctorRoutes, PeopleSick, NumHealthcareProviders,
  CommunityStat, HealthStat,

  RouterThreadsActive,

  TransitVehicleRoutes, TimeSpentTraveling, RecordedTrips, AverageTripTime,

  numStatistics
};
*/

enum TimePeriods {
  PeriodAll,
  Period20Y,
  Period5Y,
  PeriodYear,
  PeriodDay,
  numTimePeriods
};

const float timePeriodLength[numTimePeriods] = {
  FLT_MAX,
  20*4,
  5*4,
  1*4,
  1
};

struct TimeSeries {
  float startDate;
  float timeStep;
  bool hasData;
  vector<float> values;
};

enum EconType {
  OurCityEcon, NeighborCityEcon, InvisibleCityEcon, NationalEcon,
  ChunkEcon,
  numEconTypes
};

struct Econ {
  int flags;
  float multiplier;
  uint8_t type;
  item parent;
  vec3 location;
  TimeSeries statistics[numStatistics+1];
  float presentValues[numStatistics+1];
  char* name;
};

const int _econExists = 1 << 0;
const int _econAggregate = 1 << 1;

const double defaultInflation = 1.0f;

item ourCityEconNdx();
item nationalEconNdx();
item addEcon(EconType type, vec3 location, char* name, item parent);
Econ* getEcon(item ndx);
const char* getEconName(item ndx);
item getEcon(vec3 loc);
void initEcons();
item sizeEcons();
void rebuildStats();

float targetUnemploymentRate(item econNdx);
float nationalUnemploymentRate();
float getNationalInterestRate();
float getEconomicDeterminant();
float getInflationRate();
float getInflationMultiplier();
float moneyToTime(double amount);
float zoneDemand(item econ, item zone);
float zoneDemandSoft(item zone);

const char* timePeriodName(int tp);
const char* statName(item ndx);
const char* statCategoryName(item ndx);
vector<Statistic> statsCategory(item ndx);
float convertStatistic(Statistic stat, float val);
float getStatistic(item econ, Statistic stat);
float getStatisticNow(item econNdx, Statistic stat);
float getStatisticAt(item econ, Statistic stat, float time);
float getStatisticSum(item econ, Statistic stat, float time);
float getStatisticAvg(item econ, Statistic stat, float time);
float getStatisticMin(item econ, Statistic stat, float time);
float getStatisticMax(item econ, Statistic stat, float time);
const char* getStatisticCode(item stat);
item getStatisticForCode(const char* code);
void adjustStat(item econ, item ndx, float adj);
void setStat(item econ, item ndx, float adj);
void resetStat(item ndx);
bool isStatRateCounter(Statistic stat);
bool isStatDuration(Statistic stat);

TimeSeries getTimeSeries(item econ, Statistic stat);
bool timeSeriesHasData(item econ, Statistic stat);
void updateStatistics(double duration);
void writeStatistics(FileBuffer* file);
void readStatistics(FileBuffer* file, int version);
void resetStatistics();

