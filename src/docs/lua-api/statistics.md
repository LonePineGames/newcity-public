NewCity LUA API - Statistics

##### get(number _stat_, [number _econ_=CityEcon]) => number

Takes the Statistic index _stat_ and returns the current value of that Statistic.

_econ_ is an optional parameter, which can be CityEcon or NationalEcon.

> get(StatPopulation) => $get(StatPopulation)$
> get(StatNationalStockIndex, NationalEcon) => $get(StatNationalStockIndex, NationalEcon)$
> formatDuration(get(StatAverageTripTime)) => "$formatDuration(get(StatAverageTripTime))$"

---

##### set(number _stat_, [number _econ_=CityEcon,] number _value_) => number

Sets the Statistic _stat_ to _value_.

_econ_ is an optional parameter, which can be CityEcon or NationalEcon.

---

##### getPast(number _stat_, number _duration_, [number _econ_=CityEcon]) => number

Takes the Statistic index _stat_ and time value _duration_ and returns the value of that Statistic, at _duration_ before the present moment.

_econ_ is an optional parameter, which can be CityEcon or NationalEcon.

Statistics are recorded every hour. Statistics may not exist for certain times for various reasons. In these cases, getPast() returns the best available value, and will default to returning the same value as get(_stat_) if no information is available.

> getPast(StatPopulation, OneYear) => $getPast(StatPopulation, OneYear)$
> getPast(StatNationalStockIndex, OneYear, NationalEcon) => $getPast(StatNationalStockIndex, OneYear, NationalEcon)$
> formatDuration(getPast(StatAverageTripTime, OneDay)) => "$formatDuration(getPast(StatAverageTripTime, OneDay))$"

---

##### delta(number _stat_, number _duration_, [number _econ_=CityEcon]) => number

Takes the Statistic index _stat_ and time value _duration_ and returns the difference between value of that Statistic at present moment, and the value at _duration_ before the present moment.

_econ_ is an optional parameter, which can be CityEcon or NationalEcon.

Equivalent to get(_stat_, _econ_) - getPast(_stat_, _duration_, _econ_).

Statistics are recorded every hour. Statistics may not exist for certain times for various reasons. In these cases, delta() uses the best available value, and will default to returning 0 if no information in available.

> delta(StatPopulation, OneYear) => $delta(StatPopulation, OneYear)$
> delta(StatNationalStockIndex, OneYear, NationalEcon) => $delta(StatNationalStockIndex, OneYear, NationalEcon)$
> formatDuration(delta(StatAverageTripTime, OneDay)) => "$formatDuration(delta(StatAverageTripTime, OneDay))$"

---

##### growth(number _stat_, number _duration_, [number _econ_=CityEcon]) => number

Takes the Statistic index _stat_ and time value _duration_ and returns the ratio, minus 1, between value of that Statistic at present moment, and the value at _duration_ before the present moment.

_econ_ is an optional parameter, which can be CityEcon or NationalEcon.

Equivalent to (get(_stat_, _econ_) / getPast(_stat_, _duration_, _econ_)) - 1.

Statistics are recorded every hour. Statistics may not exist for certain times for various reasons. In these cases, growth() uses the best available value, and will default to returning 0 if no information in available. Growth will not return NaN or Infinite values, even if the value of getPast(_stat_, _duration_) is 0.

> formatPercent(growth(StatPopulation, OneYear)) => $formatPercent(growth(StatPopulation, OneYear))$
> formatPercent(growth(StatNationalStockIndex, OneYear, NationalEcon)) => $formatPercent(growth(StatNationalStockIndex, OneYear, NationalEcon))$
> formatPercent(growth(StatAverageTripTime, OneDay)) => $formatPercent(growth(StatAverageTripTime, OneDay))$

---

##### sum(number _stat_, number _duration_, [number _econ_=CityEcon]) => number

Takes the Statistic index _stat_ and time value _duration_ and returns the sum of all hourly values since _duration_ before the present moment.

_econ_ is an optional parameter, which can be CityEcon or NationalEcon.

Statistics are recorded every hour. Statistics may not exist for certain times for various reasons. In these cases, sum() uses the available values, and will default to returning 0 if no information in available.

> sum(StatPopulationGrowth, OneYear) => $sum(StatPopulationGrowth, OneYear)$
> sum(StatCashflowStat, OneYear) => $sum(StatCashflowStat, OneYear)$

---

##### avg(number _stat_, number _duration_, [number _econ_=CityEcon]) => number

Takes the Statistic index _stat_ and time value _duration_ and returns the average of all hourly values since _duration_ before the present moment.

_econ_ is an optional parameter, which can be CityEcon or NationalEcon.

Equivalent to sum(_stat_, _duration_, _econ_) / (24*_duration_).

Statistics are recorded every hour. Statistics may not exist for certain times for various reasons. In these cases, avg() uses the available values, and will default to returning 0 if no information in available.

