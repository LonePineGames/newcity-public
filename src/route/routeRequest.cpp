#include "routeRequest.hpp"

#include "../economy.hpp"
#include "../error.hpp"
#include "../graph/transit.hpp"
#include "../graph/transitRouter.hpp"
#include "../graph/stop.hpp"
#include "../person.hpp"
#include "../pool.hpp"
#include "../time.hpp"
#include "../vehicle/update.hpp"

#include "location.hpp"
#include "router.hpp"

#include "spdlog/spdlog.h"

Pool<RouteRequest> requests_g;
uint64_t routeKey(RouteRequest route);

void resetRouteRequests_g() {
  requests_g.clear();
}

RouteRequest getRouteRequest_g(item ndx) {
  return *(requests_g.get(ndx));
}

void clearRequest(RouteRequest* req) {
  req->type = RouteForNull;
  req->flags = 0;
  req->ndx = 0;
  req->source = 0;
  req->dest = 0;
  req->element = 0;
  req->sourceLoc = req->destLoc = vec3(0,0,0);
  req->subRequests.clear();
  req->steps.clear();
  vector<item> swap1;
  req->subRequests.swap(swap1);
  vector<Location> swap2;
  req->steps.swap(swap2);
}

void clearRequest(item ndx) {
  RouteRequest* req = requests_g.get(ndx);
  clearRequest(req);
  req->ndx = ndx;
}

void freeRequest(item reqNdx, int depth) {
  if (depth > 20) return;
  if (reqNdx <= 0 || reqNdx > requests_g.size()) return;
  RouteRequest* req = requests_g.get(reqNdx);
  int numSubs = req->subRequests.size();
  for (int i = 0; i < numSubs; i++) {
    item subReqNdx = req->subRequests[i];
    freeRequest(subReqNdx, depth+1);
  }
  clearRequest(reqNdx);
  requests_g.free(reqNdx);
}

void freeRequest(item reqNdx) {
  freeRequest(reqNdx, 0);
}

void toRoute(RouteRequest* req, Route* result) {
  result->flags = 0;
  result->version = 0;
  result->source = req->source;
  result->destination = req->dest;
  result->currentStep = 0;
  result->steps.fromVector(req->steps);
}

void finishRoute_g(item reqNdx) {
  if (reqNdx <= 0 || reqNdx > requests_g.size()) return;
  RouteRequest* req = requests_g.get(reqNdx);
  int numSubs = req->subRequests.size();

  //SPDLOG_INFO("finishRoute_g {}-{}/{} type:{} flags:{:b} subs:{} steps:{}",
      //req->element, reqNdx, req->ndx, req->type, req->flags,
      //req->subRequests.size(), req->steps.size());
  //SPDLOG_INFO("{}", routeString(req->steps, -1));

  if (req->flags & _routeMeta) {

    // Are all the subRequests done?
    for (int i = 0; i < numSubs; i++) {
      item subReqNdx = req->subRequests[i];
      RouteRequest* subReq = requests_g.get(subReqNdx);
      if (!(subReq->flags & _routeCompleted)) return;
    }

    req->flags |= _routeCompleted;

    if (req->flags & _routeChoice) {
      // Find best one
      RouteRequest* bestReq = 0;
      float bestCost = FLT_MAX;

      for (int i = 0; i < numSubs; i++) {
        item subReqNdx = req->subRequests[i];
        RouteRequest* subReq = requests_g.get(subReqNdx);
        bool transit = subReq->flags & _routeTransit;
        if (transit && subReq->steps.size() <= 2) continue;
        Cup<Location> steps;
        steps.fromVector(subReq->steps);
        RouteInfo info = computeRouteInfo_g(&steps, false, false);
        float totalCost = info.costAdjustedTime;

        if (steps.size() > 0 && totalCost < bestCost) {
          bestReq = subReq;
          bestCost = totalCost;
        }

        steps.clear();
      }

      if (bestReq != 0) {
        req->steps.clear();
        req->steps.insert(req->steps.end(),
            bestReq->steps.begin(), bestReq->steps.end());
      }

    } else {
      // Concatenate subRequests
      req->steps.clear();
      for (int i = 0; i < numSubs; i++) {
        item subReqNdx = req->subRequests[i];
        RouteRequest* subReq = requests_g.get(subReqNdx);
        req->steps.insert(req->steps.end(),
            subReq->steps.begin(), subReq->steps.end());
      }

      /*
      for (int i = 0; i < req->steps.size(); i++) {
        if (locationType(req->steps[i]) == LocTransitStop) {
          SPDLOG_INFO("concat {}:{} {}", req->element, req->ndx,
              routeString(req->steps, -1));
          break;
        }
      }
      */
    }

    for (int i = 0; i < numSubs; i++) {
      item subReqNdx = req->subRequests[i];
      freeRequest(subReqNdx);
    }
    req->subRequests.clear();
  }

  if (req->type == RouteForMeta) {
    finishRoute_g(req->element);

  } else {
    Route route;
    toRoute(req, &route);
    if (req->type == RouteForVehicle) {
      finishVehicleRoute(req->element, &route);
    } else if (req->type == RouteForPerson) {
      finishPersonRoute(req->element, &route);
    } else if (req->type == RouteForTransitLine) {
      updateTransitRoute_g(req->element, &route);
    }

    freeRequest(reqNdx);
  }
}

