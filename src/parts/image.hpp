#pragma once

#include "part.hpp"

void selectFrameInFilename(char* filename, int camTime);
Part* image(vec2 loc, float width, char* filename, float* y);
Part* image(vec2 loc, float width, item texture, float* y);

