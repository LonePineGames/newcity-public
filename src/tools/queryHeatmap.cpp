#include "../color.hpp"
#include "../draw/camera.hpp"
#include "../draw/entity.hpp"
#include "../draw/mesh.hpp"
#include "../draw/shader.hpp"
#include "../draw/texture.hpp"
#include "../error.hpp"
#include "../game/feature.hpp"
#include "../game/game.hpp"
#include "../graph/stop.hpp"
#include "../graph/transit.hpp"
#include "../heatmap.hpp"
#include "../label.hpp"
#include "../lot.hpp"
#include "../option.hpp"
#include "../person.hpp"
#include "../renderGraph.hpp"
#include "../renderUtils.hpp"
#include "../vehicle/renderVehicle.hpp"
#include "../selection.hpp"
#include "../sound.hpp"
#include "../zone.hpp"

#include "../parts/article.hpp"
#include "../parts/block.hpp"
#include "../parts/root.hpp"
#include "../parts/slider.hpp"
#include "../parts/textBox.hpp"
#include "../parts/tooltip.hpp"

#include "query.hpp"
#include "tool.hpp"

#include "spdlog/spdlog.h"

static item lastHeatmap = -1;
static bool wasHeatmapSelected = false;
static bool showHeatmapInfoPanel = true;
static item hmCursor;
static item iconEntityNdx;

const HeatMapIndex heatmapOrder[] = {
  Pollution, Crime, Education, Prosperity, Value, Density,
  CommunityHM, HealthHM, 
  TrafficHeatMap, TransitHeatMap, ZoneHeatMap, RoadHeatMap,
};

const char* const heatmapLabels[] = {
  "Pollution", // X
  "Value", // V
  "Density", // F
  "Crime", // C
  "Education", // G
  "Prosperity", // J
  "Community", // N
  "Health", // H
  "Traffic", // T
  "Transit", // T
  "Zones", // Z
  "Road Map" // M
};

const char* heatmapButtonKeyLabels[] =
  {"X", "V", "F", "C", "G", "J", "N", "H", "T", "T", "Z", "M"};
const int heatmapButtonKeys[] = {
  GLFW_KEY_X,
  GLFW_KEY_V,
  GLFW_KEY_F,
  GLFW_KEY_C,
  GLFW_KEY_G,
  GLFW_KEY_J,
  GLFW_KEY_N,
  GLFW_KEY_H,
  GLFW_KEY_T,
  GLFW_KEY_T,
  GLFW_KEY_Z,
  GLFW_KEY_M,
  0,0,0,0,0,
};

const char* const heatmapMaxLabel[] = {
  "Toxic",
  "High Value",
  "City Center",
  "High Crime",
  "College Educated",
  "Prosperous",
  "Welcoming",
  "Healthy",
  "Slow Traffic",
  "Transit Line",
};

const char* const heatmapMidLabel[] = {
  "Polluted",
  "",
  "",
  "Some Crime",
  "High School",
  "",
  "",
  "",
  "",
  "",
};

const char* const heatmapMinLabel[] = {
  "Clean",
  "Low Value",
  "Rural",
  "No Crime",
  "No Education",
  "Abandoned",
  "Isolated",
  "Sick",
  "No Traffic",
  "Roadway",
};

const char* const heatmapCitipediaPage[] = {
  "heatmaps/hmPollution",
  "heatmaps/hmValue",
  "heatmaps/hmDensity",
  "heatmaps/hmCrime",
  "heatmaps/hmEducation",
  "heatmaps/hmProsperity",
  "heatmaps/hmCommunity",
  "heatmaps/hmHealth",
  "infoviews/ivTraffic",
  "infoviews/ivTransit",
  "infoviews/ivZones",
  "infoviews/ivRoadmap",
};

