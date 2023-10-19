#pragma once

#include "../cup.hpp"
#include "../serialize.hpp"
#include "../route/route.hpp"

const int transitFrequencyDivisions = 2;

struct TransitLeg {
  item stop;
  float timeEstimate;
  float timeRecord;
  float waitRecord;
};

struct TransitLine {
  uint32_t flags;
  uint8_t system;
  uint8_t maxCars;
  uint8_t color;
  Cup<TransitLeg> legs;
  Cup<item> vehicles;
  float headway[transitFrequencyDivisions];
  float lastVehicleTime;
  item passengersNow;
  item passengersEver;
  item vehiclesEver;
  item cursor;
  char* name;
};

const int _transitExists = 1 << 0;
const int _transitEnabled = 1 << 1;
const int _transitComplete = 1 << 2;
const int _transitDefault = 1 << 3;
const int _transitBidding = 1 << 4;
const int _transitDesigning = 1 << 5;
const int _transitDesigned = 1 << 6;

struct TransitSystemBid {
  char* firmName;
};

struct TransitSystem {
  uint32_t flags;
  uint8_t type;
  uint8_t power;
  uint8_t automation;
  uint8_t lineStyle;
  uint8_t logo;
  uint8_t configType;
  uint8_t color[2];
  float ticketPrice;
  float transferPrice;
  float designDate;
  uint8_t maxCars;
  int numLines;
  vector<item> features;
  vector<TransitSystemBid> bids;
  char* name;
};

item getTransitCursor();
void setTransitCursor(item val);

void setCurrentLine(item l);
item getCurrentLine();
item addTransitLine(item system);
TransitLine* getTransitLine(item ndx);
item getTransitLineColor(item ndx);
vec3 getTransitLineColorInPalette(item ndx);
vector<item> suggestStationColors(item ndx);

void setCurrentTransitSystem(item s);
item getCurrentTransitSystem();
item addTransitSystem();
void callForTransitBids(item systemNdx);
void selectTransitBid(item systemNdx, item bidNdx);
void acceptTransitDesign(item systemNdx);
TransitSystem* getTransitSystem(item ndx);
void removeTransitSystem(item ndx);
item getSystemGraphType(item systemNdx);
void addTransitSystemFeature(item sysNdx, item featureNdx);
void setTransitLineColor(item lineNdx, int val);
void setTransitSystemColor(item sysNdx, int color, int val);

item insertStopIntoLine(item l, item s);
void removeStopFromLine(item l, item s);
void removeStopFromLineByIndex(item l, item s);

void removeVehicleFromTransitLine_g(item vNdx);
void removeTransitLine(item ndx);
void setTransitVisible(bool visible);
bool isTransitVisible();
void renderTransitLeg(item mesh, item lineNdx, item leg, vec3 offset);
item sizeTransitLines();
item sizeTransitSystems();
void finishTransit();
void updateTransit(double duration);
void updateTransitRoute_g(item ndx, Route* route);
void renderTransitNumbers();
void paintTransit();
void resetTransit();
void initTransit();
void setupTransitDesigner();
void readTransit_g(FileBuffer* file);
void writeTransit_g(FileBuffer* file);

