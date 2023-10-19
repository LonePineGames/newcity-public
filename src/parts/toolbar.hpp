#ifndef PART_TOOLBAR_H
#define PART_TOOLBAR_H

#include "part.hpp"

bool isRoadTool();
bool isQueryTool();
bool isZoneTool();
bool isAmenityTool();
bool isBulldozerTool();
bool isBlueprintTool();
bool isUndergroundViewSelected();
bool toggleUndergroundView(Part* part,InputEvent event);
void setUndergroundViewSelected(bool val);
bool getShowInstructions();
void setShowInstructions(bool show);
int getCurrentTool();
void setTool(int tool);
void setBlueprintTool();
Part* toolbar();
void tool_key_callback(InputEvent event);
void tool_mouse_button_callback(InputEvent event);
void tool_mouse_move_callback(InputEvent event);
void resetTools();
void initTools();

#endif