const char* const heatmapInfo[] = {
  "Pollution is caused by farms, factories, and traffic. "
    "Parks counteract pollution. "
    "Pollution inhibits density and land value. ",
  "Land value measures the desirability of an area. "
    "High land value areas are likely to densify. ",
  "Density measures the people and businesses in an area. ",
  "Crime is caused by economic stress such as unemployment. "
    "Crime inhibits density and land value. ",
  "Education increases land value, reduces crime, and allows factories and "
    "offices. Education is neccessary for the city to advance.",
  "Prosperity measures the economic activity of an area. "
    "Economic stress (such as unemployment) reduces prosperity and can lead "
    "to the neighborhood being abandoned.",
  "Community represents the strength of the community bonds in your city. "
  "It is increased by recreation, agriculture, social amenities, and mass "
  "transit.",
  "Health represents the health and well-being of the citizens. "
  "It is increased by recreational amenities like parks, social services, "
  "low crime, and low pollution.",
  "Transportation is the key to Prosperity. Bad traffic can prevent people "
    "from getting to jobs, shopping, and amenities.",
  "To build transit lines, select the road tool (2), then the transit tab (T)",
  "When re-zoning, existing buildings will not be destroyed, but they "
    "will be rebuilt over time.",
  "Expressways are shown in orange. Major roads are white."
   " Other roads are gray. Train lines are red."
};

bool openIssueLegend(Part* part, InputEvent event);
bool setHeatMap(Part* part, InputEvent event);
void setQueryInfoForce();

bool isQueryHeatmapSet() {
  return pinHeatmaps() && wasHeatmapSelected;
}

void heatmap_mouse_button_callback(InputEvent event) {
}

void allocHeatmapCursorEntity() {
  if (hmCursor != 0) return;

  hmCursor = addEntity(PaletteShader);
  iconEntityNdx = addEntity(WSUIShader);

  Entity* cursorEntity = getEntity(hmCursor);
  cursorEntity->texture = paletteTexture;
  setEntityVisible(hmCursor, true);
  //setEntityBringToFront(hmCursor, true);
  setEntityTransparent(hmCursor, false);
  setEntityHighlight(hmCursor, true);
  createMeshForEntity(hmCursor);

  Entity* iconEntity = getEntity(iconEntityNdx);
  iconEntity->texture = iconTexture;
  setEntityVisible(iconEntityNdx, true);
  //setEntityBringToFront(iconEntityNdx, true);
  setEntityTransparent(iconEntityNdx, false);
  setEntityHighlight(iconEntityNdx, true);
  createMeshForEntity(iconEntityNdx);

  Mesh* mesh = getMeshForEntity(hmCursor);
  makeCone(mesh, vec3(0,0,1), vec3(0,0,-1), 1, colorTransparentWhite, true);
  bufferMesh(cursorEntity->mesh);
}

void deallocHeatmapCursorEntity() {
  if (hmCursor == 0) return;

  removeEntity(hmCursor);
  removeEntity(iconEntityNdx);
  hmCursor = 0;
  iconEntityNdx = 0;
}

void makeIcon(Mesh* mesh, vec3 ico, vec3 center, float size) {
  vec3 right = vec3(size, 0, 0);
  vec3 down = vec3(0, size, 0);
  vec3 topLeft = -.5f*(right+down) + center;
  Line icoLine = iconToSpritesheet(ico);

  makeQuad(mesh, topLeft, topLeft+right,
      topLeft+down, topLeft+right+down, icoLine.start, icoLine.end);
}

