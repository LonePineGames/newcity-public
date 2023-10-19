#pragma once

#include "../lane.hpp"
#include "../main.hpp"
#include "../route/route.hpp"
#include "../serialize.hpp"

const int numVehicleColors = 32;

struct VehicleDescription {
  item style;
};

struct Vehicle {
  // Physics and Render need
  int flags;
  vec3 location;
  vec3 velocity;
  float yaw;
  float pitch;
  item trailer;
  item trailing;
  item yieldTo;
  item yieldFrom;

  // Physics needs
  vec2 acceleration;
  float distanceSinceMerge;
  float aggressiveness;
  float creationTime;
  float laneBlockEnterTime;
  item vehicleAhead[2];
  GraphLocation pilot;
  GraphLocation laneLoc;
  GraphLocation destination;
  Route route;

  // Render needs
  item entity;
  item lightEntity;
  item numberEntity;
  item numberShadowEntity;
  item lastNumberRendered;

  // Bookkeeping
  vector<item> travelGroups;
  item numPassengers;
  item transitLine;
  item model;
};

const int _vehicleExists = 1 << 0;
const int _vehicleHeadlights = 1 << 1;
const int _vehicleIsMerging = 1 << 2;
const int _vehicleIsBraking = 1 << 3;
const int _vehiclePlaced = 1 << 4;
const int _vehicleHasRoute = 1 << 5;
const int _vehicleIsTransit = 1 << 6;
const int _vehicleComplete = 1 << 7;
const int _vehicleSettling = 1 << 8;
const int _vehicleWanderer = 1 << 9;
const int _vehicleIsFrieght = 1 << 10;
const int _vehicleColorShift = 16;
const int _vehicleColorMask = 255 << _vehicleColorShift;
const int _vehicleSwapMask =
  _vehicleIsMerging | _vehiclePlaced | _vehicleHasRoute | _vehicleComplete;

VehicleDescription createVehicleDescription();
void writeVehicleDescription(FileBuffer* file, VehicleDescription desc);
VehicleDescription readVehicleDescription(FileBuffer* file, int version);

item addVehicle_g(item style, item travelGroup, Route* route);
item addTestVehicle_g(GraphLocation start, GraphLocation dest);
item addFreightVehicle_g(item travelGroup, Route* route);
item addTransitVehicle_g(item transitLine, Route* route);
item addWanderer_g(item modelNdx, vec3 loc, float yaw);
Vehicle* getVehicle(item ndx);
void removeVehicle(item ndx);
void freeVehicle(item ndx);
item vehicleColor(Vehicle* vehicle);
void selectVehicleAndPause(item ndx);
void repairVehicleDestinationLane(item ndx);
void rerouteVehicle(item ndx);
void invalidateVehicleRoute(item ndx);
void validateVehicles(char* msg);
bool isVehicleActive_g(item ndx);
void setVehicleActive_g(item ndx);
vec3 getVehicleCenter_g(item ndx);
float getEffectiveTrafficRate();
float getTransitMoneyMultiplier();

item getRandomVehicle();
void initVehicles();
void writeVehicles(FileBuffer* file);
void readVehicles(FileBuffer* file, int version);
void vehiclePassengersToTravelGroups51_g();
void resetVehicles();
item nearestVehicle(Line ml);
float getVehicleRate();
item countVehicles();
item sizeVehicles();
void initVehiclesEntities();
void renderVehicles();
void placeVehicles();
void placeVehicle(item ndx);
void settleVehicles();
void removeAllVehicles_g();

