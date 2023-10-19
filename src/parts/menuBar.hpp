#ifndef PART_MENU_BAR_H
#define PART_MENU_BAR_H

#include "part.hpp"

enum FPSMode {
  ShowNoFPS, ShowFPS, ShowMS, ShowTime12, ShowTime24,
  numFPSModes
};

int getFPSMode();
void setFPSMode(int mode);
bool toggleUI(Part* part, InputEvent event);
Part* menuBar(float uiX, float uiY, bool showUI);

#endif
