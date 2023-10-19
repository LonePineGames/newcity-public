#include "blueprint.hpp"

#include "draw/camera.hpp"
#include "game/feature.hpp"
#include "game/game.hpp"
#include "land.hpp"
#include "lot.hpp"
#include "plan.hpp"
#include "platform/file.hpp"
#include "tools/blueprint.hpp"
#include "util.hpp"

#include "spdlog/spdlog.h"

#include <set>
#include <unordered_map>

const int blueprintVersion = 0;
const float maxBPSize = tileSize*125;

const int bitMask12 = 4095;
const int bitMask8  =  255;
const int bitMask4  =   15;
const int bitMask2  =    3;

static Blueprint draftBlueprint;
static Blueprint bufferBlueprint;
static vector<Blueprint> blueprints;
static vector<TextBoxState> bpTBs;
static item selectedBlueprintNdx;
static vec3 nextZonesLoc;
static Blueprint nextZonesBP;
static bool applyBPZones = false;

const int configTableLength = 10;
const Configuration configTable[configTableLength] = {
  {1, 0, ConfigTypeRoad, StopSignStrategy, 0},
  {2, 1, ConfigTypeRoad, TrafficLightStrategy, 0},
  {3, 2, ConfigTypeRoad, TrafficLightStrategy, _configMedian},
  {2, 1, ConfigTypeRoad, StopSignStrategy, _configOneWay},
  {4, 1, ConfigTypeRoad, TrafficLightStrategy, _configOneWay},
  {1, 2, ConfigTypeExpressway, JointStrategy, _configOneWay},
  {2, 3, ConfigTypeExpressway, JointStrategy, _configOneWay},
  {3, 3, ConfigTypeExpressway, JointStrategy, _configOneWay},
  {4, 3, ConfigTypeExpressway, JointStrategy, _configOneWay},
  {5, 3, ConfigTypeExpressway, JointStrategy, _configOneWay},
};

int getConfigIndex(Configuration config) {
  int lanes = config.numLanes;
  if (config.type == ConfigTypeExpressway) {
    lanes = (lanes < 1) ? 1 : (lanes > 5 ? 5 : lanes);
    return lanes + 4;

  } else if (config.flags & _configOneWay) {
    return (lanes <= 2) ? 3 : 4;

  } else {
    lanes = (lanes < 1) ? 1 : (lanes > 3 ? 3 : lanes);
    return lanes - 1;
  }
}

Line clampBPLine(Line l) {
  if (l.end.x > l.start.x) {
    l.end.x = l.start.x + std::min(l.end.x - l.start.x, maxBPSize*2);
  } else {
    l.end.x = l.start.x + std::max(l.end.x - l.start.x, -maxBPSize*2);
  }
  if (l.end.y > l.start.y) {
    l.end.y = l.start.y + std::min(l.end.y - l.start.y, maxBPSize*2);
  } else {
    l.end.y = l.start.y + std::max(l.end.y - l.start.y, -maxBPSize*2);
  }
  float z = std::max(std::max(l.start.z, l.end.z), beachLine);
  l.start.z = z;
  l.end.z = z;
  return l;
}

vector<item> getBPElems(Line l) {
  Box b = alignedBox(l);
  vector<item> elems = getGraphCollisions(b);
  set<item> result;
  vec3 center = .5f*(l.start + l.end);

  // Test edges for validity, add nodes
  for (int i = 0; i < elems.size(); i++) {
    item ndx = elems[i];
    if (ndx <= 0) continue;
    Edge* e = getEdge(ndx);
    if (!(e->flags & _graphExists)) continue;
    bool valid = true;

    for (int i = 0; i < 2; i++) {
      Node* n = getNode(e->ends[i]);
      if (!(n->flags & _graphExists)) {
        valid = false;
        break;
      }
      if (boxDistance(b, n->center) > tileSize*.5f) {
      //if (abs(n->center.x-center.x) > maxBPSize ||
          //abs(n->center.y-center.y) > maxBPSize) {
        valid = false;
        break;
      }
    }

    if (valid) {
      result.insert(e->ends[0]);
      result.insert(e->ends[1]);
      result.insert(ndx);
    }
  }

  return vector<item>(result.begin(), result.end());
}

