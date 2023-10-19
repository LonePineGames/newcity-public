#include "renderParts.hpp"

#include "tooltip.hpp"

#include "../color.hpp"
#include "../draw/entity.hpp"
#include "../draw/texture.hpp"
#include "../renderUtils.hpp"
#include "../string.hpp"
#include "../string_proxy.hpp"
#include "../util.hpp"

#include "spdlog/spdlog.h"
#include <stdio.h>
#include <algorithm>

item currentPartEntity = 0;
vector<item> partsEntities;

item getPartEntity(Shader s) {
  if (currentPartEntity < partsEntities.size()) {
    item result = partsEntities[currentPartEntity];
    currentPartEntity ++;
    Entity* e = getEntity(result);
    e->shader = s;
    setEntityVisible(result, true);
    return result;

  } else {
    item result = addEntity(s);
    createMeshForEntity(result);
    Mesh* mesh = getMeshForEntity(result);
    mesh->flags |= _meshStreamDraw;
    partsEntities.push_back(result);
    currentPartEntity ++;
    return result;
  }
}

struct PartRenderer {
  item textEntity;
  item iconEntity;
  item paletteEntity;
  item shadowEntity;
  char foregroundColor;
  vec3 start;
  Line clip;
  vec2 mouseLoc;
  bool lowered;
  bool highlight;
};

PartRenderer makeRenderer(vec2 mouseLoc) {
  PartRenderer result;
  result.lowered = false;
  result.highlight = false;
  result.textEntity = 0;
  result.iconEntity = 0;
  result.paletteEntity = 0;
  result.shadowEntity = 0;
  result.foregroundColor = PickerPalette::White;
  result.start = vec3(0,0,0);
  result.mouseLoc = mouseLoc;
  result.clip = line(vec3(-100000, -100000, 0),
    vec3(100000, 100000, 0));
  return result;
}

PartRenderer makeRenderer(PartRenderer parent, Part* part) {
  float padding = part->padding;
  vec3 start = parent.start + part->dim.start;
  vec3 size = part->dim.end;
  vec3 end = start + size + vec3(padding*2.f, padding*2.f, 0);

  PartRenderer result;
  result.lowered = parent.lowered;
  result.highlight = parent.highlight;
  result.start = start + vec3(padding, padding, 2);
  result.mouseLoc = parent.mouseLoc;
  if (part->foregroundColor != PickerPalette::Transparent) {
    result.foregroundColor = part->foregroundColor;
  } else {
    result.foregroundColor = parent.foregroundColor;
  }
  uint32_t entityDataFlags = result.foregroundColor;

  if (part->flags & _partClip) {
    vec3 clipStart = vec3(std::max(start.x, parent.clip.start.x),
      std::max(start.y, parent.clip.start.y), start.z);
    vec3 clipEnd = vec3(std::min(end.x, parent.clip.end.x),
      std::min(end.y, parent.clip.end.y), parent.clip.end.z);
    result.clip = line(clipStart, clipEnd);
  } else {
    result.clip = parent.clip;
  }

  result.clip.end.z += part->dim.end.z;

  result.paletteEntity = getPartEntity(UIPaletteShader);
  Entity* pEntity = getEntity(result.paletteEntity);
  if (part->flags & _partTextured) pEntity->flags |= _entityRedHighlight;
  pEntity->texture = paletteTexture;
  pEntity->dataFlags = PickerPalette::White;
  setEntityClip(result.paletteEntity, result.clip);

  result.textEntity = getPartEntity(UITextShader);
  Entity* tEntity = getEntity(result.textEntity);
  tEntity->texture = textTexture;
  tEntity->dataFlags = entityDataFlags;
  setEntityClip(result.textEntity, result.clip);

  result.iconEntity = getPartEntity(UIIconShader);
  Entity* iEntity = getEntity(result.iconEntity);
  iEntity->texture = iconTexture;
  iEntity->dataFlags = entityDataFlags;
  setEntityClip(result.iconEntity, result.clip);

  result.shadowEntity = getPartEntity(UIShadowShader);
  Entity* sEntity = getEntity(result.shadowEntity);
  sEntity->texture = paletteTexture;
  sEntity->dataFlags = PickerPalette::White;
  setEntityClip(result.shadowEntity, result.clip);

  if (part->flags & _partTextShadow) {
    tEntity->flags |= _entityBlueHighlight;
    iEntity->flags |= _entityBlueHighlight;
  } else {
    tEntity->flags &= ~_entityBlueHighlight;
    iEntity->flags &= ~_entityBlueHighlight;
  }

  if (part->flags & _partDesaturate) {
    pEntity->flags |= _entityDesaturate;
  } else {
    pEntity->flags &= ~_entityDesaturate;
  }


  return result;
}

