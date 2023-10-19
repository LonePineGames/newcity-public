#include "textureSelect.hpp"

#include "../building/building.hpp"
#include "../building/buildingTexture.hpp"
#include "../building/design.hpp"
#include "../building/designPackage.hpp"
#include "../building/renderBuilding.hpp"
#include "../draw/texture.hpp"
#include "../icons.hpp"
#include "../string.hpp"
#include "../string_proxy.hpp"

#include "button.hpp"
#include "designConfigPanel.hpp"
#include "hr.hpp"
#include "icon.hpp"
#include "image.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "scrollbox.hpp"
#include "textBox.hpp"

#include "spdlog/spdlog.h"

static ScrollState tspScroll;
static TextBoxState tspSearchTB;
bool isTSPSearching = false;
bool isTSPIllum = false;
const float tspPadding = 0.25;
char* tspSearchText = 0;
item hoverNdx = 0;

static bool focusSearchBox(Part* part, InputEvent event) {
  focusTextBox(&tspSearchTB);
  return true;
}

static bool unfocusSearchBox(Part* part, InputEvent event) {
  focusTextBox(0);
  return true;
}

static bool selectTexture(Part* part, InputEvent event) {
  focusTextBox(0);

  Building* b = getBuilding(getSelectedDesignNdx());
  addTextureToPackage(b->design, part->itemData, isTSPIllum);
  b->color = numMatchingTexturesForBuilding(getSelectedDesignNdx())-1;
  paintBuilding(getSelectedDesignNdx());
  closeTextureSelect(part, event);

  return true;
}

static bool showHoverText(Part* part, InputEvent event) {
  hoverNdx = part->itemData;
  return true;
}

Part* textureSelectPanel(vec2 loc, vec2 size, item state) {
  bool illumination = state == TSSIllumination;
  isTSPIllum = illumination;
  Part* result = panel(loc, size);
  result->padding = tspPadding;
  r(result, label(vec2(0,0), 1, strdup_s(!illumination ? "Select Texture" : "Select Illumination")));
  size -= vec2(tspPadding*2, tspPadding*2);
  r(result, button(vec2(size.x-1,0), iconX, vec2(1, 1), closeTextureSelect, 0));
  //r(result, hr(vec2(0,1), size.x));

  tspSearchTB.text = &tspSearchText;
  float searchBoxX = 8.5f;
  float scale = 1.f;
  r(result, icon(vec2(searchBoxX-scale, 0),
        vec2(scale, scale), iconQuery));
  Part* tb = r(result, textBox(vec2(searchBoxX,0),
        vec2(size.x-searchBoxX-1-tspPadding, scale), &tspSearchTB));
  tb->onClick = focusSearchBox;
  tb->onCustom = unfocusSearchBox;
  bool search = tspSearchText != 0 && strlength(tspSearchText) > 0;
  if (!search) {
    Part* lbl = r(result, label(vec2(searchBoxX,0), scale, strdup_s("Search")));
    lbl->foregroundColor = PickerPalette::GrayDark;
  }

  float scrollBoxY = 1 + tspPadding;
  float scrollSize = size.y-scrollBoxY;
  Part* scroll = scrollbox(vec2(0,0), vec2(size.x,scrollSize));

  float x = 0;
  float y = 0;
  float maxY = 0;
  const float scrollSizeXInner = size.x - 0.5f - tspPadding;
  const float imgWidth = scrollSizeXInner/4.f;
  bool anyMatches = false;
  item numMatches = 0;
  const float hoverTextScale = 0.75f;
  Part* ttPanel = 0;

  float scrollShowMax = tspScroll.amount + tspScroll.maxTarget + scrollSize;

  vector<item> textures;
  for (int i = 1; i <= numBuildingTextures(); i++) {
    if (isBuildingTextureIllumination(i) != illumination) continue;
    item textureNdx = getDrawTextureForBuildingTexture(i);
    if (textureNdx == 0) continue;
    textures.push_back(textureNdx);
  }

  vector<item> packageTexs = getAllPackageTextures(illumination);
  for (int i = 0; i < packageTexs.size(); i++) {
    item textureNdx = packageTexs[i];
    if (textureNdx == 0) continue;
    textures.push_back(textureNdx);
  }

  for (int i = 0; i < textures.size(); i++) {
    float trash;
    item textureNdx = textures[i];
    Part* img = image(vec2(x, y), imgWidth-tspPadding, textureNdx, &trash);

    // Filter on search
    if (search) {
      std::string fileStr = img->text;
      if (fileStr.find(tspSearchText) == std::string::npos) {
        freePart(img);
        continue;
      }
    }

    r(scroll, img);
    img->onClick = selectTexture;
    img->onHover = showHoverText;
    img->itemData = textureNdx;
    img->flags |= _partHover;

    float imgY = img->dim.end.y;
    if (imgY <= imgWidth+0.1) {
      img->dim.end.y = imgY = imgY*2;
      img->dim.end.x *= 2;
      img->dim.end.x += tspPadding;
    }

    if (x + img->dim.end.x > scrollSizeXInner) {
      y += maxY + tspPadding;
      x = 0;
      maxY = 0;
      img->dim.start.x = 0;
      img->dim.start.y = y;
    }

    // Show filename on hover
    if (textureNdx == hoverNdx) {
      vec2 pos = vec2(x-imgWidth*.5f, img->dim.start.y + img->dim.end.y + tspPadding*2);
      float txtWidth = stringWidth(img->text)*(hoverTextScale-0.25f) + 0.25f;
      Part* ttTxt = label(vec2(0,0), hoverTextScale, strdup_s(img->text));
      ttPanel = panel(pos, vec2(txtWidth, hoverTextScale));
      ttPanel->dim.start.z += 10;
      ttPanel->renderMode = RenderDarkBox;
      r(ttPanel, ttTxt);

      r(result, ttPanel);
    }

    numMatches ++;
    x += img->dim.end.x + tspPadding;
    if (imgY > maxY) maxY = imgY;

    if(y >= scrollShowMax) break;
  }

  r(result, scrollboxFrame(vec2(0, scrollBoxY), vec2(size.x,scrollSize), &tspScroll, scroll));

  if (ttPanel != 0) {
    float tty = ttPanel->dim.start.y;
    tty -= tspScroll.amount;
    const float maxPos = scrollSize - hoverTextScale - tspPadding;
    if (tty > maxPos) tty = maxPos;
    tty += 1.25;
    ttPanel->dim.start.y = tty;
  }

  hoverNdx = 0;

  return result;
}

