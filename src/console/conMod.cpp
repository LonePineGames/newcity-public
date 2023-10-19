#include "conMod.hpp"

// Is the specified character a recognized modifier?
bool consoleIsModifier(char c) {
  if(c == '\0') {
    return false;
  }
  for(int i = 0; i < conMods.size(); i++) {
    if(c == conMods[i].value) {
      return true;
    }
  }
  return false;
}

// Same as basic isMod
// Passes the mod's index out via the reference if found and returns success 
bool consoleIsModifier(char c, int& index) {
  if(c == '\0') {
    return false;
  }
  for(int i = 0; i < conMods.size(); i++) {
    if(c == conMods[i].value) {
      index = i;
      return true;
    }
  }
  return false;
}

// Attempts to get a modifier that matches the character
// Passes the mod out via the reference if found and returns success
bool consoleGetModifier(char c, ConMod& mod) {
  int index = -1;
  if(consoleIsModifier(c,index)) {
    mod.value = conMods[index].value;
    mod.type = conMods[index].type;
    return true;
  }
  return false;
}

// Attempts to get a modifier that matches the character
ConMod consoleGetModifier(char c) {
  int index = -1;
  ConMod mod;
  if(consoleIsModifier(c,index)) {
    mod.value = conMods[index].value;
    mod.type = conMods[index].type;
  }
  return mod;
}