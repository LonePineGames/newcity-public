#include "elevation.hpp"

#include "../game/feature.hpp"
#include "../graph.hpp"
#include "../heatmap.hpp"
#include "../icons.hpp"
#include "../land.hpp"
#include "../option.hpp"
#include "../string_proxy.hpp"
#include "../tutorial.hpp"
#include "../util.hpp"

#include "../parts/button.hpp"
#include "../parts/hr.hpp"
#include "../parts/icon.hpp"
#include "../parts/label.hpp"
#include "../parts/panel.hpp"
#include "../parts/toolbar.hpp"
#include "../parts/tooltip.hpp"

#include "spdlog/spdlog.h"

Configuration elevate(Elevation elevation, Configuration config) {
  int el = elevation.zOffset << _configElevationOffset;
  el = el & _configElevationMask;
  config.flags &= ~_configElevationMask;
  config.flags |= el;

  if (!elevation.moveEarth && elevation.zOffset != 0) {
    config.flags |= _configDontMoveEarth;
  } else {
    config.flags &= ~_configDontMoveEarth;
  }

  return config;
}

void elevationSelect(bool select, Elevation* elevation) {
  bool viz = select && elevation->zOffset < 0 && !elevation->moveEarth
    || getHeatMap() == TransitHeatMap; 
  setUndergroundView(viz);
  setUndergroundViewSelected(isUndergroundView());
}

bool elevationUp(Part* part, InputEvent event) {
  Elevation* elevation = (Elevation*) part->ptrData;
  elevation->zOffset ++;
  if(elevation->zOffset > maxZOffset) {
    elevation->zOffset = maxZOffset;
  }
  stopBlinkingFeature(FRoadElevation);
  elevationSelect(true, elevation);
  reportTutorialUpdate(ChangeElevationInTT);
  return true;
}

bool elevationDown(Part* part, InputEvent event) {
  Elevation* elevation = (Elevation*) part->ptrData;
  elevation->zOffset --;
  if(elevation->zOffset < minZOffset) {
    elevation->zOffset = minZOffset;
  }
  stopBlinkingFeature(FRoadElevation);
  elevationSelect(true, elevation);
  reportTutorialUpdate(ChangeElevationInTT);
  return true;
}

bool toggleViaductTunnel(Part* part, InputEvent event) {
  Elevation* elevation = (Elevation*) part->ptrData;
  elevation->moveEarth = !elevation->moveEarth;
  elevationSelect(true, elevation);
  reportTutorialUpdate(SelectedTunnelViaductInTT);
  return true;
}

Part* elevationWidget(vec2 loc, Elevation* elevation) {
  Part* result = panel(loc, vec2(2.5,3.875));
  result->padding = 0.125f;
  result->flags |= _partLowered;
  //result->renderMode = RenderTransparent;
  item z = elevation->zOffset;
  if (z != 0 || isFeatureEnabled(FRoadElevation)) {

    Part* buttUp = button(vec2(0,0), iconUp, elevationUp);
    buttUp->inputAction = ActTTRaiseElevation;
    buttUp->itemData = 1;
    buttUp->ptrData = elevation;
    buttUp->flags &= ~_partFreePtr;
    setPartTooltipValues(buttUp,
      TooltipType::RoadRaiseEle);
    if (z > 0) buttUp->flags |= _partHighlight;
    if (blinkFeature(FRoadElevation)) buttUp->flags |= _partBlink;
    r(result, buttUp);

    r(result, labelCenter(vec2(0.f,1.f), vec2(2.5f,.85f), sprintf_o("%s%dm", z == 0 ? " " : z < 0 ? "" : "+", int(z*c(CZTileSize)))));
    r(result, labelCenter(vec2(0.f,1.65f), vec2(2.5f,.6f), strdup_s(z < 0 ? "Blw Grnd" : "Abv Grnd")));
    r(result, labelCenter(vec2(0.f,2.f), vec2(2.5f,.6f), strdup_s(
            (z <  0 &&  elevation->moveEarth) ? "Trench" :
            (z <  0 && !elevation->moveEarth) ? "Tunnel" :
            (z == 0                         ) ? "Surface" :
            (z >= 0 &&  elevation->moveEarth) ? "Embankment" :
            (z >= 0 && !elevation->moveEarth) ? "Viaduct" : "Err")));


    Part* buttDown = button(vec2(0,2.5), iconDown, elevationDown);
    buttDown->inputAction = ActTTLowerElevation;
    buttDown->itemData = -1;
    buttDown->ptrData = elevation;
    buttDown->flags &= ~_partFreePtr;
    setPartTooltipValues(buttDown,
      TooltipType::RoadLowerEle);
    if (z < 0) buttDown->flags |= _partHighlight;
    if (blinkFeature(FRoadElevation)) buttDown->flags |= _partBlink;
    r(result, buttDown);

    for (int i = 0; i < 2; i ++) {
      Part* buttEarth = button(vec2(1.25,i*2.5), i ? iconUnderground : iconCauseway,
          toggleViaductTunnel);
      buttEarth->inputAction = ActTTTunnelMode;
      buttEarth->itemData = 0;
      buttEarth->ptrData = elevation;
      buttEarth->flags &= ~_partFreePtr;
      setPartTooltipValues(buttEarth,
        i == 0 ? TooltipType::RoadViaduct : TooltipType::RoadTunnel);
      if (!(elevation->moveEarth) && (z==0 || ((z<=0) == i))) {
        buttEarth->flags |= _partHighlight;
      }
      r(result, buttEarth);
    }
  }

  return result;
}

