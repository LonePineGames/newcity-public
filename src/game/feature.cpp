#include "feature.hpp"

#include "game.hpp"
#include "../heatmap.hpp"
#include "../option.hpp"
#include "../string_proxy.hpp"

unsigned char featureEnabled[numFeatures/8+1];
unsigned char featureBlink[numFeatures/8+1];

void resetFeatures() {
  for (int i = 0; i < numFeatures/8+1; i++) {
    featureEnabled[i] = 0;
  }
}

const char* featureCodes[] = {
  "FBudget", "FLoanTerm", "FSellBuilding", "FShutDownBuilding",
  "FPropertyTax", "FSalesTax", "FFinesAndFees",
  "FSpeedControl", "FPopulation", "FUnemployment", "FWeather",
  "FEconomyPanel", "FNoteChart", "FNoteBudget", "FNoteObject",
  "FQueryTool", "FRoadTool", "FZoneTool", "FBuildingTool", "FBulldozerTool",
  "FObservePollution", "FObserveValue", "FObserveDensity", "FObserveCrime",
  "FObserveEducation", "FObserveProsperity",
  "FRoadStreet", "FRoadAve", "FRoadBlvd", "FRoadOneWay2", "FRoadOneWay4",
  "FRoadExpressways", "FRoadRepair", "FRoadPillars", "FRoadElevation",
  "FRoadCut", "FRoadPlanner", "FRoadSuspensionBridge",
  "FUnused2", "FUnused3",
  "FNewGame", "FPlay", "FBuildingDesigner", "FTestMode",
  "FBlueprint", "FMods",
  "FZoneResidential", "FZoneRetail", "FZoneFarm", "FZoneGovernment",
  "FZoneOffice", "FZoneFactory", "FZoneMixedUse",
  "FZonePark", "FUnused5", "FUnused6", "FUnused7",
  "FBus", "FRail", "FPedestrian", "FTram",
  "FObserveHealth", "FObserveCommunity",
  "FObserveTraffic", "FObserveTransit", "FObserveZones", "FObserveUnderground",
  "FLabel", "FRouteInspector", "FHeatmaps",
  "FTransitSystems", "FGrid", "FObserveRoadMap",
  "FNewspaper",
  "FPlaceTrees", "FEarthworks",
  "FFuelTax"
};

const char* featureDescriptor[] = {
  "Budget Available",
  "New Budget Tool: Set Loan Term",
  "New Budget Tool: Sell Amenity",
  "New Budget Tool: Shut Down Amenity",
  "New Budget Tool: Property Tax",
  "New Budget Tool: Sales Tax",
  "New Budget Tool: Fines And Fees",
  "Speed Control Available",
  "Census Started",
  "Unemployment Census Started",
  "Weather Station Started",
  "Economic Charts Available",
  "New Feature: Track Charts",
  "New Feature: Track Budget Items",
  "New Feature: Track People and Buildings",

  "Query Tool Available",
  "Transportation Tool Available",
  "Zone Tool Available",
  "Amenity Tool Available",
  "Bulldozer Tool Available",
  "Observe Pollution in Query Tool",
  "Observe Value in Query Tool",
  "Observe Density in Query Tool",
  "Observe Crime in Query Tool",
  "Observe Education in Query Tool",
  "Observe Prosperity in Query Tool",
  "",

  "New Road: Avenues",
  "New Road: Boulevards",
  "New Road: One Way (2-lane)",
  "New Road: One Way (4-lane)",
  "Expressways Available",
  "Road Repair Available",
  "Bridges Available",
  "Road Elevation Available",
  "New Feature: Cut Road Segments in Half",
  "New Feature: Plan Roads Before Building Them",
  "Suspension Bridges Available",
  "Commercial Zones Available",
  "Industrial Zones Available",
  "New Game Available (in case you made a mistake)",
  "",

  "Building Designer Available (See Main Menu)",
  "Test Mode Available (See Main Menu)",
  "Blueprints Available",
  "Modpacks Available",
  "Residential Zones Available",
  "Retail Zones Available",
  "Farm Zones Available",
  "Government Zones Available (but they shouldn't be)",
  "Office Zones Available",
  "Factory Zones Available",
  "Mixed Use Zones Available",
  "Park Lots Available in Zone Tool",
  "", "", "",
  "Buses Available",
  "Rail Available",
  "Pedestrian Paths Available",
  "Trams Available",

  "Observe Health in Query Tool",
  "Observe Community in Query Tool",
  "Observe Traffic in Query Tool",
  "Observe Transit Map",
  "Observe Zones in Query Tool",
  "Observe Underground Structures",
  "Labels Available in Query Tool",
  "Route Inspector Available in Query Tool",
  "Heatmaps Available",
  "Transit Systems Available",
  "Gridless Road Placement in Transportation Tool",
  "Observe Road Map",
  "Newspaper Available",
  "Place Trees with the Bulldozer Tool",
  "Raise and Lower Terrain with the Bulldozer Tool",
  "New Budget Tool: Fuel Tax",
};

