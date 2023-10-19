#include "icons.hpp"

#include "item.hpp"

#include "spdlog/spdlog.h"

#include <string>
#include <unordered_map>
using namespace std;

unordered_map<string, item> iconsMap;
const float spriteSpacing = 1.0 / (spriteSheetSize * spriteResolution);

const vec3 iconsTable[numIcons] = {
  #define ICO(N,V) V,
  #include "iconsEnum.hpp"
  #undef ICO
};

vec3 getIcon(const char* ico) {
  return iconsTable[iconsMap[ico]];
}

void assignIcon(item ico, const char* name) {
  iconsMap[name] = ico;
}

void initIconsMap() {
  iconsMap.clear();
  #define ICO(N,V) assignIcon(Icons::Icon##N, "Icon" #N);
  #include "iconsEnum.hpp"
  #undef ICO
}

Line iconToSpritesheet(vec3 icon, float wind) {
  return line(
    vec3(icon.x/spriteSheetSize + spriteSpacing,
      icon.y/spriteSheetSize + spriteSpacing, wind),
    vec3((icon.x+1)/spriteSheetSize - spriteSpacing,
      (icon.y+1)/spriteSheetSize - spriteSpacing, wind));
}

Line iconToSpritesheet(vec3 icon) {
  return iconToSpritesheet(icon, 0);
}

