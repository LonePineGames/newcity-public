#include "option.hpp"

#include "cup.hpp"
#include "draw/camera.hpp"
#include "draw/entity.hpp"
#include "game/game.hpp"
#include "game/version.hpp"
#include "input.hpp"
#include "parts/menuBar.hpp"
#include "parts/toolbar.hpp"
#include "parts/tutorialPanel.hpp"
#include "platform/mod.hpp"
#include "renderLand.hpp"
#include "serialize.hpp"
#include "sound.hpp"
#include "time.hpp"
#include "tutorial.hpp"

#include "spdlog/spdlog.h"
#include <stdio.h>

const int _optfileModSet = 1 << 0;

item optionFlags = _optionMetric | _optionChangelogClosed;

Cup<unsigned char> featureWasUsed;
Cup<unsigned char> featureEnabledGlobal;

bool isUserBurger() { // Whether to use metric or freedom
  std::string local = std::locale().name();
  SPDLOG_INFO("User Locale: {}", local);
  return startsWith(local.c_str(), "en.US");
  //return local == "en.US";
  //return local.language() == "en" && local.country() == "US";
}

void setOptionsClassicRMB(bool val) {
  if(val) {
    optionFlags |= _optionClassicRMB;
  } else {
    optionFlags &= ~_optionClassicRMB;
  }
}

uint32_t getOptionsFlags() {
  return optionFlags;
}

void setOptionsFlags(uint32_t flags) {
  optionFlags = flags;
}

bool getOptionsClassicRMB() {
  return optionFlags & _optionClassicRMB;
}

void setOptionsStartPaused(bool val) {
  if(val) {
    optionFlags |= _optionStartPaused;
  } else {
    optionFlags &= ~_optionStartPaused;
  }
}

bool getOptionsStartPaused() {
  return optionFlags & _optionStartPaused;
}

void setOptionsEdgeScrolling(bool val) {
  if (val) {
    optionFlags |= _optionEdgeScrolling;
  } else {
    optionFlags &= ~_optionEdgeScrolling;
  }
}

bool getOptionsEdgeScrolling() {
  return optionFlags & _optionEdgeScrolling;
}

void setOptionsLockMouse(bool val) {
  if (val) {
    optionFlags |= _optionLockMouse;
  } else {
    optionFlags &= ~_optionLockMouse;
  }
}

bool getOptionsLockMouse() {
  return optionFlags & _optionLockMouse;
}

bool isChangelogClosed() {
  return optionFlags & _optionChangelogClosed;
}

void setChangelogClosed(bool closed) {
  if (closed) {
    optionFlags |= _optionChangelogClosed;
  } else {
    optionFlags &= ~_optionChangelogClosed;
  }
}

bool alwaysShowIssuesIcons() {
  return optionFlags & _optionIssuesIcons;
}

void setAlwaysShowIssuesIcons(bool val) {
  if (val) {
    optionFlags |= _optionIssuesIcons;
  } else {
    optionFlags &= ~_optionIssuesIcons;
  }
}

bool useMetric() {
  return optionFlags & _optionMetric;
}

void setUseMetric(bool val) {
  if (val) {
    optionFlags |= _optionMetric;
  } else {
    optionFlags &= ~_optionMetric;
  }
}

bool debugMode() {
  // #ifdef LP_DEBUG
    return optionFlags & _optionDebugMode;
  // #else
    // return false;
  // #endif
}

void setDebugMode(bool val) {
  if (val) {
    optionFlags |= _optionDebugMode;
  } else {
    optionFlags &= ~_optionDebugMode;
  }
}

bool hasCompletedTutorial() {
  // return false;
  return optionFlags & _optionHasCompletedTutorial;
}

void setHasCompletedTutorial(bool val) {
  setSeenTutorialPopup(true);
  if (val) {
    optionFlags |= _optionHasCompletedTutorial;
  } else {
    optionFlags &= ~_optionHasCompletedTutorial;
  }

  /*
  // Sync with TutorialState
  TutorialState* ptr = getTutorialStatePtr();
  if (ptr != 0) {
    ptr->setTutorialActive(tutorialMode());
    ptr->resetState();
  }
  */
}

bool seenTutorialPopup() {
  return optionFlags & _optionSeenTutorialPopup;
}

void setSeenTutorialPopup(bool val) {
  if (val) {
    optionFlags |= _optionSeenTutorialPopup;
  } else {
    optionFlags &= ~_optionSeenTutorialPopup;
  }
}

