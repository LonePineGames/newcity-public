#ifndef BOOST_ALL_NO_LIB
	#define BOOST_ALL_NO_LIB
#endif

#include "router.hpp"

#include "../economy.hpp"
#include "../error.hpp"
#include "../graph/transit.hpp"
#include "../graph/transitRouter.hpp"
#include "../graph/stop.hpp"
#include "../game/task.hpp"
#include "../lane.hpp"
#include "../pool.hpp"
#include "../thread.hpp"

#include "broker.hpp"
#include "cache.hpp"
#include "heap.hpp"
#include "routeRequest.hpp"

#include <deque>
#include "spdlog/spdlog.h"
#include <boost/dynamic_bitset.hpp>

/*
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/lockfree/spsc_queue.hpp>
using namespace boost::multi_index;
using namespace boost::lockfree;
*/

vector<Location> aStarLaneBlocks_r(RouteRequest req, int num);
vector<Location> aStarTransit_r(RouteRequest req);
void readRouter(FileBuffer* file, int version);
void writeRouter(FileBuffer* file);
RouteRequest readRoute(FileBuffer* file, int version);
void writeRoute(FileBuffer* file, RouteRequest route);
uint64_t routeKey(RouteRequest route);
void pushReqToFinish(RouteRequest req);

enum BatchState {
  BatchLoading, BatchReady, BatchProcessing, BatchUnloading,
  numBatchStates
};

//vector<std::atomic<BatchState>> batchState;
std::deque<std::atomic<BatchState>> batchState;
vector<vector<RouteRequest>> batches;
static std::atomic<int> threadsAlive(0);
static std::atomic<int> threadsActive(0);

static bool shouldRoutingContinue = true;
std::atomic<bool> clearRoutesIn(false);
std::atomic<bool> routingDone(false);
static vector<item> toEnqueue;
static Pool<RouteRequest> toFinish_g;
boost::dynamic_bitset<> toFinishActive_g;
item numToFinishTask = 0;
item currentRouteToFinish = 0;
item currentThreadBatch = 0;

const uint64_t routeCacheCapacity = 10*1000*1000;
Cache<uint64_t, RouteRequest> routeCache(routeCacheCapacity, routeKey);
static double routesToDiscard = 0;
uint32_t lastRequestNumber = 0;

static Cup<Heap> openHeaps;

void sendRouteBatches(double duration) {
  if (getStatistic(ourCityEconNdx(), RouterQueueSize) <
      c(CRouterStopDiscardingThreshold)) {
    routesToDiscard += duration * c(CRouterCacheDiscardRate);
    while (routesToDiscard >= 1) {
      if (routeCache.size() == 0) {
        routesToDiscard = 0;
        break;
      }
      routeCache.discardRandom();
      routesToDiscard --;
      adjustStat(ourCityEconNdx(), RoutesDiscarded, 1);
    }
  }

  reverse(toEnqueue.begin(), toEnqueue.end());

  currentThreadBatch = (currentThreadBatch + 1) % c(CNumRouterThreads);
  item i = currentThreadBatch;

  //for (int i = 0; i < c(CNumRouterThreads); i++) {
    BatchState state = batchState[i];
    if (state == BatchLoading) {
      bool anyToEnqueue = toEnqueue.size() > 0;
      while (batches[i].size() < c(CRouterBatchSize) && toEnqueue.size() > 0) {
        item ndx = toEnqueue[toEnqueue.size() - 1];
        batches[i].push_back(getRouteRequest_g(ndx));
        toEnqueue.pop_back();
      }
      if (batches[i].size() >= c(CRouterBatchSize) ||
          (anyToEnqueue && toEnqueue.size() == 0)) {
        batchState[i] = BatchReady;
      }

    } else if (state == BatchUnloading) {
      for (int j = 0; j < batches[i].size(); j++) {
        adjustStat(ourCityEconNdx(), RoutesProcessed, 1);
        RouteRequest route = batches[i][j];
        routeCache.put(route);
        pushReqToFinish(route);
      }
      batches[i].clear();
      batchState[i] = BatchLoading;
    }
  //}

  reverse(toEnqueue.begin(), toEnqueue.end());

  int numInQueue = toEnqueue.size();
  setStat(ourCityEconNdx(), RouterQueueSize, numInQueue);
  setStat(ourCityEconNdx(), RouterThreadsActive, threadsActive);
}

