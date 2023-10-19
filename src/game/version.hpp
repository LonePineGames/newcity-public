#pragma once

const int majorVersion = 0;
const int minorVersion = 0;
#include "version_generated.hpp"
//const int saveVersion = 57;
//const int patchVersion = 5;
//const char* const patchLetter = "";

const char* versionString();
const char* versionStringUnderscored();

#ifdef _WIN32
  const char* const platform = "Windows";
#else
  const char* const platform = "Linux";
#endif

int combinedVersionNumber();

