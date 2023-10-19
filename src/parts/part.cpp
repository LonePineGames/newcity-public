#include "part.hpp"

#include "../draw/buffer.hpp"
#include "../draw/camera.hpp"
#include "../draw/entity.hpp"
#include "../sound.hpp"
#include "../util.hpp"

#include "console.hpp"
#include "error.hpp"
#include "loader.hpp"
#include "renderParts.hpp"
#include "root.hpp"
#include "tooltip.hpp"

#include <stdio.h>
#include <csignal>

Part* rootPart = NULL;
Part* focusedPart = NULL;
vec2 lastMouseLoc;

Part* part(vec2 loc) {
  return part(line(vec3(loc, 0), vec3(0,0,0)));
}

Part* part(Line dim) {
  Part* part = (Part*) malloc(sizeof(Part));
  part->dim = dim;
  part->texture = line(vec3(0,0,0), vec3(0,0,0));
  part->text = NULL;
  part->ttType = 0;
  part->ttText = "";
  part->onClick = NULL;
  part->onHover = NULL;
  part->onScroll = NULL;
  part->onText = NULL;
  part->onKeyDown = NULL;
  part->onCustom = NULL;
  part->inputAction = InputAction::ActNone;
  part->renderMode = RenderTransparent;
  part->flags = _partFreePtr;
  part->contents = NULL;
  part->numContents = 0;
  part->foregroundColor = PickerPalette::Transparent;
  part->padding = 0;
  part->itemData = 0;
  part->vecData = vec3(0,0,0);
  part->ptrData = NULL;
  part->lineHeight = 0;
  return part;
}

// Adds to result when building Part
Part* r(Part* branch, Part* leaf) {
  if (leaf == 0 || branch == 0) return leaf;
  branch->numContents ++;
  branch->contents = (Part**) realloc(branch->contents,
    branch->numContents * sizeof(Part*));
  branch->contents[branch->numContents-1] = leaf;
  return leaf;
}

void freePart(Part* part) {
  for (int i = 0; i < part->numContents; i++) {
    freePart(part->contents[i]);
  }
  free(part->contents);
  if (part->text != NULL) {
    free(part->text);
  }
  if (part->ptrData != NULL && part->flags & _partFreePtr) {
    free(part->ptrData);
  }
  free(part);
}

void focus(Part* part) {
  setFreezeCamera(part != 0);
  focusedPart = part;
}

void resetParts() {
  if (focusedPart != 0) {
    setFreezeCamera(false);
    focusedPart = NULL;
  }
  resetPartsRender();
  if (rootPart != NULL) {
    freePart(rootPart);
    rootPart = NULL;
  }
}

bool testInput(Part* part, InputEvent event, vec2 offset) {
  if (!event.isMouse) {
    return false;
  }

  bool dimClip = isInDim(event.mouseLoc-offset, part->dim);
  if (dimClip && part->renderMode != RenderTransparent &&
      part->renderMode != RenderHidden) {
    return true;
  } else if (dimClip || !(part->flags & _partClip)) {
    for (int i = 0; i < part->numContents; i++) {
      if (testInput(part->contents[i], event,
        offset + vec2(part->dim.start) + vec2(part->padding, part->padding))) {
        return true;
      }
    }
  }
  return false;
}

bool isInDim(Part* part, vec2 offset) {
  if (part->renderMode == RenderSpan) {
    vec2 subOff = offset + vec2(part->dim.start) +
      vec2(part->padding, part->padding);
    for (int i = 0; i < part->numContents; i++) {
      if (isInDim(part->contents[i], subOff)) {
        return true;
      }
    }
    return false;

  } else {
    return isInDim(offset, part->dim);
  }
}


bool acceptInput(Part* part, InputEvent event, vec2 offset) {
  if (event.isMouse && part->renderMode == RenderHidden) {
    return false;
  }

  bool dimClip = !event.isMouse || isInDim(part, event.mouseLoc-offset);

  if (dimClip || !(part->flags & _partClip)) {
    for (int i = 0; i < part->numContents; i++) {
      bool result = acceptInput(part->contents[i], event,
        offset + vec2(part->dim.start) + vec2(part->padding, part->padding));
      if (result) {
        return true;
      }
    }
  }

  if (dimClip) {
    InputEvent transformedEvent = event;
    transformedEvent.mouseLoc -= offset + vec2(part->dim.start);
    if ((event.isMouse && event.action == GLFW_PRESS && part->onClick != NULL) || inputActionValid(part->inputAction, event) && inputActionPressed(part->inputAction)) {
      bool result = part->onClick(part, transformedEvent);
      if (result) {
        if (!(part->flags & _partDisableSounds)) {
          playSound(_soundClickCancel);
        }
        return true;
      }
    }

    if (!event.isMouse && event.action == GLFW_PRESS &&
      part->onKeyDown != NULL
    ) {
      if(consoleIsOpen() && part->itemData != CONSOLE_VAL) {
        return true;
      }

      bool result = part->onKeyDown(part, transformedEvent);
      if (result) {
        if (!(part->flags & _partDisableSounds)) {
          playSound(_soundClickCancel);
        }
        return true;
      }
    }

    if (event.action == MOUSE_MOVED_INPUT && part->onHover != NULL) {
      bool result = part->onHover(part, transformedEvent);
      if (result) {
        return true;
      }
    }

    if (event.action == UNICODE_INPUT && part->onText != NULL) {
      bool result = part->onText(part, transformedEvent);
      if (result) {
        return true;
      }
    }

    if (event.action == SCROLL && part->onScroll != NULL) {
      bool result = part->onScroll(part, transformedEvent);
      if (result) {
        return true;
      }
    }
  }
  return false;
}

bool acceptInput(InputEvent event) {
  if (rootPart == NULL) {
    return false;
  } else if (focusedPart != 0 && acceptInput(focusedPart, event, vec2(0,0))) {
    return true;
  } else if (acceptInput(rootPart, event, vec2(0,0))) {
    return true;
  } else {
    return testInput(rootPart, event, vec2(0,0));
  }
}

void setPartTooltipValues(Part* part, int type) {
  if(part == 0) {
    return;
  }

  part->ttType = type;
  part->ttText = getTooltipText(type);
}

void setPartTooltipValues(Part* part, int type, const char* text) {
  if(part == 0) {
    return;
  }

  part->ttType = type;
  part->ttText = text;
}

void renderUI() {
  float aspectRatio = getAspectRatio();
  resetParts();
  rootPart = root(aspectRatio);
  renderPart(rootPart, lastMouseLoc);
}

void renderErrorLoadPanel() {
  float aspectRatio = getAspectRatio();
  resetParts();
  rootPart = errorLoadPanel(aspectRatio);
  renderPart(rootPart, lastMouseLoc);
}

vec2 transformMouseLoc(vec2 mouseLoc) {
  mat4 matrix = inverse(getUICamera().projection);
  lastMouseLoc = vec2(matrix * vec4(mouseLoc, 0, 1));
  return lastMouseLoc;
}

