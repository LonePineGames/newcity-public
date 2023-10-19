#include "router.hpp"

#include <stdio.h>

/*
 *  1------2--------3
 *        /        /
 *       /        /
 *      4--------5
 */
static const int num = 5;
static float sampleGraph[num][num] = {
// 1  2  3  4  5
  {0, 1, 0, 0, 0}, //1
  {1, 0, 1, 1, 0}, //2
  {0, 1, 0, 0, 2}, //3
  {0, 1, 0, 0, 0}, //4
  {0, 0, 2, 0, 0}, //5
};

static Cup<item> getDrains(item j) {
  Cup<item> result;
  for(int i=1; i <= num; i++) {
    if (sampleGraph[i-1][j-1] > 0) {
      result.push_back(i);
    }
  }
  return result;
}

float getDistance(item a, item b) {
  return sampleGraph[a-1][b-1];
}

float getEstimate(item a, item b) {
  return 0;
}

static item noop(item e) {
  return e;
}

void testRoutingGeneric() {
  Router router;
  router.size = num;
  router.start = 1;
  router.end = 5;

  router.getDrains = getDrains;
  router.getDistance = getDistance;
  router.getEstimate = getEstimate;
  router.encode = noop;
  router.decode = noop;

  vector<item> result = route(router);
  if (result.size() != 4) {
    printf("wrong result size %d\n", result.size());
    throw "";
  }
}

void testRouting() {
  printf("testing routing\n");
  testRoutingGeneric();
  cleanUp();
}

