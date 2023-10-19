#include "../draw/camera.hpp"
#include "../error.hpp"
#include "../game/feature.hpp"
#include "../game/game.hpp"
#include "../graph/stop.hpp"
#include "../graph/transit.hpp"
#include "../heatmap.hpp"
#include "../label.hpp"
#include "../lot.hpp"
#include "../person.hpp"
#include "../renderGraph.hpp"
#include "../vehicle/renderVehicle.hpp"
#include "../selection.hpp"
#include "../sound.hpp"
#include "../zone.hpp"

#include "../parts/block.hpp"
#include "../parts/root.hpp"
#include "../parts/slider.hpp"
#include "../parts/textBox.hpp"
#include "../parts/tooltip.hpp"

#include "tool.hpp"

#include "spdlog/spdlog.h"

enum QuerySubTools {
  QueryInnerTool, HeatmapTool, RouteTool, LabelTool,
  numQuerySubTools
};

const Feature subToolFeature[] = {
  FQueryTool, FHeatmaps, FRouteInspector, FLabel,
};

item currentQueryTool = 0;

void setQueryInfoForce(bool reset);

#include "queryHeatmap.cpp"
#include "queryInner.cpp"
#include "queryLabel.cpp"
#include "queryRoutes.cpp"

Tool* querySubTools[numQuerySubTools] = {
  &toolQueryInner,
  &toolHeatmap,
  &toolRoute,
  &toolLabel,
};

void query_mouse_button_callback(InputEvent event) {
  querySubTools[currentQueryTool]->mouse_button_callback(event);
}

void query_mouse_move_callback(InputEvent event) {
  querySubTools[currentQueryTool]->mouse_move_callback(event);
}

void query_select() {
  querySubTools[currentQueryTool]->select();
  setQueryInfoForce(false);
}

void query_reset() {
  querySubTools[currentQueryTool]->reset();
  setQueryInfoForce(true);
}

void setQuerySubTool(item tool) {
  querySubTools[currentQueryTool]->reset();
  currentQueryTool = tool;
  stopBlinkingFeature(subToolFeature[tool]);
  querySubTools[currentQueryTool]->select();
  setQueryInfoForce(false);
  if (currentQueryTool == RouteTool) {
    reportTutorialUpdate(SelectedRouteInspector);
  }
}

bool toggleQueryTool(Part* part, InputEvent event) {
  setQuerySubTool(part->itemData);
  return true;
}

Part* query_render(Line dim) {
  Part* result = panel(dim);

  Line innerDim = line(vec3(0,0,0), dim.end);
  innerDim.end.x -= 0.2;
  innerDim.end.y -= 1.2;
  r(result, querySubTools[currentQueryTool]->render(innerDim));

  Part* tabPanel = panel(vec2(0,6), vec2(10,1));
  tabPanel->flags |= _partLowered;
  r(result, tabPanel);

  for (int i = 0; i < numQuerySubTools; i++) {
    if (!isFeatureEnabled(subToolFeature[i])) continue;
    Part* butt = button(vec2(i, 0.f), querySubTools[i]->icon,
        toggleQueryTool, i);
    butt->inputAction = (InputAction)(((int)ActQuerySubtool+i));

    bool blink = blinkFeature(subToolFeature[i]);
    if (!blink && i == HeatmapTool) {
      for (int j = 0; j < numHeatMaps; j++) {
        if (blinkFeature(featureForHeatmap(j))) blink = true;
      }
    }

    if (blink) {
      butt->flags |= _partBlink;
    }

    if (i == currentQueryTool) {
      butt->flags |= _partHighlight;
    }
    r(tabPanel, butt);
  }

  return result;
}

std::string query_name() {
  return "Query";
}

bool query_visible() {
  return true;
  //#ifdef LP_DEBUG
    //return true;
  //#else
    //return getGameMode() == ModeTest || numPeople() > 1000;
  //#endif
}

Tool toolQuery = {
  query_mouse_button_callback,
  query_mouse_move_callback,
  query_select,
  query_reset,
  query_visible,
  iconQuery,
  query_render,
  queryInstructionPanel,
  query_name,
};

void setQueryInfoForce(bool reset) {
  if (isHeatMapIntense() && showHeatmapInfoPanel) {
    toolQuery.flags |= _toolForceInstructions;
  } else if (!reset && currentQueryTool == RouteTool) {
    toolQuery.flags |= _toolForceInstructions;
  } else {
    toolQuery.flags &= ~_toolForceInstructions;
  }
}

bool isRouteTool() {
  return isQueryTool() && currentQueryTool == RouteTool;
}

void setQueryInfoForce() {
  setQueryInfoForce(false);
}

Tool* getToolQuery() {
  return &toolQuery;
}

