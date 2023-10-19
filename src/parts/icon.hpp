#ifndef PART_ICON_H
#define PART_ICON_H

#include "part.hpp"

Part* icon(vec2 start, vec3 icon);
Part* icon(vec2 start, vec2 size, vec3 icon);
Part* icon(vec2 start, float ySize, float padding, vec3 icon, char* text);

#endif
