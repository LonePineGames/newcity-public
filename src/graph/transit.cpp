#include "transit.hpp"

#include "config.hpp"
#include "stop.hpp"
#include "transitRouter.hpp"

#include "../configuration.hpp"
#include "../color.hpp"
#include "../draw/entity.hpp"
#include "../draw/mesh.hpp"
#include "../draw/shader.hpp"
#include "../draw/texture.hpp"
#include "../game/feature.hpp"
#include "../game/game.hpp"
#include "../graph.hpp"
#include "../economy.hpp"
#include "../heatmap.hpp"
#include "../icons.hpp"
#include "../lane.hpp"
#include "../name.hpp"
#include "../parts/leftPanel.hpp"
#include "../pool.hpp"
#include "../renderGraph.hpp"
#include "../renderUtils.hpp"
#include "../selection.hpp"
#include "../string.hpp"
#include "../route/routeRequest.hpp"
#include "../time.hpp"
#include "../tools/road.hpp"
#include "../vehicle/transitPhysics.hpp"
#include "../vehicle/vehicle.hpp"
#include "../util.hpp"

#include "../parts/messageBoard.hpp"

#include <string.h>

Pool<TransitLine>* transitLines = Pool<TransitLine>::newPool(10);
Pool<TransitSystem>* transitSystems = Pool<TransitSystem>::newPool(10);

typedef set<item> itemset;
typedef itemset::iterator itemsetIter;

static itemset toRender;
static item stopUpdateNdx;
static bool transitVisible = false;
static item currentLine = 0;
static item currentSystem = 0;
static Cup<Cup<item>> tlEntities;
static Cup<item> stopNumEntities;
static Cup<item> stopNumLast;
static Cup<Route> transitRoutes_g;

void resetTransit() {
  resetStops();
  transitLines->clear();
  transitSystems->clear();
  currentLine = 0;
  currentSystem = 0;
  stopUpdateNdx = 0;

  for (int i = 0; i < tlEntities.size(); i++) {
    tlEntities.get(i)->clear();
  }
  tlEntities.clear();
  stopNumEntities.clear();
  stopNumLast.clear();
  toRender.clear();
  transitRoutes_g.clear();
}

void initTransit() {
  bool hasBus = false;
  bool hasNewTrack = false;

  for (int i = 1; i <= transitSystems->size(); i++) {
    TransitSystem* sys = getTransitSystem(i);
    if (!(sys->flags & _transitDefault)) continue;

    if (streql(sys->name, "Bus")) {
      hasBus = true;
    }

    if (streql(sys->name, "NewTrack")) {
      hasNewTrack = true;
    }
  }

  if (isFeatureEnabled(FBus) && !hasBus) {
    item busNdx = addTransitSystem();
    TransitSystem* bus = getTransitSystem(busNdx);
    if (bus->name != 0) free(bus->name);
    bus->name = strdup_s("Bus");
    bus->logo = 0;
    bus->color[0] = 22;
    bus->color[1] = 21;
    bus->flags |= _transitDefault | _transitComplete;
    bus->type = getGraphFeature("GrTracRubber");
    bus->power = getGraphFeature("GrPowerDiesel");
    bus->automation = getGraphFeature("GrAutoManual");
    addTransitSystemFeature(busNdx, bus->type);
    addTransitSystemFeature(busNdx, bus->power);
    addTransitSystemFeature(busNdx, bus->automation);
    bus->designDate = 0;
    bus->ticketPrice = getInflationMultiplier() * c(CDefaultTicketPrice);
    bus->transferPrice = getInflationMultiplier() * c(CDefaultTransferPrice);
  }

  if (isFeatureEnabled(FRail) && !hasNewTrack) {
    item trainNdx = addTransitSystem();
    TransitSystem* train = getTransitSystem(trainNdx);
    if (train->name != 0) free(train->name);
    train->name = strdup_s("NewTrack");
    train->logo = iconNewTrackNdx;
    train->color[0] = 7;
    train->color[1] = 23;
    train->flags |= _transitDefault | _transitComplete;
    train->type = getGraphFeature("GrTracMetal");
    train->power = getGraphFeature("GrPowerDiesel");
    train->automation = getGraphFeature("GrAutoManual");
    addTransitSystemFeature(trainNdx, train->type);
    addTransitSystemFeature(trainNdx, train->power);
    addTransitSystemFeature(trainNdx, train->automation);
    train->designDate = 0;
    train->ticketPrice = getInflationMultiplier()*c(CNewTrackTicketPrice);
    train->transferPrice = getInflationMultiplier()*c(CNewTrackTransferPrice);
  }
}

bool desaturateLine(item line) {
  if (line == 0) return true;
  if (getLeftPanel() != TransitPanel) return false;
  if (currentLine != 0 && line != currentLine) return true;
  if (getSelection() != 0 && getSelectionType() != SelectionStop &&
      !isTransitTool()) return true;
  TransitLine* l = getTransitLine(line);
  if (!(l->flags & _transitEnabled)) return true;
  if (currentSystem != 0 && l->system != currentSystem) return true;
  return false;
}