void finishRouting(double duration) {
  while (numToFinishTask > 0) {
    queueTask_g(TaskFinish1000Routes, duration);
    numToFinishTask -= 1000;
  }
  numToFinishTask = 0;
}

void finish1000Routes(double duration) {
  item toFinish_gS = toFinish_g.size();
  int i = 0;
  for (int k = 0; k < toFinish_gS; k++) {
    if (i >= 1005) break;
    currentRouteToFinish = (currentRouteToFinish % toFinish_g.size()) + 1;
    if (toFinishActive_g.size() <= currentRouteToFinish) {
      toFinishActive_g.resize((currentRouteToFinish+1)*2);
    }
    if (!toFinishActive_g[currentRouteToFinish]) continue;
    finishOneRoute_g(*toFinish_g.get(currentRouteToFinish));
    toFinish_g.free(currentRouteToFinish);
    toFinishActive_g[currentRouteToFinish] = false;
    i++;
  }
}

void enqueueRoute_g(item ndx) {
  adjustStat(ourCityEconNdx(), RouteRequests, 1);
  RouteRequest req = getRouteRequest_g(ndx);

  std::tuple<bool, RouteRequest> cacheHit = routeCache.get(req);
  bool pushToFinish = false;

  if (std::get<0>(cacheHit)) {
    RouteRequest result = std::get<1>(cacheHit);

    if (result.steps.size() > 0 &&
          !isRouteValid_g(result.steps, req.source, req.dest)) {
      adjustStat(ourCityEconNdx(), RouteCacheErrors, 1);
      invalidateRouteCache(req.source, req.dest);
      toEnqueue.push_back(ndx);

    } else {
      adjustStat(ourCityEconNdx(), RouteCacheHits, 1);
      req.steps = result.steps;
      pushToFinish = true;
    }

  } else {
    adjustStat(ourCityEconNdx(), RouteCacheMisses, 1);
    if (getStatistic(ourCityEconNdx(), RouterQueueSize) >
        c(CRouterQueueCapacity)) {
      req.steps.clear();
      pushToFinish = true;
    } else {
      toEnqueue.push_back(ndx);
    }
  }

  if (pushToFinish) {
    pushReqToFinish(req);
  }
}

void pushReqToFinish(RouteRequest req) {
  item ndx = toFinish_g.create();

  RouteRequest* reqInFinish = toFinish_g.get(ndx);
  vector<item> swap1;
  reqInFinish->subRequests.swap(swap1);
  vector<Location> swap2;
  reqInFinish->steps.swap(swap2);
  *reqInFinish = req;

  if (toFinishActive_g.size() <= ndx) toFinishActive_g.resize((ndx+1)*2);
  toFinishActive_g[ndx] = true;
  numToFinishTask ++;
}

vector<Location> routeInstant(Location source, Location dest, bool transit) {
  adjustStat(ourCityEconNdx(), RouteRequests, 1);

  RouteRequest route;
  route.flags = 0;
  route.type = RouteInstant;
  route.element = 0;
  route.source = source;
  route.dest = dest;
  if (transit) {
    route.flags |= _routeTransit;
    route.sourceLoc = getBlockLoc_r(source);
    route.destLoc = getBlockLoc_r(dest);
  }

  std::tuple<bool, RouteRequest> cacheHit = routeCache.get(route);
  if (std::get<0>(cacheHit)) {
    adjustStat(ourCityEconNdx(), RouteCacheHits, 1);
    RouteRequest result = std::get<1>(cacheHit);
    if (isRouteValid_g(result.steps, source, dest)) {
      return result.steps;
    } else {
      adjustStat(ourCityEconNdx(), RoutesInvalidated, 1);
      invalidateRouteCache(source, dest);
    }

  } else {
    adjustStat(ourCityEconNdx(), RouteCacheMisses, 1);
  }

  return transit ? aStarTransit_r(route) : aStarLaneBlocks_r(route, 0);
}