item getFeatureCodeByStr(std::string code) {
  // Checks against a parallel array of const char* elements, take note! 
  // May throw OOB errors if the Feature enum is updated but not the 
  // const char* parallel array. Also does not normalize capitalization
  for(int i = 0; i < Feature::numFeatures; i++) {
    if(streql(code.c_str(), featureCodes[i])) return i;
  }

  return -1;
}

const char* getFeatureCode(item ndx) {
  return featureCodes[ndx];
}

const char* getFeatureDescriptor(item ndx) {
  return featureDescriptor[ndx];
}

bool isFeatureEnabled(item ndx) {
  return true;
  /*
  if (!tutorialMode()
        //&& ndx != FBuildingTool
        && ndx != FRoadBlvd
        && ndx != FRoadOneWay2
        && ndx != FRoadOneWay4
        && ndx != FRoadExpressways
        && ndx != FRoadSuspensionBridge
        && ndx != FRoadRepair
        && ndx != FBus
        && ndx != FRail
        && ndx != FTransitSystems
        && ndx != FPedestrian
        && ndx != FZoneOffice
        && ndx != FZoneMixedUse
        && ndx != FZonePark
        && ndx != FEarthworks
    ) {
    return true;
  }
  return featureEnabled[ndx/8] & 1 << ndx%8;
  */
}

void setFeatureEnabled(item ndx, bool val) {
  if (val) {
    featureEnabled[ndx/8] |= 1 << ndx%8;
    setFeatureEnabledGlobal(ndx);
  } else {
    featureEnabled[ndx/8] &= ~(1 << ndx%8);
  }
}

bool isFeatureBlink(item ndx) {
  return featureBlink[ndx/8] & 1 << ndx%8;
}

bool blinkFeature(item ndx) {
  return isFeatureBlink(ndx);
  //return isFeatureEnabled(ndx) && !wasFeatureUsed(ndx) && isFeatureBlink(ndx);
}

void setFeatureBlink(item ndx, bool val) {
  if (val) {
    featureBlink[ndx/8] |= 1 << ndx%8;
  } else {
    featureBlink[ndx/8] &= ~(1 << ndx%8);
  }
}

void stopBlinkingFeature(item ndx) {
  setFeatureBlink(ndx, false);
  featureUsed(ndx);
}

void stopBlinkingFeatures() {
  for(int ndx = 0; ndx < Feature::numFeatures; ndx++) {
    setFeatureBlink(ndx, false);
  }
}

Feature featureForHeatmap(item ndx) {
  if (ndx == CommunityHM) {
    return FObserveCommunity;
  } else if (ndx == HealthHM) {
    return FObserveHealth;
  } else if (ndx == TrafficHeatMap) {
    return FObserveTraffic;
  } else if (ndx == TransitHeatMap) {
    return FObserveTransit;
  } else if (ndx == ZoneHeatMap) {
    return FObserveZones;
  } else if (ndx == RoadHeatMap) {
    return FObserveRoadMap;
  } else {
    return (Feature)(FObservePollution+ndx);
  }
}

