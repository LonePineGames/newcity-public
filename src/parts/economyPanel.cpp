#include "economyPanel.hpp"

#include "../amenity.hpp"
#include "../board.hpp"
#include "../business.hpp"
#include "../color.hpp"
#include "../economy.hpp"
#include "../game/achievement.hpp"
#include "../game/feature.hpp"
#include "../heatmap.hpp"
#include "../icons.hpp"
#include "../person.hpp"
#include "../selection.hpp"
#include "../string_proxy.hpp"
#include "../tutorial.hpp"
#include "../vehicle/vehicle.hpp"
#include "../zone.hpp"

#include "amenityInfo.hpp"
#include "blank.hpp"
#include "button.hpp"
#include "chart.hpp"
#include "hr.hpp"
#include "icon.hpp"
#include "label.hpp"
#include "leftPanel.hpp"
#include "messageBoard.hpp"
#include "panel.hpp"
#include "scrollbox.hpp"
#include "textBox.hpp"

static ScrollState achievementsScroll, effectsScroll;

enum EconomyPanelTab {
  CityStatsTab, AchievementsTab, EffectsTab,
  numEconomyTabs
};

const vec3 economyTabIcon[numEconomyTabs] = {
  iconChart, iconCheck, iconBuildingCategory[2]
};

static EconomyPanelTab economyTab = CityStatsTab;
static const int numCharts = 6;
static Statistic charts[numCharts] = {
  Population, (Statistic)-1, (Statistic)-1,
  (Statistic)-1, (Statistic)-1, (Statistic)-1};
static StatisticCategory chartCategories[numCharts] = {
  PopulationStats, (StatisticCategory)-1, (StatisticCategory)-1,
  (StatisticCategory)-1, (StatisticCategory)-1, (StatisticCategory)-1};
static item chartEcon[numCharts] = {2,2,2,2,2,2};
static bool wasSearch[numCharts] = {false,false,false,false,false,false};
static ScrollState chartScroll[numCharts];
static const float chartSize = 8.5;
static const float chartWidth = 12;
static bool epExpanded = false;
static int timePeriod = 2;

static int searchChart = -1;
static TextBoxState searchChartTBState;
static char* searchChartText = 0;

bool toggleEconomyPanel(Part* part, InputEvent event) {
  if (getLeftPanel() == EconomyPanel) {
    setLeftPanel(NoPanel);
  } else {
    setLeftPanel(EconomyPanel);
    stopBlinkingFeature(FEconomyPanel);

    if (economyTab == CityStatsTab) {
      reportTutorialUpdate(TutorialUpdateCode::SelectedChartsPanel);
    } else if (economyTab == EffectsTab) {
      reportTutorialUpdate(TutorialUpdateCode::SelectedAmenityScoresPanel);
    }
  }

  return true;
}

bool setEconomyTab(Part* part, InputEvent event) {
  economyTab = (EconomyPanelTab) part->itemData;
  if (economyTab == CityStatsTab) {
    reportTutorialUpdate(TutorialUpdateCode::SelectedChartsPanel);
  } else if (economyTab == EffectsTab) {
    reportTutorialUpdate(TutorialUpdateCode::SelectedAmenityScoresPanel);
  }
  return true;
}

bool setChartEcon(Part* part, InputEvent event) {
  chartEcon[(int)part->vecData.x] = part->itemData;
  return true;
}

bool setChartCategory(Part* part, InputEvent event) {
  item chartNum = part->vecData.x;
  chartCategories[chartNum] = (StatisticCategory)part->itemData;
  wasSearch[chartNum] = false;
  return true;
}

bool setChart(Part* part, InputEvent event) {
  item chartNum = part->vecData.x;
  charts[chartNum] = (Statistic)part->itemData;

  if (part->itemData >= 0) {
    reportTutorialUpdate(TutorialUpdateCode::SelectedChart);
  }

  if (searchChart == chartNum) {
    searchChart = -1;
    focusTextBox(0);
    chartEcon[chartNum] = ourCityEconNdx();
    wasSearch[chartNum] = true;
  }

  // Set the correct category
  for (int cat = 0; cat < numStatisticCategories; cat++) {
    vector<Statistic> category = statsCategory(cat);
    for (int j = 0; j < category.size(); j++) {
      if (category[j] == charts[chartNum]) {
        chartCategories[chartNum] = (StatisticCategory)cat;
        return true;
      }
    }
  }

  return true;
}