item getTransitLineColor(item lineNdx) {
  item color = 0;

  if (lineNdx > 0 && lineNdx <= transitLines->size()) {
    TransitLine* line = getTransitLine(lineNdx);
    if (line->color == 255) {
      TransitSystem* system = getTransitSystem(line->system);
      color = system->color[0];
    } else {
      color = line->color;
    }
  }

  return color;
}

vec3 getTransitLineColorInPalette(item lineNdx) {
  return getColorInPalette(getTransitLineColor(lineNdx));
}

void paintTransit() {
  bool viz = transitVisible &&
    (getSelection() == 0 || getSelectionType() == SelectionStop);
  for (int i = 0; i < tlEntities.size(); i++) {
    Cup<item>* ents = tlEntities.get(i);
    for (int j = 0; j < ents->size(); j++) {
      item eNdx = ents->at(j);
      if (eNdx != 0) {
        setEntityVisible(eNdx, viz);
        bool desat = desaturateLine(i);
        setEntityDesaturate(eNdx, desat);
        setEntityRaise(eNdx, desat ? 0 : (i%7+1));
      }
    }
  }

  /*
  for (int i = 1; i <= transitLines->size(); i++) {
    TransitLine* line = getTransitLine(i);
    line->vehicles.removeByValue(0);
    for (int j = 0; j < line->vehicles.size(); j++) {
      renderVehicleNumber(line->vehicles[j]);
    }
  }
  */

  if (transitVisible) {
    renderTransitNumbers();

  } else {
    for (int i = 0; i < stopNumEntities.size(); i++) {
      item eNdx = stopNumEntities[i];
      if (eNdx == 0) continue;
      setEntityVisible(eNdx, false);
    }

    /*
    for (int i = 0; i < vehicleNumEntities.size(); i++) {
      item eNdx = vehicleNumEntities[i];
      if (eNdx == 0) continue;
      item sNdx = vehicleNumShadow[i];
      setEntityVisible(sNdx, false);
      setEntityVisible(eNdx, false);
    }
    */
  }
}

item getCurrentTransitSystem() {
  return currentSystem;
}

void setCurrentTransitSystem(item s) {
  currentSystem = s;
  if (currentLine != 0) {
    TransitLine* line = getTransitLine(currentLine);
    if (line->system != s) {
      currentLine = 0;
    }
  }
  paintTransit();
}

item getCurrentLine() {
  return currentLine;
}

void setCurrentLine(item l) {
  currentLine = l;
  if (l != 0) {
    currentSystem = getTransitLine(l)->system;
  }
  paintTransit();
}

item sizeTransitSystems() {
  return transitSystems->size();
}

item sizeTransitLines() {
  return transitLines->size();
}

TransitSystem* getTransitSystem(item ndx) {
  return transitSystems->get(ndx);
}

TransitLine* getTransitLine(item ndx) {
  return transitLines->get(ndx);
}

void setTransitVisible(bool visible) {
  transitVisible = visible;
  paintTransit();
}

bool isTransitVisible() {
  return transitVisible;
}

item getTransitCursor() {
  if (getCurrentLine() == 0) return -1;
  TransitLine* tl = getTransitLine(getCurrentLine());
  return tl->cursor;
}

void setTransitCursor(item val) {
  if (getCurrentLine() == 0) return;
  TransitLine* tl = getTransitLine(getCurrentLine());
  tl->cursor = val;
}

item addTransitSystem() {
  item ndx = transitSystems->create();
  TransitSystem* system = getTransitSystem(ndx);
  system->flags = _transitExists | _transitEnabled;
  system->type = 0; //TSTNull;
  system->power = 0; //TSPNull;
  system->automation = 0; //TSANull;
  system->configType = ConfigTypeHeavyRail;
  system->lineStyle = 0;
  system->logo = 0;
  system->color[0] = 0;
  system->color[1] = 0;
  system->numLines = 0;
  system->maxCars = c(CMaxCarsPerVehicle);
  system->ticketPrice = c(CDefaultTicketPrice);
  system->transferPrice = c(CDefaultTransferPrice);
  system->designDate = getCurrentDateTime();
  system->name = strdup_s("New Transit System");
  system->features.clear();
  return ndx;
}

void addTransitSystemFeature(item sysNdx, item featureNdx) {
  if (featureNdx == 0) return;

  TransitSystem* system = getTransitSystem(sysNdx);
  system->features.push_back(featureNdx);
  GraphFeature* feature = getGraphFeature(featureNdx);

  if (feature->code != 0) {
    if (strstr(feature->code, "Rubber")) {
      system->configType = ConfigTypeRoad;
    } else if (strstr(feature->code, "Metal")) {
      system->configType = ConfigTypeHeavyRail;
    } else if (strstr(feature->code, "Monorail")) {
      system->configType = ConfigTypeMonorail;
    } else if (strstr(feature->code, "MonoHang")) {
      system->configType = ConfigTypeMonoHang;
    } else if (strstr(feature->code, "MagLev")) {
      system->configType = ConfigTypeMagLev;
    }
  }

  if (feature->maxCars > 0  && feature->maxCars < system->maxCars) {
    system->maxCars = feature->maxCars;
  }
}

