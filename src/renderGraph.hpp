#ifndef RENDER_GRAPH_H
#define RENDER_GRAPH_H

#include "item.hpp"
#include "draw/mesh.hpp"

void renderStopLights(item ndx, int* stopLightTextureIndex);
void renderNode(item ndx);
void renderEdge(item ndx);
void renderStopDisc(Mesh* m, item ndx, vec3 offset);
void renderLane(Mesh* mesh, item laneNdx, float start, float end,
    float width, item laneOffset, vec3 offset, vec3 tx);
void renderLane(Mesh* mesh, item laneNdx, float start, float end);
void renderLane(Mesh* mesh, item laneNdx);
void renderLaneBlock(Mesh* mesh, item ndx);
void setElementHighlight(item ndx, bool highlight);

#endif