bool wasFeatureUsed(item ndx) {
  if (getGameMode() == ModeTest) return true;
  featureWasUsed.ensureSize(ndx/8+1);
  return featureWasUsed[ndx/8] & (1 << (ndx%8));
}

void featureUsed(item ndx) {
  if (wasFeatureUsed(ndx)) return;
  featureWasUsed.ensureSize(ndx/8+1);
  unsigned char byte = featureWasUsed[ndx/8] | (1 << (ndx%8));
  featureWasUsed.set(ndx/8, byte);
  writeOptions();
}

bool isFeatureEnabledGlobal(item ndx) {
  if (hasCompletedTutorial()) return true;
  featureEnabledGlobal.ensureSize(ndx/8+1);
  return featureEnabledGlobal[ndx/8] & (1 << (ndx%8));
}

void setFeatureEnabledGlobal(item ndx) {
  if (isFeatureEnabledGlobal(ndx)) return;
  featureEnabledGlobal.ensureSize(ndx/8+1);
  unsigned char byte = featureEnabledGlobal[ndx/8] | (1 << (ndx%8));
  featureEnabledGlobal.set(ndx/8, byte);
  writeOptions();
}

bool hasSeenLogUploadMessage() {
  return optionFlags & _optionSeenLogUploadMessage;
}

void setSeenLogUploadMessage() {
  optionFlags |= _optionSeenLogUploadMessage;
}

bool hasEnabledLogUpload() {
  return optionFlags & _optionLogUpload;
}

void setLogUpload(bool val) {
  if (val) {
    optionFlags |= _optionLogUpload;
  } else {
    optionFlags &= ~_optionLogUpload;
  }
}

void toggleLogUpload() {
  if (optionFlags & _optionLogUpload) {
    optionFlags &= ~_optionLogUpload;
  } else {
    optionFlags |= _optionLogUpload;
  }
}

DaylightMode getDaylightMode() {
  return (DaylightMode) ((optionFlags & _optionDaylightModeMask) >>
    _optionDaylightModeShift);
}

void setDaylightMode(DaylightMode mode) {
  optionFlags &= ~_optionDaylightModeMask;
  optionFlags |= mode << _optionDaylightModeShift;
}

void toggleShowPollution() {
  if (optionFlags & _optionHidePollution) {
    optionFlags &= ~_optionHidePollution;
  } else {
    optionFlags |= _optionHidePollution;
  }
}

bool isShowPollution() {
  return !(optionFlags & _optionHidePollution);
}

bool getOptionsTreesVisible() {
  return !(optionFlags & _optionHideTrees);
}

void setOptionsTreesVisible(bool visible) {
  !(visible) ?
    optionFlags |= _optionHideTrees :
    optionFlags &= ~_optionHideTrees ;
}

bool getContourLinesVisible() {
  return !(optionFlags & _optionHideContourLines);
}

void setContourLinesVisible(bool visible) {
  !(visible) ?
    optionFlags |= _optionHideContourLines:
    optionFlags &= ~_optionHideContourLines;
}

void toggleShowWeather() {
  if (optionFlags & _optionHideWeather) {
    optionFlags &= ~_optionHideWeather;
  } else {
    optionFlags |= _optionHideWeather;
  }
}

bool isShowWeather() {
  return !(optionFlags & _optionHideWeather);
}

void setShowNewspaperMessage(bool val) {
  if (val) {
    optionFlags &= ~_optionHideNewspaperMessage;
  } else {
    optionFlags |= _optionHideNewspaperMessage;
  }
}

bool wasTestMode() {
  return !(optionFlags & _optionWasTestMode);
}

void setWasTestMode(bool val) {
  if (val) {
    optionFlags &= ~_optionWasTestMode;
  } else {
    optionFlags |= _optionWasTestMode;
  }
}

bool isShowNewspaperMessage() {
  return !(optionFlags & _optionHideNewspaperMessage);
}

void setPinHeatmaps(bool val) {
  if (val) {
    optionFlags |= _optionPinHeatmaps;
  } else {
    optionFlags &= ~_optionPinHeatmaps;
  }
}

bool pinHeatmaps() {
  return optionFlags & _optionPinHeatmaps;
}