void renderImage(PartRenderer renderer, Part* part, Line dim) {
  item entityNdx = getPartEntity(UIIconShader);
  Entity* entity = getEntity(entityNdx);
  entity->texture = getTexture(part->text);
  setEntityClip(entityNdx, renderer.clip);

  if (renderer.highlight) {
    entity->dataFlags = PickerPalette::CyanLight;
  } else if (part->foregroundColor != PickerPalette::Transparent) {
    entity->dataFlags = part->foregroundColor;
  } else {
    entity->dataFlags = renderer.foregroundColor;
  }

  if (part->flags & _partDesaturate) {
    entity->flags |= _entityDesaturate;
  } else {
    entity->flags &= ~_entityDesaturate;
  }

  vec3 c0 = dim.start;
  vec3 c3 = dim.start + dim.end;
  c0.z += 0.25f;
  c3.z = c0.z;
  vec3 c1 = vec3(c3.x, c0.y, c0.z);
  vec3 c2 = vec3(c0.x, c3.y, c0.z);

  makeQuad(getMeshForEntity(entityNdx),
    c0, c1, c2, c3, vec3(0,0,0), vec3(1,1,0));
}

void renderBox(PartRenderer renderer, Line dim, float zOff, vec3 color) {
  vec3 c0 = dim.start;
  vec3 c3 = dim.start + dim.end;
  c0.z += zOff;
  c3.z = c0.z;
  vec3 c1 = vec3(c3.x, c0.y, c0.z);
  vec3 c2 = vec3(c0.x, c3.y, c0.z);

  makeQuad(getMeshForEntity(renderer.paletteEntity),
    c0, c1, c2, c3, color, color);
}

void renderGradient(PartRenderer renderer, Line dim, float zOff,
    vec3 startColor, vec3 endColor, vec3 adjustment, bool rotate) {
  vec3 c0 = dim.start;
  vec3 c3 = dim.start + dim.end;
  c0.z += zOff;
  c3.z = c0.z;
  vec3 c1 = vec3(c3.x, c0.y, c0.z);
  vec3 c2 = vec3(c0.x, c3.y, c0.z);
  vec3 up = vec3(0, 0, 1);
  vec3 t0 = startColor;
  vec3 t3 = endColor;
  vec3 t1 = vec3(endColor.x, startColor.y, startColor.z);
  vec3 t2 = vec3(startColor.x, endColor.y, startColor.z);

  if (rotate) {
    vec3 temp = t1;
    t1 = t2;
    t2 = temp;
  }

  if (adjustment.z > 0) {
    c0.y += adjustment.y;
    t0.y += adjustment.x;
  } else {
    c2.y += adjustment.y;
    t2.y += adjustment.x;
  }

  makeQuad(getMeshForEntity(renderer.paletteEntity),
    c0, c1, c2, c3,
    up, up, up, up,
    t0, t1, t2, t3);
    //startColor, endColor);
}

void renderGradient(PartRenderer renderer, Line dim, float zOff,
    vec3 startColor, vec3 endColor) {
  renderGradient(renderer, dim, zOff, startColor, endColor, vec3(0,0,0), false);
}

