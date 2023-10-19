#include "statusBar.hpp"

#include "../building/building.hpp"
#include "../building/renderBuilding.hpp"
#include "../color.hpp"
#include "../draw/camera.hpp"
#include "../economy.hpp"
#include "../game/feature.hpp"
#include "../game/game.hpp"
#include "../graph.hpp"
#include "../icons.hpp"
#include "../land.hpp"
#include "../lot.hpp"
#include "../money.hpp"
#include "../option.hpp"
#include "../person.hpp"
#include "../sound.hpp"
#include "../string_proxy.hpp"
#include "../string.hpp"
#include "../route/router.hpp"
#include "../time.hpp"
#include "../tutorial.hpp"
#include "../vehicle/travelGroup.hpp"
#include "../vehicle/vehicle.hpp"
#include "../weather.hpp"

#include "button.hpp"
#include "budgetPanel.hpp"
#include "designConfigPanel.hpp"
#include "designOrganizerPanel.hpp"
#include "economyPanel.hpp"
#include "icon.hpp"
#include "label.hpp"
#include "leftPanel.hpp"
#include "panel.hpp"
#include "root.hpp"
#include "slider.hpp"
#include "textBox.hpp"
#include "tooltip.hpp"

static TextBoxState cityNameTBState;
static bool fullCityNameBar = false;

bool speedButton(Part* part, InputEvent event) {
  /*
  // Don't allow speed changes when tutorial is active and showing a step
  TutorialState* tState = getTutorialStatePtr();
  if (tState->tutorialOKWait() && tState->tutorialActive()) {
    return false;
  }
  */

  item speed = part->itemData;
  
  if (part->itemData == 0) {
    toggleGamePause();
    return true;
  }
  
  setGameSpeed(speed);
  stopBlinkingFeature(FSpeedControl);
  return true;
}

bool toggleGamePause(Part* part, InputEvent event) {
  toggleGamePause();
  return true;
}

bool setTime(Part* part, InputEvent event) {
  setLightTime(part->vecData.x + startDay);
  if (getGameMode() == ModeBuildingDesigner) {
    designRender();
  }
  return true;
}

bool setSeason(Part* part, InputEvent event) {
  setLightSeason(part->vecData.x*4);
  if (getGameMode() == ModeBuildingDesigner) {
    designRender();
  }
  return true;
}

bool deleteAllCars(Part* part, InputEvent event) {
  removeAllTravelGroups_g();
  removeAllVehicles_g();
  return true;
}

bool toggleCityNameBar(Part* part, InputEvent event) {
  fullCityNameBar = !fullCityNameBar;
  if (!fullCityNameBar) {
    focusTextBox(0);
    cityNameTBState.flags &= _textBoxEditing;
  }
  return true;
}

bool stopEditingCityName(Part* part, InputEvent event) {
  focusTextBox(0);
  return true;
}

