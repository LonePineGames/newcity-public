#ifndef LOT_H
#define LOT_H

#include "box.hpp"
#include "item.hpp"
#include "line.hpp"
#include "serialize.hpp"
#include "util.hpp"

const int _lotExists = 1 << 0;
const int _lotMaxDensityShift = 12;
const int _lotMaxDensityMask = 15 << _lotMaxDensityShift;

const int lotMinDensity = 0;
const int lotMaxDensity = 10;
const float lotGap = tileSize*0.2f;

struct Lot {
  int flags;
  item elem;
  char zone;
  item occupant;
  item entity;
  vec3 loc;
  vec3 normal;
};

void addLots(item elem, Line line);
Lot* getLot(item lot);
void removeLot(item lot);
bool isLotVisible(item ndx);
void unoccupyLot(item lotNdx);
Box getLotBox(item ndx);
bool getLotSide(item lotNdx);
void zoneLot(item lotNdx, item zone, bool overzone);

item randomLot();
void finishLots();
void updateLots(double duration);
item getEmptyLot(item zone);
item numEmptyLots(item zone);
item nearestLot(Line mouseLine);
item nearestLot(Line ml, bool includeInvisible);
vector<item> getLotsByElem(item elem, bool side);
vector<item> getLotsByRad(vec3 searchOrigin, float radius);
int getLotMaxDensity(item lotNdx);
void setLotMaxDensity(item lotNdx, int max);
void zoneLots(vector<item> toZone, item zoneType, int densityValue);
void changeDensityLots(vector<item> toChange, int val);
void makeLots(Line line, float dist, item element);
void makeNodeLots(vec3 loc, vec3 normal, item element);
void occupyLots(vector<item> lots, item building);
vector<item> collidingLots(Box box);
void removeLots(Box box);
void orphanLots(item elem);
item numLots();
item getNumZonedLots();
void resetLots();
void initLotsEntities();
void renderLots();
bool areLotsVisible();
void setLotsVisible();
bool areParkLotsVisible();
bool isLotDensityVisible();
void writeLots(FileBuffer* file);
void readLots(FileBuffer* file, int version);

#endif