vec3 shadowColor(int x, int y) {
  return vec3(
      (x == 1 || x == 2) ? 0 : 1,
      (y == 1 || y == 2) ? 0 : 1,
      0);

  //return (x == 1 || x == 2) && (y == 1 || y == 2) ?
    //vec3(0,0,0) :
    //vec3(1,1,0);
    //xShadowColor : xShadowTransparent;
}

void renderPanel(PartRenderer renderer, Line dim, Line gradient, bool shadow) {
  vec3 c0 = dim.start;
  c0.z -= 0.5;
  vec3 c3 = dim.start + dim.end;
  c3.z = c0.z;
  vec3 c1 = vec3(c3.x, c0.y, c0.z);
  vec3 c2 = vec3(c0.x, c3.y, c0.z);
  vec3 offset = vec3(0.5, 0.5, -0.25);
  vec3 normal = vec3(0, 0, 1);
  vec3 grad0 = gradient.start;
  vec3 grad2 = gradient.end;
  vec3 grad1 = .5f*(gradient.start + gradient.end);

  makeQuad(getMeshForEntity(renderer.paletteEntity),
    c0, c1, c2, c3, normal, normal, normal, normal,
    grad0, grad1, grad2, grad2);

  if (!shadow) return;

  Mesh* shadowMesh = getMeshForEntity(renderer.shadowEntity);

  /*
  vec3 offDown = vec3(0,1,0);
  vec3 offRight = vec3(1,0,0);
  vec3 panelAlong = c3-c0;
  vec3 texAlong = panelAlong + offDown*3.f + offRight*3.f;
  //vec3 ratio = texAlong / panelAlong;
  vec3 texStart = (-offDown-offRight)/panelAlong;
  vec3 texEnd = (offDown*2.f+offRight*2.f)/panelAlong + vec3(1,1,0);
  makeQuad(shadowMesh, c0-offDown-offRight, c1-offDown+offRight*2.f,
    c2+offDown*2.f-offRight, c3+offDown*2.f+offRight*2.f,
    texStart, texEnd);
    //vec3(0,0,0), vec3(1,1,0));
  */

  //vec3 boxOffset[4] = {c0, c0+offset, c3+offset, c3+offset*2.f};
  vec3 boxOffset[4] = {c0-offset*.25f, c0+offset*.25f, c3,
    c3+offset*.5f};
  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {

      vec3 tr = vec3(boxOffset[x].x, boxOffset[y].y, offset.z);
      vec3 bl = vec3(boxOffset[x+1].x, boxOffset[y+1].y, offset.z);
      vec3 tl = vec3(bl.x, tr.y, tr.z);
      vec3 br = vec3(tr.x, bl.y, tr.z);
      vec3 xtr = shadowColor(x,y);
      vec3 xtl = shadowColor(x+1,y);
      vec3 xbr = shadowColor(x,y+1);
      vec3 xbl = shadowColor(x+1,y+1);
      if (x == y) {
        makeQuad(shadowMesh, tl, tr, bl, br, normal, normal, normal, normal,
            xtl, xtr, xbl, xbr);
      } else {
        makeQuad(shadowMesh, tr, tl, br, bl, normal, normal, normal, normal,
            xtr, xtl, xbr, xbl);
      }
    }
  }
}

void renderPanel(PartRenderer renderer, Line dim) {
  renderPanel(renderer, dim, line(colorPanelGrad1, colorPanelGrad0), true);
}

void renderPanel(PartRenderer renderer, Line dim, Line gradient) {
  renderPanel(renderer, dim, gradient, true);
}

void renderPanelTooltip(PartRenderer renderer, Line dim) {
  renderPanel(renderer, dim,
      line(colorTransparentBlack, colorTransparentBlack), true);
}

void renderText(PartRenderer renderer, Line dim, const char* text,
    float lineHeight, float firstLineIndent) {
  dim.start.z += 0.25;
  renderString(getMeshForEntity(renderer.textEntity), text, dim.start,
    lineHeight, firstLineIndent);
}

void renderText(PartRenderer renderer, Line dim, const char* text,
    float lineHeight) {
  renderText(renderer, dim, text, lineHeight, 0);
}

