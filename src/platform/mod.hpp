#pragma once

#include "../serialize.hpp"

const char* modDirectory();
const char* modDirectoryNonNull();
const char* getMod();
const char* getNextMod();
void setModNext();
void setMod(const char* mod);
void selectMod(const char* mod);
bool isModsEnabled();