void clearBlueprint(Blueprint* bp) {
  if (bp->name != 0) {
    free(bp->name);
  }
  bp->name = strdup_s("New Blueprint");
  bp->flags = _blueprintFine;
  bp->edges.clear();
  bp->nodes.clear();
}

void clearDraftBlueprint() {
  clearBlueprint(&draftBlueprint);
}

void clearBufferBlueprint() {
  clearBlueprint(&bufferBlueprint);
}

Blueprint* getDraftBlueprint() {
  return &draftBlueprint;
}

Blueprint* getBufferBlueprint() {
  return &bufferBlueprint;
}

Blueprint* getActiveBlueprint() {
  if (bufferBlueprint.flags & _blueprintExists) {
    return &bufferBlueprint;
  } else {
    return &draftBlueprint;
  }
}

item saveBlueprint(Blueprint* bp) {
  item ndx = blueprints.size();
  bp->flags |= _blueprintComplete;
  blueprints.push_back(*bp);
  bp->name = strdup_s(bp->name);
  TextBoxState tb;
  bpTBs.push_back(tb);
  selectedBlueprintNdx = ndx;
  return ndx;
}

item saveDraftBlueprint() {
  return saveBlueprint(&draftBlueprint);
}

void computeCost(Blueprint* bp) {
  bp->cost = 0;
  for (int i = 0; i < bp->edges.size(); i++) {
    BlueprintEdge e = bp->edges[i];
    BlueprintNode n0 = bp->nodes[e.ends[0]];
    BlueprintNode n1 = bp->nodes[e.ends[1]];
    bp->cost += edgeCost(n0.loc, n1.loc, e.config, n0.config, n1.config);
  }
}

vector<item> setBufferBlueprint(vector<item> graphElems, vector<item> lots) {
  Blueprint bp;
  bp.flags = _blueprintFine | _blueprintExists;
  bp.nodes.clear();
  bp.edges.clear();

  vec3 mass = vec3(0,0,0);
  int numNodes = 0;
  unordered_map<item, item> elemIndexTable;

  for (int i = 0; i < graphElems.size(); i++) {
    item ndx = graphElems[i];
    if (ndx >= 0) continue;
    Node* n = getNode(ndx);
    mass += n->center;
    elemIndexTable[ndx] = -numNodes-1;
    numNodes ++;

    if ((bp.flags & _blueprintFine) &&
        n->config.type == ConfigTypeRoad) {
      bp.flags &= ~_blueprintFine;
    }
  }

  vec3 center = mass / float(numNodes);
  float alignmentModulo = (bp.flags & _blueprintFine) ?
    tileSize : tileSize*5;
  vec3 alignment = vec3(
      round(center.x/alignmentModulo)*alignmentModulo,
      round(center.y/alignmentModulo)*alignmentModulo,
      0);

  for (int i = 0; i < graphElems.size(); i++) {
    item ndx = graphElems[i];

    if (ndx > 0) {
      Edge* e = getEdge(ndx);
      BlueprintEdge bpEdge;
      bpEdge.flags = 0;
      bpEdge.config = e->config;
      bpEdge.zone[0] = 0;
      bpEdge.zone[1] = 0;
      for (int j = 0; j < 2; j++) {
        item nodeNdx = elemIndexTable[e->ends[j]];
        bpEdge.ends[j] = -nodeNdx-1;
      }
      elemIndexTable[ndx] = bp.edges.size()+1;
      bp.edges.push_back(bpEdge);

    } else if (ndx < 0) {
      Node* n = getNode(ndx);
      BlueprintNode bpNode;
      bpNode.flags = n->pillar == 0 ? 0 : _blueprintNodePillar;
      bpNode.loc = n->center - alignment;
      float z = n->center.z - pointOnLandNatural(n->center).z -
        c(CRoadRise)*2;
      z = round(z/c(CZTileSize))*c(CZTileSize);
      bpNode.loc.z = z;
      bpNode.config = n->config;
      bpNode.zone = 0;
      bp.nodes.push_back(bpNode);
    }
  }

  vector<item> resultLots;
  for (int i = 0; i < lots.size(); i++) {
    item lotNdx = lots[i];
    Lot* lot = getLot(lotNdx);
    if (lot->zone == 0) continue;
    resultLots.push_back(lotNdx);
    item elemNdx = elemIndexTable[lot->elem];
    if (elemNdx < 0) {
      bp.nodes[-elemNdx-1].zone = lot->zone;
    } else if (elemNdx > 0) {
      bool side = getLotSide(lotNdx);
      bp.edges[elemNdx-1].zone[side] = lot->zone;
    }
  }

  if (bufferBlueprint.name) free(bufferBlueprint.name);
  bufferBlueprint = bp;
  bufferBlueprint.name = strdup_s("New Blueprint");
  bufferBlueprint.flags |= _blueprintExists;
  computeCost(&bufferBlueprint);
  return resultLots;
}

