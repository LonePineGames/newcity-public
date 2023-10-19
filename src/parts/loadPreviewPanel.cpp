#include "loadPreviewPanel.hpp"

#include "blank.hpp"
#include "icon.hpp"
#include "image.hpp"
#include "label.hpp"
#include "panel.hpp"

#include "../building/design.hpp"
#include "../platform/file.hpp"
#include "../icons.hpp"
#include "../string_proxy.hpp"
#include "../zone.hpp"

#include "spdlog/spdlog.h"
#include <cstring>

const float prevPad = 0.5;

Design designPreview;
const char* lastFilename = 0;

Part* loadPreviewPanel(vec2 loc, vec2 size, const char* filename) {
  if (filename == 0) return blank();

  Part* result = panel(loc, size);
  result->flags |= _partTextShadow;
  result->padding = prevPad;
  float y = 0;

  string imageFile = filename;
  if (!endsWith(filename, ".png")) imageFile += ".png";

  // Only add image part if file exists
  if (fileExists(imageFile)) {
    Part* img = r(result, image(vec2(-prevPad, -prevPad), size.x,
      strdup_s(imageFile.c_str()), &y));
    img->dim.start.z -= 0.5f;
    if (y > 0) result->dim.end.y = y;
    y = 0;
  }

  bool design = endsWith(filename, ".design");

  // Commenting this out because streql throws errors with uninitialized lastFilename? 
  /*
  if (lastFilename == 0 || !streql(filename, lastFilename)) {
    lastFilename = filename;
    if (design) {
      readDesign(filename, &designPreview);
    }
  }
  */

  lastFilename = filename;

  if (design) {
    readDesign(filename, &designPreview);

    r(result, label(vec2(0, y), 1, strdup_s(
      designPreview.displayName == 0 || strlen(designPreview.displayName) == 0 ?
        designPreview.name : designPreview.displayName)));
    y ++;

    r(result, label(vec2(0, y), 1, strdup_s(zoneName[designPreview.zone])));
    y ++;

    bool isHotel = designPreview.flags & _designIsHotel;
    float x = 0;
    if (designPreview.numFamilies > 0) {
      r(result, labelRight(vec2(x, y), vec2(2,1),
            sprintf_o("%d", designPreview.numFamilies)));
      r(result, icon(vec2(x+2, y), vec2(1,1),
            isHotel ? iconHotelRoom : iconFamily));
      x+=3;
    }
    for (int i = 0; i < 5; i++) {
      if (designPreview.numBusinesses[i] == 0) continue;
      r(result, labelRight(vec2(x, y), vec2(2,1),
            sprintf_o("%d", designPreview.numBusinesses[i])));
      r(result, icon(vec2(x+2, y), vec2(1,1), iconBusinessType[i]));
      x+=3;
    }
    if (x > 0) y ++;
    x = 0;

    r(result, labelRight(vec2(0, y), vec2(1,1),
          sprintf_o("%d", int(designPreview.minDensity*10))));
    r(result, icon(vec2(1, y), vec2(1,1), iconDensity));
    r(result, labelRight(vec2(2, y), vec2(1,1),
          sprintf_o("%d", int(designPreview.minLandValue*10))));
    r(result, icon(vec2(3, y), vec2(1,1), iconLandValue));
    y ++;

    r(result, label(vec2(0,y), 1, sprintf_o("%d-%d", designPreview.minYear,
            designPreview.maxYear)));
    y ++;

    for (int i = 0; i < numEffects; i++) {
      if (designPreview.effect[i] == 0) continue;

      item val = designPreview.effect[i];
      bool neg = val < 0;
      if (neg) val = -val;
      r(result, labelRight(vec2(x, y), vec2(2,1),
            sprintf_o("%s%d", neg?"-":"+", val)));
      r(result, icon(vec2(x+2, y), vec2(1,1), getEffectIcon(i)));
      x+=3;
      if (x > 11) {
        y ++;
        x = 0;
      }
    }

    if (x != 0) y ++;

    if (designPreview.zone == GovernmentZone) {
      r(result, label(vec2(0,y), 1, printMoneyString(designPreview.cost)));
      char* perYear = printMoneyString(designPreview.maintenance);
      r(result, label(vec2(3,y), 1, sprintf_o("%s/yr", perYear)));
      free(perYear);
      y++;
    }
  }

  return result;
}

