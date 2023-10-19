#include "stop.hpp"

#include "graphParent.hpp"
#include "transit.hpp"

#include "../city.hpp"
#include "../color.hpp"
#include "../draw/entity.hpp"
#include "../draw/mesh.hpp"
#include "../draw/shader.hpp"
#include "../draw/texture.hpp"
#include "../graph.hpp"
#include "../icons.hpp"
#include "../lane.hpp"
#include "../person.hpp"
#include "../pool.hpp"
#include "../renderUtils.hpp"
#include "../time.hpp"
#include "../tools/road.hpp"
#include "../util.hpp"
#include "../vehicle/travelGroup.hpp"

#include "spdlog/spdlog.h"

Pool<Stop>* stops = Pool<Stop>::newPool(10);
Cup<item> stopEntities;
Cup<item> stopCoverageEntities;
item stopCoverageMesh = 0;
item busSignMesh = 0;
item busSignMeshSimple = 0;
item trainSignMesh = 0;
static const vec3 xGray = vec3(0.5/spriteSheetSize, 4.5/spriteSheetSize, 1);
static const vec3 xBlue = vec3(7.5/spriteSheetSize, 4.5/spriteSheetSize, 1);

void makeStopSignMeshes() {
  stopCoverageMesh = addMesh();
  busSignMesh = addMesh();
  busSignMeshSimple = addMesh();
  trainSignMesh = addMesh();

  for (int i = 0; i < 2; i++) {
    Mesh* m = getMesh(i == 0 ? busSignMesh : busSignMeshSimple);

    // Sign
    makeTransitSign(m, vec3(0.2,-2.,0), vec3(0,1,0), 2.5,
        iconBusSign, iconBusSignBack);
    //Awning
    makeAngledCube(m, vec3(0.6,3.1,3.0), vec3(1.7,0,-.25), vec3(0,-4.2,0),
        vec3(0.025,0,0.2), true, xBlue);
    // Foundation
    makeCube(m, vec3(1.5,1.0,-1), vec3(2.7,4.2,1.25), xGray, true, false);

    if (i == 0) {
      for (int y = 0; y < 2; y ++) {
        for (int x = 0; x < 2; x ++) {
          // Supports
          makeCube(m, vec3(x*1.5+.75,-1+y*4,.25), vec3(.1,.1,3.0-x*.25f),
              xGray, true, true);
        }
        // Bench supports
        makeCube(m, vec3(1.5,-1+y*4,0.75), vec3(1.6,.09,.1), xGray, true, true);
      }

      //Bench
      makeCube(m, vec3(2.,1.0,0.75), vec3(0.5,4.0,.25), xGray, true, true);
    }
  }

  Mesh* t = getMesh(trainSignMesh);
  makeTransitSign(t, vec3(0.2,-2.,0), vec3(0,1,0), 1.5,
      iconTrainSign, iconBusSignBack);

  Mesh* sc = getMesh(stopCoverageMesh);
  makeDisc(sc, vec3(0,0,0), c(CStopRadius)*2.f, 24, colorWhite);

  bufferMesh(stopCoverageMesh);
  bufferMesh(busSignMesh);
  bufferMesh(busSignMeshSimple);
  bufferMesh(trainSignMesh);
}

void renderStop(item ndx) {
  Stop* stop = getStop(ndx);
  if (!(stop->flags & _stopExists)) return;
  Lane* lane = getLane(stop->graphLoc.lane);

  if (busSignMesh == 0) {
    makeStopSignMeshes();
  }

  stopEntities.ensureSize(ndx+1);
  item eNdx = stopEntities[ndx];
  if (eNdx == 0) {
    eNdx = addEntity(PaletteShader);
    stopEntities.set(ndx, eNdx);
  }

  stopCoverageEntities.ensureSize(ndx+1);
  item scNdx = stopCoverageEntities[ndx];
  if (scNdx == 0) {
    scNdx = addEntity(TransitCoverageShader);
    stopCoverageEntities.set(ndx, scNdx);
  }

  bool train = true;
  if (stop->graphLoc.lane >= 10) {
    LaneBlock* blk = getLaneBlock(stop->graphLoc);
    item graphElem = blk->graphElements[1];
    if (graphElem > 0) {
      Configuration config = getElementConfiguration(graphElem);
      if (config.type == ConfigTypeRoad) train = false;
    }
  }

  Entity* e = getEntity(eNdx);
  e->flags |= _entityTransit;
  e->texture = iconTexture;
  e->mesh = train ? trainSignMesh : busSignMesh;
  e->simpleMesh = train ? 0 : busSignMeshSimple;
  e->simpleDistance = 400;
  setCull(eNdx, 20, 2000);
  setEntityTransparent(eNdx, !(stop->flags & _stopComplete));

  float dap = stop->graphLoc.dap/lane->length;
  vec3 loc = interpolateSpline(lane->spline, dap);
  vec3 normal = interpolateSpline(lane->spline, dap - 0.01) - loc;
  vec3 placeLoc = stop->location - uzNormal(normal)*
    (c(CLaneWidth)-c(CSholderWidth)) * trafficHandedness();
  float ang = atan2(normal.x, normal.y);
  placeEntity(eNdx, placeLoc, -ang, 0);

  Entity* sc = getEntity(scNdx);
  sc->flags |= _entityTransit;
  sc->texture = paletteTexture;
  sc->mesh = stopCoverageMesh;
  placeEntity(scNdx, placeLoc, 0, 0);
  setCull(scNdx, c(CStopRadius)*2, 100000);
  setEntityVisible(scNdx, true);
}

