#pragma once

#include "item.hpp"

#include <cstdint>

const int _optionSeenLogUploadMessage = 1 << 0;
const int _optionLogUpload = 1 << 1;
const int _optionMetric = 1 << 2;
const int _optionDebugMode = 1 << 3;
const int _optionHasCompletedTutorial = 1 << 4;
const int _optionSeenTutorialPopup = 1 << 5;
const int _optionIssuesIcons = 1 << 6;
const int _optionChangelogClosed = 1 << 7;
const int _optionHideWeather = 1 << 10;
const int _optionClassicRMB = 1 << 11;
const int _optionHidePollution = 1 << 12;
const int _optionHideTrees = 1 << 13;
const int _optionHideContourLines = 1 << 14;
const int _optionHideNewspaperMessage = 1 << 15;
const int _optionHideLots = 1 << 16;
const int _optionHideGridlines = 1 << 17;
const int _optionWasTestMode = 1 << 18;
const int _optionStartPaused = 1 << 19;
const int _optionEdgeScrolling = 1 << 20;
const int _optionLockMouse = 1 << 21;
const int _optionPinHeatmaps = 1 << 22;

const int _optionDaylightModeShift = 8;
const int _optionDaylightModeMask = 3 << _optionDaylightModeShift;

enum DaylightMode {
  DaylightGameTime, DaylightAlwaysDay, DaylightSystemTime, DaylightAlwaysNight,
  numDaylightModes
};

void readOptions();
void writeOptions();

uint32_t getOptionsFlags();
void setOptionsFlags(uint32_t flags);
void setOptionsClassicRMB(bool val);
bool getOptionsClassicRMB();
void setOptionsStartPaused(bool val);
bool getOptionsStartPaused();
void setOptionsEdgeScrolling(bool val);
bool getOptionsEdgeScrolling();
void setOptionsLockMouse(bool val);
bool getOptionsLockMouse();
void setChangelogClosed(bool closed);
bool isChangelogClosed();
bool hasSeenInfoMessage();
void setHasSeenInfoMessage(bool seen);
bool hasEnabledLogUpload();
void setHasEnabledLogUpload(bool enabled);
bool hasCompletedTutorial();
void setHasCompletedTutorial(bool enabled);
void setUseMetric(bool val);
bool useMetric();
void setDebugMode(bool val);
bool debugMode();
bool alwaysShowIssuesIcons();
void setAlwaysShowIssuesIcons(bool val);
bool seenTutorialPopup();
void setSeenTutorialPopup(bool val);
void setLogUpload(bool val);
void toggleShowWeather();
bool isShowWeather();
void toggleShowPollution();
bool isShowPollution();
bool getOptionsTreesVisible();
void setOptionsTreesVisible(bool visible);
void setContourLinesVisible(bool visible);
bool getContourLinesVisible();
void setShowNewspaperMessage(bool show);
bool isShowNewspaperMessage();
void setWasTestMode(bool val);
bool wasTestMode();
void setPinHeatmaps(bool val);
bool pinHeatmaps();

DaylightMode getDaylightMode();
void setDaylightMode(DaylightMode mode);

void setFeatureEnabledGlobal(item ndx);
bool isFeatureEnabledGlobal(item ndx);
bool wasFeatureUsed(item ndx);
void featureUsed(item ndx);

bool hasSeenLogUploadMessage();
void setSeenLogUploadMessage();
bool hasEnabledLogUpload();
void toggleLogUpload();