void setDraftBlueprintToBuffer() {
  if (!(bufferBlueprint.flags & _blueprintExists)) return;
  if (bufferBlueprint.nodes.size() == 0) return;
  if (bufferBlueprint.edges.size() == 0) return;

  selectedBlueprintNdx = -1;
  char* name = draftBlueprint.name;
  if (name == 0) name = strdup_s("New Blueprint");
  draftBlueprint = bufferBlueprint;
  draftBlueprint.name = name;
}

void rotateDraftBlueprint() {
  for (int i = 0; i < draftBlueprint.nodes.size(); i++) {
    BlueprintNode* n = &draftBlueprint.nodes[i];
    vec3 loc = n->loc;
    loc = vec3(-loc.y, loc.x, loc.z);
    n->loc = loc;
  }
}

void flipDraftBlueprint() {
  Camera cam = getMainCamera();
  bool axis = abs(cam.direction.y) > abs(cam.direction.x);
  float x = axis ? -1 : 1;
  float y = axis ? 1 : -1;

  for (int i = 0; i < draftBlueprint.nodes.size(); i++) {
    BlueprintNode* n = &draftBlueprint.nodes[i];
    vec3 loc = n->loc;
    loc = vec3(x*loc.x, y*loc.y, loc.z);
    n->loc = loc;
  }

  for (int i = 0; i < draftBlueprint.edges.size(); i++) {
    BlueprintEdge* e = &draftBlueprint.edges[i];
    char swap = e->zone[0];
    e->zone[0] = e->zone[1];
    e->zone[1] = swap;
  }
}

void applyDraftBlueprint(vec3 loc) {
  if (!(draftBlueprint.flags & _blueprintExists)) return;
  unordered_map<item, item> nodeIndexTable;
  for (int i = 0; i < draftBlueprint.nodes.size(); i++) {
    BlueprintNode n = draftBlueprint.nodes[i];
    vec3 nLoc = loc + n.loc;
    nLoc = pointOnLandNatural(nLoc);
    nLoc.z += n.loc.z + loc.z;
    nodeIndexTable[i] = getOrCreateNodeAt(nLoc, n.config);
  }

  for (int i = 0; i < draftBlueprint.edges.size(); i++) {
    BlueprintEdge e = draftBlueprint.edges[i];
    item n0 = nodeIndexTable[e.ends[0]];
    item n1 = nodeIndexTable[e.ends[1]];
    if (n0 != n1) {
      item eNdx = addEdge(n0, n1, e.config);
      addPlan(GraphElementPlan, eNdx);
      split(eNdx);
    }
  }
}

Box blueprintBound(Blueprint* bp, vec3 loc) {
  float ms = getMapSize();
  float minX = ms;
  float maxX = 0;
  float minY = ms;
  float maxY = 0;

  for (int i = 0; i < bp->nodes.size(); i++) {
    vec3 center = bp->nodes[i].loc;
    if (center.x < minX) minX = center.x;
    if (center.x > maxX) maxX = center.x;
    if (center.y < minY) minY = center.y;
    if (center.y > maxY) maxY = center.y;
  }

  minX -= tileSize*2;
  minY -= tileSize*2;
  maxX += tileSize*2;
  maxY += tileSize*2;

  return alignedBox(line(vec3(minX,minY,0)+loc, vec3(maxX,maxY,0)+loc));
}