void initStopEntities() {
  stopEntities.resize(stops->size()+1);
  stopCoverageEntities.resize(stops->size()+1);
  for (int i = 1; i <= stops->size(); i++) {
    Stop* stop = getStop(i);
    if (!(stop->flags & _stopExists)) continue;
    stopEntities.set(i, addEntity(PaletteShader));
    stopCoverageEntities.set(i, addEntity(TransitCoverageShader));
  }
}

void setStopHighlight(item ndx, bool highlight) {
  if (stopEntities.size() <= ndx) return;
  item eNdx = stopEntities[ndx];
  if (eNdx == 0) return;
  setEntityHighlight(eNdx, highlight);
}

void setStopRedHighlight(item ndx, bool highlight) {
  if (stopEntities.size() <= ndx) return;
  item eNdx = stopEntities[ndx];
  if (eNdx == 0) return;
  setEntityRedHighlight(eNdx, highlight);
}

item findDistinctEdge(Node* node, item edge0) {
  const char* name = getEdge(edge0)->name;
  for (int i = 0; i < node->edges.size(); i++) {
    item eNdx = node->edges[i];
    if (eNdx == edge0) continue;
    Edge* e = getEdge(eNdx);
    if (strlength(e->name) > 2 && !streql(name, e->name)) {
      return node->edges[i];
    }
  }
  return 0;
}

void positionStop(item ndx) {
  Stop* stop = getStop(ndx);
  LaneBlock* blk = getLaneBlock(stop->graphLoc);
  item laneNdx = stop->graphLoc.lane;
  laneNdx = (laneNdx/10)*10 + blk->numLanes - 1;
  stop->graphLoc.lane = laneNdx;

  Lane* lane = getLane(stop->graphLoc);
  float dap = stop->graphLoc.dap/lane->length;
  vec3 loc = interpolateSpline(lane->spline, dap);
  vec3 normal = loc - interpolateSpline(lane->spline, dap - 0.01);
  normal = uzNormal(normal) * trafficHandedness();
  loc -= normal*c(CLaneWidth)*1.5f;
  stop->location = loc;
  renderStop(ndx);
}

void nameStop(item ndx) {
  Stop* stop = getStop(ndx);
  if (stop->name != 0) {
    free(stop->name);
    stop->name = 0;
  }

  item cityNdx = nearestCity(stop->location);
  if (cityNdx > 0) {
    City* city = getCity(cityNdx);
    float dist = vecDistance(stop->location, city->visualCenter);
    if (dist < 100) {
      stop->name = strdup_s(city->name);
      return;
    }
  }

  Configuration config;
  config.flags = 0;
  config.type = ConfigTypeRoad;
  item edge0 = nearestEdge(stop->location, false, config);
  item edge1 = 0;

  if (edge0 != 0) {
    Edge* edge = getEdge(edge0);
    float minDist = 1000000;
    for (int i = 0; i < 2; i++) {
      Node* node = getNode(edge->ends[i]);
      float dist = vecDistance(node->center, stop->location);
      if (i == 0 || dist < minDist) {
        minDist = dist;
        edge1 = findDistinctEdge(node, edge0);
      }
    }
  }

  if (edge1 != 0 && edge0 != 0) {
    stop->name = sprintf_o("%s & %s",
        getEdge(edge0)->name, getEdge(edge1)->name);
  } else if (edge0 != 0) {
    stop->name = strdup_s(getEdge(edge0)->name);
  } else {
    stop->name = strdup_s("Stop");
  }
}

