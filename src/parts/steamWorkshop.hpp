//-----------------------------------------------------------------------------
// steamWorkshop - Defines and renders the UI element for the Steam Workshop
//-----------------------------------------------------------------------------

#pragma once

#include <string>

enum WSTab {
  Browse,
  Upload,
  NumTabs,

  Home,
  Search,
  Subscribed,
};

std::string workshopGetStringForTab(WSTab tab);
Part* steamWorkshop(float aspectRatio);
bool openDesignInWorkshop(Part* part, InputEvent event);

