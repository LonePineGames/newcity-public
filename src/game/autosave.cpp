
#include "spdlog/spdlog.h"

const float defaultTimeToAutosave = 300.0f;

static float autosaveInterval = defaultTimeToAutosave; // To be loaded from Lua
float timeSinceAutosave = 0;
static atomic<bool> willAutosave(false);

void initAutosave() {
  if (c(CAutosaveDisabled)) {
    setAutosaveInterval(0.f);
  } else {
    setAutosaveInterval(c(CAutosaveInterval));
  }
}

void setAutosaveInterval(float time) {
  if (time < 0) {
    SPDLOG_WARN("Attempted to set autosaveInterval to invalid value {}; "
        "use nonnegative value.", time);
    time = 0;
  }
  timeSinceAutosave = 0;
  autosaveInterval = time;
  SPDLOG_INFO("Autosave interval: {}", autosaveInterval);
}

float getAutosaveInterval() {
  return autosaveInterval;
}

const char* autosaveFilename() {
  return "autosave";
}

void updateAutosave(double duration) {
  if(autosaveInterval == 0) {
    timeSinceAutosave = 0;
    return;
  }

  timeSinceAutosave += std::min(duration, 1.);
  if (timeSinceAutosave > autosaveInterval) {
    timeSinceAutosave -= autosaveInterval;
    if (timeSinceAutosave > autosaveInterval) {
      timeSinceAutosave = autosaveInterval;
    }
    playSound(_soundSaveLoad);
    willAutosave = true;
  }
}

void doAutosave(double duration) {
  if (getGameMode() == ModeDesignOrganizer) return;
  willAutosave = false;
  loaderAutosave();
  //FileBuffer* file = writeDataToFileBuffer(0, false);
  //autosave(file);
}

bool nextFrameWillAutosave() {
  return willAutosave;
}

bool testDoAutosave(double duration) {
  return willAutosave;
}

bool autosave() {
  if (getGameMode() == ModeDesignOrganizer) return false;
  FileBuffer* file = writeDataToFileBuffer(0, false);
  return autosave(file);
}

bool autosave(FileBuffer* file) {
  if (getGameMode() == ModeDesignOrganizer) return false;
  if (!gameLoaded) return false;
  return saveGameInner(file, autosaveFilename());
}

