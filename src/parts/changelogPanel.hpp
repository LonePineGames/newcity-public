#pragma once

// For reading changelog info for now, will read in from file later
#include "changelogData.hpp"

#include "part.hpp"

float getYValFromText(float lineSize, float panelWidth, float charSize, char* txt);
bool toggleMiniChangelog(Part* part, InputEvent event);
Part* changelogPanel();

