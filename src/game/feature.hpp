#pragma once

#include "../item.hpp"
#include <string>

enum Feature {
  FBudget, FLoanTerm, FSellBuilding, FShutDownBuilding,
  FPropertyTax, FSalesTax, FFinesAndFees,
  FSpeedControl, FPopulation, FUnemployment, FWeather,
  FEconomyPanel, FNoteChart, FNoteBudget, FNoteObject,
  FQueryTool, FRoadTool, FZoneTool, FBuildingTool, FBulldozerTool,
  FObservePollution, FObserveValue, FObserveDensity, FObserveCrime,
  FObserveEducation, FObserveProsperity,
  FRoadStreet, FRoadAve, FRoadBlvd, FRoadOneWay2, FRoadOneWay4,
  FRoadExpressways, FRoadRepair, FRoadPillars, FRoadElevation,
  FRoadCut, FRoadPlanner,
  FRoadSuspensionBridge, FUnused2, FUnused3,
  FNewGame, FPlay, FBuildingDesigner, FTestMode,
  FBlueprint, FMods,
  FZoneResidential, FZoneRetail, FZoneFarm, FZoneGovernment,
  FZoneOffice, FZoneFactory, FZoneMixedUse,
  FZonePark, FUnused5, FUnused6, FUnused7,
  FBus, FRail, FPedestrian, FTram,
  FObserveHealth, FObserveCommunity,
  FObserveTraffic, FObserveTransit, FObserveZones,
  FObserveUnderground,
  FLabel, FRouteInspector, FHeatmaps,
  FTransitSystems, FGrid,
  FObserveRoadMap, FNewspaper,
  FPlaceTrees, FEarthworks,
  FFuelTax,
  numFeatures
};

item getFeatureCodeByStr(std::string code);
const char* getFeatureCode(item ndx);
const char* getFeatureDescriptor(item ndx);
bool isFeatureEnabled(item ndx);
bool blinkFeature(item ndx);
void stopBlinkingFeature(item ndx);
void stopBlinkingFeatures();
void setFeatureBlink(item ndx, bool val);
void setFeatureEnabled(item ndx, bool val);
Feature featureForHeatmap(item ndx);
void resetFeatures();