vector<Location> routeInstant(Location source, Location dest) {
  return routeInstant(source, dest, false);
}

void invalidateRouteCache(Location source, Location dest) {
  adjustStat(ourCityEconNdx(), RoutesInvalidated, 1);
  RouteRequest deet;
  clearRequest(&deet);
  deet.flags = 0;
  deet.source = source;
  deet.dest = dest;
  routeCache.erase(deet);
  knownRouteErase_g(source, dest);
}

void invalidateRouteCache(Route* route) {
  invalidateRouteCache(route->source, route->destination);
}

void routerQueueLoop() {
  shouldRoutingContinue = true;
  int myNumber = threadsAlive ++;
  SPDLOG_INFO("Hi, I'm router thread {}", myNumber);

  while (shouldRoutingContinue) {
    if (clearRoutesIn || batchState[myNumber] != BatchReady) {
      sleepMilliseconds(10);

    } else {
      threadsActive ++;
      startLaneRouterMemoryBarrier_r();
      batchState[myNumber] = BatchProcessing;

      for (int i = 0; i < batches[myNumber].size(); i++) {
        RouteRequest route = batches[myNumber][i];

        if (route.flags & _routeTransit) {
          route.steps = aStarTransit_r(route);
        } else {
          route.steps = aStarLaneBlocks_r(route, myNumber+1);
        }

        batches[myNumber][i] = route;
      }

      if (--threadsActive == 0) {
        endLaneRouterMemoryBarrier_r();
      }
      batchState[myNumber] = BatchUnloading;
    }
  }

  threadsAlive --;
  if (threadsAlive <= 0) {
    routingDone = true;
  }
}

