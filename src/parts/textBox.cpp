#include "textBox.hpp"

#include "block.hpp"
#include "button.hpp"
#include "label.hpp"
#include "panel.hpp"

#include "../icons.hpp"
#include "../string.hpp"
#include "../string_proxy.hpp"

#include "spdlog/spdlog.h"

#include <algorithm>

static const float cursorWidth = 0.1;
static const float cursorPadding = 0.1;
static const float textBoxPadding = 0.1;
TextBoxState* focusedTB = 0;

void focusTextBox(TextBoxState* state) {
  if (focusedTB != 0) {
    focusedTB->flags &= ~_textBoxEditing;
  }

  focusedTB = state;
  if (focusedTB != 0) {
    focusedTB->flags |= _textBoxEditing;
  }
}

void focusTB(Part* part) {
  focus(part);
  if (part != 0) {
    TextBoxState* state = (TextBoxState*) part->ptrData;
    focusedTB = state;
  }
}

bool focusTB(Part* part, InputEvent event) {
  focusTB(part);
  return true;
}

bool editTB(Part* part, InputEvent event) {
  TextBoxState* state = (TextBoxState*) part->ptrData;
  if (state != focusedTB) return false;
  if (event.unicode > 0) {
    char* textSwap = *(state->text);
    *(state->text) = sprintf_o("%s%c", textSwap, (char)event.unicode);
    free(textSwap);
  }
  return true;
}

bool editTBKey(Part* part, InputEvent event) {
  TextBoxState* state = (TextBoxState*) part->ptrData;
  if (state != focusedTB) return false;
  if (event.key == GLFW_KEY_BACKSPACE) {
    int length = strlength(*(state->text));
    if (length == 0) {
      return true;
    }
    (*(state->text))[length-1] = '\0';
  } else if (event.key == GLFW_KEY_ENTER || event.key == GLFW_KEY_ESCAPE) {
    if (part->onCustom != 0) {
      part->onCustom(part, event);
    }
  }
  return true;
}

bool setTBEditing(Part* part, InputEvent event) {
  TextBoxState* state = (TextBoxState*) part->ptrData;
  if (part->itemData) {
    state->flags |= _textBoxEditing;
    focusTB(part);
  } else {
    state->flags &= ~_textBoxEditing;
    if (state == focusedTB) {
      focusTB(0);
    }
  }
  return true;
}

Part* textBox(vec2 start, vec2 size, char* text) {
  Part* result = panel(start, size);
  result->flags |= _partHighlight | _partClip;
  result->padding = textBoxPadding;

  float innerY = size.y - textBoxPadding*2;
  float width = stringWidth(text) * innerY;
  float xoff = std::min(size.x-width - cursorWidth -
      cursorPadding*2 - textBoxPadding*2, 0.f);

  Part* textBox = part(vec2(xoff, 0));
  textBox->dim.end = vec3(0, innerY, 0);
  textBox->renderMode = RenderText;
  textBox->text = text;
  r(result, textBox);

  //Cursor
  if (int(getCameraTime()*2) % 2 == 0) {
    r(result, block(vec2(width+xoff+cursorPadding, 0),
          vec2(cursorWidth, innerY)));
  }

  return result;
}

Part* textBox(vec2 start, vec2 size, char* text, uint32_t cursorPos) {
  Part* result = panel(start,size);
  result->flags |= _partHighlight | _partClip;
  result->padding = textBoxPadding;

  // Find total length of string
  // Then find substring based on cursorPos
  // Offset cursorPos by the width of the characters in the substring
  float strWidth = stringWidth(text);
  float cursorPosOffset = 0.0f;
  std::string temp = std::string(text).substr(0,cursorPos);
  for(int c = 0; c < temp.length(); c++) {
    cursorPosOffset += characterWidth(temp[c]);
  }

  float innerY = size.y - textBoxPadding*2;

  Part* textBox = part(vec2(0,0));
  textBox->dim.end = vec3(strWidth,innerY,0);
  textBox->renderMode = RenderText;
  textBox->text = text;
  r(result,textBox);

  // Render blinking cursor
  if(int(getCameraTime()*2) % 2 == 0) {
    r(result,block(vec2((cursorPosOffset)*innerY,0),
      vec2(cursorWidth*0.5f,innerY)));
  }

  return result;
}

Part* textBox(vec2 start, vec2 size, TextBoxState* state) {
  if (*(state->text) == 0) *(state->text) = strdup_s("");
  Part* result = panel(start, size);
  result->flags |= _partHighlight | _partClip;
  result->padding = textBoxPadding;
  result->onClick = focusTB;
  if (state == focusedTB) {
    focus(result);
    result->onText = editTB;
    result->onKeyDown = editTBKey;
    result->ptrData = state;
    result->flags &= ~_partFreePtr;
  }

  float innerY = size.y - textBoxPadding*2;
  float width = stringWidth(*(state->text)) * innerY;
  float xoff = std::min(size.x-width - cursorWidth -
      cursorPadding*2 - textBoxPadding*2, 0.f);

  Part* textBox = part(vec2(xoff, 0));
  textBox->dim.end = vec3(0, innerY, 0);
  textBox->renderMode = RenderText;
  textBox->text = strdup_s(*(state->text));
  r(result, textBox);

  //Cursor
  if (state == focusedTB && int(getCameraTime()*2) % 2 == 0) {
    r(result, block(vec2(width+xoff+cursorPadding, 0),
          vec2(cursorWidth, innerY)));
  }

  return result;
}

Part* textBoxLabel(vec2 start, vec2 size, TextBoxState* state, vec3 ico,
    const char* altText) {
  if (*(state->text) == 0) *(state->text) = strdup_s("");
  float xs = size.x;
  float ys = size.y;
  Part* result = panel(start, size);
  result->renderMode = RenderTransparent;
  bool editing = state->flags & _textBoxEditing;

  if (editing) {
    Part* tb = textBox(vec2(0,0), vec2(xs-ys,ys), state);
    tb->onCustom = setTBEditing;
    tb->itemData = 0;
    r(result, tb);
  } else if (*(state->text) != 0 && strlength(*(state->text)) > 0) {
    r(result, label(vec2(0,0), ys, strdup_s(*(state->text))));
  } else if (altText != 0) {
    r(result, label(vec2(0,0), ys, strdup_s(altText)));
  }

  Part* butt = button(vec2(xs-ys,0),
      editing ? iconCheck : ico,
      vec2(ys, ys), setTBEditing, !editing);
  butt->ptrData = state;
  butt->flags &= ~_partFreePtr;
  r(result, butt);

  return result;
}

Part* textBoxLabel(vec2 start, vec2 size, TextBoxState* state) {
  return textBoxLabel(start, size, state, iconPencil, 0);
}