void callForTransitBids(item systemNdx) {
  TransitSystem* system = getTransitSystem(systemNdx);
  system->flags |= _transitBidding;
}

void selectTransitBid(item systemNdx, item bidNdx) {
  TransitSystem* system = getTransitSystem(systemNdx);
  system->flags |= _transitDesigning;
  system->designDate = round(getCurrentDateTime() + c(CTransitDesignTime)) +
    9.f/24.f; // 9AM

  TransitSystemBid bid = system->bids[bidNdx];

  for (int i = 0; i < system->bids.size(); i++) {
    if (i == bidNdx) continue;
    free(system->bids[i].firmName);
  }

  system->bids.clear();
  system->bids.push_back(bid);
}

void acceptTransitDesign(item systemNdx) {
  TransitSystem* system = getTransitSystem(systemNdx);
  system->flags |= _transitComplete;
  removeMessageByObject(TransitDesignMessage, systemNdx);
}

void removeTransitSystem(item ndx) {
  TransitSystem* system = getTransitSystem(ndx);

  for (int i = 1; i <= transitLines->size(); i++) {
    TransitLine* line = getTransitLine(i);
    if (!(line->flags & _transitExists)) continue;
    if (line->system != ndx) continue;
    removeTransitLine(i);
  }

  for (int i = 0; i < system->bids.size(); i++) {
    free(system->bids[i].firmName);
  }

  if (ndx == currentSystem) currentSystem = 0;
  removeMessageByObject(TransitDesignMessage, ndx);
  free(system->name);
  system->name = 0;
  system->flags = 0;
  system->features.clear();
  system->bids.clear();
  transitSystems->free(ndx);
}

item addTransitLine(item systemNdx) {
  item ndx = transitLines->create();
  TransitLine* tl = getTransitLine(ndx);
  TransitSystem* system = getTransitSystem(systemNdx);
  system->numLines ++;
  tl->flags = _transitExists | _transitEnabled;
  tl->system = systemNdx;
  tl->legs.clear();
  tl->vehicles.clear();
  tl->lastVehicleTime = getCurrentDateTime();
  tl->passengersNow = 0;
  tl->passengersEver = 0;
  tl->vehiclesEver = 0;
  tl->name = sprintf_o("%s #%d", system->name, system->numLines);
  tl->cursor = 0;
  tl->maxCars = system->maxCars;
  tl->color = 255;

  for (int i = 0; i < transitFrequencyDivisions; i++) {
    tl->headway[i] = c(CDefaultTransitHeadway)*oneHour/60;
  }

  return ndx;
}

void removeTransitLine(item ndx) {
  TransitLine* tl = getTransitLine(ndx);

  for (int i = 0; i < tl->legs.size(); i++) {
    item stopNdx = tl->legs[i].stop;
    Stop* s = getStop(stopNdx);
    removeFromVector(&s->lines, ndx);
    if (s->lines.size() == 0) {
      removeStop(stopNdx);
    }
  }
  tl->legs.clear();

  for (int i = tl->vehicles.size()-1; i >= 0; i--) {
    removeVehicle(tl->vehicles[i]);
  }
  tl->vehicles.clear();

  if (tlEntities.size() > ndx) {
    Cup<item>* entities = tlEntities.get(ndx);
    for (int i = 0; i < entities->size(); i++) {
      removeEntityAndMesh(entities->at(i));
    }
    entities->clear();
  }

  if (ndx == currentLine) currentLine = 0;
  free(tl->name);
  tl->name = 0;
  tl->flags = 0;
  transitLines->free(ndx);
}

void removeVehicleFromTransitLine_g(item vNdx) {
  Vehicle* v = getVehicle(vNdx);
  TransitLine* tl = getTransitLine(v->transitLine);
  tl->vehicles.removeByValue(vNdx);
}

vector<Location> getSubrouteInner(item l, item s) {
  TransitLine* tl = getTransitLine(l);
  item s0Ndx = tl->legs[s].stop;
  item s1Ndx = tl->legs[s+1].stop;
  Location s0Loc = transitStopLocation(s0Ndx);
  Location s1Loc = transitStopLocation(s1Ndx);
  vector<Location> result;
  Route tRoute = transitRoutes_g[l];
  int stepS = tRoute.steps.size();
  item start = 0;
  item end = 0;

  for (; start < stepS && tRoute.steps[start] != s0Loc; start ++);
  end = start;
  for (; end < stepS && tRoute.steps[end] != s1Loc; end ++);

  if (start < stepS && end < stepS &&
      tRoute.steps[start] == s0Loc && tRoute.steps[end] == s1Loc) {
    item size = end - start;
    result.resize(size);
    for (int i = 0; i < size; i++) {
      result[i] = tRoute.steps[start+i];
    }
  }

  return result;
}

