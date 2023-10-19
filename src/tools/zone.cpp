#include "../economy.hpp"
#include "../icons.hpp"
#include "../lot.hpp"
#include "../renderLot.hpp"
#include "../parts/article.hpp"
#include "../parts/chart.hpp"
#include "../parts/economyPanel.hpp"
#include "../parts/leftPanel.hpp"
#include "../parts/messageBoard.hpp"
#include "../parts/scrollbox.hpp"
#include "../zone.hpp"

enum ZoneBrushType {
  BrushEdge = 0, BrushSmall, BrushMed, BrushLarge, BrushPointer,
  numZoneBrushTypes
};

const float zoneBrushRadius[] = {
  0.0f,     // Edge placeholder
  50.0f,    // Small
  150.0f,   // Med
  500.0f,   // Large
  0.0f,     // Pointer
};

const float zoneBrushSides[] ={
  0.0f,     // Edge placeholder
  100.0f,   // Small
  300.0f,   // Med
  1000.0f,  // Large
  0.0f,     // Pointer
};

const float zoneBrushButtStart = 3.0f;
const vec2 zoneIcon = vec2(2,6);
static item zoneType = 1;
static int zoneBrush = BrushMed;
static bool zoneMouseClick = false;
const static float zoneBrushSize = tileSize*1.5;
static vector<item> highlightedLots;
static ScrollState statsScroll;
static bool densityMode = false;
static int densityValue = 10;
static bool overzoneMode = true;

const char* zoneLabel[] = {
  "Remove Zone",
  "Residential",
  "Retail",
  "Agricultural",
  "Government",
  "Office",
  "Industrial",
  "Mixed Use",
  "Parks"
};

const char* zoneDemandHint[] = {
  "No Zone",
  "Residential growth occurs when unemployment is low."
    " High property taxes stifle residential growth.",
  "Retail growth occurs when there are available unemployed workers"
    " with high school diplomas."
    " Retail will not grow if there are too many open jobs,"
    " even if unemployment is high."
    " High sales taxes also stifle retail growth.",
  "Agricultural growth occurs when there are available unemployed workers."
    " Farms do not require educated workers."
    " Agriculture will not grow if there are too many open jobs,"
    " even if unemployment is high.",
  "Government",
  "Office growth occurs when there are available unemployed workers"
    " with bachelor's degrees."
    " Offices will not grow if there are too many open jobs,"
    " even if unemployment is high.",
  "Industrial growth occurs when there are available unemployed workers"
    " with high school diplomas."
    " Industry will not grow if there are too many open jobs,"
    " even if unemployment is high.",
  "Mixed Use grows when there is demand for Retail and Residential."
    " Mixed use buildings only develop in areas with"
    " sufficient Density and Value.",
  "Parks reduce Pollution, increase Value, and create Community."
    " Parks placed near water or on top of hills are more effective."
    " Note that park zones will not develop or spawn buildings.",

};

bool toggleZoneMessage(Part* part, InputEvent event) {
  toggleMessage(ZoneDemandMessage, part->itemData);
  return true;
}

void zone_mouse_button_callback(InputEvent event) {
  /*
  // Don't allow for zoning if Tutorial is waiting for input
  TutorialState* ptr = getTutorialStatePtr();
  if (ptr != 0 && ptr->showTutorial()) {
    zoneMouseClick = false; // Otherwise it might be stuck
    return;
  }
  */

  int button = event.button;
  int action = event.action;
  bool moddedRMB = button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && (event.mods & GLFW_MOD_CONTROL);

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    zoneMouseClick = true;

  } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    zoneMouseClick = false;

  } else if (moddedRMB) {
    item sampleNdx = nearestLot(event.mouseLine, true);
    Lot* sample = getLot(sampleNdx);
    if (sample->zone != GovernmentZone) {
      zoneType = sample->zone;
      densityValue = getLotMaxDensity(sampleNdx);
    }
  }
}

static void clearHighlights() {
  for (int i=0; i < highlightedLots.size(); i++) {
    renderLot(highlightedLots[i]);
  }
  highlightedLots.clear();
}

