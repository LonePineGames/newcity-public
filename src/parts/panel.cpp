#include "panel.hpp"

Part* panel(vec2 start, vec2 size) {
  return panel(line(vec3(start,0), vec3(size,0)));
}

Part* panel(Line dim) {
  Part* result = part(dim);
  result->flags |= _partIsPanel;
  result->renderMode = RenderPanel;
  return result;
}

