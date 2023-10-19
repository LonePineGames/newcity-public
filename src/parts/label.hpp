#ifndef PART_LABEL_H
#define PART_LABEL_H

#include "part.hpp"

Part* label(vec2 loc, float size, char* text);
Part* label(vec2 loc, vec2 size, char* text);
Part* label(vec2 start, float size, char* text, bool red);
Part* labelRed(vec2 loc, float size, char* text);
Part* labelCenter(vec2 start, vec2 size, char* text);
Part* labelRight(vec2 start, vec2 size, char* text);
Part* multiline(vec2 start, vec2 size, char* text, float* ySize);

#endif
