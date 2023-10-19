//-----------------------------------------------------------------------------
// tutorialPanel - The left-panel popup for the tutorial
// (c) 2021 Lone Pine Games, LLC.
//-----------------------------------------------------------------------------

#pragma once

#include "button.hpp"
#include "hr.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "part.hpp"
#include "../game/game.hpp"
#include "../tutorial.hpp"
#include "../string_proxy.hpp"

bool isTutorialPanelOpen();
void setTutorialPanelOpen(bool open);
Part* tutorialPanel();

