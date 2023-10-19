#ifndef RENDER_LAND_H
#define RENDER_LAND_H

#include "item.hpp"
#include "line.hpp"
#include "land.hpp"

bool getTreesVisible();
void setTreesVisible(bool visible);
void renderSeemHider();
item numRenderChunksQueued();
//void renderChunk(ChunkIndex ndx);
void queueRenderChunk(ChunkIndex ndx);
void queueRenderChunk(RenderChunkIndex ndx);
void startRenderLoop();
void stopRenderLoop();
void renderLandQueue();
void renderLandStep();
void resetLandRender();
void clearTileDataCache();

#endif
