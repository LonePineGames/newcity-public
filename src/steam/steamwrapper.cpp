//-----------------------------------------------------------------------------
// steamwrapper - Wrapper and API for Steamworks functionality,
// including initialization, cleanup, and management
//-----------------------------------------------------------------------------

#include "steamwrapper.hpp"


//-----------------------------------------------------------------------------
// Steam State struct - Contains all pertinent Steam client data
//-----------------------------------------------------------------------------
struct steam_state_struct {
private:
  bool initialized;
  CSteamID steamId;
  std::string steamName;
public:
  void setInitialized(bool state) {
    initialized = state;
  }

  bool isInitialized() {
    return initialized;
  }

  // Returns true on success
  bool setSteamId(CSteamID id) {
    if(!id.IsValid()) {
      SPDLOG_ERROR("Attempted to set SteamId to invalid value");
      return false;
    }

    steamId = id;
    return true;
  }

  CSteamID getSteamId() {
    return steamId;
  }

  uint64_steam getSteamIdLong() {
    return steamId.ConvertToUint64();
  }

  // Returns success
  bool setSteamName(std::string name) {
    if(name.length() == 0) {
      SPDLOG_ERROR("Attempted to set SteamName to null value");
      return false;
    }

    steamName = name;
    return true;
  }

  std::string getSteamName() {
    return steamName;
  }
};

// Static global variables
static steam_state_struct steam_state;

//-----------------------------------------------------------------------------
// Steamworks Initialization and Maintanence
//-----------------------------------------------------------------------------
bool steam_init() {
  steam_state.setInitialized(SteamAPI_Init());

  if(steam_state.isInitialized()) {
    steam_state.setSteamId(SteamUser()->GetSteamID());
    steam_state.setSteamName(SteamFriends()->GetPersonaName());
  }

  return steam_state.isInitialized();
}

bool steam_isActive() {
  return steam_state.isInitialized();
}

void steam_shutdown() {
  steam_state.setInitialized(false);
  SteamAPI_Shutdown();
}

void steam_tick() {
  SteamAPI_RunCallbacks();
}

//-----------------------------------------------------------------------------
// Basic Steam functionality
//-----------------------------------------------------------------------------

// Returns a string with all available Steam info
std::string steam_getSteamInfo() {
  if(!steam_state.isInitialized()) {
    return "Steam is not initialized";
  }

  return std::string(sprintf_o("Steam info: Status(%d), SteamId(%d), DisplayName(%s)",
    (int)steam_state.isInitialized(), steam_state.getSteamIdLong(), steam_getSteamName().c_str()));
}

// Returns a string with just the current Steam persona name
std::string steam_getSteamName() {
  if(!steam_state.isInitialized()) {
    return STEAM_ERR;
  }

  return steam_state.getSteamName();
}

// Fetches a Steam overlay pchDialog string based on an enum val
const char* steam_getOverlayPchDialog(int val) {
  switch(val) {
    case STEAM_OVERLAY_ENUM::FRIENDS:
      return "friends";
    case STEAM_OVERLAY_ENUM::COMMUNITY:
      return "community";
    case STEAM_OVERLAY_ENUM::PLAYERS:
      return "players";
    case STEAM_OVERLAY_ENUM::SETTINGS:
      return "settings";
    case STEAM_OVERLAY_ENUM::OFFICIALGAMEGROUP:
      return "officialgamegroup";
    case STEAM_OVERLAY_ENUM::STATS:
      return "stats";
    case STEAM_OVERLAY_ENUM::ACHIEVEMENTS:
      return "achievements";
    default:
      return 0;
  }
}

// Checks if the Steam overlay enum val passed matches a valid pchDialog string
bool steam_isValidOverlayPchDialog(int val) {
  return steam_getOverlayPchDialog(val) != 0;
}

// Returns success - NOTE: pchDialog specifying overlay may not refer to a valid overlay
// TODO: Sanitize pchDialog
bool steam_openOverlay(int val) {
  if(!steam_state.isInitialized()) return false;

  const char* pch = steam_getOverlayPchDialog(val);

  if(pch == 0) {
    SPDLOG_ERROR("Attempted to open Steam Overlay with invalid pchDialog value");
    return false;
  }

  SteamFriends()->ActivateGameOverlay(pch);
  return true;
}

// Returns success
bool steam_openOverlayWebPage(std::string url)
{
  if(url.length() == 0) {
    return false;
  }

  SteamFriends()->ActivateGameOverlayToWebPage(url.c_str());
  return true;
}

// For debugging only
void steam_spdlogInfo() {
  SPDLOG_INFO("Steam: Status({}), SteamId({}), DisplayName({})",
    steam_state.isInitialized(), steam_state.getSteamIdLong(), steam_getSteamName());
}