void finishOneRoute_g(RouteRequest result) {
  if (result.ndx <= 0 || result.ndx > requests_g.size()) return;
  RouteRequest* request = requests_g.get(result.ndx);

  request->steps.clear();
  request->steps.insert(request->steps.end(),
      result.steps.begin(), result.steps.end());
  request->flags |= _routeCompleted;
  finishRoute_g(request->ndx);
}

void enqueueRequest_g(item reqNdx, int depth) {
  if (depth > 100) return;
  if (reqNdx <= 0 || reqNdx > requests_g.size()) return;
  RouteRequest* req = requests_g.get(reqNdx);

  if (req->flags & _routeMeta) {
    // Enqueue all sub requests
    int numSubs = req->subRequests.size();
    for (int i = 0; i < numSubs; i++) {
      item subReqNdx = req->subRequests[i];
      if (subReqNdx == reqNdx) {
        SPDLOG_INFO("route request contains itself");
        continue;
      }
      RouteRequest* subReq = requests_g.get(subReqNdx);
      if (!(subReq->flags & _routeCompleted)) {
        enqueueRequest_g(subReqNdx, depth+1);
      }
    }

  } else if (!(req->flags & _routeCompleted)) {
    enqueueRoute_g(reqNdx);
  }
}

void enqueueRequest_g(item reqNdx) {
  enqueueRequest_g(reqNdx, 0);
}

void completeRequestInstant_g(item reqNdx) {
  if (reqNdx <= 0 || reqNdx > requests_g.size()) return;
  RouteRequest* req = requests_g.get(reqNdx);

  if (req->flags & _routeMeta) {
    // Finish all sub requests
    int numSubs = req->subRequests.size();
    for (int i = 0; i < numSubs; i++) {
      item subReqNdx = req->subRequests[i];
      RouteRequest* subReq = requests_g.get(subReqNdx);
      if (!(subReq->flags & _routeCompleted)) {
        completeRequestInstant_g(subReqNdx);
      }
    }

    finishRoute_g(reqNdx);

  } else if (!(req->flags & _routeCompleted)) {
    req->steps = routeInstant(req->source, req->dest,
        req->flags & _routeTransit);
    req->flags |= _routeCompleted;
  }
}

item r(item metaNdx, item reqNdx) {
  RouteRequest* meta = requests_g.get(metaNdx);
  RouteRequest* req = requests_g.get(reqNdx);
  meta->subRequests.push_back(reqNdx);
  meta->flags |= _routeMeta;
  req->type = RouteForMeta;
  req->element = metaNdx;
  //SPDLOG_INFO("r({},{}) {} subs",
      //metaNdx, reqNdx, meta->subRequests.size());
  return reqNdx;
}

item route() {
  item ndx = requests_g.create();
  clearRequest(ndx);
  return ndx;
}

item route(item source, item dest, item flags) {
  item ndx = route();
  RouteRequest* req = requests_g.get(ndx);
  req->flags |= flags;
  req->source = source;
  req->dest = dest;

  return ndx;
}

item route(item source, item dest) {
  return route(source, dest, 0);
}

item metaRoute(item type, item element, uint32_t flags,
    Location source, Location dest) {
  item ndx = route();
  RouteRequest* req = requests_g.get(ndx);
  req->flags = flags | _routeMeta;
  req->type = type;
  req->element = element;
  req->source = source;
  req->dest = dest;
  return ndx;
}

item metaRoute(uint32_t flags, Location source, Location dest) {
  return metaRoute(RouteForNull, 0, flags, source, dest);
}

item metaRoute(Location source, Location dest) {
  return metaRoute(RouteForNull, 0, 0, source, dest);
}

