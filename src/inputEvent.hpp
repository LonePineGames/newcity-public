#pragma once

#include "line.hpp"
#include "item.hpp"

const int UNICODE_INPUT = -1000;
const int MOUSE_MOVED_INPUT = -1001;
const int SCROLL = -1002;

enum MouseButton { LMB, MMB, RMB };

const int mouseButtonToGLFW[3] = {
  GLFW_MOUSE_BUTTON_LEFT,
  GLFW_MOUSE_BUTTON_MIDDLE,
  GLFW_MOUSE_BUTTON_RIGHT
};

struct InputEvent {
  int button;
  int key;
  int scancode;
  int action;
  int mods;
  int unicode;
  vec2 scroll;
  vec2 mouseLoc;
  vec2 cameraSpaceMouseLoc;
  Line mouseLine;
  bool doubleClick;
  bool isMouse;
  bool isButtonDown[3];
  bool isKeyDown[GLFW_KEY_LAST+1];
};