void applyBlueprintZones(Blueprint bp, vec3 loc) {
  vector<Line> edgeLines;
  vector<vec3> nodeLocs;
  vector<item> lots = collidingLots(blueprintBound(&bp, loc));
  SPDLOG_INFO("applyBlueprintZones {}", lots.size());
  if (lots.size() == 0) return;

  for (int i = 0; i < bp.nodes.size(); i++) {
    nodeLocs.push_back(bp.nodes[i].loc + loc);
  }

  for (int i = 0; i < bp.edges.size(); i++) {
    BlueprintEdge e = bp.edges[i];
    edgeLines.push_back(line(nodeLocs[e.ends[0]], nodeLocs[e.ends[1]]));
  }

  for (int i = 0; i < lots.size(); i++) {
    item lotNdx = lots[i];
    Lot* lot = getLot(lotNdx);
    item best = 0;
    item bestDist = tileSize*tileSize*4;

    if (lot->elem < 0) {
      for (int j = 0; j < bp.nodes.size(); j ++) {
        float dist = distance2DSqrd(nodeLocs[j], lot->loc);
        if (dist < bestDist) {
          best = -j-1;
          bestDist = dist;
        }
      }
    }

    for (int j = 0; j < bp.edges.size(); j ++) {
      float dist = pointLineDistance2DSqrd(lot->loc, edgeLines[j]);
      if (dist < bestDist) {
        best = j+1;
        bestDist = dist;
      }
    }

    item zone = 0;
    if (best < 0) {
      zone = bp.nodes[-best-1].zone;

    } else if (best > 0) {
      item eNdx = best-1;
      Line l = edgeLines[eNdx];
      vec3 start = l.start;
      vec3 along = l.end - start;
      bool side = cross(lot->loc - start, along).z > 0;
      zone = bp.edges[eNdx].zone[side];
    }

    if (zone != 0 && isFeatureEnabled(FZoneResidential+zone-1)) {
      zoneLot(lotNdx, zone, true); // Force overzone with blueprints
    }
  }
}

void queueBlueprintZones(Blueprint* bp, vec3 loc) {
  nextZonesLoc = loc;
  nextZonesBP = *bp;
  applyBPZones = true;
}

void applyBlueprintZones() {
  if (!applyBPZones) return;
  //if (getGameMode() != ModeGame) return;

  applyBPZones = false;
  applyBlueprintZones(nextZonesBP, nextZonesLoc);
}

item numBlueprints() {
  return blueprints.size();
}

Blueprint* getBlueprint(item ndx) {
  return &blueprints[ndx];
}

TextBoxState* getBlueprintTB(item ndx) {
  return &bpTBs[ndx];
}

void selectBlueprint(item ndx) {
  endBPSelectionMode();
  clearDraftBlueprint();
  if (draftBlueprint.name != 0) {
    free(draftBlueprint.name);
  }
  selectedBlueprintNdx = ndx;
  draftBlueprint = blueprints[ndx];
  if (draftBlueprint.name != 0) {
    draftBlueprint.name = strdup_s(draftBlueprint.name);
  }
}

item selectedBlueprint() {
  return selectedBlueprintNdx;
}

void deleteBlueprint(item ndx) {
  blueprints.erase(blueprints.begin() + ndx);
  bpTBs.erase(bpTBs.begin() + ndx);
  writeBlueprints();
}