void heatmap_mouse_move_callback(InputEvent event) {
  if (!isHeatMapIntense() || getHeatMap() < 0) {
    deallocHeatmapCursorEntity();

  } else {
    allocHeatmapCursorEntity();

    vec3 loc = landIntersect(event.mouseLine);
    if (loc.z < beachLine) loc.z = beachLine;
    float hmVal = heatMapGet(getHeatMap(), loc);
    int hmValI = round(hmVal*10);
    hmValI = clamp(hmValI, 0, 10);
    float dist = getCameraDistance();
    float size = dist/15;
    vec3 z = vec3(0,0,size*2);
    vec3 down = vec3(0,size,0);
    vec3 right = vec3(size,0,0);
    vec3 xShadowColor = iconColorDarkGray/spriteSheetSize;

    Mesh* mesh = getMeshForEntity(iconEntityNdx);
    makeQuad(mesh,
        -down*3.4f+z, -down*3.4f+right*1.2f+z, -down, -down+right*1.2f+z,
        xShadowColor, xShadowColor);

    makeIcon(mesh, iconHeatmap[getHeatMap()],
        vec3(-size*.5f, -size*2.f, size*2.f)+z, size*2.f);

    for (int i = 0; i < hmValI; i ++) {
      vec3 cloc = vec3(i%2, i/-2, 0);
      cloc += vec3(1., -1., 0);
      cloc *= size*.4f;
      cloc.y -= size;
      cloc.z = size;
      cloc += z;
      makeIcon(mesh, iconHeatmapColor[getHeatMap()], cloc, size*.4f);
    }

    bufferMesh(getEntity(iconEntityNdx)->mesh);
    placeEntity(hmCursor, loc, 0, 0, size);
    placeEntity(iconEntityNdx, loc, 0, 0);
  }
}

void heatmap_select() {
  if (wasHeatmapSelected) {
    setHeatMap((HeatMapIndex)lastHeatmap, true);
  }
  setIssuesIconsVisible();
  setQueryInfoForce();
}

void heatmap_reset() {
  if (!pinHeatmaps()) {
    setHeatMap(Pollution, false);
    setIssuesIconsVisible();
  }
  setQueryInfoForce();
  deallocHeatmapCursorEntity();
}

bool togglePinQuery(Part* part, InputEvent event) {
  setPinHeatmaps(!pinHeatmaps());
  return true;
}

bool openCitipediaHeatmapPage(Part* part, InputEvent event) {
  item hm = part->itemData;
  followLink(strdup_s(heatmapCitipediaPage[hm]));
  return true;
}

Part* heatmap_render(Line dim) {
  Part* result = panel(dim);

  r(result, label(vec2(0,0), 1, strdup_s("Heatmaps")));
  //r(result, hr(vec2(0,1.0), dim.end.x-toolPad*2));

  // Pin Button
  Part* pinButt = r(result, button(vec2(6.67,0), iconPin, togglePinQuery));
  if (pinHeatmaps()) {
    pinButt->flags |= _partHighlight;
  }

  // Heatmaps
  for (int i = 0; i <= numHeatMaps+3; i++) {
    int hm = heatmapOrder[i];
    int hmn = hm == TrafficHeatMap ? numHeatMaps :
      hm == TransitHeatMap ? numHeatMaps+1 :
      hm == ZoneHeatMap ? numHeatMaps+2 :
      hm == RoadHeatMap ? numHeatMaps+3 :
      hm;
    if (hmn < numHeatMaps &&
        !isFeatureEnabled(featureForHeatmap(hm))) continue;
    vec2 loc = i < 6 ? vec2(0.f, (i)*.8f + 1) :
                       vec2(5.f, (i-6)*.8f + 1);

    Part* buttContainer = panel(loc, vec2(4.9f, 0.8f));
    buttContainer->renderMode = RenderTransparent;
    buttContainer->itemData = hm;
    buttContainer->inputAction = (InputAction)(ActHeatmapPollution+i);
    buttContainer->onClick = setHeatMap;
    buttContainer->flags |= _partHover;
    setPartTooltipValues(buttContainer,
      TooltipType::QuePollu+i);
    if (getHeatMap() == hm && isHeatMapIntense()) {
      buttContainer->flags |= _partHighlight;
    }
    if (blinkFeature(featureForHeatmap(hm))) {
      buttContainer->flags |= _partBlink;
    }
    r(result, buttContainer);

    if (i < 6) {
      r(buttContainer, label(vec2(1.f, 0.125f), 0.7f,
            strdup_s(heatmapLabels[hmn])));
    } else {
      r(buttContainer, labelRight(vec2(0.f, 0.125f), vec2(3.95f, 0.7f),
            strdup_s(heatmapLabels[hmn])));
    }

    vec3 hmIcon =
      hm == TrafficHeatMap ? iconCar :
      hm == TransitHeatMap ? iconBus :
      hm == ZoneHeatMap ? iconHouse :
      hm == RoadHeatMap ? iconRoad :
      iconHeatmap[hm];
    Part* ico = icon(vec2(i<6?0.f:3.95f, 0.f), vec2(0.85f,0.85f), hmIcon);
    KeyBind bind = getKeyBind(buttContainer->inputAction);
    if (bind.key != GLFW_KEY_UNKNOWN) {
      ico->text = strdup_s(getKeyStr(bind.key).c_str());
    }
    r(buttContainer, ico);
  }

  return result;
}