void resetRouting() {
  resetRouteRequests_g();
  routesToDiscard = 0;
  toEnqueue.clear();
  clearRoutesIn = true;

  while (shouldRoutingContinue && threadsActive > 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  for (int i = 0; i < c(CNumRouterThreads); i++) {
    batchState[i] = BatchLoading;
    batches[i].clear();
  }

  /*
  for (int i = 0; i < openHeaps.size(); i++) {
    freeHeap(openHeaps.get(i));
  }
  openHeaps.clear();
  */

  clearRoutesIn = false;
  routeCache.clear();
  routeCache.setCapacity(c(CRouterCacheCapacity));
}

void initRoutingThreads() {
  batchState.resize(c(CNumRouterThreads));
  batches.resize(c(CNumRouterThreads));
  openHeaps.resize(c(CNumRouterThreads)+1);

  for (int i = 0; i < c(CNumRouterThreads); i++) {
    char* name = sprintf_o("ROUTER THREAD %d", i);
    startThread(name, routerQueueLoop);
    sleepMilliseconds(5);
  }
}

void killRouting() {
  shouldRoutingContinue = false;
}

bool isRoutingDone() {
  return routingDone;
}

void doSingleThreadedRouting() {
  /*
  RouteRequest route;
  while (routesIn.pop(&route, 1)) {
    route.steps = aStarLaneBlocks_r(route, 0);
    finishOneRoute_g(route);
  }
  */
}

int routeCacheSize() {
  return routeCache.size();
}

vector<Location> constructRoute(Location* last,
  Location u, Location start) {

  vector<Location> result;
  while (u != start) {

    if (u <= 0 || u >= getNumLaneBlocks_r()) {
      SPDLOG_WARN("Route contained bad element {}", format(u));
      return vector<Location>();
    }

    if (result.size() > 100000) {
      SPDLOG_WARN("Route contained a cycle or very long route {}", format(u));
      return vector<Location>();
    }

    result.push_back(decodeLane_r(u));
    u = last[u];
  }

  result.push_back(decodeLane_r(u));
  reverse(result.begin(), result.end());
  return result;
}

vector<Location> aStarLaneBlocks_r(RouteRequest req, int heapNum) {
  if (openHeaps.size() <= heapNum) return vector<Location>();
  Location source = req.source;
  Location dest = req.dest;
  int numVerts = getNumLaneBlocks_r();
  Location* last = (Location*) calloc(numVerts+1, sizeof(Location));
  Location start = encodeLane_r(source);
  Location end = encodeLane_r(dest);
  Heap* openHeap = openHeaps.get(heapNum);
  reinitRouterHeap(openHeap, numVerts*2);
  //Heap openHeap = initRouterHeap(numVerts+1);
  int iter = 0;

  for (int i = 0; i <= numVerts; i++) {
    last[i] = 0;
  }

  heapPriority(openHeap, start, laneRoutingEstimate_r(source,
        dest));

  for (int j = 0; j <= numVerts+1; j++) {
    Location u = heapPop(openHeap);
    Location uDecode = decodeLane_r(u);
    if (u == 0 || u > numVerts) continue;
    if (openHeap->priority[u] == FLT_MAX) continue;
    if (!blockIsOpen_r(uDecode)) continue;

    if (end == u) {
      //freeHeap(openHeap);
      vector<Location> result = constructRoute(last, u, start);
      free(last);
      return result;
    }

    iter ++;
    float prevDist = openHeap->priority[u];
    item numDrains = 0;
    item* drains = getDrains_r(uDecode, &numDrains);
    for (int d = 0; d < numDrains; d++) {
      Location drainDecoded = drains[d];
      if (drainDecoded == 0) continue;
      Location drain = encodeLane_r(drainDecoded);
      float distance = laneBlockCost_r(drainDecoded) * randFloat(1., 1.05);
      float drainDist = openHeap->priority[drain];
      float totalDist = prevDist + distance;

      if (prevDist != FLT_MAX && totalDist < drainDist) {
        last[drain] = u;
        float estimate = laneRoutingEstimate_r(drainDecoded, dest);
        heapPriority(openHeap, drain, totalDist + estimate);
      }
    }
  }

  //SPDLOG_WARN("aStarLaneBlocks_r failed iter:{}", iter);
  //freeHeap(&openHeap);
  free(last);
  return vector<Location>();
}

bool isIn(item u, vector<item> source) {
  item sourceSize = source.size();
  for (int i = 0; i < sourceSize; i++) {
    if (u == source[i]) return true;
  }
  return false;
}

vector<Location> constructTransitRoute(item* last,
  item u, vector<item> source,
  Location* lastEntry, Location* lastExit) {

  vector<Location> result;

  while (!isIn(u, source)) {
    if (u <= 0 || u > getNumStops_r()) {
      SPDLOG_WARN("Route contained bad element");
      return vector<Location>();
    }
    if (result.size() > 10000) {
      SPDLOG_WARN("Route contained a cycle or very long route\n");
      return vector<Location>();
    }

    result.push_back(transitStopLocation(u));

    item exitNdx = lastExit[u];
    item entryNdx = lastEntry[u];

    if (exitNdx != 0 || entryNdx != 0) {
      if (locationType(exitNdx) != LocTransitLeg) {
        SPDLOG_INFO("bad exit {}", format(exitNdx));
      }
      if (locationType(entryNdx) != LocTransitLeg) {
        SPDLOG_INFO("bad entry {}", format(entryNdx));
      }
    }

    if (exitNdx != 0 && entryNdx != 0) {
      result.push_back(lastExit[u]);
      result.push_back(lastEntry[u]);
    }

    u = last[u];
  }

  result.push_back(transitStopLocation(u));
  reverse(result.begin(), result.end());
  return result;
}

vector<Location> aStarTransit_r(RouteRequest request) {
  vector<item> source = getNearbyStops_r(request.sourceLoc);
  vector<item> dest = getNearbyStops_r(request.destLoc);
  int numVerts = getNumStops_r();
  item* last = (item*) calloc(numVerts+1, sizeof(item));
  Location* lastEntry = (Location*) calloc(numVerts+1, sizeof(Location));
  Location* lastExit = (Location*) calloc(numVerts+1, sizeof(Location));
  Heap open = initRouterHeap(numVerts+1);
  item sourceSize = source.size();
  item destSize = dest.size();
  item bestDest = 0;

  for (int i = 0; i <= numVerts; i++) {
    last[i] = 0;
  }

  for (int i = 0; i < sourceSize; i++) {
    item sourceNdx = source[i];
    float bestCost = FLT_MAX;
    for (int j = 0; j < destSize; j++) {
      item destNdx = dest[j];
      float cost = transitRoutingEstimate_r(sourceNdx, destNdx);
      if (cost < bestCost) {
        bestDest = destNdx;
        bestCost = cost;
      }
    }
    heapPriority(&open, sourceNdx, bestCost);
  }

  for (int j = 0; j < numVerts; j++) {
    item u = heapPop(&open);
    if (u <= 0 || open.priority[u] == FLT_MAX) continue;
    //if (!blockIsOpen_r(decodeLane_r(u))) continue;

    for (int i = 0; i < destSize; i++) {
      if (dest[i] == u) {
        freeHeap(&open);
        vector<Location> result = constructTransitRoute(last, u, source,
            lastEntry, lastExit);
        free(last);
        free(lastEntry);
        free(lastExit);
        return result;
      }
    }

    float prevCost = open.priority[u];

    if (lastExit[u] > 0) {
      vector<item> stops = getNearbyStops_r(u);
      for (int s = 0; s < stops.size(); s++) {
        item stopNdx = stops[s];
        float cost = walkingCostStops_r(u, stopNdx);
        float drainCost = open.priority[stopNdx];
        float totalCost = prevCost + cost;

        if (prevCost != FLT_MAX && totalCost < drainCost) {
          last[stopNdx] = u;
          lastEntry[stopNdx] = 0;
          lastExit[stopNdx] = 0;
          float estimate = transitRoutingEstimate_r(stopNdx, bestDest);
          heapPriority(&open, stopNdx, totalCost + estimate);
        }
      }
    }

    vector<Location> entries = getLegsForStop_r(u);
    for (int s = 0; s < entries.size(); s++) {
      Location entryNdx = entries[s];
      float* cost = getLineLegCostTable_r(entryNdx);
      if (cost == NULL) continue;
      uint32_t num = getLineLength_r(entryNdx);
      float totalCost = prevCost + getLegWaitCost_r(entryNdx) + cost[0];

      for (uint32_t l = 1; l < num; l++) {
        Location exitNdx = entryNdx+l;
        Location stopNdx = getStopForLeg_r(exitNdx);
        if (stopNdx <= 0) continue;
        float drainCost = open.priority[stopNdx];

        if (u != stopNdx && prevCost != FLT_MAX && totalCost < drainCost) {
          last[stopNdx] = u;
          lastEntry[stopNdx] = entryNdx;
          lastExit[stopNdx] = exitNdx;
          float estimate = transitRoutingEstimate_r(stopNdx, bestDest);
          heapPriority(&open, stopNdx, totalCost + estimate);
        }

        totalCost += cost[l];
      }
    }
  }

  freeHeap(&open);
  free(last);
  free(lastEntry);
  free(lastExit);
  return vector<Location>();
}

uint64_t routeKey(RouteRequest route) {
  uint32_t flags = (route.flags & _routeTransit) ? 1 : 0;
  uint64_t key = route.source;
  key *= INT_MAX;
  key += route.dest;
  key += flags;
  return key;
  //return route.source*INT_MAX + route.dest + flags;
}

void writeRoute(FileBuffer* file, RouteRequest route) {
  fwrite_char(file, route.type);
  fwrite_uint32_t(file, route.flags);
  fwrite_item(file, route.element);
  fwrite_item(file, route.source);
  fwrite_item(file, route.dest);
  fwrite_location_vector(file, &route.steps);
}

RouteRequest readRoute(FileBuffer* file, int version) {
  RouteRequest r;
  r.type = fread_char(file);
  if (version > 58 || (version == 58 && file->patchVersion >= 1)) {
    r.flags = fread_uint32_t(file);
  }
  r.element = fread_item(file, version);
  r.source = fread_item(file, version);
  r.dest = fread_item(file, version);
  fread_location_vector(file, &r.steps, version);
  if (version < 51) {
    reverse(r.steps.begin(), r.steps.end());
  }
  return r;
}

void readRouter(FileBuffer* file, int version) {
  if (version >= 48) {
    routeCache.read(file, version, readRoute);
  }
  if (version < 51) {
    routeCache.clear();
  }
}

void writeRouter(FileBuffer* file) {
  routeCache.write(file, writeRoute);
  defragmentRouteRequests();
}