vector<Location> getSubroute(item l, item s) {
  TransitLine* tl = getTransitLine(l);
  if (s >= tl->legs.size() - 1) {
    return vector<Location>();

  } else if (transitRoutes_g.size() <= l) {
    routeTransitLineInstant_g(l);
    if (transitRoutes_g.size() <= l) {
      return vector<Location>();
    } else {
      return getSubrouteInner(l, s);
    }

  } else {
    vector<Location> result = getSubrouteInner(l, s);
    if (result.size() == 0) {
      routeTransitLineInstant_g(l);
      result = getSubrouteInner(l, s);
    }
    return result;
  }
}

void updateLegTimeEstimate(item l, item s) {
  TransitLine* tl = getTransitLine(l);
  if (tl->legs.size() <= s+1) return;
  vector<Location> route = getSubroute(l, s);
  Cup<Location> routeCup;
  routeCup.fromVector(route);
  float time = computeRouteInfo_g(&routeCup, false, false).travelTime;
  TransitLeg* leg = tl->legs.get(s);
  leg->timeEstimate = time;
  if (leg->timeRecord == 0) leg->timeRecord = leg->timeEstimate;
}

void updateLegTimeEstimate(item l) {
  TransitLine* tl = getTransitLine(l);
  for (int i = 0; i < tl->legs.size(); i++) {
    updateLegTimeEstimate(l, i);
  }
}

item insertStopIntoLine(item l, item s) {
  TransitLine* tl = getTransitLine(l);
  Stop* stop = getStop(s);
  item tlS = tl->legs.size();
  if (tl->cursor > tl->legs.size()) {
    tl->cursor = tl->legs.size();
  }

  if (transitRoutes_g.size() > l) clearRoute(transitRoutes_g.get(l));
  toRender.insert(l);
  if (!isInVector(&stop->lines, l)) {
    stop->lines.push_back(l);
  }

  TransitLeg leg;
  leg.stop = s;
  leg.timeEstimate = 0;
  leg.waitRecord = tl->headway[0]*.5f; //+tl->headway[1])* .25f;
  tl->legs.insertAt(tl->cursor, leg);
  updateLegTimeEstimate(l, tl->cursor);
  if (tl->cursor+1 < tlS) updateLegTimeEstimate(l, tl->cursor+1);
  tl->cursor ++;

  return tl->cursor-1;
}

void removeStopFromLine(item l, item s) {
  if (transitRoutes_g.size() > l) clearRoute(transitRoutes_g.get(l));
  toRender.insert(l);
  TransitLine* tl = getTransitLine(l);
  Stop* stop = getStop(s);
  removeFromVector(&stop->lines, l);

  vector<item> toUpdate;
  int offset = 0;
  int numLegs = tl->legs.size();
  bool lastWasRemoved = false;
  for (int i = 0; i < numLegs; i ++) {
    if (tl->legs[i].stop == s) {
      offset ++;
      if (i < tl->cursor) tl->cursor --;
      if (!lastWasRemoved) {
        lastWasRemoved = true;
        toUpdate.push_back(i-1);
      }
    } else if (offset > 0) {
      tl->legs.set(i-offset, tl->legs[i]);
      lastWasRemoved = false;
    }
  }
  tl->legs.resize(numLegs - offset);

  for (int i = 0; i < toUpdate.size(); i++) {
    item stopNdx = toUpdate[i];
    updateLegTimeEstimate(l, stopNdx);
  }
}

void removeStopFromLineByIndex(item l, item s) {
  TransitLine* tl = getTransitLine(l);
  if (s < 0 || s >= tl->legs.size()) return;
  if (transitRoutes_g.size() > l) clearRoute(transitRoutes_g.get(l));
  toRender.insert(l);

  item stopNdx = tl->legs[s].stop;
  tl->legs.remove(s);
  //updateLegTimeEstimate(l, s-1);
  if (s < tl->cursor) {
    tl->cursor --;
  }

  bool hasStop = false;
  for (int i = 0; i < tl->legs.size(); i++) {
    if (tl->legs[i].stop == stopNdx) {
      hasStop = true;
      break;
    }
  }

  if (!hasStop) {
    Stop* stop = getStop(stopNdx);
    removeFromVector(&stop->lines, l);
    if (stop->lines.size() == 0) {
      removeStop(stopNdx);
    }
  }
}

void removeLastStopFromLine(item l) {
  TransitLine* tl = getTransitLine(l);
  if (tl->legs.size() > 0) {
    removeStopFromLineByIndex(l, tl->legs.size()-1);
  }
}