void zone_mouse_move_callback(InputEvent event) {
  clearHighlights();

  if(zoneBrush < 0 || zoneBrush >= numZoneBrushTypes) {
    return;
  }

  if(zoneBrush == BrushEdge) {
    stopGuide();

    Configuration config;
    config.flags = 0;
    config.type = ConfigTypeRoad;
    item elem = nearestElement(event.mouseLine, false, config);
    // If no nearest element, return
    if(elem == 0) return;

    bool side = false;
    Line l = getLine(elem);
    vec3 mi = landIntersect(event.mouseLine);

    // If using edge detection, and the distance is too great, don't show
    if(pointLineDistance(mi, l) >= tileSize * 4) return;
    // Detect which side of road
    if(elem > 0) side = cross(mi - l.start, l.end - l.start).z > 0;
    // Get the lots
    highlightedLots = getLotsByElem(elem, side);

  } else if(zoneBrush == BrushPointer) {
    stopGuide();
    highlightedLots.push_back(nearestLot(event.mouseLine, true));

  } else { // Radius Brush
    vec3 mi = landIntersect(event.mouseLine);
    setGuide(mi, zoneBrushRadius[zoneBrush]);
    highlightedLots = getLotsByRad(mi, zoneBrushRadius[zoneBrush]);
  }

  if (!overzoneMode) {
    for(int i = highlightedLots.size()-1; i >= 0; i--) {
      item lotNdx = highlightedLots[i];
      Lot* lot = getLot(lotNdx);
      if (lot->zone != NoZone && lot->zone != zoneType) {
        highlightedLots.erase(highlightedLots.begin()+i);
      }
    }
  }

  if(zoneMouseClick) {
    if(!densityMode) { // Regular Zoning mode
      bool reportTutorial = false;

      for(int i = 0; i < highlightedLots.size(); i++) {
        Lot* lot = getLot(highlightedLots[i]);
        if(lot->zone != zoneType || 
          (!overzoneMode && lot->zone == NoZone)) {
          reportTutorial = true;
          playSound(_soundClickPrimary);
          break;
        }
      }

      // Report to tutorial if zoning occurred
      if (reportTutorial) {
        switch (zoneType) {
          case ZoneTypes::NoZone:
            reportTutorialUpdate(TutorialUpdateCode::ZonedNoZone);
            break;
          case ZoneTypes::ParkZone:
            reportTutorialUpdate(TutorialUpdateCode::ZonedParkZone);
            break;
          case ZoneTypes::ResidentialZone:
            reportTutorialUpdate(TutorialUpdateCode::ZonedResidential);
            break;
          case ZoneTypes::MixedUseZone:
            reportTutorialUpdate(TutorialUpdateCode::ZonedMixedUse);
            break;
          case ZoneTypes::RetailZone:
            reportTutorialUpdate(TutorialUpdateCode::ZonedRetail);
            break;
          case ZoneTypes::OfficeZone:
            reportTutorialUpdate(TutorialUpdateCode::ZonedOffice);
            break;
          case ZoneTypes::FarmZone:
            reportTutorialUpdate(TutorialUpdateCode::ZonedAgriculture);
            break;
          case ZoneTypes::FactoryZone:
            reportTutorialUpdate(TutorialUpdateCode::ZonedFactory);
            break;
          case ZoneTypes::GovernmentZone:
            reportTutorialUpdate(TutorialUpdateCode::ZonedGovernment);
            break;
        }
      }

      zoneLots(highlightedLots, zoneType, densityValue);
    } else { // Density mode
      for(int i = 0; i < highlightedLots.size(); i++) {
        if(getLotMaxDensity(highlightedLots[i]) != densityValue) {
          playSound(_soundClickPrimary);
          break;
        }
      }
      changeDensityLots(highlightedLots, densityValue);
    }
  }

  if(!densityMode) { // When zoning, render lots as desired zone type
    for(int i = 0; i < highlightedLots.size(); i++) {
      renderLotAs(highlightedLots[i], zoneType, densityValue, true);
    }
  } else { // When changing density, render lot as the lot's zone type
    for(int i = 0; i < highlightedLots.size(); i++) {
      Lot* lot = getLot(highlightedLots[i]);
      renderLotAs(highlightedLots[i], lot->zone, densityValue, true);
    }
  }
}