Part* statusBar(float uiX) {
  float padding = 0.1;
  Part* statusBar = part(line(vec3(0,0,0), vec3(25, 1+padding*2, 0)));
  item mode = getGameMode();
  bool designer = (mode == ModeBuildingDesigner) ||
    (mode == ModeDesignOrganizer);
  bool game = mode == ModeGame && !isFixBuildingMode();
  bool testMode = mode == ModeTest;
  float x = 0;
  bool showMoneyBarOrSpace = !designer && isFeatureEnabled(FBudget);
  float moneyBarWidth = 7.f; //game ? 5.5+padding*2 : 6.0;

  // City Name Bar
  if (game) {
    bool editing = fullCityNameBar && cityNameTBState.flags & _textBoxEditing;
    float nameLength = stringWidth(getCityName())*.75 + .25f
      + (editing ? 2 : 0);

    if (nameLength < 2) nameLength = 2;

    // Seed busines
    // MinGW needs a special sprintf format for long longs, apparently
    // https://stackoverflow.com/questions/8987698/sscanf-returning-wrong-numbers-for-long-long-int-variables
    #ifdef MINGW
      const char* seedFormat = "%dx%d - %I64d";
    #else
      const char* seedFormat = "%dx%d - %lld";
    #endif

    LandConfiguration landConfig = getLandConfig();
    char* hSeedStr = sprintf_o(seedFormat,
        landConfig.landSize, landConfig.landSize, landConfig.heightSeed);

    float extraY = 0.0f; // Extra Y size for seed strs
    if(fullCityNameBar) {
      float hSeedLen = stringWidth(hSeedStr)*0.75f;
      extraY = 1.0f;
      // Set nameLength to be at least as much as the largest seed str
      nameLength += 1 + padding;
      if(nameLength < hSeedLen) nameLength = hSeedLen;
    }
    float nameBarX = nameLength + padding*2;

    Part* nameBar = r(statusBar,
        panel(vec2(x,0), vec2(nameBarX, 1+fullCityNameBar+extraY+padding*2)));
    nameBar->padding = padding;
    // Extended name bar overlaps changelog when extended in Main Menu, so only setting the callback for HiddenMenu
    // Also added z-value fix for nameBar text overlapping other UI elements 
    // This fix makes it render weird on the Main Menu, at least. So only setting values when HiddenMenu as well...
    if(getMenuMode() == HiddenMenu) {
      nameBar->onClick = toggleCityNameBar;
      nameBar->dim.start.z = -5.0f;
      nameBar->dim.end.z = -5.0f;
    } else {
      fullCityNameBar = false;
    }

    if (fullCityNameBar) {
      cityNameTBState.text = getCityNamePtr();
      Part* nameTB = r(nameBar, textBoxLabel(vec2(0,0),
            vec2(nameLength-padding, 1), &cityNameTBState));
      nameTB->onCustom = stopEditingCityName;

      char* diffLbl = isLeftHandTraffic() ?
          sprintf_o("%s, Left", difficultyName[getGameDifficulty()]) :
          strdup_s(difficultyName[getGameDifficulty()]);
      r(nameBar, label(vec2(0,1), 0.85, diffLbl));

      // Height seed
      Part* hSeedCon = panel(vec2(0.0f, 2.0f), vec2(nameLength, 1.f));
      hSeedCon->renderMode = RenderTransparent;
      setPartTooltipValues(hSeedCon, TooltipType::GenSeedH);

      Part* hSeedLbl = label(vec2(0.0f, 0.0f), 0.85f,
        (char*)hSeedStr);

      r(hSeedCon, hSeedLbl);
      r(nameBar, hSeedCon);

    } else {
      r(nameBar, label(vec2(0,0), 1, strdup_s(getCityName())));
      free(hSeedStr);
    }

    /*
    if (editing) {
      Part* nameTB = r(nameBar,
          textBox(vec2(0,0), vec2(nameLength-1-padding, 1), &cityNameTBState));
      nameTB->onCustom = stopEditingCityName;
      r(nameBar, button(vec2(nameLength-1, 0), iconCheck,
            vec2(1, 1), stopEditingCityName, 0));
    } else {
      r(nameBar, label(vec2(0,0), 1, strdup_s(getCityName())));
    }
    */

    x += nameBarX + 0.5;
  }

  // Time and Date Bar
  if (game) {
    Part* timeBar = panel(vec2(x,0), vec2(10, 1+padding*2));
    timeBar->padding = padding;
    float time = getCurrentDateTime();
    r(timeBar, label(vec2(0,0), 1,
          printDateTimeStringSanYear(getCurrentDateTime())));
    int year = getCurrentYear();
    r(timeBar, labelRight(vec2(7-padding*2,0), vec2(3,1),
          sprintf_o("%d", year)));
    r(statusBar, timeBar);
    x += 10.5;
  }

  if (getMenuMode() != HiddenMenu) return statusBar;

  // Population Bar
  if (game && isFeatureEnabled(FPopulation)) {
    char* popString;
    int pop = numPeople(ourCityEconNdx());
    if (pop >= 1000000000) {
      popString = sprintf_o("%d,%03d,%03d,%03d",
          pop/1000000000, (pop/1000000)%1000, (pop/1000)%1000, pop%1000);
    } else if (pop >= 1000000) {
      popString = sprintf_o("%d,%03d,%03d",
          pop/1000000, (pop/1000)%1000, pop%1000);
    } else if (pop >= 1000) {
      popString = sprintf_o("%d,%03d", pop/1000, pop%1000);
    } else {
      popString = sprintf_o("%3d", pop);
    }
    float popBarSize = stringWidth(popString) + 1;

    Part* popBar = panel(vec2(x,0), vec2(popBarSize+padding*2, 1+padding*2));
    popBar->padding = padding;
    setPartTooltipValues(popBar,
      TooltipType::GenGraphs);
    if (isFeatureEnabled(FEconomyPanel)) {
      popBar->onClick = toggleEconomyPanel;
      popBar->inputAction = ActChartsPanel;
      popBar->flags |= _partHover;
    }

    popBar->renderMode = RenderPanelGradient;
    popBar->texture = line(colorGoldGrad1, colorGoldGrad0);
    popBar->flags |= _partTextShadow;

    if (getLeftPanel() == EconomyPanel) {
      popBar->texture = line(colorGoldGrad0, colorGoldGrad0);
    }

    if (getStatisticSum(ourCityEconNdx(), PopulationGrowth, getCurrentDateTime() - 1) < -1000) {
      popBar->foregroundColor = PickerPalette::RedLight;
    }
    if (blinkFeature(FEconomyPanel) || blinkFeature(FNoteChart)) {
      popBar->flags |= _partBlink;
    }
    r(popBar, icon(vec2(0,0), vec2(1,1), iconPersonMan));
    r(popBar, label(vec2(1,0), 1, popString));
    r(statusBar, popBar);
    x += popBarSize + .5f + padding*2;
  }

  if (!designer && !isFixBuildingMode() && isFeatureEnabled(FBudget)
      && getMenuMode() == HiddenMenu) {
    Part* moneyBar = panel(vec2(x,0),
        vec2(moneyBarWidth+padding*2, 1+padding*2));
    moneyBar->padding = padding;

    vec3 white = getColorInPalette(PickerPalette::GrayLight);
    Line mbGrad = line(white, white);
    moneyBar->renderMode = RenderPanelGradient;
    moneyBar->texture = mbGrad;
    moneyBar->flags |= _partTextured;

    setPartTooltipValues(moneyBar,
      TooltipType::GenBudget);
    if (game && isFeatureEnabled(FBudget)) {
      moneyBar->onClick = toggleBudgetPanel;
      moneyBar->inputAction = ActBudgetPanel;
      moneyBar->flags |= _partHover;
    }

    if (getLeftPanel() == BudgetPanel) {
      //vec3 gray = getColorInPalette(PickerPalette::GrayDark);
      //moneyBar->texture = line(gray, gray);
    } else if (game && (blinkFeature(FBudget) || blinkFeature(FPropertyTax) ||
        blinkFeature(FSalesTax) || blinkFeature(FFinesAndFees) ||
        blinkFeature(FLoanTerm) || blinkFeature(FNoteBudget))) {
      moneyBar->flags |= _partBlink;
    }
    r(statusBar, moneyBar);

    money currentBalance;
    char* moneyString;
    const char* loc = "";
    if (game) {
      Budget currentBudget = getBudget(0);
      currentBalance = currentBudget.line[BudgetBalance];
      if (currentBalance < 1000) {
        currentBalance += currentBudget.line[LineOfCredit];
        loc = currentBalance < 0 ? " Over" : " Loan";
      }
    } else {
      currentBalance = getStatistic(ourCityEconNdx(), RoadSystemValue);
      loc = " Value";
    }

    char* temp = printPaddedMoneyString(currentBalance);
    moneyString = sprintf_o("%s%s", temp, loc);
    free(temp);

    /*
    float am = currentBalance;
    if (am > 0) {

      char* temp = printPaddedPositiveMoneyString(am);
      moneyString = sprintf_o("%s%s", temp, loc);
      free(temp);
    } else {
      moneyString = strdup_s(game ? "OVERDRAWN" : "Scenario Editor");
    }
    */

    Part* moneyLbl = r(moneyBar, label(vec2(0,0), 1, moneyString));

    if (game && getCredit() < 1000000) {
      moneyLbl->foregroundColor = PickerPalette::RedDark;
    } else {
      moneyLbl->foregroundColor = PickerPalette::Black;
    }

    x += moneyBarWidth + .5f + padding*2;
  }

  if (designer || isFixBuildingMode()) {
    r(statusBar, getGameMode() == ModeBuildingDesigner ? designConfigPanel()
        : designOrganizerPanel());
    x += 17;
    if (isFixBuildingMode()) {
      return statusBar;
    }
  }

  /*
  if (testMode) {
    Part* titleBar = panel(vec2(x,0),
        vec2(7, 1+padding*2));
    titleBar->padding = padding;
    r(titleBar, label(vec2(0,0), 1, strdup_s("Test Mode")));
    r(statusBar, titleBar);
    x += 7.5;
  }
  */

  // TIME SLIDER
  if (!game) {
    float timeSizeX = 5.0f;
    float timeSizeY = 1.0f;

    Part* timeBar = panel(vec2(x,0),
        vec2(timeSizeX+padding*2,timeSizeY+padding*2));
    timeBar->padding = padding;
    r(timeBar, slider(vec2(0,0), vec2(timeSizeX,timeSizeY),
      getLightTime(), setTime));

    // Icons representing night/day pos on slider
    Part* ico = r(timeBar, icon(vec2(0,0), vec2(1,1),
      iconWeatherMoon));
    ico->flags |= _partTransparent;
    ico = r(timeBar, icon(vec2(2,0), vec2(1,1),
      iconWeatherSun));
    ico->flags |= _partTransparent;
    ico = r(timeBar, icon(vec2(4,0), vec2(1,1),
      iconWeatherMoon));
    ico->flags |= _partTransparent;

    r(statusBar, timeBar);
    x += 5.5 + padding*2;

    Part* seasonBar = panel(vec2(x,0),
        vec2(timeSizeX+padding*2,timeSizeY+padding*2));
    seasonBar->padding = padding;
    r(seasonBar, slider(vec2(0,0), vec2(timeSizeX,timeSizeY),
      getLightSeason()*.25f, setSeason));

    //ico = r(seasonBar, icon(vec2(0,0), vec2(1,1),
      //iconWeatherCold));
    //ico->flags |= _partTransparent;
    ico = r(seasonBar, icon(vec2(2,0), vec2(1,1),
      iconWeatherSun));
    ico->flags |= _partTransparent;
    ico = r(seasonBar, icon(vec2(4,0), vec2(1,1),
      iconWeatherCold));
    ico->flags |= _partTransparent;

    r(statusBar, seasonBar);
    x += 5.5 + padding*2;
  }

  // Number of Cars
  if (testMode) {
    Part* rateBar = panel(vec2(x,0), vec2(8+padding*2, 1+padding*2));
    rateBar->padding = padding;
    //r(rateBar, label(vec2(0,0), 1, sprintf_o("%'d Cars, %'d/Hour",
            //countVehicles(), int(getVehicleRate()/6))));
    r(rateBar, labelRight(vec2(0,0), vec2(7,1),
          sprintf_o("%d", countVehicles())));
    r(rateBar, icon(vec2(7,0), vec2(1,1), iconCar));
    rateBar->onClick = toggleEconomyPanel;
    rateBar->inputAction = ActChartsPanel;
    rateBar->flags |= _partHover;
    r(statusBar, rateBar);
    x += 8.5 + padding*2;
  }

  // Clear the Roads button
  if (testMode) {
    const char* delS = "Clear the Roads";
    float delW = stringWidth(delS)*.8+.1;
    Part* deleteBar = panel(vec2(x,0), vec2(delW+padding*2, 1+padding*2));
    deleteBar->padding = padding;
    r(deleteBar, button(vec2(0,0), vec2(delW,1), strdup_s(delS),
          deleteAllCars));
    r(statusBar, deleteBar);
    x += delW + 0.5 + padding*2;
  }

  /*
  // Unemployment
  if (game && getAspectRatio() > 1.4 &&
      (showAllStatusBars || isFeatureEnabled(FUnemployment))) {
    Part* unempBar = panel(vec2(x,0), vec2(6+padding*2, 1+padding*2));
    unempBar->padding = padding;
    if (showAllStatusBars || isFeatureEnabled(FEconomyPanel)) {
      unempBar->onClick = toggleEconomyPanel;
      unempBar->flags |= _partHover;
    }
    if (blinkFeature(FEconomyPanel) || blinkFeature(FNoteChart)) {
      unempBar->flags |= _partBlink;
    }
    if (unemploymentRate() > 0.1) {
      unempBar->foregroundColor = PickerPalette::RedLight;
    }
    r(unempBar, label(vec2(0,0), 1, sprintf_o("%2d%% Unemployed",
      int(unemploymentRate()*100))));
    r(statusBar, unempBar);
    x += 6.5 + padding*2;
  }
  */

  // Weather bar
  if (game && getAspectRatio() > 1.4 &&
      isFeatureEnabled(FWeather)) {
    Weather w = getWeather();
    Part* weatherBar = panel(vec2(x,0), vec2(2.5f+padding*2, 1+padding*2));
    r(statusBar, weatherBar);
    x += 3.0f + padding*2;
    weatherBar->padding = padding;
    float tempSize = 0.8;
    r(weatherBar, label(vec2(0.1,0.1), tempSize,
          useMetric() ? sprintf_o("%2dC", int(w.temp)) :
          sprintf_o("%2dF", int(w.temp*1.8 + 32))));

    vec3 ico = getWeatherIconVec3();
    r(weatherBar, icon(vec2(1.5f,0.f), ico));
  }

  // SPEED BAR
  if (!designer &&
      (testMode || isFeatureEnabled(FSpeedControl))) {
    int size = debugMode() ? 8 : 5;
    Part* speedBar = panel(vec2(x, 0),
      vec2(size+padding*2, 1+padding*2));
    x += size + .5 + padding*2;
    speedBar->padding = padding;
    item gameSpeed = getGameSpeed();
    item effectiveSpeed = getEffectiveGameSpeed();
    float currentTime = getCurrentTime();
    int currentTimeSecs = currentTime * gameDayInRealSeconds;

    for (int i=0; i < size; i++) {
      Part* butt = button(vec2(i,0), iconGameSpeed[i>4?4:i], speedButton, i);
      setPartTooltipValues(butt,
        TooltipType::GenSpdPause+i);
      butt->inputAction = (InputAction)(ActPauseGame+i);

      if (i == gameSpeed) {
        butt->flags |= _partHighlight;
      } else if (i == effectiveSpeed) {
        butt->foregroundColor = PickerPalette::RedLight;
      }
      if (i == 2 && blinkFeature(FSpeedControl)) butt->flags |= _partBlink;

      r(speedBar, butt);
    }
    r(statusBar, speedBar);
  }

  return statusBar;
}

