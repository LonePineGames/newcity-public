#include "hr.hpp"

static const float ruleWidth = 0.1;

Part* hr(vec2 loc, float length) {
  Part* result = part(loc);
  result->dim.end = vec3(length, ruleWidth, 0);
  result->renderMode = RenderWhiteBox;
  return result;
}