void zone_select() {
  zoneMouseClick = false;
}

void zone_reset() {
  zoneMouseClick = false;
  clearHighlights();
  stopGuide();
}

bool setZoneType(Part* part, InputEvent event) {
  int nextZone = part->itemData;
  stopBlinkingFeature(FZoneResidential+zoneType-1);
  stopBlinkingFeature(FZoneResidential+nextZone-1);
  zoneType = nextZone;
  return true;
}

bool setZoneBrush(Part* part, InputEvent event) {
  int brush = part->itemData;
  if(brush < 0 || brush >= numZoneBrushTypes) return false;

  zoneBrush = brush;
  if(zoneBrush == BrushEdge || zoneBrush == BrushPointer) {
    stopGuide();
  }
  return true;
}

bool toggleDensityMode(Part* part, InputEvent event) {
    //(event.action == GLFW_PRESS && event.key == GLFW_KEY_B)) {
  densityMode = !densityMode;
  return true;
}

bool openCitipediaZonePage(Part* part, InputEvent event) {
  item zone = part->itemData;
  string article = zoneCode[zone];
  article = "zones/" + article;
  followLink(strdup_s(article.c_str()));
  return true;
}


bool getOverzoneMode() {
  return overzoneMode;
}

bool toggleOverzone(Part* part, InputEvent event) {
  overzoneMode = !overzoneMode;
  return true;
}

void setDensityValue(int val) {
  if(val < lotMinDensity) {
    densityValue = lotMinDensity;
    return;
  } 

  if(val > lotMaxDensity) {
    densityValue = lotMaxDensity;
    return;
  }

  densityValue = val;
}

bool zone_density_slider_callback(Part* part, InputEvent event) {
  float result = part->vecData.x;
  result = int(result*10.0f);
  setDensityValue(result);
  return true;
}

