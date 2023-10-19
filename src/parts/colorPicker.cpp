#include "colorPicker.hpp"

#include "../color.hpp"

#include "panel.hpp"
#include "block.hpp"

Part* colorPicker(vec2 loc, float width, item current, InputCallback pick,
    float extraData) {
  float boxOuter = width*4/numColors;
  float boxInner = boxOuter*2/3;
  float boxPad = .5f*(boxOuter - boxInner);
  float height = boxOuter*4;
  Part* result = panel(loc, vec2(width, height));
  result->renderMode = RenderTransparent;

  for (int i = 0; i < numColors; i++) {
    float lx = boxOuter*(i/4);
    float ly = boxOuter*(i%4);

    vec3 color = getColorInPalette(i);
    Part* colorButt = r(result, panel(vec2(lx,ly), vec2(boxOuter, boxOuter)));
    colorButt->renderMode = RenderTransparent;
    r(colorButt, gradientBlock(vec2(boxPad, boxPad),
          vec2(boxInner,boxInner), color, color));
    colorButt->onClick = pick;
    colorButt->itemData = i;
    colorButt->vecData.x = extraData;
    if (i == current) {
      colorButt->flags |= _partHighlight;
    }
  }

  return result;
}

