#include "optionsPanel.hpp"

#include "../building/building.hpp"
#include "../draw/camera.hpp"
#include "../draw/entity.hpp"
#include "../game/game.hpp"
#include "../icons.hpp"
#include "../input.hpp"
#include "../lot.hpp"
#include "../main.hpp"
#include "../option.hpp"
#include "../renderLand.hpp"
#include "../sound.hpp"
#include "../string_proxy.hpp"
#include "../time.hpp"
#include "../weather.hpp"

#include "spdlog/spdlog.h"

#include "button.hpp"
#include "hr.hpp"
#include "icon.hpp"
#include "label.hpp"
#include "messageBoard.hpp"
#include "panel.hpp"
#include "root.hpp"
#include "slider.hpp"
#include "scrollbox.hpp"
#include "textBox.hpp"

enum OptionsTabEnum {
  General = 0,
  Visual,
  Audio,
  Video,
  KeyMap,
  NumTabs
};

static OptionsTabEnum optionsTab = OptionsTabEnum::General;
static std::string optionsSongName = "";
const char* optionsTabGenTxt = "General";
const char* optionsTabKeyTxt = "Controls";
const char* optionsTabVisTxt = "Visual";
const char* optionsTabAudTxt = "Audio";
const char* optionsTabVidTxt = "Video";
const char* optionsBtnRestore = "Restore Defaults";
const char* optionsBtnCollisions = "Allow Collisions";

// Keymap static vars
static ScrollState keymapScrollState;
static InputAction keymapChangeAction = InputAction::ActNone;
static TextBoxState searchTB;
static char* searchText = 0;

const double collisionTimerTarget = 7.0;
static double collisionTimer = 0.0;

const float intervalSteps = 30.f;

const char* windowModeNames[numWindowModes] = {
  "Windowed Fullscreen", "Windowed", "Fullscreen"
};

DaylightMode daylightModeOrder[numDaylightModes] = {
  DaylightGameTime,
  DaylightAlwaysDay,
  DaylightAlwaysNight,
  DaylightSystemTime,
};

const char* daylightModeNames[numDaylightModes] = {
  "Show Time of Day",
  "Always Day",
  "Sync Daylight with System Time",
  "Always Night",
};

const int numValidMsaaSamples = 5;
item validMsaaSamples[numValidMsaaSamples] = {
  1, 2, 4, 8, 16
};

static bool focusSearchBox(Part* part, InputEvent event) {
  focusTextBox(&searchTB);
  return true;
}

static bool unfocusSearchBox(Part* part, InputEvent event) {
  focusTextBox(0);
  return true;
}

void setOptionsSongName(std::string name) {
  if(name.length() == 0) return;
  optionsSongName = name;
}

bool setOptionsTab(Part* part, InputEvent event) {
  OptionsTabEnum newTab = (OptionsTabEnum)part->itemData;

  if(newTab < 0 || newTab >= OptionsTabEnum::NumTabs) {
    SPDLOG_INFO("Invalid Options Tab value {} passed to setOptionsTab", (int)newTab);
    return true;
  }

  // If switching to the keymap, do a check for collisions 
  if (newTab == OptionsTabEnum::KeyMap) {
    detectKeyCollisionsAll();
  }

  // If switching off keymap when changing key, reset action
  if (optionsTab == OptionsTabEnum::KeyMap && newTab != optionsTab) {
    keymapChangeAction = InputAction::ActNone;
  }

  optionsTab = newTab;
  return true;
}

bool setDaylightMode(Part* part, InputEvent event) {
  setDaylightMode((DaylightMode) part->itemData);
  return true;
}

bool toggleOptionsFlag(Part* part, InputEvent event) {
  uint32_t optionsFlags = getOptionsFlags();
  uint32_t _flag = part->itemData;
  if (optionsFlags & _flag) {
    optionsFlags &= ~_flag;
  } else {
    optionsFlags |= _flag;
  }
  setOptionsFlags(optionsFlags);
  setLotsVisible();
  writeOptions();
  return true;
}