void renderTextCentered(PartRenderer renderer, Line dim,
    const char* text, float lineHeight) {
  dim.start.z += 0.25;
  renderStringCentered(getMeshForEntity(renderer.textEntity),
      text, dim.start, vec3(lineHeight, 0, 0), vec3(0, lineHeight, 0));
}

void renderIcon(PartRenderer renderer, Line dim, Line texture) {
  vec3 c0 = dim.start;
  vec3 c3 = dim.start + dim.end;
  c3.z = dim.start.z;
  vec3 c1 = vec3(c3.x, c0.y, c0.z);
  vec3 c2 = vec3(c0.x, c3.y, c0.z);

  makeQuad(getMeshForEntity(renderer.iconEntity), c0, c1, c2, c3,
    texture.start, texture.end);
}

void renderTooltip(PartRenderer renderer, Line dim, const char* text) {
  renderPanelTooltip(renderer, dim);

  Line lineText = dim;
  lineText.start.x += ttPadding;
  lineText.start.y += ttPadding;
  lineText.end.x += ttPadding;
  lineText.end.y += ttPadding;

  renderText(renderer, lineText, (char*)text, ttTxtSize);
}

void renderPart(Part* part, PartRenderer renderer) {
  if (part->renderMode == RenderHidden) {
    return;
  }

  Line dim = line(part->dim.start + renderer.start,
    part->dim.end);
  bool colorChanged = (part->foregroundColor != PickerPalette::Transparent) &&
      (part->foregroundColor != renderer.foregroundColor);
  if (colorChanged || (part->flags & _partIsPanel) ||
      (part->flags & _partTransparent)) {
    renderer = makeRenderer(renderer, part);
  }

  bool dimClip = part->renderMode == RenderSpan ? isInDim(part, renderer.mouseLoc - vec2(renderer.start))
    : isInDim(renderer.mouseLoc, dim);
  if ((part->flags & _partHighlight) || ((part->flags & _partHover) && (dimClip||inputActionPressed(part->inputAction)))) {
    renderer.highlight = true;

    if (part->renderMode == RenderSpan) {
      for (int i=0; i < part->numContents; i++) {
        part->contents[i]->flags |= _partHighlight;
      }

    } else if (renderer.lowered) {
      renderGradient(renderer, dim, -0.5,
          colorRaisedGrad0, colorRaisedGrad1);
    } else {
      renderGradient(renderer, dim, -0.5, colorLoweredGrad0, colorLoweredGrad1);
    }
  }

  // Handle tooltip
  if (dimClip && part->ttText != 0 && !streql(part->ttText, "")) {

    // Update active tooltip and last hover time, if tooltip has changed
    updateTooltip(part);

    if (getTooltipShow() &&
        (getCameraTime() - getTooltipLastHover()) > ttTimeToShow) {
      Line ttDim;
      const char* ttText = part->ttText;
      ttDim.start.x = renderer.mouseLoc.x;
      ttDim.start.y = renderer.mouseLoc.y + ttYOffset;
      ttDim.start.z = 40;
      vec2 textDim = stringDimensions(ttText);
      textDim.y *= 1.1;
      ttDim.end.x = ttTxtSize*textDim.x + (ttPadding*3);
      ttDim.end.y = ttTxtSize*textDim.y + (ttPadding*2);
      ttDim.end.z = 0;

      Line clip = renderer.clip;
      float uiX = uiGridSizeX*getAspectRatio();
      clip.start.x = clamp(clip.start.x, 0.f, uiX);
      clip.start.y = clamp(clip.start.y, 0.f, uiGridSizeY);
      clip.end.x = clamp(clip.end.x, 0.f, uiX);
      clip.end.y = clamp(clip.end.y, 0.f, uiGridSizeY);

      //SPDLOG_INFO("ttDim ({},{};{},{}) clip ({},{};{},{})",
          //ttDim.start.x, ttDim.start.y, ttDim.end.x, ttDim.end.y,
          //clip.start.x, clip.start.y, clip.end.x, clip.end.y);

      if (ttDim.start.x + ttDim.end.x > clip.end.x)
        ttDim.start.x -= ttDim.end.x;
      if (ttDim.start.y + ttDim.end.y > clip.end.y)
        ttDim.start.y -= ttDim.end.y + ttYOffset;
      if (ttDim.start.x < clip.start.x) ttDim.start.x = clip.start.x;
      if (ttDim.start.y < clip.start.y) ttDim.start.y = clip.start.y;

      renderTooltip(renderer, ttDim, ttText);

      /*
      glfwGetWindowSize(getWindow(), &xRes, &yRes);
      glfwGetCursorPos(getWindow(), &xPos, &yPos);

      // Y value higher than safezone
      if(yPos > (double)(yRes*ttSafeZoneCoeff)) {
        ttYStart -= (ttYOffset*2);
      }

      // X value higher than safezone
      if(ttXStart+ttXEnd > 50.0f) {
        ttXStart -= ttXEnd;
      }

      if(ttXStart < 0) {
        ttXStart = 0;
      }

      if(ttYStart < 0) {
        ttYStart = 0;
      }

      ttDim.start = vec3(ttXStart, ttYStart, 10.0f);
      ttDim.end = vec3(ttXEnd, ttYEnd, 10.0f);

      renderTooltip(renderer, ttDim, ttText);
      */
    }
  }

  if (part->flags & _partBlink) {
    if (int(getCameraTime()*2) % 2 == 0) {
      renderGradient(renderer, dim, -0.5,
          colorRaisedGrad0, colorRaisedGrad1);
    }
  }
  if (part->flags & _partRaised) {
    renderGradient(renderer, dim, -0.5,
        colorPanelGrad0, colorPanelGrad1);
  }
  if (part->flags & _partLowered) {
    renderer.lowered = true;
    renderGradient(renderer, dim, -0.5, colorLoweredGrad0, colorLoweredGrad1);
  }
  //if (part->flags & _partRedText) {
    //setEntityRedHighlight(renderer.textEntity, true);
    //setEntityRedHighlight(renderer.iconEntity, true);
  //}
  //if (part->flags & _partBlackText) {
    //setEntityBlueHighlight(renderer.textEntity, true);
    //setEntityBlueHighlight(renderer.iconEntity, true);
  //}
  if (part->flags & _partTransparent) {
    setEntityTransparent(renderer.textEntity, true);
    setEntityTransparent(renderer.paletteEntity, true);
    setEntityTransparent(renderer.iconEntity, true);
    setEntityTransparent(renderer.shadowEntity, true);
  }

  if (part->renderMode == RenderPanel) {
    renderPanel(renderer, dim);
  } else if (part->renderMode == RenderPanelGradient) {
    renderPanel(renderer, dim, part->texture, true);
  } else if (part->renderMode == RenderShadow) {
    renderGradient(renderer, dim, -0.75, colorTransparent0, colorTransparent1);
  } else if (part->renderMode == RenderWhiteBox) {
    renderBox(renderer, dim, 0, colorWhite);
  } else if (part->renderMode == RenderDarkBox) {
    renderGradient(renderer, dim, -0.25, colorDarkGrad0, colorDarkGrad1);

  } else if (part->renderMode == RenderGradient) {
    renderGradient(renderer, dim, 0, part->texture.start, part->texture.end);
  } else if (part->renderMode == RenderGradientRotated) {
    renderGradient(renderer, dim, 0,
        part->texture.start, part->texture.end, vec3(0,0,0), true);

  } else if (part->renderMode == RenderGradientAdjusted) {
    renderGradient(renderer, dim, 0,
        part->texture.start, part->texture.end, part->vecData, false);

  } else if (part->renderMode == RenderImage) {
    renderImage(renderer, part, dim);

  } else if (part->renderMode == RenderSpan) {
    Line dim2 = dim;
    float p = part->padding;
    dim2.start.x += p;
    dim2.start.y += p;
    dim2.end.x -= p*2;
    dim2.end.y -= p*2;
    if (part->lineHeight == 0) {
      part->lineHeight = dim2.end.y;
    }

    if (part->flags & _partAlignCenter) {
      float l = stringWidth(part->text) * part->lineHeight;
      dim2.start.x += (dim2.end.x - l) / 2;

    } else if (part->flags & _partAlignRight) {
      float l = stringWidth(part->text) * part->lineHeight;
      dim2.start.x += dim2.end.x - l;
    }

    renderText(renderer, dim2, part->text, part->lineHeight, part->vecData.x);

  } else if (part->renderMode == RenderText) {
    Line dim2 = dim;
    float p = part->padding;
    dim2.start.x += p;
    dim2.start.y += p;
    dim2.end.x -= p*2;
    dim2.end.y -= p*2;
    if (part->lineHeight == 0) {
      part->lineHeight = dim2.end.y;
    }

    if (part->flags & _partAlignCenter) {
      float l = stringWidth(part->text) * part->lineHeight;
      dim2.start.x += (dim2.end.x - l) / 2;

    } else if (part->flags & _partAlignRight) {
      float l = stringWidth(part->text) * part->lineHeight;
      dim2.start.x += dim2.end.x - l;
    }

    renderText(renderer, dim2, part->text, part->lineHeight);

  } else if (part->renderMode == RenderIconAndText) {
    Line dim2 = dim;
    float p = part->padding;
    dim2.start.x += p + dim.end.y;
    dim2.start.y += p;
    dim2.end.x -= p*2;
    dim2.end.y -= p*2;
    if (part->lineHeight == 0) {
      part->lineHeight = dim2.end.y;
    }
    renderText(renderer, dim2, part->text, part->lineHeight);

    Line dim3 = dim;
    dim3.end.x = dim.end.y;
    renderIcon(renderer, dim3, part->texture);

  } else if (part->renderMode == RenderIcon) {
    renderIcon(renderer, dim, part->texture);

    bool anyText = false;
    string text = "";
    if (part->text != 0) {
      anyText = true;
      text = part->text;
    }

    if (part->inputAction != InputAction::ActNone) {
      KeyBind bind = getKeyBind(part->inputAction);
      if (bind.key != GLFW_KEY_UNKNOWN) {
        text = getKeyStr(bind.key);
        anyText = true;
      }
    }

    if (anyText) {
      Line dim2 = dim;
      float width = stringWidth(text.c_str()) * 0.4;
      dim2.start += vec3(dim.end.x-width+0.05, dim.end.y-0.35, 80.0);
      dim2.end = vec3(width, 0.4, 0);
      renderBox(renderer, dim2, -0.1, colorDarkGrad0);

      Line dim3 = dim2;
      dim3.start += vec3(0.025, 0.025, 0.25);
      dim3.end = vec3(0.35, 0.35, 0);
      renderText(renderer, dim2, text.c_str(), dim2.end.y);
    }
  }

  for (int i=0; i < part->numContents; i++) {
    renderPart(part->contents[i], renderer);
  }
}

void resetPartsEntities() {
  for (int i=0; i < partsEntities.size(); i++) {
    removeEntityAndMesh(partsEntities[i]);
  }
  partsEntities.clear();
}

void resetPartsRender() {
  for (int i=0; i < partsEntities.size(); i++) {
    setEntityVisible(partsEntities[i], false);
    setEntityRedHighlight(partsEntities[i], false);
    setEntityTransparent(partsEntities[i], false);
    getEntity(partsEntities[i])->flags &= ~_entityDesaturate;
    getEntity(partsEntities[i])->flags &= ~_entityBlueHighlight;
  }
  currentPartEntity = 0;
}

void renderPart(Part* part, vec2 mouseLoc) {
  PartRenderer renderer = makeRenderer(mouseLoc);
  renderPart(part, renderer);

  for (int i=0; i < currentPartEntity; i++) {
    bufferMeshForEntity(partsEntities[i]);
  }
}

