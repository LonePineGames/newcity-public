//-----------------------------------------------------------------------------
// coninput.hpp - Contains the functions for parsing user input
// and potential statuses which may result
//-----------------------------------------------------------------------------

#pragma once

#include <ctype.h>

#include "../platform/lua.hpp"

#include "../string.hpp"
#include "../string_proxy.hpp"

#include "conCmd.hpp"
#include "conDisplay.hpp"
#include "conMod.hpp"

enum ConStatus {
  SUCCESS = 0,
  INVALID_CMD = 1,
  INVALID_ARG = 2,
  NO_CALLBACK = 3,
  NULL_PTR = 4,
  UNKNOWN_MOD = 5,
  INVALID_MOD = 6,
  DIR_READ_ERROR = 7,
  FILE_READ_ERROR = 8,
  OBJ_LOAD_ERROR = 9,
  LUA_VAR_ERROR = 10,
  LUA_VAL_ERROR = 11,
  LUA_ALL_ERROR = 12,
  NUM_ERRORS
};

uint32_t consoleGetInputMemIndex();
bool consoleTrySetInputMemIndex(uint32_t i);
std::string* consoleGetInputMemArr();
std::string* consoleGetInputMemArr(int& size);
std::string consoleGetInputMemAtIndex();
const char* consoleGetError(ConStatus e);
int consoleHandleLua(std::string input);
int consoleParseInput(std::string input);
