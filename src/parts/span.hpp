#pragma once

#include "part.hpp"

Part* span(vec2 start, float indent, vec2 size, char* text, vec2* end);
Part* span(vec2 start, float indent, vec2 size, char* text, float* y);
Part* spanCenter(vec2 start, float indent, vec2 size, char* text, vec2* end);
Part* spanCenter(vec2 start, float indent, vec2 size, char* text, float* y);

