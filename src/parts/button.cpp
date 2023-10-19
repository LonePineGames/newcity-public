#include "button.hpp"

#include "../icons.hpp"
#include "../string_proxy.hpp"

#include "icon.hpp"
#include "label.hpp"
#include "panel.hpp"

Part* button(vec2 start, vec2 size, char* text, InputCallback callback) {
  Part* result = part(start);
  result->dim.end = vec3(size, 0);
  result->renderMode = RenderText;
  result->text = text;
  result->onClick = callback;
  result->flags |= _partHover;
  result->padding = 0.1;
  return result;
}

Part* buttonCenter(vec2 start, vec2 size, char* text, InputCallback callback) {
  Part* result = button(start, size, text, callback);
  result->flags |= _partAlignCenter;
  return result;
}

Part* button(vec2 start, vec3 icon, InputCallback callback) {
  return button(start, icon, callback, 0);
}

Part* button(vec2 start, vec3 icon, InputCallback callback, item itemData) {
  return button(start, icon, vec2(1,1), callback, itemData);
}
Part* button(vec2 start, vec3 icon, vec2 size, InputCallback callback,
    item itemData) {
  Part* result = part(start);
  result->dim.end = vec3(size, 0);
  result->renderMode = RenderIcon;
  result->texture = iconToSpritesheet(icon);
  result->onClick = callback;
  result->itemData = itemData;
  result->flags |= _partHover;
  result->padding = 0.1;
  return result;
}

Part* button(vec2 start, vec3 icon, vec2 size, char* text,
  InputCallback callback, item itemData) {

  Part* result = part(start);
  result->dim.end = vec3(size, 0);
  result->renderMode = RenderIconAndText;
  result->texture = iconToSpritesheet(icon);
  result->text = text;
  result->onClick = callback;
  result->itemData = itemData;
  result->flags |= _partHover;
  result->padding = 0.1;
  return result;
}

Part* superButton(vec2 start, vec2 size, vec3 ico, char* text,
  InputCallback callback, item itemData, InputAction action, bool highlight) {

  Part* result = panel(start, size);
  result->onClick = callback;
  result->itemData = itemData;
  result->renderMode = RenderTransparent;
  result->inputAction = action;
  result->flags |= _partHover;
  if (highlight) {
    result->flags |= _partHighlight;
  }
  result->padding = 0.1;

  Part* icoPart = icon(vec2(-0.1,-0.1), vec2(size.y-0.1, size.y-0.1), ico);
  r(result, icoPart);

  KeyBind bind = getKeyBind(action);
  if (bind.key != GLFW_KEY_UNKNOWN) {
    icoPart->text = strdup_s(getKeyStr(bind.key).c_str());
  }

  Part* lbl = label(vec2(size.y-0.2,-0.1), size.y-0.1, text);
  r(result, lbl);

  return result;
}

