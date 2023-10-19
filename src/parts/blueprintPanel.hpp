#pragma once

#include "part.hpp"
#include "textBox.hpp"

#include "../blueprint.hpp"

Part* blueprintPanel(vec2 start, vec2 size, Blueprint* bp,
    TextBoxState* tb, item ndx);