bool toggleLogUpload(Part* part, InputEvent event) {
  toggleLogUpload();
  writeOptions();
  return true;
}

bool toggleClassicRMB(Part* part, InputEvent event) {
  bool newVal = !getCameraClassicRMB();
  setCameraClassicRMB(newVal);
  setOptionsClassicRMB(newVal);
  writeOptions();
  return true;
}

bool toggleStartPaused(Part* part, InputEvent event) {
  bool newVal = !getOptionsStartPaused();
  setOptionsStartPaused(newVal);
  writeOptions();
  return true;
}

bool toggleEdgeScrolling(Part* part, InputEvent event) {
  bool newVal = !getOptionsEdgeScrolling();
  setCameraEdgeScrolling(newVal);
  setOptionsEdgeScrolling(newVal);
  writeOptions();
  return true;
}

bool toggleLockMouse(Part* part, InputEvent event) {
  bool newVal = !getOptionsLockMouse();
  setLockMouse(newVal);
  setOptionsLockMouse(newVal);
  writeOptions();
  return true;
}

bool toggleMuted(Part* part, InputEvent event) {
  VolumeControl v = (VolumeControl)part->itemData;
  bool muted = getMuted(v);
  setMuted(v, !muted);
  writeOptions();
  return true;
}

bool toggleShowWeather(Part* part, InputEvent event) {
  toggleShowWeather();
  writeOptions();
  return true;
}

bool toggleShowPollution(Part* part, InputEvent event) {
  toggleShowPollution();
  writeOptions();
  return true;
}

bool toggleTreesVisible(Part* part, InputEvent event) {
  bool newVisible = !getTreesVisible();
  setTreesVisible(newVisible);
  setOptionsTreesVisible(newVisible);
  writeOptions();
  return true;
}

bool toggleContourLinesVisible(Part* part, InputEvent event) {
  bool newVisible = !getContourLinesVisible();
  setContourLinesVisible(newVisible);
  writeOptions();
  return true;
}

bool toggleShowNewsaperMessage(Part* part, InputEvent event) {
  setShowNewspaperMessage(!isShowNewspaperMessage());
  if (isShowNewspaperMessage()) {
    addMessage(NewspaperMessage, 0);
  } else {
    removeMessageByObject(NewspaperMessage, 0);
  }
  writeOptions();
  return true;
}

bool setVolume(Part* part, InputEvent event) {
  VolumeControl v = (VolumeControl)part->itemData;
  bool muted = getMuted(v);
  float volume = part->vecData.x;
  setMuted(v, volume == 0);
  setVolume(v, volume);
  writeOptions();
  return true;
}

bool setFPSCap(Part* part, InputEvent event) {
  int cap = 120 * part->vecData.x + 15;
  cap = (cap/15)*15;
  setFPSCap(cap > 120 ? -1 : cap);
  writeOptions();
  return true;
}

bool setBrightness(Part* part, InputEvent event) {
  setBrightness(part->vecData.x);
  writeOptions();
  return true;
}

bool setMsaaSamples(Part* part, InputEvent event) {
  float sliderVal = part->vecData.x;
  item ndx = sliderVal*(numValidMsaaSamples-1);
  item aa = validMsaaSamples[ndx];
  if (aa != getMsaaSamples()) {
    setMsaaSamples(aa);
    writeOptions();
  }
  return true;
}

bool setFOV(Part* part, InputEvent event) {
  float val = int(part->vecData.x*18)/36.f;
  setFOV(val);
  writeOptions();
  renderSkybox();
  renderWeatherBox();
  return true;
}

bool setAutosaveInterval(Part* part, InputEvent event) {
  float secs = part->vecData.x*c(CMaxAutosaveInterval);
  secs += intervalSteps;
  if (secs > c(CMaxAutosaveInterval)) secs = 0;
  float val = int(secs/intervalSteps)*intervalSteps;
  setAutosaveInterval(val);
  writeOptions();
  return true;
}