void writeBlueprint(FileBuffer* buffer, Blueprint* bp) {
  if (bp->edges.size() > bitMask12 || bp->nodes.size() > bitMask12) return;

  fwrite_char(buffer, '\n');
  fwrite_char(buffer, '[');
  fwrite_string_no_term(buffer, bp->name);
  fwrite_char(buffer, ']');
  FileBuffer* bpBuf = makeFileBufferPtr();

  int numNodes = bp->nodes.size();
  int numEdges = bp->edges.size();
  unsigned int header = 0;
  header |= blueprintVersion << 24;
  header |= numNodes << 12;
  header |= numEdges <<  0;
  fwrite_int(bpBuf, header);

  for (int i = 0; i < numNodes; i++) {
    BlueprintNode n = bp->nodes[i];
    unsigned int nodeInfo = 0;
    int x = round(n.loc.x / tileSize);
    int y = round(n.loc.y / tileSize);
    int z = round(n.loc.z / c(CZTileSize)) + 7;
    int p = n.flags & _blueprintNodePillar;
    int f = getConfigIndex(n.config);
    nodeInfo |= (x & bitMask8) << 24;
    nodeInfo |= (y & bitMask8) << 16;
    nodeInfo |= (z & bitMask4) << 12;
    nodeInfo |= (p & bitMask2) << 10;
    nodeInfo |= (f & bitMask8) <<  0;
    fwrite_int(bpBuf, nodeInfo);
  }

  for (int i = 0; i < numEdges; i++) {
    BlueprintEdge e = bp->edges[i];
    unsigned int edgeInfo = 0;
    int n0 = e.ends[0];
    int n1 = e.ends[1];
    int f  = getConfigIndex(e.config);
    edgeInfo |= (n0 & bitMask12) << 20;
    edgeInfo |= (n1 & bitMask12) <<  8;
    edgeInfo |= (f  & bitMask8 ) <<  0;
    fwrite_int(bpBuf, edgeInfo);
  }

  for (int i = 0; i < numEdges; i++) {
    char z0 = bp->edges[i].zone[0];
    char z1 = bp->edges[i].zone[1];
    char z = 0;
    z |= (z0 & bitMask4) << 4;
    z |= (z1 & bitMask4) << 0;
    fwrite_char(bpBuf, z);
  }

  for (int i = 0; i < (numNodes+1)/2; i++) {
    char z0 = bp->nodes[i*2].zone;
    char z1 = i*2+1 >= numNodes ? 0 : bp->nodes[i*2+1].zone;
    char z = 0;
    z |= (z0 & bitMask4) << 4;
    z |= (z1 & bitMask4) << 0;
    fwrite_char(bpBuf, z);
  }

  char* bpStr = bufferToBase64(bpBuf);
  freeBuffer(bpBuf);
  fwrite_string_no_term(buffer, bpStr);
  free(bpStr);
  fwrite_char(buffer, '\n');
}

Blueprint readBlueprint(FileBuffer* buffer) {
  Blueprint result;
  result.flags = 0;

  if (buffer->cursor >= buffer->length) return result;
  char f = fread_char(buffer);
  while(isspace(f)) {
    if (buffer->cursor >= buffer->length) return result;
    f = fread_char(buffer);
  }
  if (f != '[') return result;

  char* name = fread_string_until(buffer, ']');
  if (buffer->cursor >= buffer->length) return result;
  char* bpStr = fread_string_until_whitespace(buffer);
  FileBuffer* bpBuf = base64ToBuffer(bpStr);
  if (bpBuf == 0) {
    SPDLOG_WARN("Bad Blueprint; base64 {}", bpStr);
    return result;
  }
  free(bpStr);

  if (bpBuf->cursor >= bpBuf->length) return result;
  int flags = _blueprintExists | _blueprintComplete | _blueprintFine;
  unsigned int header = fread_int(bpBuf);
  unsigned int version  = (header >> 24) & bitMask8;
  unsigned int numNodes = (header >> 12) & bitMask12;
  unsigned int numEdges = (header >>  0) & bitMask12;
  int dataLength = 1 + numEdges*5 + numNodes*4 + (numNodes+1)/2;
  if (numNodes <= 0 || numEdges <= 0 || bpBuf->length < dataLength) {
    SPDLOG_WARN("Bad Blueprint; nodes:{} edges:{} data:{}/{}",
        numNodes, numEdges, bpBuf->length, dataLength);
    freeBuffer(bpBuf);
    return result;
  }

  for (int i = 0; i < numNodes; i++) {
    unsigned int nodeInfo = fread_int(bpBuf);
    signed char x = (nodeInfo >> 24) & bitMask8;
    signed char y = (nodeInfo >> 16) & bitMask8;
    signed char z = (nodeInfo >> 12) & bitMask4;
    int         p = (nodeInfo >> 10) & bitMask2; // Pillar
    //int       u = (nodeInfo >>  8) & bitMask2; // Unused
    int         f = (nodeInfo >>  0) & bitMask8; // Config

    if (f >= configTableLength) {
      SPDLOG_WARN("Bad Blueprint; node config:{}", f);
      freeBuffer(bpBuf);
      return result;
    }

    BlueprintNode n;
    n.loc = vec3(x*tileSize, y*tileSize, (z-7)*c(CZTileSize));
    n.flags = p ? _blueprintNodePillar : 0;
    n.config = configTable[f];
    result.nodes.push_back(n);
    if (n.config.type == ConfigTypeRoad) {
      flags &= ~_blueprintFine;
    }
  }

  for (int i = 0; i < numEdges; i++) {
    unsigned int edgeInfo = fread_int(bpBuf);
    int n0 = (edgeInfo >> 20) & bitMask12;
    int n1 = (edgeInfo >>  8) & bitMask12;
    int f  = (edgeInfo >>  0) & bitMask8;

    if (f >= configTableLength || n0 >= numNodes || n1 >= numNodes) {
      SPDLOG_WARN("Bad Blueprint; edge config:{} n0:{} n1:{}", f);
      freeBuffer(bpBuf);
      return result;
    }

    BlueprintEdge e;
    e.ends[0] = n0;
    e.ends[1] = n1;
    e.config = configTable[f];
    result.edges.push_back(e);
  }

  for (int i = 0; i < numEdges; i++) {
    char z = fread_char(bpBuf);
    char z0 = (z >> 4) & bitMask4;
    char z1 = (z >> 0) & bitMask4;
    result.edges[i].zone[0] = z0;
    result.edges[i].zone[1] = z1;
  }

  for (int i = 0; i < (numNodes+1)/2; i++) {
    char z = fread_char(bpBuf);
    char z0 = (z >> 4) & bitMask4;
    char z1 = (z >> 0) & bitMask4;
    result.nodes[i*2].zone = z0;
    if (i*2+1 < result.nodes.size()) {
      result.nodes[i*2+1].zone = z1;
    }
  }

  result.flags = flags;
  result.name = name;
  computeCost(&result);
  freeBuffer(bpBuf);
  return result;
}