> avg(StatPopulationGrowth, OneYear) => $avg(StatPopulationGrowth, OneYear)$
> avg(StatCashflowStat, OneYear) => $avg(StatCashflowStat, OneYear)$

---

##### min(number _stat_, number _duration_, [number _econ_=CityEcon]) => number

Takes the Statistic index _stat_ and time value _duration_ and returns the minimum of all hourly values since _duration_ before the present moment.

_econ_ is an optional parameter, which can be CityEcon or NationalEcon.

Statistics are recorded every hour. Statistics may not exist for certain times for various reasons. In these cases, min() uses the available values, and will default to returning 0 if no information in available.

> min(StatPopulationGrowth, OneYear) => $min(StatPopulationGrowth, OneYear)$
> min(StatCashflowStat, OneYear) => $min(StatCashflowStat, OneYear)$

---

##### max(number _stat_, number _duration_, [number _econ_=CityEcon]) => number

Takes the Statistic index _stat_ and time value _duration_ and returns the minimum of all hourly values since _duration_ before the present moment.

_econ_ is an optional parameter, which can be CityEcon or NationalEcon.

Statistics are recorded every hour. Statistics may not exist for certain times for various reasons. In these cases, max() uses the available values, and will default to returning 0 if no information in available.

> max(StatPopulationGrowth, OneYear) => $max(StatPopulationGrowth, OneYear)$
> max(StatCashflowStat, OneYear) => $max(StatCashflowStat, OneYear)$

---

### List of Statistics