void renderArrow(Mesh* m, GraphLocation loc, float dapOff, vec3 offset) {
  loc.dap += dapOff;

  vec3 tip = getLocation(loc);
  loc.dap -= 2;
  vec3 ualong = normalize(getLocation(loc) - tip) * trafficHandedness();
  vec3 along = ualong * c(CTransitLineWidth) *.5f;
  vec3 right = zNormal(along);
  tip += right*2.5f;
  tip -= offset;
  tip.z += 2;
  vec3 base = tip + along*1.5f;
  makeTriangle(m, tip, base + right, base - right, colorWhite);
}

void renderTransitLeg(item meshNdx, item lineNdx, item leg, vec3 offset) {
  TransitLine* tl = getTransitLine(lineNdx);
  Mesh* m = getMesh(meshNdx);
  offset.z -= 9;
  vec3 color = getTransitLineColorInPalette(lineNdx);

  if (leg < tl->legs.size()-1) {
    updateLegTimeEstimate(lineNdx, leg);

    const float stopGap = c(CTransitLineWidth)*1.25;
    Stop* s0 = getStop(tl->legs[leg].stop);
    Stop* s1 = getStop(tl->legs[leg+1].stop);
    vector<Location> route = getSubroute(lineNdx, leg);

    renderArrow(m, s0->graphLoc, stopGap*1.5f+c(CTransitLineWidth),
        offset - vec3(0,0,8));
    //renderArrow(m, s1->graphLoc, -stopGap*1.5f, offset);

    for (int j = 0; j < route.size(); j++) {
      item lNdx = route[j];
      if (locationType(lNdx) == LocLaneBlock) {
        LaneBlock* blk = getLaneBlock(lNdx);
        lNdx = getLaneIndex(lNdx, 0); //blk->numLanes-1);
        bool start = j == 0;
        bool end = j == route.size()-1;

        renderLane(m, lNdx,
            start ? s0->graphLoc.dap + stopGap : 0,
            end ? s1->graphLoc.dap - stopGap : -1,
            c(CTransitLineWidth)*0.7f, -2, offset, color);
      }
    }
  }
}

void renderTransitLine(item ndx, int flags) {
  TransitLine* tl = getTransitLine(ndx);
  tlEntities.ensureSize(ndx+1);
  Cup<item>* entities = tlEntities.get(ndx);

  for (int i = 0; i < entities->size(); i++) {
    removeEntityAndMesh(entities->at(i));
  }
  entities->clear();

  for (int i = 0; i < tl->legs.size(); i++) {
    item eNdx = addEntity(PaletteShader);
    entities->push_back(eNdx);
    Entity* e = getEntity(eNdx);
    e->texture = paletteTexture;
    e->flags |= _entityTransit;
    setEntityVisible(eNdx, transitVisible);
    bool desat = desaturateLine(ndx);
    setEntityDesaturate(eNdx, desat);
    setEntityRaise(eNdx, 4 - desat);
    setCull(eNdx, 0, 100000);

    createMeshForEntity(eNdx);
    Mesh* m = getMeshForEntity(eNdx);
    Stop* s0 = getStop(tl->legs[i].stop);
    vec3 offset = s0->location;
    offset.z = 0;

    if (i < tl->legs.size()-1) {
      Stop* s1 = getStop(tl->legs[i+1].stop);
      vector<Location> route = getSubroute(ndx, i);
      vector<vec3> points;
      points.push_back(s0->location);
      points.push_back(s1->location);

      for (int j = 0; j < route.size(); j++) {
        Location lNdx = route[j];
        if (locationType(lNdx) == LocLaneBlock) {
          LaneBlock* blk = getLaneBlock(lNdx);
          lNdx = getLaneIndex(lNdx, blk->numLanes-1);
          Lane* l = getLane(lNdx);
          points.push_back(l->ends[0]);
          points.push_back(l->ends[1]);
        }
      }

      offset = vec3(0,0,0);
      for (int j = 0; j < points.size(); j++) {
        offset += points[j];
      }
      offset *= 1./points.size();
      float maxDist = 0;
      for (int j = 0; j < points.size(); j++) {
        float dist = distance2DSqrd(points[j], offset);
        if (dist > maxDist) {
          maxDist = dist;
        }
      }

      offset.z = -4;
      setCull(eNdx, sqrt(maxDist), 100000);

      renderTransitLeg(e->mesh, ndx, i, offset);
    }

    renderStopDisc(m, tl->legs[i].stop, offset);

    offset.z = 0;
    placeEntity(eNdx, offset, 0, 0);
    bufferMesh(e->mesh);
  }
}

void renderTransitLine(item ndx) {
  item flags = getTransitLine(ndx)->flags;
  if (!(flags & _transitExists)) return;
  renderTransitLine(ndx, flags);
}

static void iterate(itemset& s, void (*callback)(item ndx, int flags)) {
  for (itemsetIter it = s.begin(); it != s.end(); it++) {
    item el = *it;
    if (el <= 0) continue;
    if (el > transitLines->size()) continue;
    int flags = getTransitLine(el)->flags;
    if (!(flags & _transitExists)) continue;
    callback(el, flags);
  }
  s.clear();
}

