#include "newGamePanel.hpp"

#include "spdlog/spdlog.h"

#include "../game/game.hpp"
#include "../land.hpp"
#include "../icons.hpp"
#include "../string_proxy.hpp"

#include "button.hpp"
#include "hr.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "root.hpp"
#include "slider.hpp"
#include "textBox.hpp"
#include "tooltip.hpp"

const float ngPad = 0.5;
const float ngColWidth = 9.5;
const float ngButtWidth = ngColWidth-ngPad*2;
const float ngWidth = ngColWidth*3 + ngPad*2;
const float ngHeight = 17.25;
const float ngScale = 1;
static TextBoxState cityNameTBState;
static TextBoxState hSeedState;
static char* hSeedTxtPtr = 0;
static item newGameModeTab = ModeGame;

enum NGPTextBox {
  CityName = 0,
  HeightSeed,
  NumTextBoxes
};

const int numLandSizes = 6;
const int landSize[] = {
  10,
  25,
  50,
  75,
  100,
  125
};

bool selectNewGameModeTab(Part* part, InputEvent event) {
  newGameModeTab = part->itemData;
  return true;
}

bool generateMap(Part* part, InputEvent event) {
  closeMenus();

  // Parse textboxes for new seeds
  LandConfiguration *nextLandConfig = getNextLandConfig();
  char* derefHSeed = hSeedState.text != 0 ? *hSeedState.text : 0;
  if(derefHSeed != 0) {
    int64_t hSeedVal = -1;
    sscanf(derefHSeed, "%lld", &hSeedVal);
    if(hSeedVal >= 0) {
      SPDLOG_INFO("Setting nextLandConfig seed to {}", hSeedVal);
      nextLandConfig->heightSeed = hSeedVal;
      nextLandConfig->treeSeed = hSeedVal;
      nextLandConfig->flags |= _landCustomSeed;
    } else {
      handleError("Invalid Seed");
    }
  } else {
    handleError("Invalid Seed");
  }

  // Reset text pointers for next time
  resetSeedTxtPtrs();

  gameLoad(LoadTargetNew, (GameMode)newGameModeTab, 0);
  return true;
}

bool setLandSize(Part* part, InputEvent event) {
  getNextLandConfig()->landSize = part->itemData;
  return true;
}

bool toggleLandFlag(Part* part, InputEvent event) {
  bool val = getNextLandConfig()->flags & part->itemData;
  if (val) {
    getNextLandConfig()->flags &= ~part->itemData;
  } else {
    getNextLandConfig()->flags |= part->itemData;
  }
  return true;
}

bool setGameDiff(Part* part, InputEvent event) {
  GameDifficulty diff = (GameDifficulty)part->itemData;
  if(diff >= numDifficulties) {
    diff = DifficultyVeryHard;
  } else if(diff < 0) {
    diff = DifficultyEasy;
  }
  setNextGameDifficulty(diff);
  return true;
}

bool setNextGameAccelerated(Part* part, InputEvent event) {
  setNextGameAccelerated((bool)part->itemData);
  return true;
}

bool setNextLeftHandTraffic(Part* part, InputEvent event) {
  setNextLeftHandTraffic((bool)part->itemData);
  return true;
}

Part* landConfigToggle(float y, const char* lbl, uint32_t flag) {
  Part* result = button(vec2(0,y),
        getNextLandConfig()->flags & flag ? iconCheck : iconNull,
        vec2(ngButtWidth, ngScale), strdup_s(lbl),
        toggleLandFlag, flag);
  return result;
}

Part* diffConfigToggle(float x, float y, const char* lbl, GameDifficulty diff) {
  Part* result = button(vec2(x, y),
    diff == getNextGameDifficulty() ? iconCheck : iconNull,
    vec2(ngButtWidth, ngScale), strdup_s(lbl),
    setGameDiff, diff);
  return result;
}

bool focusNewGamePanelTextBox(Part* part, InputEvent event) {
  NGPTextBox tb = (NGPTextBox)part->itemData;

  switch(tb) {
    case NGPTextBox::CityName:
      focusTextBox(&cityNameTBState);
      break;
    case NGPTextBox::HeightSeed:
      focusTextBox(&hSeedState);
      break;
    default:
      SPDLOG_INFO("Attempted to focus an invalid textbox in NewGamePanel");
      break;
  }

  return true;
}

void resetSeedTxtPtrs() {
  if(hSeedTxtPtr != 0) {
    free(hSeedTxtPtr);
    hSeedTxtPtr = 0;
  }
}