bool startChartSearch(Part* part, InputEvent event) {
  searchChart = part->itemData;
  focusTextBox(&searchChartTBState);
  return true;
}

bool selectChartSearch(Part* part, InputEvent event) {
  searchChart = part->vecData.x;
  return true;
}

bool unfocusChartSearch(Part* part, InputEvent event) {
  focusTextBox(0);
  return true;
}

bool stopChartSearch(Part* part, InputEvent event) {
  item chartNum = part->itemData;
  charts[chartNum] = (Statistic)-1;
  wasSearch[chartNum] = false;
  searchChart = -1;
  focusTextBox(0);
  return true;
}

bool focusChartSearchBox(Part* part, InputEvent event) {
  focusTextBox(&searchChartTBState);
  return true;
}

bool setTimePeriod(Part* part, InputEvent event) {
  timePeriod = part->itemData;
  return true;
}

void setEPChart(item stat, item tp) {
  item chartNum = 0;
  charts[chartNum] = (Statistic)stat;
  chartEcon[chartNum] = ourCityEconNdx();
  economyTab = CityStatsTab;
  //timePeriod = tp;

  wasSearch[chartNum] = false;
  if (searchChart == 0) {
    searchChart = -1;
    focusTextBox(0);
  }

  // Set the correct category
  for (int cat = 0; cat < numStatisticCategories; cat++) {
    vector<Statistic> category = statsCategory(cat);
    for (int j = 0; j < category.size(); j++) {
      if (category[j] == charts[chartNum]) {
        chartCategories[chartNum] = (StatisticCategory)cat;
        return;
      }
    }
  }
}

bool toggleChartMessage(Part* part, InputEvent event) {
  item econ = part->vecData.x;
  if (econ == 0) econ = ourCityEconNdx();
  toggleMessage(ChartMessage, part->itemData, econ);
  stopBlinkingFeature(FNoteChart);
  return true;
}

bool selectChart(Part* part, InputEvent event) {
  setLeftPanel(EconomyPanel);
  setEPChart(part->itemData/10, part->itemData%10);
  chartEcon[0] = part->vecData.x;
  return true;
}

bool toggleAmenityEffectMessage(Part* part, InputEvent event) {
  toggleMessage(AmenityEffectMessage, part->itemData);
  return true;
}

bool toggleEPExpanded(Part* part, InputEvent event) {
  epExpanded = !epExpanded;
  return true;
}

