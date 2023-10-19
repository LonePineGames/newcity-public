#pragma once

#include "button.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "part.hpp"
#include "scrollbox.hpp"
#include "textBox.hpp"

#include "../console/conDisplay.hpp"
#include "../console/conInput.hpp"

#include "../draw/camera.hpp"

#include "../icons.hpp"
#include "../string_proxy.hpp"
#include "../util.hpp"

const int CONSOLE_VAL = 7777777;

Part* console();
Part* console(vec2 pos);