void issueLegendPanel(Part* panel, float x, float y) {
  // Let's build the Issue Icon Legend
  float initY = y;
  float textSize = 0.8f;
  float stagger = 1.0f;
  float txtYAdj = 0.1;
  float lgndPad = 0.2;

  // Header
  //r(panel, label(vec2(x, y), 1.0f, strdup_s("Issue Icon Legend")));
  //r(panel, hr(vec2(0, 1.1f), panel->dim.end.x - toolPad * 4));
  //y += 2.0f;

  // Icons and labels
  Part* ico = r(panel, icon(vec2(x, y), iconWorker));
  ico->foregroundColor = PickerPalette::RedLight;
  r(panel, label(vec2(x + stagger-lgndPad, y+txtYAdj), textSize, strdup_s("Unemployed")));
  y += textSize-0.1;

  ico = r(panel, icon(vec2(x+stagger, y), iconHelpWanted));
  ico->foregroundColor = PickerPalette::RedLight;
  r(panel, label(vec2(x+stagger*2-lgndPad, y+txtYAdj), textSize, strdup_s("Needs Workers")));
  y += textSize-0.1;

  ico = r(panel, icon(vec2(x, y), iconFood));
  ico->foregroundColor = PickerPalette::RedLight;
  r(panel, label(vec2(x + stagger-lgndPad, y+txtYAdj), textSize, strdup_s("Can't Reach Retail")));
  y = initY;
  x = x + (panel->dim.end.x - x)*.5f-.5f;

  ico = r(panel, icon(vec2(x, y), iconTrade));
  ico->foregroundColor = PickerPalette::RedLight;
  r(panel, label(vec2(x+stagger-lgndPad,y+txtYAdj), textSize, strdup_s("Needs Customers")));
  y += textSize-0.1;

  ico = r(panel, icon(vec2(x+stagger, y), iconTruck));
  ico->foregroundColor = PickerPalette::RedLight;
  r(panel, label(vec2(x + stagger*2-lgndPad, y+txtYAdj), textSize, strdup_s("Needs Freight")));
  y += textSize-0.1;

  ico = r(panel, icon(vec2(x+stagger*2, y), iconSick));
  ico->foregroundColor = PickerPalette::RedLight;
  r(panel, label(vec2(x + stagger*3-lgndPad, y+txtYAdj), textSize, strdup_s("Sick")));
}