bool setMeshQuality(Part* part, InputEvent event) {
  float sliderVal = part->vecData.x;
  if (sliderVal >= 1) {
    sliderVal = 50.f;

  } else if (sliderVal <= 0.5) {
    sliderVal *= 2;
    sliderVal = round(sliderVal*9)+1;
    sliderVal /= 10.f;

  } else {
    sliderVal -= 0.5;
    sliderVal *= 2;
    sliderVal = round(sliderVal*9)+1;
  }

  setMeshQuality(sliderVal);
  writeOptions();
  return true;
}

bool setWindowMode(Part* part, InputEvent event) {
  WindowMode mode = (WindowMode)part->itemData;
  setWindowMode(mode);
  writeOptions();
  return true;
}

bool toggleUseMetric(Part* part, InputEvent event) {
  setUseMetric(!useMetric());
  writeOptions();
  return true;
}

bool toggleDebugMode(Part* part, InputEvent event) {
  setDebugMode(!debugMode());
  writeOptions();
  return true;
}

bool toggleIssuesIcons(Part* part, InputEvent event) {
  setAlwaysShowIssuesIcons(!alwaysShowIssuesIcons());
  setIssuesIconsVisible();
  writeOptions();
  return true;
}

bool toggleAllowCollisions(Part* part, InputEvent event) {
  setAllowCollisions(!getAllowCollisions());
  // Write to input file?
  return true;
}

bool keymapStartSetNewKey(Part* part, InputEvent event) {
  if (part == NULL) {
    SPDLOG_ERROR("Null part in keymapStartSetNewKey");
    return false;
  }

  focusTextBox(0);
  if (event.isMouse && event.isButtonDown[RMB]) {
    unbindAction((InputAction)part->itemData);
    return true;
  }

  // Abort key change if clicked again
  if (part->itemData == (int)keymapChangeAction) {
    keymapChangeAction = InputAction::ActNone;
    return true;
  }

  keymapChangeAction = (InputAction)part->itemData;
  return true;
}

bool keymapFinishSetNewKey(int32_t newKey) {

  bindKeyToAction(keymapChangeAction, newKey);
  // Sweep all keybinds for collisions; this should reset flags if we are no longer colliding
  detectKeyCollisionsAll();
  keymapChangeAction = InputAction::ActNone;

  return true;
}

bool keymapRestoreDefaults(Part* part, InputEvent event) {
  // Start at 1; ActNone is 0
  for (int i = 1; i < InputAction::NumActions; i++) {
    if (!bindKeyToAction((InputAction)i, getDefaultKeyForAction((InputAction)i), false)) {
      SPDLOG_ERROR("Error restoring keybind for action {} to default key", getActionStr((InputAction)i, true));
    }
  }

  // Check for any key collisions, just in case
  detectKeyCollisionsAll();

  SPDLOG_INFO("Restored all keybinds to default keys");
  return true;
}

void closeOptionsMenu() {
  keymapChangeAction = InputAction::ActNone;
  writeInputFile(); // Write any changes in the keymap to the input.data file
  clearCollidedAll(); // Clear the collision flags for keybinds
}