item addStop(GraphLocation graphLoc) {
  item ndx = stops->create();
  Stop* stop = getStop(ndx);
  stop->flags = _stopExists;
  stop->type = 0;
  stop->graphLoc = graphLoc;
  stop->travelGroups.clear();
  stop->numWaiting = 0;
  stop->lines.clear();

  positionStop(ndx);
  nameStop(ndx);
  return ndx;
}

Stop* getStop(item ndx) {
  return stops->get(ndx);
}

void removeStop(item ndx) {
  Stop* stop = getStop(ndx);
  if (!(stop->flags & _stopExists)) return;

  removeLastAddedStop(ndx);

  for (int i = stop->lines.size() - 1; i >= 0; i--) {
    removeStopFromLine(stop->lines[i], ndx);
  }
  stop->lines.clear();

  if (stopEntities.size() > ndx) {
    removeEntity(stopEntities[ndx]);
    stopEntities.set(ndx, 0);
  }

  if (stopCoverageEntities.size() > ndx) {
    removeEntity(stopCoverageEntities[ndx]);
    stopCoverageEntities.set(ndx, 0);
  }

  free(stop->name);
  stop->name = 0;
  stop->travelGroups.clear();
  stop->numWaiting = 0;
  stop->flags = 0;
  stops->free(ndx);
}

money stopCost(item ndx) {
  return getInflation() * c(CStopCost);
}

const char* stopLegalMessage(item ndx) {
  return 0;
}

void completeStop(item ndx) {
  Stop* stop = getStop(ndx);
  stop->flags |= _stopComplete;
  positionStop(ndx);

  if (stop->graphLoc.lane >= 10) {
    LaneBlock* blk = getLaneBlock(stop->graphLoc);
    item graphElem = blk->graphElements[1];
    if (graphElem > 0) {
      renderGraphElement(graphElem);
    }
  }
}

void updateStop(item ndx) {
  Stop* stop = getStop(ndx);

  if (stop->flags & _stopExists) {
    if (stop->lines.size() == 0) {
      removeStop(ndx);
      return;
    }
  }

  item tgS = stop->travelGroups.size();
  if (tgS > 0) {
    item tgNdx = stop->travelGroups[randItem(tgS)];
    TravelGroup* group = getTravelGroup_g(tgNdx);
    Route route = group->route;
    item routeS = route.steps.size();
    Location legLoc = 0;

    for (int i = 0; i < routeS; i++) {
      Location loc = route.steps[i];
      if (locationType(loc) == LocTransitStop && locationNdx(loc) == ndx) {

        // find associated legLoc
        if (i > 0) {
          Location test = route.steps[i-1];
          if (locationType(test) == LocTransitLeg) {
            legLoc = test;
          }
        }

        if (i < routeS-1) {
          Location test = route.steps[i+1];
          if (locationType(test) == LocTransitLeg) {
            legLoc = test;
          }
        }
      }
    }

    bool invalid = !(stop->flags & _stopExists) ||
        legLoc == 0 || locationType(legLoc) != LocTransitLeg;

    if (!invalid) {
      item lineNdx = locationLineNdx(legLoc);
      item legNdx = locationLegNdx(legLoc);
      TransitLine* line = getTransitLine(lineNdx);
      if (!(line->flags & _transitEnabled) ||
          legNdx < 0 || legNdx > line->legs.size() ||
          isInVector(&stop->lines, lineNdx)) {
        invalid = true;

      } else {
        item stopNdx = line->legs[legNdx].stop;
        if (stopNdx != ndx) invalid = true;
      }
    }

    if (group->members.size() == 0) {
      invalid = true;
    } else {
      Person* person = getPerson(group->members[0]);
      if (getCurrentDateTime() - person->enterTime > c(CMaxVehicleAge)) {
        invalid = true;
      }
    }

    if (invalid) {
      removeTravelGroup_g(tgNdx);
    }
  }
}

