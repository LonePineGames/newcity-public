#pragma once

#include "part.hpp"

enum AboutLink {
  LP_WEB = 0,
  LP_REDDIT,
  LP_DISCORD,
  LP_STEAM,
  LP_TWITTER,
  LP_YOUTUBE,
  LP_TWITCH,
  LP_FACEBOOK,
  LP_EMAIL,
  OBJLOAD_LICENSE,
  NUM_LINKS
};

Part* aboutPanel(float aspectRatio);