Part* zone_render(Line dim) {

  // X value for left aligned buttons like econ and msg
  float leftX = 0.125f;
  float zonePad = 0.125f;

  Part* result = panel(dim);
  r(result, label(vec2(0,0), 1, strdup_s("Zone")));

  // Zone brush buttons
  for(int b = 0; b < numZoneBrushTypes; b++) {
    Part* butt = button(vec2(zoneBrushButtStart + b, 0),
        iconZoneBrushType[b], setZoneBrush);
    butt->itemData = b;
    butt->inputAction = (InputAction)(ActZTBrushLine+b);
    setPartTooltipValues(butt,
      TooltipType::ZoneBrushEdge+b);

    if(zoneBrush == b) {
      butt->flags |= _partHighlight;
    }

    r(result, butt);
  }

  //r(result, hr(vec2(0,1), dim.end.x-toolPad*2));

  if(!densityMode) { // Regular zoning mode
    for(int i=0; i < numZoneTypes-1; i++) {
      int z = zoneOrder[i];
      if(i != 0 && !isFeatureEnabled(FZoneResidential+z-1)) continue;
      vec2 loc = i == 0 ? vec2(0.f, 5.f) :
        i == 1 ? vec2(0.f, 3.75f) :
        vec2(i*1.25f - .75f, 2.f);
      vec2 size = i <= 1 ? vec2(1.25f, 1.25f) : vec2(1.25f, 4.25f);

      Part* buttContainer = panel(loc, size);
      buttContainer->padding = zonePad;
      buttContainer->renderMode = RenderTransparent;
      buttContainer->itemData = z;
      buttContainer->inputAction = (InputAction)(ActZTNoZone + z);
      buttContainer->onClick = setZoneType;
      buttContainer->flags |= _partHover;
      setPartTooltipValues(buttContainer,
        TooltipType::ZoneDezone+z);

      if(z == zoneType) {
        buttContainer->flags |= _partHighlight;
      }
      if(blinkFeature(FZoneResidential+z-1)) {
        buttContainer->flags |= _partBlink;
      }

      r(result, buttContainer);

      // Creates the demand chart for a zone type
      // First vec2 is location, second vec2 is size
      if(i > 1) {
        float demand = zoneDemandSoft(z);
        if (demand > 0) demand = pow(demand, 0.5);
        r(buttContainer, icon(vec2(0, 3.f),
          vec2(1, -demand*2.8f), iconZoneColor[z]));
      }

      Part* butt = button(vec2(0, i <= 1 ? 0 : 3), iconZone[z], setZoneType);
      butt->itemData = z;
      r(buttContainer, butt);

      KeyBind bind = getKeyBind(buttContainer->inputAction);
      if (bind.key != GLFW_KEY_UNKNOWN) {
        butt->text = strdup_s(getKeyStr(bind.key).c_str());
      }

      if(z == zoneType) {
        if(z != NoZone) {
          float lblx = i <= 1 ? 0.75 : i;
          r(result, label(vec2(lblx, 6.25f),
            vec2(i <= 1 ? 2.f : 5.f, 0.75f), strdup_s(zoneLabel[zoneType])));
          r(result, button(vec2(lblx-0.75f, 6.25f), iconCitipedia, vec2(.75f, .75f), openCitipediaZonePage, z));

        } else {
          r(result, label(vec2(0.f, 6.25f),
            0.75f, strdup_s(zoneLabel[zoneType])));
        }
      }
    }

    r(result, labelRight(vec2(6.f, 1.2f), vec2(4.f, 0.85f),
          strdup_s("Demand")));
    r(result, label(vec2(9.f, 1.875f), 0.75f, strdup_s("-Hi")));
    r(result, label(vec2(9.f, 4.75f), 0.75f, strdup_s("-Lo")));

  } else { // Density Control mode
    float densVal = densityValue/10.0f;
    float slideX = leftX+2.0f+zonePad;
    float slideW = dim.end.x-slideX-zonePad-1;
    float slideY = 3.35f;
    Part* densSlider = slider(vec2(slideX, slideY),
      vec2(slideW, 1.0f), densVal, zone_density_slider_callback);
    r(result, densSlider);
    r(result, icon(vec2(slideX-1, slideY), vec2(1,1), iconHouse));
    r(result, icon(vec2(slideX+slideW, slideY), vec2(1,1),
          iconZoneMono[OfficeZone]));
    r(result, labelRight(vec2(6.f, 2.35f), vec2(4.f, 0.85f),
          sprintf_o("Level %d", densityValue)));
    r(result, labelRight(vec2(6.f, 1.2f), vec2(4.f, 0.85f),
          strdup_s("Density Control")));
  }

  // Density control button
  Part* densButt = panel(vec2(leftX, 2.35f), vec2(1, 1.35f));
  densButt->renderMode = RenderTransparent;
  densButt->flags |= _partHover;
  densButt->onClick = toggleDensityMode;
  //densButt->onKeyDown = toggleDensityMode;
  setPartTooltipValues(densButt, TooltipType::ZoneDensity);
  if(densityMode) densButt->flags |= _partHighlight;

  Part* ico = r(densButt, icon(vec2(0, 0), iconApartment));
  //ico->text = strdup_s("B");
  r(densButt, labelCenter(vec2(0, .65f), vec2(1-zonePad, 0.75),
        sprintf_o("%d", densityValue)));
  r(result, densButt);

  // Overzone Toggle
  Part* overButt = button(vec2(2, 0), iconOverzone,
    vec2(1.0f, 1.0f), toggleOverzone, 0);
  overButt->inputAction = ActZTOverzoneMode;
  setPartTooltipValues(overButt,
    TooltipType::ZoneOverzone);
  if (overzoneMode) overButt->flags |= _partHighlight;
  r(result, overButt);

  // Message button
  Part* zoneButt = button(vec2(leftX, 1.25f), iconPin, toggleZoneMessage);
  setPartTooltipValues(zoneButt,
    TooltipType::GenMsg);
  if (hasMessage(ZoneDemandMessage, 0)) {
    zoneButt->flags |= _partHighlight;
  }
  r(result, zoneButt);

  /*
  // Economy button
  if (isFeatureEnabled(FEconomyPanel)) {
    Part* econButt = button(vec2(leftX,1.25f), iconChart, toggleEconomyPanel);
    setPartTooltipValues(econButt,
      TooltipType::GenGraphs);
    if (getLeftPanel() == EconomyPanel) {
      econButt->flags |= _partHighlight;
    }
    r(result, econButt);
  }
  */

  return result;
}