* StatPopulation
* StatPopulationGrowth
* StatNumHomeless
* StatNumFamilies
* StatFamiliesMovingIn
* StatFamiliesMovingOut
* StatTotalMoves
* StatNumEmployed
* StatNumUnemployed
* StatNumOpenPositions
* StatUnemploymentRate
* StatUnusedStat1
* StatUnusedStat2
* StatUnusedStat3
* StatUnusedStat4
* StatUnusedStat5
* StatUnusedStat6
* StatUnusedStat7
* StatNationalUnemploymentRate
* StatTargetUnemploymentRate
* StatResidentialDemand
* StatUnusedStat10
* StatUnusedStat11
* StatNumBuildings
* StatBuildingsConstructed
* StatNumResidentialBuildings
* StatNumRetailBuildings
* StatNumFarmBuildings
* StatNumGovBuildings
* StatNumOfficeBuildings
* StatNumFactoryBuildings
* StatNumMixedUseBuildings
* StatUnusedStat9
* StatNumHomes
* StatNumSingleFam
* StatNumMultiFam
* StatPercentMultiFam
* StatEmptyHomes
* StatNumShops
* StatNumOffices
* StatPercentOffices
* StatEmptyShops
* StatNumFarms
* StatNumFactories
* StatPercentFactories
* StatEmptyFarms
* StatNumBusinesses
* StatBusinessesCreated
* StatBusinessesClosed
* StatNumRetailBiz
* StatNumRetailOpen
* StatNumOfficeBiz
* StatNumOfficeOpen
* StatNumAgriculturalBiz
* StatNumAgriculturalOpen
* StatNumIndustrialBiz
* StatNumIndustrialOpen
* StatPercentRetail
* StatNumGovBuildingsEnabled
* StatNumParks
* StatNumParksEnabled
* StatNumSchools
* StatNumSchoolsEnabled
* StatNumCommServices
* StatNumCommServicesEnabled
* StatNumUniBuildings
* StatNumUniBuildingsEnabled
* StatNumRoadSegments
* StatNumIntersections
* StatPavementMSq
* StatRoadSystemValue
* StatNumVehicles
* StatTotalTrips
* StatLongTrips
* StatTripsHome
* StatWorkCommutes
* StatSocialVisits
* StatStoreTrips
* StatFreightShipments
* StatSchoolCommutes
* StatParkTrips
* StatCommunityEngagements
* StatIntercityVisitors
* StatIntercityTrips
* StatRouteRequests
* StatRouteCacheHits
* StatRouteCacheMisses
* StatRouteCacheHitRate
* StatRoutesProcessed
* StatFailedRoutes
* StatVehicleRoutes
* StatPersonRoutes
* StatHomeRoutes
* StatWorkRoutes
* StatStoreRoutes
* StatFreightRoutes
* StatOtherRoutes
* StatFailedDistanceTests
* StatRoutesInvalidated
* StatRouteCacheSize
* StatPollutionStat
* StatValueStat
* StatDensityStat
* StatCrimeStat
* StatEducationStat
* StatProsperityStat
* StatAggregatePropertyValue
* StatRetailTransactions
* StatInflationRate
* StatInflationIndex
* StatNationalInterestRate
* StatCityBondRate
* StatNationalStockIndex
* StatNationalStockTrajectory
* StatEconomicWavePhase
* StatEconomicDeterminant
* StatPropertyTaxRateStat
* StatSalesTaxRateStat
* StatFinesAndFeesRateStat
* StatPropertyTaxStat
* StatSalesTaxStat
* StatFinesAndFeesStat
* StatFuelTaxIncomeStat
* StatAmenityIncomeStat
* StatTransitIncomeStat
* StatAssetSalesIncomeStat
* StatRoadBuildExpensesStat
* StatExpwyBuildExpensesStat
* StatRailBuildExpensesStat
* StatPillarBuildExpensesStat
* StatEminentDomainExpensesStat
* StatRepairExpensesStat
* StatBuildingBuildExpensesStat
* StatMiscDiscExpensesStat
* StatEducationExpensesStat
* StatRecreationExpensesStat
* StatServicesExpensesStat
* StatUniversityExpensesStat
* StatTransitExpensesStat
* StatTotalIncomeStat
* StatTotalExpensesStat
* StatTotalDiscretionaryStat
* StatTotalMandatoryStat
* StatTotalEarningsStat
* StatAssetsStat
* StatLineOfCreditStat
* StatLoanInterestStat
* StatCashflowStat
* StatBudgetBalanceStat
* StatCashAvailableStat
* StatWeatherTempStat
* StatWeatherPressureStat
* StatWeatherCloudsStat
* StatWeatherPercipitationStat
* StatWeatherSnowPackStat
* StatWeatherDroughtStat
* StatWeatherWindSpeedStat
* StatRouterQueueSize
* StatNumLaneBlocks
* StatTeleports
* StatVehicleTrips
* StatFailedTimeTests
* StatNumHired
* StatNumQuitOrFired
* StatPeopleProcessed
* StatNumGovBuildingsOpen
* StatJobInterviews
* StatInterviewRoutes
* StatNumNoEdu
* StatNumHSEdu
* StatNumBclEdu
* StatNumPhdEdu
* StatNoEduPercent
* StatHSEduPercent
* StatBclEduPercent
* StatPhdEduPercent
* StatNumWorkers
* StatNumNoEduWorkers
* StatNumHSEduWorkers
* StatNumBclEduWorkers
* StatNumPhdEduWorkers
* StatNumNoEduUnemployed
* StatNumHSEduUnemployed
* StatNumBclEduUnemployed
* StatNumPhdEduUnemployed
* StatNoEduUnemployment
* StatHSEduUnemployment
* StatBclEduUnemployment
* StatPhdEduUnemployment
* StatNumPositions
* StatNumNoEduPositions
* StatNumHSPositions
* StatNumBclPositions
* StatNumPhDPositions
* StatNumOpenNoEduPositions
* StatNumOpenHSPositions
* StatNumOpenBclPositions
* StatNumOpenPhDPositions
* StatEmptyOffices
* StatEmptyFactories
* StatEntities
* StatEntitiesDrawn
* StatNumCollegeStudents
* StatMaxCollegeStudents
* StatNumCollegeBunks
* StatNumEmptyBunks
* StatNumDorms
* StatNumColleges
* StatNumProfessors
* StatNumStudentsMovingIn
* StatNumStudentsDroppingOut
* StatNumStudentsGraduating
* StatRetailDemand
* StatFarmDemand
* StatFactoryDemand
* StatOfficeDemand
* StatRetailGrowth
* StatFarmGrowth
* StatFactoryGrowth
* StatOfficeGrowth
* StatNumAbandoned
* StatAbandonedResidentialBuildings
* StatAbandonedRetailBuildings
* StatAbandonedFarmBuildings
* StatAbandonedGovernmentBuildings
* StatAbandonedOfficeBuildings
* StatAbandonedFactoryBuildings
* StatAbandonedMixedUseBuildings
* StatRoutesDiscarded
* StatTransitRoutes
* StatFuelPrice
* StatTransitTrips
* StatResidentialZoneDemand
* StatRetailZoneDemand
* StatFarmZoneDemand
* StatGovernmentZoneDemand
* StatOfficeZoneDemand
* StatFactoryZoneDemand
* StatMixedUseZoneDemand
* StatParkZoneDemand
* StatParkLots
* StatRouteCacheErrors
* StatRoutesBrokered
* StatCPUTime
* StatRoutesProcessedPerSecond
* StatVehiclesSettling
* StatVehiclesFree
* StatNumTravelGroups
* StatTravelGroupsSettling
* StatTravelGroupsFree
* StatFPS
* StatDoctorsVisits
* StatDoctorRoutes
* StatPeopleSick
* StatNumHealthcareProviders
* StatCommunityStat
* StatHealthStat
* StatRouterThreadsActive
* StatTransitVehicleRoutes
* StatTimeSpentTraveling
* StatRecordedTrips
* StatAverageTripTime
* StatDeaths
* StatDeathYears
* StatLifeExpectancy
* StatNumTouristsNow
* StatTouristRating
* StatNumHotelRooms
* StatNumEmptyHotelRooms
* StatNumTouristsArriving
* StatNumTouristsLeaving
* StatMemories
* StatTouristTransactions
