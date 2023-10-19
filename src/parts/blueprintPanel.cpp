#include "blueprintPanel.hpp"

#include "../color.hpp"
#include "../game/feature.hpp"
#include "../icons.hpp"
#include "../string_proxy.hpp"
#include "../zone.hpp"

#include "button.hpp"
#include "icon.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "textBox.hpp"

bool deleteBlueprint(Part* part, InputEvent event) {
  deleteBlueprint(part->itemData);
  return true;
}

Part* blueprintPanel(vec2 start, vec2 size, Blueprint* bp,
    TextBoxState* tb, item ndx) {
  float pad = size.y*.1;
  float scal = size.y*.5f - pad;
  Part* result = panel(start, size);
  result->padding = pad;
  result->renderMode = RenderPanelGradient;
  if (ndx == selectedBlueprint()) {
    result->texture = line(colorBlueGrad1, colorBlueGrad0);
  } else {
    result->texture = line(colorBlueGrad0, colorBlueGrad1);
  }
  tb->text = &bp->name;
  r(result, textBoxLabel(vec2(0,0), vec2(size.x-pad*2, scal), tb));
  if (ndx >= 0) {
    r(result, button(vec2(size.x-pad*2-scal,scal), iconTrash, vec2(scal,scal),
          deleteBlueprint, ndx));
  }

  float x = 0;
  int numExpwy = 0;
  int numRoads = 0;
  int zones[9] = {0,0,0,0,0,0,0,0,0};

  for (int i = 0; i < bp->nodes.size(); i++) {
    BlueprintNode n = bp->nodes[i];
    zones[n.zone] ++;
  }

  for (int i = 0; i < bp->edges.size(); i++) {
    BlueprintEdge e = bp->edges[i];
    if (e.config.type == ConfigTypeRoad) {
      numRoads ++;
      zones[e.zone[0]] ++;
      zones[e.zone[1]] ++;
    } else if (e.config.type == ConfigTypeExpressway) {
      numExpwy ++;
    }
  }

  r(result, label(vec2(x, scal), scal,
        printMoneyString(bp->cost * getInflation())));
  x += scal*2.5;

  if (numExpwy > 0) {
    r(result, icon(vec2(x, scal), vec2(scal, scal), iconExpressway));
    x += scal;
    r(result, label(vec2(x,scal), scal, sprintf_o("%d", numExpwy)));
    x += scal*1.5;
  }

  if (numRoads > 0) {
    r(result, icon(vec2(x, scal), vec2(scal, scal), iconRoad));
    x += scal;
    r(result, label(vec2(x,scal), scal, sprintf_o("%d", numRoads)));
    x += scal*1.5;
  }

  int numZones = 0;
  for (int i = 1; i < 8; i++) {
    int z = zoneOrder[i];
    if (!isFeatureEnabled(FZoneResidential+z-1)) continue;
    if (zones[z] > 0) {
      r(result, icon(vec2(x+1-numZones*.5f, scal+.1f-numZones*.1f),
            vec2(scal, scal), iconZone[z]));
      numZones ++;
    }
  }

  return result;
}