void zoneInstructionPanel(Part* panel) {
  if (zoneType == ParkZone) {
    r(panel, label(vec2(0,0), .85, strdup_s("Parks")));
  } else {
    r(panel, label(vec2(0,0), .85,
          sprintf_o("Factors in %s Demand", zoneName[zoneType])));
  }

  vector<Statistic> stats;
  bool isBiz = zoneType != ResidentialZone && zoneType != ParkZone;

  if (zoneType == ResidentialZone || zoneType == MixedUseZone) {
    stats.push_back(EmptyHomes);
    stats.push_back(PropertyTaxRateStat);
    stats.push_back(UnemploymentRate);
  }

  if (zoneType == RetailZone || zoneType == MixedUseZone) {
    stats.push_back(EmptyShops);
    stats.push_back(PercentRetail);
    stats.push_back(SalesTaxRateStat);

  } else if (zoneType == OfficeZone) {
    stats.push_back(EmptyOffices);

  } else if (zoneType == FarmZone) {
    stats.push_back(EmptyFarms);

  } else if (zoneType == FactoryZone) {
    stats.push_back(EmptyFactories);
  }

  if (isBiz) {
    EducationLevel edu = zoneType == OfficeZone ? BachelorsDegree :
      zoneType == FarmZone ? NoEducation : HSDiploma;
    //for (int j = edu; j < numEducationLevels; j++) {
      int j = edu;
      stats.push_back((Statistic)(NoEduUnemployment + j));
      stats.push_back((Statistic)(NumOpenNoEduPositions + j));
      //stats.push_back((Statistic)(NumNoEduPositions + j));
    //}

    stats.push_back(ProsperityStat);
  }

  stats.push_back(AverageTripTime);

  float width = panel->dim.end.x - panel->padding*2;
  float height = panel->dim.end.y - 1 - panel->padding*2;
  float scl = 1.3f;
  float y = 0;
  float xSize = width-1;
  item timePeriod = TimePeriods::Period5Y;
  Part* statsPanel = scrollbox(vec2(0,0), vec2(width, height));

  r(statsPanel, multiline(vec2(0,0), vec2(xSize, 0.75f),
        strdup_s(zoneDemandHint[zoneType]), &y));

  for (int i = 0; i < stats.size(); i++) {
    item statItem = stats[i]*10 + timePeriod;

    Part* pin = r(statsPanel, button(vec2(xSize-1, y),
          iconPin, toggleChartMessage));
    pin->dim.start.z += 15;
    pin->dim.end.z += 15;
    pin->itemData = statItem;
    if (hasMessage(ChartMessage, statItem) ||
        hasMessage(ChartMessage, -statItem)) {
      pin->flags |= _partHighlight;
    }

    Part* chrt = chart(vec2(0,y), vec2(xSize, scl),
      ourCityEconNdx(), stats[i], timePeriod, true, true);
    chrt->onClick = selectChart;
    chrt->itemData = statItem;
    r(statsPanel, chrt);

    y += scl + .1f;
  }

  float dy;
  r(statsPanel, multiline(vec2(0,y), vec2(xSize, 0.75f), sprintf_o(
      "Note: People will not travel more than %d hours for work."
      " Find traffic problems using the Traffic view.",
      int(c(CMaxCommute))), &dy));
  y += dy;

  r(panel, scrollboxFrame(vec2(0,1),
      vec2(width, height), &statsScroll, statsPanel));
}

std::string zone_name() {
  return "Zones";
}

bool zone_visible() {
  return true;
  //return numCompleteEdges() >= 5;
}

Tool toolZone = {
  zone_mouse_button_callback,
  zone_mouse_move_callback,
  zone_select,
  zone_reset,
  zone_visible,
  iconHouse,
  zone_render,
  zoneInstructionPanel,
  zone_name,
};

Tool* getToolZone() {
  return &toolZone;
}

