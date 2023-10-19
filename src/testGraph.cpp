#ifndef BOOST_ALL_NO_LIB
	#define BOOST_ALL_NO_LIB
#endif

#include "graph.hpp"
#include "lane.hpp"

#include <stdio.h>

static Configuration config;

void testSimpleAddRemove() {
  item in0 = addNode(vec3(0,   0, 0), config);
  item in1 = addNode(vec3(0, 200, 0), config);
  item ie = addEdge(in0, in1, config);

  Node* n0 = getNode(in0);
  Node* n1 = getNode(in1);
  if (!(n0->flags & _graphExists) || !(n1->flags & _graphExists)) {
    printf("Node created but it doesn't exist\n");
    throw "";
  }

  Edge* edge = getEdge(ie);
  for (int i=0; i<2; i++) {
    LaneBlock* b0 = getLaneBlock(edge->laneBlocks[i]);
    if (!(b0->flags & _laneExists)) {
      printf("LaneBlock doesn't exist\n");
      throw "";
    }
    if (b0->sources.size() != 1) {
      printf("LaneBlock has wrong number of sources\n");
      throw "";
    }
    if (b0->drains.size() != 1) {
      printf("LaneBlock has wrong number of drains\n");
      throw "";
    }
    if (b0->lanes[0].sources.size() != 1) {
      printf("Lane has wrong number of sources\n");
      throw "";
    }
    if (b0->lanes[0].drains.size() != 1) {
      printf("Lane has wrong number of drains\n");
      throw "";
    }
  }

  removeEdge(ie, true);
  if (edge->flags & _graphExists) {
    printf("Edge removed but it still exists\n");
    throw "";
  }
  if ((n0->flags & _graphExists) || (n1->flags & _graphExists)) {
    printf("Node has edges removed but it still exists\n");
    throw "";
  }
}

void testConnect() {
  item in[3] = {
    addNode(vec3(0,   0, 0), config),
    addNode(vec3(0, 200, 0), config),
    addNode(vec3(0, 400, 0), config),
  };

  Configuration edgeFourLaneConfig;
  edgeFourLaneConfig.numLanes = 2;
  item ie[2] = {
    addEdge(in[0], in[1], edgeFourLaneConfig),
    addEdge(in[1], in[2], edgeFourLaneConfig)
  };

  for (int i=0; i<2; i++) {
    Edge* edge = getEdge(ie[i]);

    for (int j=0; j<2; j++) {
      LaneBlock* b0 = getLaneBlock(edge->laneBlocks[j]);
      if (!(b0->flags & _laneExists)) {
        printf("LaneBlock doesn't exist\n");
        throw "";
      }
      if (b0->sources.size() != (j != i) + 1) {
        printf("LaneBlock has wrong number of sources: %d, j %d, i %d\n",
            b0->sources.size(), j, i);
        throw "";
      }
      if (b0->drains.size() != (j == i) + 1) {
        printf("LaneBlock has wrong number of drains\n");
        throw "";
      }
      if (b0->numLanes != edgeFourLaneConfig.numLanes) {
        printf("LaneBlock has wrong number of lanes\n");
        throw "";
      }

      for (int k=0; k < b0->numLanes; k++) {
        if (b0->lanes[k].sources.size() != (k==1) + (j != i)) {
          printf("Lane has wrong number of sources\n");
          throw "";
        }
        if (b0->lanes[k].drains.size() != (k==0) + (j == i)) {
          printf("Lane has wrong number of drains: %d, k %d, j %d, i %d\n",
            b0->lanes[k].drains.size(), k, j, i);
          throw "";
        }
      }
    }
  }
}

void testSplit() {
  return;
  item in[] = {
    addNode(vec3(100,   0, 10), config),
    addNode(vec3(100, 200, 10), config),
    addNode(vec3(  0, 100, 10), config),
    addNode(vec3(200, 100, 10), config)
  };

  item ie0 = addEdge(in[0], in[1], config);
  complete(ie0, config);
  item ie1 = addEdge(in[2], in[3], config);
  complete(ie1, config);

  item centerN = getEdge(getNode(in[0])->edges[0])->ends[1];
  Node* center = getNode(centerN);
  if (!(center->flags & _graphExists)) {
    printf("Node created but it doesn't exist\n");
    throw "";
  }
  if (!(center->flags & _graphComplete)) {
    printf("Node completed but it isn't complete\n");
    throw "";
  }
  if (center->edges.size() != 4) {
    printf("Node numEdges(%d) was not what was expected(%d)\n", 
      center->edges.size(), 4);
    throw "";
  }

  for (int i = 0; i < 4; i ++) {
    Node* n = getNode(in[i]);
    if (!(n->flags & _graphExists)) {
      printf("Node created but it doesn't exist\n");
      throw "";
    }
    if (!(n->flags & _graphComplete)) {
      printf("Node completed but it isn't complete\n");
      throw "";
    }
    if (n->edges.size() != 1) {
      printf("Node numEdges was not what was expected");
      throw "";
    }

    item en = n->edges[0];
    Edge* e = getEdge(en);
    if (!(e->flags & _graphExists)) {
      printf("Edge created but it doesn't exist\n");
      throw "";
    }
    if (!(e->flags & _graphComplete)) {
      printf("Edge completed but it isn't complete\n");
      throw "";
    }

    int end = e->ends[0] != in[i];
    if (e->ends[end] != in[i]) {
      printf("Edge doesn't have associated node attached");
      throw "";
    }
    if (e->ends[!end] != centerN) {
      printf("Edge doesn't have center node attached");
      throw "";
    }

    LaneBlock* b0 = getLaneBlock(e->laneBlocks[end]);
    if (!(b0->flags & _laneExists)) {
      printf("LaneBlock doesn't exist\n");
      throw "";
    }
    if (b0->sources.size() != 1) {
      printf("LaneBlock has wrong number of sources\n");
      throw "";
    }
    if (b0->drains.size() != 3) {
      printf("LaneBlock has wrong number of drains\n");
      throw "";
    }
    if (b0->lanes[0].sources.size() != 1) {
      printf("LaneBlock has wrong number of sources\n");
      throw "";
    }
    if (b0->lanes[0].drains.size() != 3) {
      printf("LaneBlock has wrong number of drains\n");
      throw "";
    }

    LaneBlock* b1 = getLaneBlock(e->laneBlocks[!end]);
    if (!(b1->flags & _laneExists)) {
      printf("LaneBlock doesn't exist\n");
      throw "";
    }
    if (b1->sources.size() != 3) {
      printf("LaneBlock has wrong number of sources\n");
      throw "";
    }
    if (b1->drains.size() != 1) {
      printf("LaneBlock has wrong number of drains\n");
      throw "";
    }
    if (b1->lanes[0].sources.size() != 3) {
      printf("Lane has wrong number of sources\n");
      throw "";
    }
    if (b1->lanes[0].drains.size() != 1) {
      printf("Lane has wrong number of drains\n");
      throw "";
    }
  }
}

void cleanUp() {
  resetGraph();
  resetLanes();
}

void testGraph() {
  testSimpleAddRemove();
  cleanUp();

  testConnect();
  cleanUp();

  testSplit();
  cleanUp();
}

