#include "icon.hpp"

#include "../icons.hpp"

Part* icon(vec2 start, vec2 size, vec3 icon) {
  Part* result = part(start);
  result->dim.end = vec3(size, 0);
  result->renderMode = RenderIcon;
  result->texture = iconToSpritesheet(icon);
  return result;
}

Part* icon(vec2 start, vec3 ico) {
  return icon(start, vec2(1,1), ico);
}

Part* icon(vec2 start, float ySize, float padding, vec3 icon, char* text) {
  Part* result = part(start);
  result->dim.end = vec3(0, ySize, 0);
  result->padding = padding;
  result->renderMode = RenderIconAndText;
  result->texture = iconToSpritesheet(icon);
  result->text = text;
  return result;
}

