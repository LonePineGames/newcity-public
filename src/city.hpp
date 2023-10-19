#ifndef CITY_H
#define CITY_H

#include "main.hpp"
#include "item.hpp"
#include "serialize.hpp"

const int _cityExists = 1 << 0;
const int _cityMajor = 1 << 1;

struct City {
  int flags;
  char* name;
  item node;
  item laneBlock;
  item econ;
  vec3 nodeCenter;
  vec3 visualCenter;
  vec3 normal;
  vector<item> buildings;
  item entity;
};

City* getCity(item ndx);

void resetCities();
void initCitiesEntities();
void renderCities();
void generateCities();
void updateCities(double duration);
item intersectCity(Line mouseLine);
item nearestCity(vec3 loc);
void setCitiesVisible(bool viz);
item countNeighbors();

void readCities(FileBuffer* file, int version);
void writeCities(FileBuffer* file);

#endif
