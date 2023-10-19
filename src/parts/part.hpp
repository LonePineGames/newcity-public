#pragma once

#include "../color.hpp"
#include "../input.hpp"
#include "../line.hpp"

enum RenderMode {
  RenderTransparent, RenderHidden, RenderText, RenderIcon,
  RenderPanel, RenderPanelGradient, RenderWhiteBox,
  RenderDarkBox, RenderRedBox,
  RenderShadow, RenderIconAndText, RenderGradient, RenderGradientAdjusted,
  RenderGradientRotated, RenderSpan, RenderImage,
  numRenderModes
};

const int _partIsPanel = 1 << 0;
const int _partHighlight = 1 << 1;
const int _partRaised = 1 << 2;
const int _partLowered = 1 << 3;
const int _partHover = 1 << 4;
const int _partFreePtr = 1 << 5;
const int _partClip = 1 << 6;
const int _partVertical = 1 << 7;
const int _partAlignCenter = 1 << 8;
const int _partAlignRight = 1 << 9;
const int _partDisableSounds = 1 << 10;
const int _partBlink = 1 << 11;
const int _partTransparent = 1 << 12;
const int _partDesaturate = 1 << 13;
const int _partTextShadow = 1 << 14;
const int _partTextured = 1 << 15;

const float uiGridSizeX = 30;
const float uiGridSizeY = 30;

struct Part;

typedef bool (*InputCallback)(Part* part, InputEvent event);

struct Part {
  Line dim;
  Line texture;
  Part** contents;
  int numContents;
  char* text;
  int ttType;
  const char* ttText;
  InputCallback onClick;
  InputCallback onHover;
  InputCallback onScroll;
  InputCallback onKeyDown;
  InputCallback onText;
  InputCallback onCustom;
  InputAction inputAction = InputAction::ActNone;
  item renderMode;
  item itemData;
  char foregroundColor = PickerPalette::Transparent;
  void* ptrData;
  vec3 vecData;
  int flags;
  float padding;
  float lineHeight = 0;
};

Part* part(vec2 loc);
Part* part(Line dim);
void freePart(Part* part);
void focus(Part* part);
void renderParts();
void resetParts();
bool acceptInput(InputEvent event);
void setPartTooltipValues(Part* part, int type);
void setPartTooltipValues(Part* part, int type, const char* text);
Part* r(Part* branch, Part* leaf);
void renderUI();
void renderErrorLoadPanel();
vec2 transformMouseLoc(vec2 mouseLoc);
bool isInDim(Part* part, vec2 offset);

