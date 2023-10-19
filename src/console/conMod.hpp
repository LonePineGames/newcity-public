//-----------------------------------------------------------------------------
// conmod.hpp - Contains modifiers for use with commands/vars 
// in the console.
//-----------------------------------------------------------------------------

#pragma once

#include <vector>
#include <string>

enum ConModType
{
  INVALID = -1,
  LUA = 0,
  HELP = 1,
  NUM_TYPES
};

struct ConMod
{
  char value;
  ConModType type;
  std::string help;

  ConMod(char c, ConModType t, std::string h) {
    value = c;
    type = t;
    help = h;
  }

  ConMod() {
    value = 0;
    type = ConModType::INVALID;
    help = "";
  }
};

const std::vector<ConMod> conMods
{
  // Lua value
  ConMod('C',
  ConModType::LUA, 
  "Prefix for Lua variables"),
  // Print help for variable or command
  ConMod('?',
  ConModType::HELP, 
  "Shortcut for displaying a command's help text"),
};

bool consoleIsModifier(char c);
bool consoleIsModifier(char c,int& index);
bool consoleGetModifier(char c, ConMod& modPtr);
ConMod consoleGetModifier(char c);