void renderTransitNumberForStop(item stopNdx) {
  Stop* stop = getStop(stopNdx);
  item eNdx = stopNumEntities[stopNdx];
  if (!(stop->flags & _stopExists) ||
      !(stop->flags & _stopComplete)) {
    if (eNdx != 0) {
      removeEntityAndMesh(eNdx);
      stopNumEntities.set(stopNdx, 0);
    }
    return;
  }

  int num = 0;
  for (int i = 0; i < stop->travelGroups.size(); i++) {
    num += getTravelGroupSize_v(stop->travelGroups[i]);
  }

  stop->numWaiting = num;
  int cap = 50;
  bool newEntity = (eNdx == 0);

  if (newEntity) {
    eNdx = addEntity(TextShader);
    stopNumEntities.set(stopNdx, eNdx);
    Entity* e = getEntity(eNdx);
    e->texture = textTexture;
    e->flags |= _entityTransit;
    setCull(eNdx, 0, 100000);
  }

  GraphLocation gl = stop->graphLoc;
  gl.lane = getLaneIndex(gl.lane, 0);
  Lane* lane = getLane(gl);
  vec3 loc = getLocation(gl);
  gl.dap += 0.1;
  vec3 norm = uzNormal(getLocation(gl)-loc);
  loc += norm * (c(CTransitLineWidth)*-1.25f) * trafficHandedness();
  loc.z += 24;
  placeEntity(eNdx, loc, 0, 0);

  setEntityVisible(eNdx, transitVisible);
  setEntityRedHighlight(eNdx, num > cap*.5f);
  setEntityBlueHighlight(eNdx, false);
  setEntityRaise(eNdx, 4);

  if (!newEntity && stopNumLast[stopNdx] == num) return;

  stopNumLast.set(stopNdx, num);
  createMeshForEntity(eNdx);
  Mesh* m = getMeshForEntity(eNdx);

  float fontSize =
    num >= 1000 ? 25 :
    num >= 100 ? 30 :
    num >= 10 ? 45 : 50;
  char* nStr = sprintf_o("%d", num);
  norm.z = 0;
  vec3 along = zNormal(norm);
  norm *= -fontSize*.5f;
  norm += along * (-.5f * fontSize * (.1f + stringWidth(nStr)));
  norm.z ++;

  renderString(m, nStr, norm, along*fontSize);
  bufferMeshForEntity(eNdx);
  free(nStr);
}

void updateTransit(double duration) {
  float time = getCurrentDateTime();
  float adjTime = fmod(getLightTime()+0.75f, 1.f);
  int div = adjTime*transitFrequencyDivisions;
  div = div%transitFrequencyDivisions;
  item lineS = transitLines->size();

  for (int i = 1; i <= lineS; i++) {
    TransitLine* line = getTransitLine(i);
    if (!(line->flags & _transitExists)) continue;
    if (!(line->flags & _transitEnabled)) continue;
    float timeSinceVehicle = time - line->lastVehicleTime;

    if (timeSinceVehicle > line->headway[div]) {
      Route route;
      if (transitRoutes_g.size() > i) {
        copyRoute(transitRoutes_g.get(i), &route);
      } else {
        clearRoute(&route);
      }

      // Launch a new vehicle
      item vNdx = addTransitVehicle_g(i, &route);
      clearRoute(&route);
      if (vNdx != 0) {
        line->vehicles.push_back(vNdx);
        line->lastVehicleTime = time;
        line->vehiclesEver ++;
      }
    }
  }

  item stopS = sizeStops();
  if (stopS > 0) {
    stopUpdateNdx = (stopUpdateNdx % stopS) + 1;
    updateStop(stopUpdateNdx);
  }

  renderTransitNumbers();

  for (int i = 1; i <= transitSystems->size(); i++) {
    TransitSystem* system = getTransitSystem(i);
    if (!(system->flags & _transitExists)) continue;
    if (system->flags & _transitComplete) continue;

    if (system->flags & _transitDesigning) {
      if (getGameMode() == ModeTest ||
          system->designDate < getCurrentDateTime()) {
        if (randFloat() < c(CTransitDesignDelayChance)) {
          system->designDate ++;
        } else {
          system->flags |= _transitDesigned;
          addMessage(TransitDesignMessage, i);
        }
      }

    } else if (system->flags & _transitBidding) {
      int bidS = system->bids.size();
      if (bidS < c(CNumTransitBids) && randFloat() < duration*.2f) {
        TransitSystemBid bid;
        bid.firmName = randomName(EngineeringFirmName);
        system->bids.push_back(bid);
      }
    }
  }
}

void setTransitSystemColor(item sysNdx, int color, int val) {
  TransitSystem* system = getTransitSystem(sysNdx);
  system->color[color] = val;
  if (color == 0) {
    for (int i = 1; i <= transitLines->size(); i++) {
      TransitLine* line = getTransitLine(i);
      if (line->system == sysNdx &&
          line->color == 255 &&
          (line->flags & _transitExists)) {
        toRender.insert(i);
      }
    }
  }
}

