#ifndef PART_TEXT_BOX_H
#define PART_TEXT_BOX_H

#include "part.hpp"

const int _textBoxEditing = 1 << 0;

struct TextBoxState {
  char** text = 0;
  int flags = 0;
  InputCallback onSubmit;
};

Part* textBox(vec2 loc, vec2 size, char* text);
Part* textBox(vec2 loc, vec2 size, char* text, uint32_t cursorPos);
Part* textBox(vec2 loc, vec2 size, TextBoxState* state);
Part* textBoxLabel(vec2 loc, vec2 size, TextBoxState* state);
Part* textBoxLabel(vec2 loc, vec2 size, TextBoxState* state, vec3 ico,
    const char* altText);

void focusTextBox(TextBoxState* state);

#endif
