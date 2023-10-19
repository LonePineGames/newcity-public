#include "name.hpp"

#include "string_proxy.hpp"
#include "util.hpp"

#include "platform/lookup.hpp"

#include "spdlog/spdlog.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
using namespace std;

const char* country = "us";

vector<char*> nameList[numNameTypes];

void initNames(item type, const char* filename) {
  FILE *file;
  char* fn_base = sprintf_o("locale/%s/%s.txt", country, filename);
  string fn = lookupFile(fn_base, 0);
  free(fn_base);
  int len = 255;

  if (file = fopen(fn.c_str(), "r")) {
    char* line = (char*) malloc(sizeof(char)*len);
    while(fgets(line, len, file) != NULL) {
      char* trimmedLine = trimWhiteSpace(line);
      if (trimmedLine[0] == 0) {
        continue;
      }
      nameList[type].push_back(strdup_s(trimmedLine));
    }
    free(line); // last one isn't used
    fclose(file);
  }
}

void initNames() {
  resetNames();
  initNames(RoadName, "road");
  initNames(FamilyName, "family");
  initNames(GivenMaleName, "given_male");
  initNames(GivenFemaleName, "given_female");
  initNames(RetailName, "retail");
  initNames(OfficeName, "office");
  initNames(IndustrialName, "industrial");
  initNames(GovRetailName, "government_retail");
  initNames(FarmName, "farm");
  initNames(CityName, "city");
  initNames(AmenityName, "amenity");
  initNames(BuildingName, "building");
  initNames(ContributorName, "contributor");
  initNames(EngineeringFirmName, "engineering_firm");
  initNames(NewspaperName, "newspaper");
  initNames(SportsTeamName, "teams");
  initNames(HotelName, "hotel");
}

void resetNames() {
  for (int i=0; i < numNameTypes; i++) {
    for (int j=0; j < nameList[i].size(); j++) {
      free(nameList[i][j]);
    }
    nameList[i].clear();
  }
}

char* randomName(item type) {
  if (type == RoadName && randFloat() < c(CRoadContributorNameChance)) {
    return randomName(AmenityName);
  }
  return strdup_s(nameList[type][randItem(nameList[type].size())]);
}