void setTransitLineColor(item lineNdx, int val) {
  TransitLine* line = getTransitLine(lineNdx);
  line->color = val;
  toRender.insert(lineNdx);
}

item getSystemGraphType(item systemNdx) {
  return getTransitSystem(systemNdx)->configType;
  /*
  TransitSystem* system = getTransitSystem(systemNdx);
  if (system->type == TSTBus) { return ConfigTypeRoad;
  } else if (system->type == TSTRail) {return ConfigTypeHeavyRail;
  } else if (system->type == TSTMonorail) {return ConfigTypeMonorail;
  } else if (system->type == TSTHangingMonorail) {return ConfigTypeMonoHang;
  } else if (system->type == TSTRubberTyred) {return ConfigTypeRoad;
  } else if (system->type == TSTMagLev) {return ConfigTypeMagLev;
  } else { return ConfigTypeRoad; }
  */
}

vector<item> suggestStationColors(item ndx) {
  if (ndx <= 0) return vector<item>(); // unimplemented
  Edge* edge = getEdge(ndx);
  set<item> linesSet;
  for (int i = 1; i <= sizeStops(); i++) {
    Stop* stop = getStop(i);
    if (!(stop->flags & _stopExists)) continue;
    item blkNdx = stop->graphLoc.lane/10;
    if (blkNdx <= 0) continue;
    if (blkNdx != edge->laneBlocks[0]/10 &&
      blkNdx != edge->laneBlocks[1]/10) continue;
    linesSet.insert(stop->lines.begin(), stop->lines.end());
  }

  vector<item> lines(linesSet.begin(), linesSet.end());
  set<int> colorSet;
  for (int i = 0; i < lines.size(); i++) {
    TransitLine* line = getTransitLine(lines[i]);
    int color = line->color;
    if (color == 255) {
      TransitSystem* system = getTransitSystem(line->system);
      color = system->color[0];
    }
    colorSet.insert(color);
  }

  vector<int> colors(colorSet.begin(), colorSet.end());
  if (colors.size() == 0) {
    colors.push_back(7);
  }
  return colors;
}

void renderTransitNumbers() {
  if (transitVisible) {
    stopNumEntities.ensureSize(sizeStops()+1);
    stopNumLast.ensureSize(sizeStops()+1);
    for (int i = 1; i <= sizeStops(); i++) {
      renderTransitNumberForStop(i);
    }

    /*
    vehicleNumEntities.ensureSize(sizeVehicles()+1);
    vehicleNumShadow.ensureSize(sizeVehicles()+1);
    vehicleNumLast.ensureSize(sizeVehicles()+1);
    for (int i = 1; i <= sizeVehicles(); i++) {
      renderTransitNumberForVehicle(i);
    }
    */
  }
}

/*
void positionTransitNumbers() {
  if (!transitVisible) return;

  for (int i = 1; i < vehicleNumEntities.size(); i++) {
    item eNdx = vehicleNumEntities[i];
    if (eNdx == 0) continue;
    Vehicle* v = getVehicle(i);
    if (!(v->flags & _vehicleExists)) continue;
    vec3 loc = v->location;
    loc.z += 40;
    placeEntity(eNdx, loc, 0, 0);
    placeEntity(vehicleNumShadow[i], loc, 0, 0);
  }
}
*/

void updateTransitRoute_g(item ndx, Route* route) {
  transitRoutes_g.ensureSize(ndx+1);
  Route prev = transitRoutes_g[ndx];
  bool changed = false;
  int rStepS = route->steps.size();
  int pStepS = prev.steps.size();
  if (rStepS == 0) return;

  if (rStepS != pStepS) {
    changed = true;
  } else {
    for (int i = 0; i < rStepS; i++) {
      if (prev.steps[i] != route->steps[i]) {
        changed = true;
        break;
      }
    }
  }

  if (!changed) return;
  copyRoute(route, transitRoutes_g.get(ndx));
  toRender.insert(ndx);
  //renderTransitLine(ndx);
  //updateLegTimeEstimate(ndx);
}

void finishTransit() {
  iterate(toRender, renderTransitLine);
}

void writeTransitSystem(FileBuffer* file, int ndx) {
  TransitSystem* system = getTransitSystem(ndx);
  fwrite_uint32_t(file, system->flags);
  fwrite_uint8_t(file, system->type);
  fwrite_uint8_t(file, system->power);
  fwrite_uint8_t(file, system->automation);
  fwrite_uint8_t(file, system->maxCars);
  fwrite_uint8_t(file, system->logo);
  fwrite_uint8_t(file, system->configType);
  fwrite_uint8_t(file, system->lineStyle);
  fwrite_float(file, system->ticketPrice);
  fwrite_float(file, system->transferPrice);
  fwrite_float(file, system->designDate);
  fwrite_string(file, system->name);
  fwrite_item_vector(file, &system->features);

  for (int i = 0; i < 2; i++) {
    fwrite_uint8_t(file, system->color[i]);
  }

  fwrite_int(file, system->bids.size());
  for (int i = 0; i < system->bids.size(); i++) {
    TransitSystemBid bid = system->bids[i];
    fwrite_string(file, bid.firmName);
  }
}

