#include "block.hpp"

#include "../color.hpp"

Part* block(vec2 loc, vec2 size) {
  Part* result = part(loc);
  result->dim.end = vec3(size, 0);
  result->renderMode = RenderWhiteBox;
  return result;
}

Part* darkBlock(vec2 loc, vec2 size) {
  Part* result = part(loc);
  result->dim.end = vec3(size, 0);
  result->renderMode = RenderDarkBox;
  return result;
}

Part* blackBlock(vec2 loc, vec2 size) {
  Part* result = part(loc);
  result->dim.end = vec3(size, 0);
  result->renderMode = RenderGradient;
  result->texture = line(colorTransparentBlack, colorTransparentBlack);
  return result;
}

Part* brightBox(vec2 loc, vec2 size) {
  Part* result = part(loc);
  result->dim.end = vec3(size, 0);
  result->renderMode = RenderGradient;
  result->texture = line(colorRaisedGrad0, colorRaisedGrad1);
  return result;
}

Part* redBlock(vec2 loc, vec2 size) {
  Part* result = part(loc);
  result->dim.end = vec3(size, 0);
  result->renderMode = RenderGradient;
  result->texture = line(colorRedGrad1, colorRedGrad0);
  return result;
}

Part* greenBlock(vec2 loc, vec2 size) {
  Part* result = part(loc);
  result->dim.end = vec3(size, 0);
  result->renderMode = RenderGradient;
  result->texture = line(colorGreenGrad1, colorGreenGrad0);
  return result;
}

Part* gradientBlock(vec2 loc, vec2 size, Line grad) {
  Part* result = part(loc);
  result->dim.end = vec3(size, 0);
  result->renderMode = RenderGradient;
  result->texture = grad;
  return result;
}

Part* gradientBlock(vec2 loc, vec2 size, vec3 grads, vec3 grade) {
  Part* result = part(loc);
  result->dim.end = vec3(size, 0);
  result->renderMode = RenderGradient;
  result->texture = line(grads, grade);
  return result;
}

Part* colorBlock(vec2 loc, vec2 size, item colorNdx) {
  Part* result = part(loc);
  result->dim.end = vec3(size, 0);
  result->renderMode = RenderGradient;
  vec3 color = getColorInPalette(colorNdx);
  result->texture = line(color, color);
  return result;
}

Part* sunkenBlock(vec2 loc, vec2 size) {
  Part* result = part(loc);
  result->dim.end = vec3(size, 0);
  result->renderMode = RenderGradient;
  result->texture = line(colorLoweredGrad1, colorLoweredGrad0);
  return result;
}