bool randomizeLand(Part* part, InputEvent event) {
  resetSeedTxtPtrs();
  randomizeNextLandConfig();
  return true;
}

bool resetLand(Part* part, InputEvent event) {
  resetSeedTxtPtrs();
  resetNextLandConfig();

  free(*getNextCityNamePtr());
  *getNextCityNamePtr() = strdup_s(getCityName());

  setNextGameDifficulty(getGameDifficulty());
  setNextGameAccelerated(getGameAccelerated());
  return true;
}

Part* newGamePanel(float aspectRatio) {
  vec2 newGamePanelSizePadded =
    vec2(ngWidth+ngPad*2, ngHeight+ngPad*2);
  float uiX = uiGridSizeX * aspectRatio;
  Part* result = panel(
      vec2(uiX,uiGridSizeY)*0.5f - newGamePanelSizePadded*0.5f,
      newGamePanelSizePadded);
  result->padding = ngPad;
  float colHeight = 7*ngScale+ngPad*2;
  float y = 0;
  float yb = 0;
  bool design = newGameModeTab == ModeBuildingDesigner;

  // Header
  r(result, label(vec2(0,0), 1.25f, strdup_s(
          newGameModeTab == ModeTest ? "New Test" :
          newGameModeTab == ModeBuildingDesigner ? "New Design" :
          newGameModeTab == ModeGame ? "New City" : "Err")));

  // Close button
  r(result, button(vec2(ngWidth-1.25f,0), iconX, vec2(1.25f, 1.25f),
        openMainMenu, 0));

  if (!design) {
    // Reset button
    Part* resetButt = r(result, button(vec2(ngWidth-2.5, 0),
        iconUndo, vec2(1.25, 1.25), resetLand, 0));
    setPartTooltipValues(resetButt, TooltipType::GenLandReset);
  }

  y += 1.25f;
  r(result, hr(vec2(0,y), ngWidth));
  y += 1;

  // Mode Tabs
  for (int i = ModeGame; i <= ModeBuildingDesigner; i++) {
    float modeTabPad = 0.5f;
    float modeTabWidth = (ngWidth)/3;
    Part* modeTab = r(result, buttonCenter(vec2(i*(modeTabWidth+modeTabPad)-0.5, -1.5), vec2(modeTabWidth, 1), strdup_s(
          i == ModeBuildingDesigner ? "New Design" :
          i == ModeTest ? "New Scenario" : "New City"),
          selectNewGameModeTab));
    modeTab->itemData = i;
    if (i == newGameModeTab) {
      modeTab->flags |= _partRaised;
      modeTab->flags &= ~_partHover;
    } else {
      modeTab->flags |= _partHighlight;
    }
  }

  if (!design) {
    // Name Text Box
    float nameTBWidth = ngColWidth*2 + ngPad;
    cityNameTBState.text = getNextCityNamePtr();
    r(result, label(vec2(0,y), 0.75, strdup_s("City Name")));
    y += 0.75;
    Part* nameTB = r(result,
        textBox(vec2(0,y), vec2(nameTBWidth, 1.25), &cityNameTBState));
    nameTB->itemData = NGPTextBox::CityName;
    nameTB->onClick = focusNewGamePanelTextBox;

    // Seed
    // MinGW needs a special sprintf format for long longs, apparently
    // https://stackoverflow.com/questions/8987698/sscanf-returning-wrong-numbers-for-long-long-int-variables
    #ifdef MINGW
      const char* seedFormat = "%I64d";
    #else
      const char* seedFormat = "%lld";
    #endif

    if(hSeedTxtPtr == 0) {
      hSeedTxtPtr = sprintf_o(seedFormat, getNextLandConfig()->heightSeed);
    }
    hSeedState.text = &hSeedTxtPtr;

    float hSeedLabelX = (ngColWidth+ngPad)*2;
    Part* hSeedLabel = r(result, label(vec2(hSeedLabelX, y-0.75),
        0.75, strdup_s("Seed")));
    Part* hSeedTB = r(result, textBox(vec2(hSeedLabelX, y),
      vec2(ngColWidth-1.25, 1.25), &hSeedState));
    hSeedTB->itemData = NGPTextBox::HeightSeed;
    hSeedTB->onClick = focusNewGamePanelTextBox;
    setPartTooltipValues(hSeedTB, TooltipType::GenSeedH);

    Part* randomButt = r(result, button(vec2(hSeedLabelX+ngColWidth-1.25, y),
        iconDice, vec2(1.25, 1.25), randomizeLand, 0));
    setPartTooltipValues(randomButt, TooltipType::GenSeedRandom);

    y += 2.25f;

    // Difficulty Column
    r(result, labelCenter(vec2(0,y), vec2(ngColWidth, 1.25f),
          strdup_s("Difficulty")));
    Part* diffPanel = panel(vec2((ngColWidth+ngPad)*0,y+1.25f),
        vec2(ngColWidth, colHeight));
    diffPanel->padding = ngPad;
    diffPanel->flags |= _partLowered | _partClip;
    r(result, diffPanel);
    yb = 0;

    for (int i = 0; i < numDifficulties; i++) {
      r(diffPanel, diffConfigToggle(0, yb, difficultyName[i], (GameDifficulty)i));
      yb += ngScale;
    }

    r(diffPanel, hr(vec2(0, colHeight-ngScale*2-ngPad*3), ngButtWidth));

    // Accelerated toggle
    bool accel = getNextGameAccelerated();
    Part* accelTgl = button(vec2(0, colHeight-ngScale*2-ngPad*2),
      accel ? iconCheck : iconNull,
      vec2(ngButtWidth, ngScale),
      sprintf_o("Accelerated Start"), setNextGameAccelerated, !accel);
    r(diffPanel, accelTgl);

    // Left Hand Traffic toggle
    bool left = getNextLeftHandTraffic();
    Part* leftTgl = button(vec2(0, colHeight-ngScale-ngPad*2),
      left ? iconCheck : iconNull,
      vec2(ngButtWidth, ngScale),
      sprintf_o("Left Hand Traffic"), setNextLeftHandTraffic, !left);
    r(diffPanel, leftTgl);

    // Terrain Column
    r(result, labelCenter(vec2(ngColWidth+ngPad,y), vec2(ngColWidth, 1.25f),
          strdup_s("Terrain")));
    Part* terrainPanel = panel(vec2((ngColWidth+ngPad)*1,y+1.25f),
        vec2(ngColWidth, colHeight));
    terrainPanel->padding = ngPad;
    terrainPanel->flags |= _partLowered | _partClip;
    r(result, terrainPanel);
    yb = 0;

    r(terrainPanel, landConfigToggle(yb, "Trees", _landTrees));
    yb += ngScale;
    r(terrainPanel, landConfigToggle(yb, "Hills", _landHills));
    yb += ngScale;
    r(terrainPanel, landConfigToggle(yb, "Water", _landWater));
    yb += ngScale;
    r(terrainPanel, landConfigToggle(yb, "Desert", _landDesert));
    yb += ngScale;
    r(terrainPanel, landConfigToggle(yb, "Superflat", _landSuperflat));
    yb += ngScale;
    r(terrainPanel, landConfigToggle(yb, "SC2K", _landSC2K));
    yb += ngScale;
    r(terrainPanel, landConfigToggle(yb, "Neighbors", _landGenerateNeighbors));
    yb += ngScale;

    // Size Column
    r(result, labelCenter(vec2((ngColWidth+ngPad)*2,y), vec2(ngColWidth, 1.25f),
          strdup_s("Size")));
    Part* sizePanel = panel(vec2((ngColWidth+ngPad)*2, y+1.25f),
        vec2(ngColWidth, colHeight));
    sizePanel->padding = ngPad;
    sizePanel->flags |= _partLowered | _partClip;
    r(result, sizePanel);

    bool hasCustomSize = c(CCustomLandSize) != 0;
    yb = 0;
    for (int i = 0; i < numLandSizes + hasCustomSize; i++) {
      int ls = i == numLandSizes ? c(CCustomLandSize) : landSize[i];
      Part* butt = button(vec2(0,yb),
        ls == getNextLandConfig()->landSize ? iconCheck : iconNull,
        vec2(ngButtWidth, ngScale),
        sprintf_o("%dx%d Chunks", ls, ls), setLandSize, ls);
      r(sizePanel, butt);
      yb += ngScale;
    }

    y += colHeight + 2 + ngPad;
  } else {
    y = 15.75;
  }

  r(result, buttonCenter(vec2(ngPad,y), vec2(ngWidth-ngPad*2, 1),
        strdup_s("Generate"), generateMap));
  y += 1;

  result->dim.start.y = (uiGridSizeY-y-ngPad)*.5f;
  result->dim.end.y = y + ngPad*3;
  return result;
}

