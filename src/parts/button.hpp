#ifndef PART_BUTTON_H
#define PART_BUTTON_H

#include "part.hpp"

Part* button(vec2 start, vec2 size, char* text, InputCallback callback);
Part* buttonCenter(vec2 start, vec2 size, char* text, InputCallback callback);
Part* button(vec2 start, vec3 icon, InputCallback callback);
Part* button(vec2 start, vec3 icon, InputCallback callback, item itemData);
Part* button(vec2 start, vec3 icon, vec2 size, InputCallback callback,
    item itemData);
Part* button(vec2 start, vec3 icon, vec2 size, char* text,
  InputCallback callback, item itemData);
Part* superButton(vec2 start, vec2 size, vec3 ico, char* text,
  InputCallback callback, item itemData, InputAction action, bool highlight);

#endif
