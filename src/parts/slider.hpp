#ifndef PART_SLIDER_H
#define PART_SLIDER_H

#include "part.hpp"

Part* slider(vec2 start, vec2 size, float value, InputCallback callback);
Part* slider(vec2 start, vec2 size, float value, InputCallback callback,
    Line tex);
Part* vslider(vec2 start, vec2 size, float value, InputCallback callback);

#endif