void readTransitSystem(FileBuffer* file, int ndx) {
  TransitSystem* system = getTransitSystem(ndx);
  system->flags = fread_uint32_t(file);
  system->type = fread_uint8_t(file);
  system->power = fread_uint8_t(file);
  system->automation = fread_uint8_t(file);
  system->maxCars = fread_uint8_t(file);
  system->logo = fread_uint8_t(file);
  system->configType = fread_uint8_t(file);
  system->lineStyle = fread_uint8_t(file);
  system->ticketPrice = fread_float(file);
  system->transferPrice = fread_float(file);
  system->designDate = fread_float(file);
  system->name = fread_string(file);
  system->numLines = 0;
  fread_item_vector(file, &system->features, file->version);

  for (int i = 0; i < 2; i++) {
    system->color[i] = fread_uint8_t(file);
  }

  int bidS = fread_int(file);
  for (int i = 0; i < bidS; i++) {
    TransitSystemBid bid;
    bid.firmName = fread_string(file);
    system->bids.push_back(bid);
  }
}

void writeTransitLine(FileBuffer* file, int ndx) {
  TransitLine* line = getTransitLine(ndx);
  fwrite_uint32_t(file, line->flags);
  fwrite_uint8_t(file, line->system);
  fwrite_uint8_t(file, line->maxCars);
  fwrite_uint8_t(file, line->color);
  fwrite_float(file, line->lastVehicleTime);
  fwrite_item(file, line->passengersNow);
  fwrite_item(file, line->passengersEver);
  fwrite_item(file, line->vehiclesEver);
  fwrite_string(file, line->name);
  line->vehicles.write(file);

  fwrite_item(file, transitFrequencyDivisions);
  for (int i = 0; i < transitFrequencyDivisions; i++) {
    fwrite_float(file, line->headway[i]);
  }

  item numLegs = line->legs.size();
  fwrite_item(file, numLegs);
  for (int i = 0; i < numLegs; i++) {
    TransitLeg leg = line->legs[i];
    fwrite_item(file, leg.stop);
    fwrite_float(file, leg.timeEstimate);
    fwrite_float(file, leg.timeRecord);
    fwrite_float(file, leg.waitRecord);
  }
}

void readTransitLine(FileBuffer* file, int ndx) {
  TransitLine* line = getTransitLine(ndx);
  line->flags = fread_uint32_t(file);
  if (file->version >= 53) {
    line->system = fread_uint8_t(file);
    line->maxCars = fread_uint8_t(file);
    line->color = fread_uint8_t(file);
  } else {
    line->system = 1; // Pre-gen bus system
    line->maxCars = 1;
    line->color = 255;
  }
  line->lastVehicleTime = fread_float(file);
  line->passengersNow = fread_item(file, file->version);
  line->passengersEver = fread_item(file, file->version);
  line->vehiclesEver = fread_item(file, file->version);
  line->name = fread_string(file);
  line->vehicles.read(file, file->version);

  item div = fread_item(file, file->version);
  for (int i = 0; i < div; i++) {
    line->headway[i] = fread_float(file);
  }

  item numLegs = fread_item(file);
  line->cursor = numLegs;
  for (int i = 0; i < numLegs; i++) {
    TransitLeg leg;
    leg.stop = fread_item(file);
    leg.timeEstimate = fread_float(file);
    leg.timeRecord = fread_float(file);
    leg.waitRecord = fread_float(file);
    line->legs.push_back(leg);
  }

  if (line->flags & _transitExists) {
    toRender.insert(ndx);

    TransitSystem* system = getTransitSystem(line->system);
    system->numLines ++;
  }
}

void readTransit_g(FileBuffer* file) {
  if (file->version >= 51) {
    if (file->version >= 53) {
      transitSystems->read(file, file->version);
      for (int i = 1; i <= transitSystems->size(); i++) {
        readTransitSystem(file, i);
      }

    } else {
      initTransit();

      if (transitSystems->size() > 0) {
        TransitSystem* bus = getTransitSystem(1);
        bus->ticketPrice = fread_float(file);
        bus->transferPrice = fread_float(file);
      } else {
        fread_float(file);
        fread_float(file);
      }
    }

    transitLines->read(file, file->version);
    for (int i = 1; i <= transitLines->size(); i++) {
      readTransitLine(file, i);
    }
  }
}

void writeTransit_g(FileBuffer* file) {
  transitSystems->write(file);
  for (int i = 1; i <= transitSystems->size(); i++) {
    writeTransitSystem(file, i);
  }

  transitLines->write(file);
  for (int i = 1; i <= transitLines->size(); i++) {
    writeTransitLine(file, i);
  }
}