Part* economyPanel() {
  float epHeight = 18.5;
  float epPadding = 0.25;
  float scale = 0.8;
  float epWidth = -epPadding + (chartWidth + epPadding)
    * (economyTab == CityStatsTab && epExpanded ? 3 : 1);

  vec2 econPanelLoc = vec2(0.1, 1.5);
  vec2 econPanelSize = vec2(epWidth+epPadding*2, epHeight+epPadding*2);
  Part* result = panel(econPanelLoc, econPanelSize);
  result->padding = epPadding;
  if (economyTab == AchievementsTab) {
    r(result, label(vec2(0,0), 1, strdup_s("Achievements")));
  } else if (economyTab == EffectsTab) {
    r(result, label(vec2(0,0), 1, strdup_s("Amenities Effects")));
  }
  //r(result, hr(vec2(0,1.2), epWidth));
  r(result, button(vec2(epWidth-1,0), iconX, toggleEconomyPanel));

  for (int i = 0; i < numEconomyTabs; i++) {
    Part* butt = button(
        vec2(chartWidth + i-numEconomyTabs-1, 0),
        economyTabIcon[i], setEconomyTab, i);
    setPartTooltipValues(butt,
      TooltipType::GraphCityStats+i);
    if (i == economyTab) {
      butt->flags |= _partHighlight;
    }
    r(result, butt);
  }

  if (economyTab == CityStatsTab) {
    Part* expandButt = button(
        vec2(chartWidth-numEconomyTabs-2, 0),
        iconPlus, toggleEPExpanded, 0);
    setPartTooltipValues(expandButt,
      TooltipType::GraphExpand);
    if (epExpanded) {
      expandButt->flags |= _partHighlight;
    }
    r(result, expandButt);

    for (int i = 0; i < numTimePeriods; i++) {
      float x = i*(1.3f);
      Part* butt = button(vec2(x,0.1f), vec2(1.3f,.8f),
          strdup_s(timePeriodName(i)), setTimePeriod);
      setPartTooltipValues(butt,
        TooltipType::GraphPeriodAll+i);
      butt->itemData = i;
      if (i == timePeriod) {
        butt->flags |= _partHighlight;
      }
      r(result, butt);
    }

    for (int i = 0; i < (epExpanded ? numCharts : 2); i++) {
      Statistic stat = charts[i];
      item econ = chartEcon[i];
      if (stat >= numStatistics) {
        stat = (Statistic)-1;
        charts[i] = stat;
      }

      float x = (i / 2) * (chartWidth + epPadding);
      float y = (i % 2) * (chartSize + epPadding) + 1.25f;

      if (searchChart == i) {
        // Search
        Part* butt = r(result, button(vec2(x,y), iconMenu, vec2(.8, .8),
            stopChartSearch, i));

        searchChartTBState.text = &searchChartText;
        Part* searchTB = r(result, textBox(vec2(x+scale,y),
              vec2(chartWidth-scale, scale), &searchChartTBState));
        searchTB->onClick = focusChartSearchBox;
        searchTB->onCustom = unfocusChartSearch;
        string searchStr = searchChartText;
        y += scale + epPadding;

        float scrollSize = chartSize-scale-epPadding;
        Part* scroll = scrollbox(vec2(0,0),
            vec2(chartWidth, scrollSize));
        float y1 = 0;

        for (int stat = 0; stat < numStatistics; stat++) {
          if (!timeSeriesHasData(ourCityEconNdx(), (Statistic)stat)) continue;

          string name = statName(stat);
          string code = getStatisticCode(stat);
          bool found = stringContainsCaseInsensitive(name, searchStr) ||
            stringContainsCaseInsensitive(code, searchStr);
          if (!found) continue;

          Part* butt = button(vec2(0,y1), vec2(chartWidth - 1, scale),
            strdup_s(statName(stat)), setChart);
          butt->itemData = stat;
          butt->vecData.x = i;
          r(scroll, butt);
          y1 += scale;
        }

        r(result, scrollboxFrame(vec2(x,y), vec2(chartWidth, scrollSize),
              &chartScroll[i], scroll));

      } else if (econ <= 0) {
        float scrollSize = chartSize;
        Part* scroll = scrollbox(vec2(0,0), vec2(chartWidth, scrollSize));
        float y1 = 0;
        for (int j = 1; j <= sizeEcons(); j++) {
          Part* butt = button(vec2(0,y1), vec2(chartWidth - 1, scale),
            strdup_s(getEconName(j)), setChartEcon);
          butt->itemData = j;
          butt->vecData.x = i;
          r(scroll, butt);
          y1 += scale;
        }
        r(result, scrollboxFrame(vec2(x,y), vec2(chartWidth, scrollSize),
              &chartScroll[i], scroll));

      } else if (stat < 0) {
        StatisticCategory cat = chartCategories[i];
        if (cat >= numStatisticCategories) {
          cat = (StatisticCategory)-1;
          chartCategories[i] = cat;
        }

        if (cat < 0) {
          Part* butt = button(vec2(x,y), iconLeft, vec2(chartWidth-scale, .8f),
              strdup_s(getEconName(econ)), setChartEcon, -1);
          setPartTooltipValues(butt,
            TooltipType::GraphChooseStat);
          butt->vecData.x = i;
          r(result, butt);

          if (searchChart != i) {
            r(result, button(vec2(x+chartWidth-scale,y), iconQuery, vec2(scale, scale), startChartSearch, i));
          }

          float scrollSize = chartSize-scale-epPadding;
          Part* scroll = scrollbox(vec2(0,0), vec2(chartWidth, scrollSize));
          float y1 = 0;
          for (int j = 0; j < numStatisticCategories; j++) {
            Part* butt = button(vec2(0,y1), vec2(chartWidth - 1, scale),
              strdup_s(statCategoryName(j)), setChartCategory);
            butt->itemData = j;
            butt->vecData.x = i;
            r(scroll, butt);
            y1 += scale;
          }
          r(result, scrollboxFrame(vec2(x,y+scale+epPadding),
                vec2(chartWidth, scrollSize), &chartScroll[i], scroll));

        } else {
          Part* butt = button(vec2(x,y), iconLeft, vec2((chartWidth-scale)*.5f, .8f), strdup_s(statCategoryName(cat)), setChartCategory, -1);
          setPartTooltipValues(butt,
            TooltipType::GraphChooseStat);
          butt->vecData.x = i;
          r(result, butt);

          if (searchChart != i) {
            r(result, button(vec2(x+(chartWidth-scale)*.5f,y), iconQuery, vec2(scale, scale), startChartSearch, i));
          }

          butt = button(vec2(x+(chartWidth+scale)*.5f,y),
              vec2((chartWidth-scale)*.5f, .8f),
              strdup_s(getEconName(econ)), setChartEcon);
          butt->flags |= _partAlignRight;
          butt->itemData = -1;
          butt->vecData.x = i;
          r(result, butt);

          float scrollSize = chartSize-scale-epPadding;
          Part* scroll = scrollbox(vec2(0,0),
              vec2(chartWidth, scrollSize));
          vector<Statistic> category = statsCategory(cat);
          float y1 = 0;
          for (int j = 0; j < category.size(); j++) {
            stat = category[j];
            if (stat < 0) break;
            if (!timeSeriesHasData(econ, stat)) continue;
            Part* butt = button(vec2(0,y1), vec2(chartWidth - 1, scale),
              strdup_s(statName(stat)), setChart);
            butt->itemData = stat;
            butt->vecData.x = i;
            r(scroll, butt);
            y1 += scale;
          }
          r(result, scrollboxFrame(vec2(x,y+scale+epPadding),
                vec2(chartWidth, scrollSize), &chartScroll[i], scroll));
        }

      } else {
        Part* chrt = chart(vec2(x,y), vec2(chartWidth, chartSize),
            econ, stat, timePeriod);
        r(result, chrt);

        Part* econLbl = r(chrt, labelCenter(vec2(chartWidth*.25f, .95f),
              vec2(chartWidth*.5f, 0.7), strdup_s(getEconName(econ))));
        econLbl->flags |= _partHover;
        econLbl->onClick = setChartEcon;
        econLbl->itemData = -1;
        econLbl->vecData.x = i;
        econLbl->dim.start.z = 5;

        Part* butt = button(vec2(.1f,.1f), wasSearch[i] ? iconQuery : iconLeft, vec2(chartWidth-1.2f, .8f), strdup_s(statName(stat)), wasSearch[i] ? selectChartSearch : setChart, -1);
        setPartTooltipValues(butt,
          TooltipType::GraphChooseStat);
        butt->vecData.x = i;
        r(chrt, butt);

        if (isFeatureEnabled(FNoteChart)) {
          item statItem = charts[i]*10 + timePeriod;
          Part* cNoteButt = button(vec2(chartWidth - .9f, .1f), iconPin,
              vec2(.8f, .8f), toggleChartMessage, statItem);
          cNoteButt->vecData.x = chartEcon[i];
          setPartTooltipValues(cNoteButt,
            TooltipType::GraphPinMessage);
          if (hasMessage(ChartMessage, stat*10+timePeriod, econ) ||
           hasMessage(ChartMessage, -stat*10-timePeriod, econ)) {
            cNoteButt->flags |= _partHighlight;
          }
          if (blinkFeature(FNoteChart)) {
            cNoteButt->flags |= _partBlink;
          }
          r(chrt, cNoteButt);
        }
      }
    }

  } else if (economyTab == AchievementsTab) {
    Part* scroll = scrollbox(vec2(0,0), vec2(epWidth,epHeight-1.5f));
    scroll->flags |= _partTextShadow;
    float sy = 0;

    for (int k = 0; k < 1; k++) {
      for (int i = getNumAchievements(); i > 0; i--) {
        if (isAchievementAcquired(i) != k) {

          Achievement ach = getAchievement(i);
          r(scroll, icon(vec2(0, sy), vec2(scale, scale),
               k == 0 ? iconCheck : iconNull));
          r(scroll, label(vec2(scale, sy), scale, strdup_s(ach.name)));
          sy += scale;

          if (k == 0) {
            float ySize = 0;
            r(scroll, multiline(vec2(scale, sy), vec2(epWidth-2,.7f),
                strdup_s(ach.text), &ySize));
            sy += ySize + epPadding;
            const char* effectText = ach.effectText;
            if (effectText != 0) {
              r(scroll, multiline(vec2(scale, sy), vec2(epWidth-2,.7f),
                  strdup_s(effectText), &ySize));
              sy += ySize + epPadding;
            }
          }
        }
      }
    }

    Part* achLbl = r(result, label(vec2(0,1.25), 0.85, sprintf_o("%d/%d Achievements Unlocked", numAchievementsUnlocked(), getNumAchievements())));
    achLbl->foregroundColor = PickerPalette::OrangeMedL;
    achLbl->flags |= _partTextShadow;

    Part* frame = r(result, scrollboxFrame(vec2(0,2.25f), vec2(epWidth, epHeight-2.25f), &achievementsScroll, scroll));
    frame->renderMode = RenderPanelGradient;
    frame->texture = line(colorGoldGrad1, colorGoldGrad0);
    frame->flags &= ~_partLowered;

    Part* slider = achievementsScroll.slider;
    if (slider != 0) {
      slider->texture = line(colorGoldGrad0, colorGoldGrad0);
      slider->renderMode = RenderGradient;
    }

  } else if (economyTab == EffectsTab) {
    Part* scroll = scrollbox(vec2(0,0), vec2(epWidth,epHeight-1.5f));
    scroll->flags |= _partTextShadow;
    float y = 0;

    for (int i = 0; i < numEffects; i++) {
      int val = getEffectValue(i);

      r(scroll, icon(vec2(0, y), vec2(1, 1), getEffectIcon(i)));
      r(scroll, labelRight(vec2(1, y), vec2(2,1),
          sprintf_o("%s%d", val > 0 ? "+":"", val)));
      r(scroll, label(vec2(3, y), 1, strdup_s(getEffectString(i))));

      Part* p = r(scroll, button(vec2(epWidth-2, y), iconPin, vec2(1,1),
            toggleAmenityEffectMessage, i));
      Part* citi = r(scroll, button(vec2(epWidth-3, y), iconCitipedia, vec2(1,1), openCitipediaEffectPage, i));

      y += scale + epPadding;

      float ySize;
      r(scroll, multiline(vec2(1, y), vec2(epWidth-3, scale),
            getMacroEffectDescriptor(i), &ySize));
      y += ySize + epPadding*2;
    }

    Part* frame = r(result, scrollboxFrame(vec2(0,1.25f), vec2(epWidth, epHeight-1.25f), &effectsScroll, scroll));
    frame->renderMode = RenderPanelGradient;
    frame->texture = line(colorGoldGrad1, colorGoldGrad0);
    frame->flags &= ~_partLowered;

    Part* slider = effectsScroll.slider;
    if (slider != 0) {
      slider->texture = line(colorGoldGrad0, colorGoldGrad0);
      slider->renderMode = RenderGradient;
    }
  }

  return result;
}

void writeEconomyPanel(FileBuffer* file) {
  fwrite_int(file, timePeriod);
  fwrite_int(file, numCharts);
  for (int i = 0; i < numCharts; i++) {
    fwrite_int(file, charts[i]);
    fwrite_int(file, chartCategories[i]);
    fwrite_int(file, chartEcon[i]);
  }
}

void readEconomyPanel(FileBuffer* file, int version) {
  if (version >= 44) {
    timePeriod = fread_int(file);
    int numC = fread_int(file);
    for (int i = 0; i < numC; i++) {
      int val = fread_int(file);
      if (i < numCharts) {
        charts[i] = (Statistic)val;
      }

      if (version >= 52) {
        int cat = fread_int(file);
        int econ = fread_int(file);
        if (i < numCharts) {
          chartCategories[i] = (StatisticCategory)cat;
          chartEcon[i] = econ;
        }
      }
    }
  }
}