void repositionStop(item stopNdx) {
  Stop* stop = getStop(stopNdx);
  if (!(stop->flags & _stopExists)) return;

  item graphElem = getElement(stop->graphLoc);
  if (graphElem < 0) {
    SPDLOG_ERROR("Transit stop on node");
    logStacktrace();
    return;

  } else if (graphElem == 0) {
    SPDLOG_ERROR("Transit stop on null edge");
    logStacktrace();
    GraphLocation newLoc = graphLocation(stop->location);
    if (newLoc.lane != 0) {
      stop->graphLoc = newLoc;
      positionStop(stopNdx);
    } else {
      removeStop(stopNdx);
    }

  } else {
    Edge* edge = getEdge(graphElem);
    if (!(edge->flags & _graphExists)) {
      vector<item> children = getGraphChildren(graphElem);
      bool side = edge->laneBlocks[1]/10 == stop->graphLoc.lane/10;
      item bestChild = 0;
      float bestChildDist = FLT_MAX;
      GraphLocation bestChildGraphLoc;
      vec3 bestChildLoc;

      for (int i = 0; i < children.size(); i++) {
        item childNdx = children[i];
        if (childNdx <= 0) continue;
        Edge* child = getEdge(childNdx);
        if (child->config.type != edge->config.type) continue;
        GraphLocation graphLoc = graphLocationForEdge(childNdx, stop->location);
        if (graphLoc.lane < 10) continue;
        bool childSide = child->laneBlocks[1]/10 == graphLoc.lane/10;
        if (side != childSide) continue;
        vec3 loc = getLocation(graphLoc);
        float dist = vecDistance(loc, stop->location);

        if (dist < bestChildDist) {
          bestChild = childNdx;
          bestChildDist = dist;
          bestChildGraphLoc = graphLoc;
          bestChildLoc = loc;
        }
      }

      if (bestChild == 0) {
        removeStop(stopNdx);
        return;
      }

      stop->graphLoc = bestChildGraphLoc;
      stop->location = bestChildLoc;
      positionStop(stopNdx);

    } else if (stop->graphLoc.lane%10-1 != edge->config.numLanes) {
      positionStop(stopNdx);
    }
  }
}

void repositionTransitStops() {
  for (int i = 1; i <= stops->size(); i++) {
    repositionStop(i);
  }
}

void renderStops() {
  for (int i = 1; i <= stops->size(); i++) {
    positionStop(i);
  }
}

item nearestStop(Line ml) {
  item best = 0;
  float bestDist = 1000000;

  for (int i = 1; i <= stops->size(); i++) {
    Stop* s = getStop(i);
    if (!(s->flags & _stopExists)) continue;
    float dist = pointLineDistance(s->location, ml);
    if (dist < bestDist) {
      best = i;
      bestDist = dist;
    }
  }

  return best;
}

void resetStops() {
  for (int i = 1; i < stops->size(); i++) {
    Stop* s = getStop(i);
    s->lines.clear();
  }
  stops->clear();

  stopEntities.clear();
  stopCoverageEntities.clear();
  stopCoverageMesh = 0;
  busSignMesh = 0;
  busSignMeshSimple = 0;
  trainSignMesh = 0;
}

item sizeStops() {
  return stops->size();
}

void writeStop(FileBuffer* file, int ndx) {
  Stop* stop = getStop(ndx);
  fwrite_uint32_t(file, stop->flags);
  fwrite_uint8_t(file, stop->type);
  fwrite_string(file, stop->name);
  fwrite_item(file, stop->plan);
  fwrite_item(file, stop->currentVehicle);
  fwrite_item_vector(file, &stop->travelGroups);
  fwrite_item(file, stop->updateNdx);
  fwrite_graph_location(file, stop->graphLoc);
  fwrite_vec3(file, stop->location);
  fwrite_item_vector(file, &stop->lines);
}

void writeStops_g(FileBuffer* file) {
  stops->write(file);
  for (int i = 1; i <= stops->size(); i++) {
    writeStop(file, i);
  }
}

void readStop(FileBuffer* file, int ndx) {
  Stop* stop = getStop(ndx);
  stop->flags = fread_uint32_t(file);
  stop->type = fread_uint8_t(file);
  stop->name = fread_string(file);
  stop->plan = fread_item(file);
  stop->currentVehicle = fread_item(file);
  fread_item_vector(file, &stop->travelGroups, file->version);
  stop->updateNdx = fread_item(file);
  stop->graphLoc = fread_graph_location(file, file->version);
  stop->location = fread_vec3(file);
  fread_item_vector(file, &stop->lines, file->version);
}

void readStops_g(FileBuffer* file) {
  if (file->version >= 51) {
    stops->read(file, file->version);
    for (int i = 1; i <= stops->size(); i++) {
      readStop(file, i);
    }
  }
}

