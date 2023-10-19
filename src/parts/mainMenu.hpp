#pragma once

#include "part.hpp"

const char* getSaveFilename();
uint32_t getLoadMenuLookupFlags();
bool openSteamWorkshop(Part* part, InputEvent event);
void setSaveFilename(char* name);
Part* mainMenu(float aspectRatio);
void setDoSaveConfirm(bool val);
void clearSaveConfirm();
void setSaveLoadModeTab(item mode);
bool saveConfirm(Part* part, InputEvent event);

