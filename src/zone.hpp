#ifndef ZONE_H
#define ZONE_H

#include "item.hpp"
#include "./parts/tooltip.hpp"

enum ZoneTypes {
  NoZone = 0, ResidentialZone, RetailZone, FarmZone, GovernmentZone,
  OfficeZone, FactoryZone, MixedUseZone, ParkZone,
  numZoneTypes,
};

const ZoneTypes zoneOrder[] = {
  NoZone, ParkZone, ResidentialZone, MixedUseZone, RetailZone,
  OfficeZone, FarmZone, FactoryZone, GovernmentZone,
};

const char* const zoneName[] = {
  "Not Zoned",
  "Residential",
  "Retail",
  "Agricultural",
  "Government",
  "Office",
  "Industrial",
  "Mixed Use",
  "Park",
};

const char* const zoneCode[] = {
  "NoZone",
  "ResidentialZone",
  "RetailZone",
  "AgriculturalZone",
  "GovernmentZone",
  "OfficeZone",
  "IndustrialZone",
  "MixedUseZone",
  "ParkZone",
};

bool getOverzoneMode();

#endif