char* blueprintToString(Blueprint* bp) {
  FileBuffer* buffer = makeFileBufferPtr();
  writeBlueprint(buffer, bp);
  fwrite_char(buffer, '\0');
  char* result = strdup_s(buffer->data);
  freeBuffer(buffer);
  return result;
}

void writeBlueprintLibrary(FileBuffer* buffer) {
  for (int i = 0; i < blueprints.size(); i++) {
    writeBlueprint(buffer, &blueprints[i]);
  }
}

char* blueprintLibraryToString() {
  FileBuffer* buffer = makeFileBufferPtr();
  writeBlueprintLibrary(buffer);
  fwrite_char(buffer, '\0');
  char* result = strdup_s(buffer->data);
  freeBuffer(buffer);
  return result;
}

void readBlueprints(FileBuffer* buffer) {
  while(buffer->cursor < buffer->length) {
    Blueprint bp = readBlueprint(buffer);
    if (!(bp.flags & _blueprintExists)) continue;
    saveBlueprint(&bp);
    free(bp.name);
  }
}

void readBlueprints(const char* src) {
  FileBuffer* buffer = bufferFromString(src);
  readBlueprints(buffer);
  freeBuffer(buffer);
}

void writeBlueprints() {
  FILE *fileHandle;
  const char* filename = "blueprints.txt";
  if (fileHandle = fopen(filename, "wb")) {
    FileBuffer buf = makeFileBuffer();
    FileBuffer* buffer = &buf;
    SPDLOG_INFO("Writing blueprints file, version {}", blueprintVersion);
    fwrite_string_no_term(buffer,
        "Blueprints: Names are editable. Copy and Paste to share. Empty and invalid lines are ignored.\n\n");

    writeBlueprintLibrary(buffer);

    writeToFile(buffer, fileHandle);
    fclose(fileHandle);
    freeBuffer(buffer);
  }
}

void resetBlueprints() {
  blueprints.clear();
  clearDraftBlueprint();
  clearBufferBlueprint();
  for (int i = 0; i < blueprints.size(); i++) {
    Blueprint* bp = &blueprints[i];
    if (bp->name != 0) {
      free(bp->name);
    }
  }
  blueprints.clear();
  bpTBs.clear();
  selectedBlueprintNdx = 0;
  applyBPZones = false;
}

void readBlueprints() {
  resetBlueprints();

  const char* filename = fileExists("blueprints.txt") ? "blueprints.txt" : "official_blueprints.txt";
  FileBuffer buf = readFromFile(filename);
  FileBuffer* buffer = &buf;

  if (buffer->length > 0) {
    SPDLOG_INFO("Reading blueprints file, version {}", blueprintVersion);

    while (buffer->cursor < buffer->length) {
      char f = fread_char(buffer);
      if (f == '\n' || f == '\r' || f == '\f' || f == '\v') {
        break;
      }
    }

    readBlueprints(buffer);
    freeBuffer(buffer);
  }
}

