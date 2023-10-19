NewCity LUA API - Features

### Feature-Related Functions

##### featureEnabled(number _feature_) => boolean

Returns true if _feature_ is enabled.

> featureEnabled(FPlay) => $featureEnabled(FPlay)$
> featureEnabled(FZonePark) => $featureEnabled(FZonePark)$
> featureEnabled(FRail) => $featureEnabled(FRail)$

---

##### setFeatureEnabled(number _feature_, boolean _enabled_)

Enables _feature_ if _enabled_ is true, or disables _feature_ if _enabled_ is false.

> setFeatureEnabled(FZoneMixedUse, true) => Enables the Mixed Use zone.

---

### List of Features

  FBudget, FLoanTerm, FSellBuilding, FShutDownBuilding,
  FPropertyTax, FSalesTax, FFinesAndFees,
  FSpeedControl, FPopulation, FUnemployment, FWeather,
  FEconomyPanel, FNoteChart, FNoteBudget, FNoteObject,
  FQueryTool, FRoadTool, FZoneTool, FBuildingTool, FBulldozerTool,
  FObservePollution, FObserveValue, FObserveDensity, FObserveCrime,
  FObserveEducation, FObserveProsperity,
  FRoadStreet, FRoadAve, FRoadBlvd, FRoadOneWay2, FRoadOneWay4,
  FRoadExpressways, FRoadRepair, FRoadPillars, FRoadElevation,
  FRoadCut, FRoadPlanner, FRoadSuspensionBridge,
  FNewGame, FPlay, FBuildingDesigner, FTestMode,
  FBlueprint, FMods, FZoneResidential, FZoneRetail,
  FZoneFarm, FZoneGovernment, FZoneOffice, FZoneFactory,
  FZoneMixedUse, FZonePark,
  FBus, FRail, FPedestrian, FTram,
  FObserveHealth, FObserveCommunity,
  FObserveTraffic, FObserveTransit, FObserveZones,
  FObserveUnderground, FLabel, FRouteInspector, FHeatmaps,
  FTransitSystems, FGrid,
  FObserveRoadMap, FNewspaper, FPlaceTrees, FEarthworks

