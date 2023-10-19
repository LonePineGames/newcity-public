//-----------------------------------------------------------------------------
// steamwrapper - Wrapper and API for Steamworks functionality,
// including initialization, cleanup, and management
//-----------------------------------------------------------------------------

#pragma once

#include <array>
#include <stdint.h>

#define uint64 uint64_steam
#define int64 int64_steam
#include "steam_api.h"
#undef uint64
#undef int64

#include "spdlog/spdlog.h"
#include "../string_proxy.hpp"

const std::string STEAM_ERR = "STEAM ERROR";
const AppId_t NewCityAppID = 1067860;
const double steam_tickrate = 1.0 / 10.0;
const int steam_timeout = 100; // Timeout time measured in ticks

// Enum for valid Steam overlay pchDialog values
enum STEAM_OVERLAY_ENUM {
  FRIENDS = 0,
  COMMUNITY,
  PLAYERS,
  SETTINGS,
  OFFICIALGAMEGROUP,
  STATS,
  ACHIEVEMENTS,
  NUM_STRINGS
};


//-----------------------------------------------------------------------------
// Steam Wrapper function headers
//-----------------------------------------------------------------------------
// Steamworks Initialization and Maintanence
bool steam_init();
bool steam_isActive();
void steam_shutdown();
void steam_tick();

// Basic Steam functionality
std::string steam_getSteamInfo();
std::string steam_getSteamName();
bool steam_isValidOverlayPchDialog(int val);
bool steam_openOverlay(int val);
bool steam_openOverlayWebPage(std::string url);
void steam_spdlogInfo();

