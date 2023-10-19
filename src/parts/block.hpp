#ifndef PART_BLOCK_H
#define PART_BLOCK_H

#include "part.hpp"

Part* block(vec2 loc, vec2 size);
Part* sunkenBlock(vec2 loc, vec2 size);
Part* darkBlock(vec2 loc, vec2 size);
Part* blackBlock(vec2 loc, vec2 size);
Part* brightBox(vec2 loc, vec2 size);
Part* redBlock(vec2 loc, vec2 size);
Part* greenBlock(vec2 loc, vec2 size);
Part* gradientBlock(vec2 loc, vec2 size, vec3 grads, vec3 grade);
Part* gradientBlock(vec2 loc, vec2 size, Line grad);
Part* colorBlock(vec2 loc, vec2 size, item color);

#endif
