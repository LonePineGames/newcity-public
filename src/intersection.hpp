#ifndef INTERSECTION_H
#define INTERSECTION_H

#include "item.hpp"
#include "graph.hpp"

vector<item> getNodeOutputs(item ndx, bool completed);
vector<item> getNodeInputs(item ndx, bool completed);
vector<item> getCompletedEdges(item ndx);
vector<item> getRenderableEdges(item ndx);
void setStopLightPhaseTexture(item nodeNdx);
int getOddManOut(item nodeNdx, vector<item> edges);
void rebuildIntersection(item nodeNdx);
void spawnVehicle(item nodeNdx, bool force);
void updateIntersections(double duration);

void swapIntersections_v(bool fromSave);
void updateIntersections_v(float duration);

#endif
