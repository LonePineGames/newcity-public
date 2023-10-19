#include "version.hpp"

#include "../string_proxy.hpp"

char* versionString_ = 0;
char* versionStringUnderscored_ = 0;

const char* versionString() {
  if (versionString_ == 0) {
    versionString_ = sprintf_o("Version %d.%d.%d.%d%s ALPHA for %s",
        majorVersion, minorVersion, saveVersion, patchVersion,
        patchLetter, platform);
  }
  return versionString_;
}

const char* versionStringUnderscored() {
  if (versionStringUnderscored_ == 0) {
    versionStringUnderscored_ = sprintf_o("v%d.%d.%d.%d%s_%s",
        majorVersion, minorVersion, saveVersion, patchVersion,
        patchLetter, platform);
  }
  return versionStringUnderscored_;
}

int combinedVersionNumber() {
  return saveVersion*100 + patchVersion;
}