Part* optionsPanel(float aspectRatio) {
  float optionsPad = 0.25;
  float oWidth = 22;
  float oHeight = 22;
  vec2 optionsPanelSizeInner =
    vec2(oWidth+optionsPad*2, oHeight+optionsPad*2);
  vec2 optionsPanelSizePadded = optionsPanelSizeInner +
    vec2(optionsPad*2, optionsPad*2);
  float uiX = uiGridSizeX * aspectRatio;
  Part* result = panel(
      vec2(uiX,uiGridSizeY)*0.5f - optionsPanelSizePadded*0.5f,
      optionsPanelSizePadded);
  result->padding = optionsPad;
  result->flags |= _partLowered;
  float halfWidth = (oWidth - optionsPad)*.5f;
  float y = 0;
  float scale = 1;
  float subScale = 0.85;

  r(result, label(vec2(0,0), 1.25f, strdup_s("Options")));
  r(result, button(vec2(oWidth-.75f,0), iconX, vec2(1.25f, 1.25f),
        openMainMenu, 0));
  y += 1.25f;
  //r(result, hr(vec2(0,y), oWidth));
  y += 0.25;

  // Tab buttons
  float tabX = 0.0f;
  float tabYS = 1 + optionsPad;
  float tabSizeX = optionsPanelSizeInner.x / (int)OptionsTabEnum::NumTabs;
  Part* genTab = button(vec2(tabX, y), vec2(tabSizeX, tabYS),
      strdup_s(optionsTabGenTxt), setOptionsTab);
  genTab->itemData = OptionsTabEnum::General;
  genTab->flags |= _partAlignCenter;
  if(optionsTab == OptionsTabEnum::General) genTab->flags |= _partRaised;
  tabX += tabSizeX;
  r(result, genTab);

  Part* visTab = button(vec2(tabX, y), vec2(tabSizeX, tabYS),
    strdup_s(optionsTabVisTxt), setOptionsTab);
  visTab->itemData = OptionsTabEnum::Visual;
  visTab->flags |= _partAlignCenter;
  if(optionsTab == OptionsTabEnum::Visual) visTab->flags |= _partRaised;
  tabX += tabSizeX;
  r(result, visTab);

  Part* vidTab = button(vec2(tabX, y), vec2(tabSizeX, tabYS),
      strdup_s(optionsTabVidTxt), setOptionsTab);
  vidTab->itemData = OptionsTabEnum::Video;
  vidTab->flags |= _partAlignCenter;
  if(optionsTab == OptionsTabEnum::Video) vidTab->flags |= _partRaised;
  tabX += tabSizeX;
  r(result, vidTab);

  Part* audTab = button(vec2(tabX, y), vec2(tabSizeX, tabYS),
      strdup_s(optionsTabAudTxt), setOptionsTab);
  audTab->itemData = OptionsTabEnum::Audio;
  audTab->flags |= _partAlignCenter;
  if(optionsTab == OptionsTabEnum::Audio) audTab->flags |= _partRaised;
  tabX += tabSizeX;
  r(result, audTab);

  Part* keyTab = button(vec2(tabX, y), vec2(tabSizeX, tabYS),
      strdup_s(optionsTabKeyTxt), setOptionsTab);
  keyTab->itemData = OptionsTabEnum::KeyMap;
  keyTab->flags |= _partAlignCenter;
  if(optionsTab == OptionsTabEnum::KeyMap) keyTab->flags |= _partRaised;
  r(result, keyTab);

  y += tabYS;

  optionsPanelSizeInner.y -= y;
  Part* tabPanel = r(result, panel(vec2(0, y), optionsPanelSizeInner));
  tabPanel->padding = optionsPad;
  y = 0;

  if(optionsTab == OptionsTabEnum::General) {
    y += (scale + optionsPad) * 0.5;

    r(tabPanel, button(vec2(0, y),
      hasEnabledLogUpload() ? iconCheck : iconNull,
      vec2(oWidth, scale), strdup_s("Enable Log Upload"),
      toggleLogUpload, 0));
    y += scale + optionsPad;

    r(tabPanel, button(vec2(0, y),
      useMetric() ? iconCheck : iconNull,
      vec2(oWidth, scale), strdup_s("Use Metric"),
      toggleUseMetric, 0));
    y += scale + optionsPad;

    #ifdef LP_DEBUG
      r(tabPanel, button(vec2(0, y),
        debugMode() ? iconCheck : iconNull,
        vec2(oWidth, scale), strdup_s("Debug Mode"),
        toggleDebugMode, 0));
      y += scale + optionsPad;
    #endif
    // y += 1.0f;

    r(tabPanel, button(vec2(0, y),
      isShowNewspaperMessage() ? iconCheck : iconNull,
      vec2(oWidth, scale), strdup_s("Show Newspaper in Sidebar"),
      toggleShowNewsaperMessage, 0));
    y += scale + optionsPad;

    r(tabPanel, button(vec2(0, y),
      getOptionsClassicRMB() ? iconCheck : iconNull,
      vec2(oWidth, scale), strdup_s("Classic RMB Camera"),
      toggleClassicRMB, 0));
    y += scale + optionsPad;

    r(tabPanel, button(vec2(0, y),
      getOptionsEdgeScrolling() ? iconCheck : iconNull,
      vec2(oWidth, scale), strdup_s("Camera Edge Scrolling"),
      toggleEdgeScrolling, 0));
    y += scale + optionsPad;

    r(tabPanel, button(vec2(0, y),
      getOptionsLockMouse() ? iconCheck : iconNull,
      vec2(oWidth, scale), strdup_s("Lock Mouse to Window"),
      toggleLockMouse, 0));
    y += scale + optionsPad;

    r(tabPanel, button(vec2(0, y),
      getOptionsStartPaused() ? iconCheck : iconNull,
      vec2(oWidth, scale), strdup_s("Start Paused"),
      toggleStartPaused, 0));
    y += scale + optionsPad;

    // Autosave slider
    float interval = getAutosaveInterval();
    char* durStr = printDurationString(interval/24/60/60);
    r(tabPanel, label(vec2(0, y), scale,
      interval == 0 ? strdup_s("Autosave Disabled") :
      sprintf_o("Autosave Every %s", durStr)));
    free(durStr);

    float slideVal = (interval-intervalSteps)/c(CMaxAutosaveInterval);
    if (interval == 0) slideVal = 1;
    slideVal = clamp(slideVal, 0.f, 1.f);
    r(tabPanel, slider(vec2(halfWidth+optionsPad, y), vec2(halfWidth, scale),
      slideVal, setAutosaveInterval));
    y += scale + optionsPad;

  } else if(optionsTab == OptionsTabEnum::Visual) {
    y += (scale + optionsPad) * 0.5;

    r(tabPanel, button(vec2(0, y),
      isShowWeather() ? iconCheck : iconNull,
      vec2(oWidth, scale), strdup_s("Show Rain and Snow"),
      toggleShowWeather, 0));
    y += scale + optionsPad;

    r(tabPanel, button(vec2(0, y),
      (getOptionsFlags() & _optionHideLots) ? iconNull : iconCheck,
      vec2(oWidth, scale), strdup_s("Show Zoned Lots"),
      toggleOptionsFlag, _optionHideLots));
    y += scale + optionsPad;

    r(tabPanel, button(vec2(0, y),
      isShowPollution() ? iconCheck : iconNull,
      vec2(oWidth, scale), strdup_s("Show Pollution"),
      toggleShowPollution, 0));
    y += scale + optionsPad;

    r(tabPanel, button(vec2(0, y),
      getTreesVisible() ? iconCheck : iconNull,
      vec2(oWidth, scale), strdup_s("Show Trees"),
      toggleTreesVisible, 0));
    y += scale + optionsPad;

    r(tabPanel, button(vec2(0, y),
      getContourLinesVisible() ? iconCheck : iconNull,
      vec2(oWidth, scale), strdup_s("Show Contour Lines"),
      toggleContourLinesVisible, 0));
    y += scale + optionsPad;

    r(tabPanel, button(vec2(0, y),
      !(getOptionsFlags() & _optionHideGridlines) ? iconCheck : iconNull,
      vec2(oWidth, scale), strdup_s("Show Grid Lines"),
      toggleOptionsFlag, _optionHideGridlines));
    y += scale + optionsPad;

    r(tabPanel, button(vec2(0, y),
      alwaysShowIssuesIcons() ? iconCheck : iconNull,
      vec2(oWidth, scale), strdup_s("Always Show Issues Icons"),
      toggleIssuesIcons, 0));
    y += scale + optionsPad;

    float winy = 0;
    Part* daylightPanel = panel(vec2(0, optionsPanelSizeInner.y-7-optionsPad*8),
      vec2(oWidth, 4+optionsPad*3));
    daylightPanel->padding = optionsPad;
    daylightPanel->flags |= _partLowered | _partClip;
    r(tabPanel, daylightPanel);
    y += daylightPanel->dim.end.y + optionsPad;

    for(int k = 0; k < numDaylightModes; k++) {
      item i = daylightModeOrder[k];
      Part* butt = button(vec2(0, winy),
        i == getDaylightMode() ? iconCheck : iconNull,
        vec2(oWidth-optionsPad*2, scale),
        strdup_s(daylightModeNames[i]), setDaylightMode, i);
      r(daylightPanel, butt);
      winy += scale;
    }

  } else if(optionsTab == OptionsTabEnum::Audio) {
    y += scale + optionsPad;
    r(tabPanel, label(vec2(0.0f, y), scale, strdup_s("Current Song:")));
    r(tabPanel, label(vec2(6.0f, y), scale,
          strdup_s(optionsSongName.c_str())));

    y += (scale + optionsPad)*4.f;
    for(int i = 0; i < numVolumes; i++) {
      bool muted = getMuted((VolumeControl)i);
      float volume = getVolume((VolumeControl)i);
      r(tabPanel, button(vec2(0, y),
        muted ? iconNull : iconCheck,
        vec2(halfWidth, scale), strdup_s(getVolumeName((VolumeControl)i)),
        toggleMuted, i));
      Part* volSlide = slider(vec2(halfWidth+optionsPad, y),
          vec2(halfWidth, scale), volume, setVolume);
      volSlide->itemData = i;
      r(tabPanel, volSlide);
      y += scale + optionsPad*2;
    }

  } else if(optionsTab == OptionsTabEnum::Video) {

    Part* fpsLbl = r(tabPanel, labelRight(vec2(0, y),
          vec2(oWidth-optionsPad, scale),
          sprintf_o("You're getting %d FPS", int(getFPS()))));
    if (getFPS() < 20) {
      fpsLbl->foregroundColor = RedMedL;
    }
    //y += (scale + optionsPad)*2.0f; // Give some extra room

    // FOV slider label
    r(tabPanel, label(vec2(0, y), scale,
      strdup_s("Camera Perspective")));
    y += scale + optionsPad;

    // FOV slider
    float fov = getFOV();
    r(tabPanel, label(vec2(0, y), scale,
      fov == 0 ? strdup_s("Orthographic Perspective") :
      sprintf_o("Field of View %ddeg", int(fov*180))));
    r(tabPanel, slider(vec2(halfWidth+optionsPad, y), vec2(halfWidth, scale),
      fov*2.f, setFOV));
    y += scale + optionsPad;

    //r(tabPanel, label(vec2(0, y), subScale,
      //strdup_s("Move slider left for Ortho, right to increase FOV")));
    //y += (scale + optionsPad)*1.0f; // Give some extra room

    if(c(CEnableFPSCap)) {
      int cap = getFPSCap();
      cap = cap < 0 ? 135 : cap;
      char* fpsLabel = cap > 120 ? strdup_s("FPS Cap: None") :
        sprintf_o("FPS Cap: %d", cap);
      r(tabPanel, label(vec2(0, y), scale, fpsLabel));
      Part* fpsSlide = slider(vec2(halfWidth+optionsPad, y),
          vec2(halfWidth, scale), (cap-15)/120.f, setFPSCap);
      r(tabPanel, fpsSlide);
      y += scale + optionsPad;
    }

    float quality = getMeshQuality();
    if (quality >= 10) {
      quality = 1;
    } else if (quality <=1) {
      quality = quality*10-1;
      quality /= 9;
      quality *= .5f;
    } else {
      quality -= 1;
      quality /= 9;
      quality *= .5f;
      quality += .5f;
    }

    r(tabPanel, label(vec2(0, y), scale, strdup_s("Level of Detail")));
    r(tabPanel, slider(vec2(halfWidth+optionsPad, y),
      vec2(halfWidth, scale), quality, setMeshQuality));
    //y += (scale + optionsPad)*1.0f;
    Part* perfLbl = r(tabPanel, label(vec2(halfWidth+optionsPad, y),
          subScale, strdup_s("Performance")));
    perfLbl->foregroundColor = PickerPalette::GrayLight;
    Part* qualLbl = r(tabPanel, labelRight(vec2(halfWidth+optionsPad, y),
      vec2(halfWidth, subScale), strdup_s("Quality")));
    qualLbl->foregroundColor = PickerPalette::GrayLight;
    y += (scale + optionsPad)*1.0f;

    item aa = getMsaaSamples();
    float aaSlider = 0;
    for (int i = 0; i < numValidMsaaSamples; i++) {
      if (validMsaaSamples[i] <= aa) {
        aaSlider = float(i)/(numValidMsaaSamples-1);
      }
    }

    char* aaLabel = aa == 1 ? strdup_s("Anti Aliasing: None") :
      sprintf_o("Anti Aliasing: %dx", aa);
    r(tabPanel, label(vec2(0, y), scale, aaLabel));
    r(tabPanel, slider(vec2(halfWidth+optionsPad, y), vec2(halfWidth, scale),
      aaSlider, setMsaaSamples));
    y += scale;
    r(tabPanel, label(vec2(scale, y), subScale,
          strdup_s("(Requires Restart)")));
    y += (subScale + optionsPad)*1.0f;

    float brightness = getBrightness();
    r(tabPanel, label(vec2(0, y), scale, strdup_s("Brightness")));
    r(tabPanel, slider(vec2(halfWidth+optionsPad, y), vec2(halfWidth, scale),
      brightness, setBrightness));
    r(tabPanel, icon(vec2(halfWidth+optionsPad, y),
          vec2(scale, scale), iconWeatherMoon));
    r(tabPanel, icon(vec2(halfWidth*2+optionsPad-scale, y),
          vec2(scale, scale), iconWeatherSun));
    y += (scale + optionsPad)*1.0f;

    float winy = 0;
    Part* videoPanel = panel(vec2(0, optionsPanelSizeInner.y-3-optionsPad*4),
      vec2(oWidth, 3+optionsPad*2));
    videoPanel->padding = optionsPad;
    videoPanel->flags |= _partLowered | _partClip;
    r(tabPanel, videoPanel);
    y += videoPanel->dim.end.y + optionsPad;

    winy = 0;
    for(int i = 0; i < numWindowModes; i++) {
      Part* butt = button(vec2(0, winy),
        i == getWindowMode() ? iconCheck : iconNull,
        vec2(oWidth-optionsPad*2, scale),
        strdup_s(windowModeNames[i]), setWindowMode, i);
      r(videoPanel, butt);
      winy += scale;
    }

  } else if(optionsTab == OptionsTabEnum::KeyMap) {
    y += (scale * 2.0f) + (optionsPad * 4.0f);

    float keymapWidth = optionsPanelSizeInner.x - (optionsPad * 2.0f);
    float bindWidth = keymapWidth - (0.5f + optionsPad*2.0f);
    float bindHeight = scale+optionsPad*3;
    float keyValOffset = 12.0f;
    float internalY = optionsPad;

    // Restore Defaults button
    r(result, button(vec2(optionsPad, y), vec2(7.0f-optionsPad, scale), strdup_s(optionsBtnRestore), keymapRestoreDefaults));
    
    // Allow Collisions toggle
    r(result, button(vec2(7.0f+optionsPad, y),
      getAllowCollisions() ? iconCheck : iconNull,
      vec2(7.0f+(optionsPad*2.0f), scale), strdup_s(optionsBtnCollisions),
      toggleAllowCollisions, 0));

    // Explanation of red collision text
    Part* collisionLabel = labelRight(vec2(optionsPad, y), vec2(keymapWidth-optionsPad, scale), strdup_s("Red Key: Collision"));
    collisionLabel->foregroundColor = PickerPalette::RedLight;
    r(result, collisionLabel);
    y += scale+optionsPad;

    r(result, label(vec2(optionsPad*2, y), scale, strdup_s("Action")));
    r(result, labelRight(vec2(optionsPad*2, y), vec2(bindWidth, scale), strdup_s("Key")));

    searchTB.text = &searchText;
    float searchBoxX = bindWidth*.25f;
    r(result, icon(vec2(searchBoxX-scale, y),
          vec2(scale, scale), iconQuery));
    Part* tb = r(result, textBox(vec2(searchBoxX,y),
          vec2(bindWidth*.5f, scale), &searchTB));
    tb->onClick = focusSearchBox;
    tb->onCustom = unfocusSearchBox;
    if (searchText == 0 || strlength(searchText) == 0) {
      Part* search = r(result, label(vec2(searchBoxX,y), scale, strdup_s("Search")));
      search->foregroundColor = PickerPalette::GrayDark;
    }

    y += scale+optionsPad;

    Part* keymapScrollbox = scrollbox(vec2(0, 0),
      vec2(keymapWidth, (InputAction::NumActions+InputGroup::NumInputGroups)*bindHeight));
    keymapScrollbox->flags |= _partLowered;

    // Check for changing action, and check for key if changing
    bool changingAction = keymapChangeAction != InputAction::ActNone;
    if (changingAction) {
      int32_t key = getFirstKeyPressed();
      if (key != GLFW_KEY_UNKNOWN) {
        keymapFinishSetNewKey(key);
      }
    }

    bool anyMatches = false;
    bool search = searchText != 0 && strlength(searchText) > 0;

    InputGroup currentGroup = InputGroup::GroupNone;
    // Start from 1, because InputAction::ActNone is 0
    for (int i = 1; i < (int)InputAction::NumActions; i++) {
      KeyBind bind = getKeyBind(i);
      InputGroup nextGroup = getInputGroupForAction(bind.action);

      // Filter on search
      if (search) {
        if (!doesSearchStringMatch(bind, searchText)) continue;
        anyMatches = true;
      }

      if (nextGroup != currentGroup) {
        currentGroup = nextGroup;
        r(keymapScrollbox, label(vec2(0.0f, internalY), scale,
          strdup_s(std::string("Category: " + getInputGroupStr(currentGroup)).c_str())));
        r(keymapScrollbox, hr(vec2(0.0f, internalY+scale+0.1f), keymapWidth-(optionsPad*4.0f)));
        internalY += bindHeight;
      }

      //r(keymapScrollbox, hr(vec2(optionsPad, internalY), bindWidth));
      //internalY += scale;
      r(keymapScrollbox, label(vec2(0, internalY), scale, 
        strdup_s(bind.actUIStr().c_str())));

      Part* keyLabel = labelRight(vec2(0, internalY), vec2(bindWidth-optionsPad, scale),
        strdup_s(keymapChangeAction == (InputAction)i ? "???" : bind.keyStr().c_str()));
      if (bind.collided) {
        keyLabel->foregroundColor = PickerPalette::RedLight;
      } else if (bind.key == GLFW_KEY_UNKNOWN) {
        keyLabel->foregroundColor = PickerPalette::OrangeLight;
      }
      r(keymapScrollbox, keyLabel);

      Part* keybindButton = button(vec2(0, internalY-optionsPad), vec2(bindWidth, scale+optionsPad*2), strdup_s(""), keymapStartSetNewKey);
      keybindButton->itemData = i;
      keybindButton->flags |= keymapChangeAction == (InputAction)i ? _partHighlight : 0;

      r(keymapScrollbox, keybindButton);
      internalY += bindHeight;
    }

    if (!anyMatches) {
      Part* noMatch = r(keymapScrollbox, label(vec2(0,internalY), scale, sprintf_o("No keymaps found matching \"%s\".", searchText)));
      noMatch->foregroundColor = PickerPalette::GrayLight;
    }

    // HR after last keybind
    //r(keymapScrollbox, hr(vec2(optionsPad, internalY), bindWidth));
    //internalY += scale;

    Part* keymapScrollboxFrame = scrollboxFrame(vec2(optionsPad, y),
      vec2(keymapWidth, oHeight - y + optionsPad),
      &keymapScrollState, keymapScrollbox);

    r(result, keymapScrollboxFrame);

  } else {
    r(tabPanel, label(vec2(0, y), 1.25f, strdup_s("ERROR")));
    y += 1.25f;
  }

  //result->dim.start.y = (uiGridSizeY-oHeight*1.5f-optionsPad)*.5f;
  //result->dim.end.y = (oHeight*1.5f)+optionsPad*2;
  return result;
}