item routeVehicleInner_g(GraphLocation source, GraphLocation dest) {
  item meta = metaRoute(source.lane, dest.lane);

  item leaderNdx = r(meta, route(source.lane, source.lane, _routeCompleted));
  RouteRequest* leader = requests_g.get(leaderNdx);
  leader->steps.push_back(source.lane);
  leader->steps.push_back(dapLocation(source.dap));

  r(meta, route(source.lane, dest.lane));

  item trailerNdx = r(meta, route(dest.lane, dest.lane, _routeCompleted));
  RouteRequest* trailer = requests_g.get(trailerNdx);
  trailer->steps.push_back(dest.lane);
  trailer->steps.push_back(dapLocation(dest.dap));

  return meta;
}

void routeVehicle_g(item vehicleNdx, GraphLocation source, GraphLocation dest) {
  item ndx = routeVehicleInner_g(source, dest);
  RouteRequest* req = requests_g.get(ndx);
  req->type = RouteForVehicle;
  req->element = vehicleNdx;
  enqueueRequest_g(ndx);
}

item routeTransitInner_g(GraphLocation source, GraphLocation dest) {
  item meta = metaRoute(_routeTransit, source.lane, dest.lane);

  item leaderNdx = r(meta, route(source.lane, source.lane, _routeCompleted));
  RouteRequest* leader = requests_g.get(leaderNdx);
  leader->steps.push_back(source.lane);

  item ndx = r(meta,
      route(source.lane, dest.lane, _routeTransit));
  RouteRequest* req = requests_g.get(ndx);
  req->sourceLoc = getLocation(source);
  req->destLoc = getLocation(dest);
  adjustStat(ourCityEconNdx(), TransitRoutes, 1);

  item trailerNdx = r(meta, route(dest.lane, dest.lane, _routeCompleted));
  RouteRequest* trailer = requests_g.get(trailerNdx);
  trailer->steps.push_back(dest.lane);

  return meta;

  /*
  item ndx = route(source.lane, dest.lane, _routeTransit);
  RouteRequest* req = requests_g.get(ndx);
  req->sourceLoc = getLocation(source);
  req->destLoc = getLocation(dest);
  adjustStat(ourCityEconNdx(), TransitRoutes, 1);
  return ndx;
  */
}

void routePerson_g(item personNdx, GraphLocation source, GraphLocation dest) {
  item meta = metaRoute(RouteForPerson, personNdx, _routeChoice,
      source.lane, dest.lane);
  adjustStat(ourCityEconNdx(), PersonRoutes, 1);

  if (getPerson(personNdx)->activity != FreightActivity) {
    r(meta, routeTransitInner_g(source, dest));
  }
  r(meta, routeVehicleInner_g(source, dest));
  enqueueRequest_g(meta);
}

void routeTransitVehicleInner_g(item meta, item lineNdx) {
  TransitLine* line = getTransitLine(lineNdx);
  int numStops = line->legs.size();

  for (int i = 0; i < numStops; i++) {
    Stop* s0 = getStop(line->legs[i].stop);

    Location source = s0->graphLoc.lane;
    Location dest = transitLegStopLocation(lineNdx, i);
    item reqNdx = r(meta, route(source, dest, _routeCompleted));
    RouteRequest* req = requests_g.get(reqNdx);
    req->steps.push_back(source);
    req->steps.push_back(dapLocation(s0->graphLoc.dap));
    req->steps.push_back(transitStopLocation(line->legs[i].stop));
    req->steps.push_back(req->dest);

    if (i+1 < numStops) {
      Stop* s1 = getStop(line->legs[i+1].stop);

      source = s0->graphLoc.lane;
      dest = s1->graphLoc.lane;
      r(meta, route(source, dest));
      adjustStat(ourCityEconNdx(), TransitVehicleRoutes, 1);
    }
  }
}

void routeTransitVehicle_g(item vehicleNdx, item lineNdx) {
  item meta = metaRoute(RouteForVehicle, vehicleNdx, 0, 0, 0);
  routeTransitVehicleInner_g(meta, lineNdx);
  enqueueRequest_g(meta);
}

void routeTransitLineInstant_g(item lineNdx) {
  item meta = metaRoute(RouteForTransitLine, lineNdx, 0, 0, 0);
  routeTransitVehicleInner_g(meta, lineNdx);
  completeRequestInstant_g(meta);
}

void defragmentRouteRequests() {
  requests_g.defragment("RouteRequests");
}

