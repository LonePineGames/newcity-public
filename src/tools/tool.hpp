#pragma once

#include "../inputEvent.hpp"
#include "../parts/part.hpp"

#include <string>

const int _toolForceInstructions = 1 << 0;

struct Tool {
  void (*mouse_button_callback) (InputEvent event);
  void (*mouse_move_callback) (InputEvent event);
  void (*select) ();
  void (*reset) ();
  bool (*isVisible) ();
  vec3 icon;
  Part* (*render) (Line dim);
  void (*instructionPanel) (Part* panel);
  std::string (*name)();
  int flags;
};