void readOptions() {
  FILE* fileHandle;
  const char* filename = "options.data";
  FileBuffer buf = readFromFile(filename);
  FileBuffer* file = &buf;

  if (file->length > 0) {
    char header[4];
    fread(&header, sizeof(char), 4, file);
    int version = fread_int(file);
    file->version = version;
    SPDLOG_INFO("reading options file, version {}", version);

    if (version < 0) {
      SPDLOG_INFO("option file corrupt, ignoring");
      freeBuffer(file);
      setMod("yours");
      writeOptions();
      return;
    }

    if (version > saveVersion) {
      SPDLOG_INFO("option file too new, ignoring");
      freeBuffer(file);
      setMod("yours");
      writeOptions();
      return;
    }

    if (version >= 37) {
      optionFlags = fread_item(file, version);
    }

    setWindowMode((WindowMode) fread_item(file, version));
    setShowInstructions(fread_item(file, version));
    for (int i = 0; i < numVolumes; i++) {
      VolumeControl v = (VolumeControl)i;
      setMuted(v, fread_item(file, version));
      setVolume(v, fread_float(file));
    }

    if (version >= 44) {
      setFPSCap(fread_int(file));
    }

    if (version >= 46) {
      setBrightness(fread_float(file));
    }

    if (version >= 52) {
      setFOV(fread_float(file));
      setCameraClassicRMB(getOptionsClassicRMB());
    }

    if (version >= 58) {
      setCameraEdgeScrolling(getOptionsEdgeScrolling());
      setLockMouse(getOptionsLockMouse());
    }

    //setFOV(c(CCameraFOV));

    if (version >= 45) {
      featureEnabledGlobal.read(file, version);
      featureWasUsed.read(file, version);
    }

    int fileFlags = fread_int(file);
    char* mod = fread_string(file);
    setMod(mod);

    if(fileFlags & _optfileModSet && mod != 0) {
      SPDLOG_INFO("Using modpack {}", mod);
    } else {
      SPDLOG_INFO("Mods disabled or mod name null");
    }

    free(mod);

    if (version >= 51) {
      setFPSMode(fread_int(file));
    }

    if (version >= 54) {
      suggestGameMode(fread_int(file));
      setTreesVisible(getOptionsTreesVisible());
    }

    if (version >= 55) {
      setMeshQuality(fread_float(file));
    } else {
      setMeshQuality(0);
    }

    if (version >= 58) {
      setAutosaveInterval(fread_float(file));
    }

    //if (version < 58 || (version == 58 && file->patchVersion < 13)) {
      //setHasCompletedTutorial(false);
    //}
    setTutorialPanelOpen(!(optionFlags & _optionHasCompletedTutorial));

    if (version >= 58) {
      item msaaSamples = fread_int(file);
      if (msaaSamples > 0 && msaaSamples <= 16) {
        setMsaaSamples(msaaSamples);
      }
    }

    freeBuffer(file);

    if (version < saveVersion) {
      writeOptions();
    }

  } else { // No Options file, write a new one
    setMod("yours");

    if (isUserBurger()) {
      optionFlags &= ~_optionMetric;
    } else {
      optionFlags |= _optionMetric;
    }

    writeOptions();
  }
}

void writeOptions() {
  FILE *fileHandle;
  const char* filename = "options.data";

  if (fileHandle = fopen(filename, "wb")) {
    FileBuffer buf = makeFileBuffer();
    FileBuffer* file = &buf;
    SPDLOG_INFO("Writing options file, version {}", saveVersion);
    fputs("OPTS", file);
    fwrite_int(file, saveVersion);
    file->version = saveVersion;

    fwrite_item(file, optionFlags);
    fwrite_item(file, getWindowMode());
    fwrite_item(file, getShowInstructions());
    for (int i = 0; i < numVolumes; i++) {
      VolumeControl v = (VolumeControl)i;
      fwrite_item(file, getMuted(v));
      fwrite_float(file, getVolume(v));
    }

    fwrite_int(file, getFPSCap());
    fwrite_float(file, getBrightness());
    fwrite_float(file, getFOV());

    featureEnabledGlobal.write(file);
    featureWasUsed.write(file);

    int fileFlags = _optfileModSet;

    const char* nextMod = getNextMod();
    if(nextMod == 0) {
      fileFlags &= ~_optfileModSet;
    }

    fwrite_int(file, fileFlags);

    fwrite_string(file, nextMod);
    fwrite_int(file, getFPSMode());
    fwrite_int(file, getGameMode());

    fwrite_float(file, getMeshQuality());

    fwrite_float(file, getAutosaveInterval());

    fwrite_int(file, getMsaaSamples());

    unsigned char* extraData =
      (unsigned char*) calloc(1024, sizeof(unsigned char));
    fwrite_data(file, extraData, 1024);
    free(extraData);

    writeToFile(file, fileHandle);
    fclose(fileHandle);
    freeBuffer(file);
  }
}

