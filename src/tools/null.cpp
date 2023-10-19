void null_mouse_button_callback(InputEvent event) {}
void null_mouse_move_callback(InputEvent event) {}
void null_select() {}
void null_reset() {}

Part* null_render(Line dim) {
  Part* result = panel(dim);
  result->renderMode = RenderTransparent;
  return result;
}

void nullInstructionPanel(Part* panel) {
  panel->renderMode = RenderTransparent;
}

std::string null_name() {
  return "";
}

bool null_visible() {
  return false;
}

Tool toolNull = {
  null_mouse_button_callback,
  null_mouse_move_callback,
  null_select,
  null_reset,
  null_visible,
  iconNull,
  null_render,
  nullInstructionPanel,
  null_name,
};