void routeInspectorInstructionPanel(Part* result);
void queryInstructionPanel(Part* panel) {
  if (isQueryTool()) {
    if (currentQueryTool == QueryInnerTool) {
      r(panel, label(vec2(1, 2), .85, strdup_s(
        "Left click on any object to\n"
        "learn about it.")));
      return;

    } else if (currentQueryTool == HeatmapTool &&
        !isHeatMapIntense()) {
      r(panel, label(vec2(1, 2), .85, strdup_s(
        "Select a Heatmap to\n"
        "observe the state of\n"
        "your city.\n")));
      return;

    } else if (currentQueryTool == RouteTool) {
      routeInspectorInstructionPanel(panel);
      return;

    } else if (currentQueryTool == LabelTool) {
      r(panel, label(vec2(1, 2), .85, strdup_s(
        "Left click anywhere to\n"
        "place a label.\n"
        "Then type text and\n"
        "press enter to finish.\n")));
      return;
    }
  }

  if (isHeatMapIntense()) {
    int hm = getHeatMap();
    int hmn = hm == TrafficHeatMap ? numHeatMaps :
      hm == TransitHeatMap ? numHeatMaps+1 :
      hm == ZoneHeatMap ? numHeatMaps+2 :
      hm == RoadHeatMap ? numHeatMaps+3 :
      hm;

    float sliderX = 2;
    float sliderLabelSizeX = 5;
    float multilineWidth = 10;
    float multilineTextSize = 0.8f;

    if (hmn == Prosperity) {
      sliderX = 1;
      sliderLabelSizeX = 3;
      multilineWidth = 9;
      multilineTextSize = 0.75f;
    }

    if (hmn <= numHeatMaps) {
      Line heatmapGrad = getHeatmapGradient(hm);
      r(panel, gradientBlock(vec2(sliderX,.9), vec2(1,4.9),
        heatmapGrad.end, heatmapGrad.start));
      r(panel, labelCenter(vec2(0,0), vec2(sliderLabelSizeX, .85),
        strdup_s(heatmapMaxLabel[hmn])));
      r(panel, labelCenter(vec2(0,5.9), vec2(sliderLabelSizeX, .85),
        strdup_s(heatmapMinLabel[hmn])));
      if (hmn == Education) {
        r(panel, labelCenter(vec2(0,3), vec2(sliderX, .85),
          strdup_s("HS")));
      }
    }

    float y = 0;
    Part* info = multiline(vec2(3.5,0),
        vec2(multilineWidth,multilineTextSize),
        strdup_s(heatmapInfo[hmn]), &y);
    if (hmn != Prosperity) {
      info->dim.start.y = 3.5 - y*.5;
    }
    r(panel, info);

    if (hmn == Prosperity) {
      issueLegendPanel(panel, sliderX+1.5, 3.5);
    }

    r(panel, button(vec2(8.75f, 5.9f), iconCitipedia, vec2(4.75f, 0.85f), strdup_s("Learn More"), openCitipediaHeatmapPage, hmn));
  }
}

bool isShowHeatmapInfoPanel() {
  return showHeatmapInfoPanel;
}

bool setHeatMap(Part* part, InputEvent event) {
  if (part == NULL) {
    SPDLOG_ERROR("Null part in setHeatMap");
    return false;
  }

  HeatMapIndex ndx = (HeatMapIndex)part->itemData;
  int hmn = ndx == TrafficHeatMap ? numHeatMaps :
    ndx == TransitHeatMap ? numHeatMaps+1 :
    ndx == ZoneHeatMap ? numHeatMaps+2 :
    ndx == RoadHeatMap ? numHeatMaps+3 :
    ndx;

  if (getHeatMap() == ndx && isHeatMapIntense()) {
    setHeatMap(Pollution, false);
    wasHeatmapSelected = false;
  } else {
    setHeatMap(ndx, true);
    wasHeatmapSelected = true;
    lastHeatmap = ndx;

    // If no tool is open and Query is enabled, 
    // open the Query tool to the Heatmap subtool
    if(getCurrentTool() == 0 && isFeatureEnabled(FQueryTool) && !pinHeatmaps()) {
      setTool(1);
      setQuerySubTool(QuerySubTools::HeatmapTool);
    }
  }
  setQueryInfoForce();
  stopBlinkingFeature(featureForHeatmap(ndx));
  return true;
}

void toggleQueryInfoPanel() {
  if (isHeatMapIntense()) {
    showHeatmapInfoPanel = !showHeatmapInfoPanel;

  } else {
    setShowInstructions(!getShowInstructions());
  }

  setQueryInfoForce();
}

bool heatmap_visible() {
  return true;
}

Tool toolHeatmap = {
  heatmap_mouse_button_callback,
  heatmap_mouse_move_callback,
  heatmap_select,
  heatmap_reset,
  heatmap_visible,
  iconGrid,
  heatmap_render,
  queryInstructionPanel,
};

