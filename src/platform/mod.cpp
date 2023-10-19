#include "mod.hpp"

#include "file.hpp"

#include "../option.hpp"
#include "../string_proxy.hpp"

char* currentMod = 0;
char* nextMod = 0;
char* currentModDirectory = 0;

bool isModsEnabled() {
  return currentMod != 0;
}

const char* modDirectory() {
  return currentModDirectory;
}

const char* modDirectoryNonNull() {
  if (currentMod == 0) {
    return "modpacks/yours/";
  } else {
    return currentModDirectory;
  }
}

void selectMod(const char* mod) {
  if (nextMod != 0) free(nextMod);

  if (mod == 0) {
    nextMod = 0;
  } else {
    nextMod = strdup_s(mod);
  }

  writeOptions();
}

void setModNext() {
  setMod(nextMod);
}

void setMod(const char* mod) {
  if (currentMod != 0) {
    free(currentMod);
    free(currentModDirectory);
  }

  if (mod == 0 || strlength(mod) == 0) {
    currentMod = 0;
    nextMod = 0;
    currentModDirectory = strdup_s("");

  } else {
    currentMod = strdup_s(mod);
    nextMod = strdup_s(currentMod);
    currentModDirectory = sprintf_o("modpacks/%s/", currentMod);
  }
}

const char* getMod() {
  return currentMod;
}

const char* getNextMod() {
  return nextMod;
}

/*
vector<char*> listMods() {
  return readDirectory("modpacks", "");
}
*/

