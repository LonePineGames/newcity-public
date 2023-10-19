#include "scrollbox.hpp"

#include "panel.hpp"
#include "slider.hpp"

#include <stdio.h>
#include <algorithm>

void resetScrollState(ScrollState* state) {
  state->target = 0;
  state->amount = 0;
  state->lastCameraTime = 0;
}

bool scrollClickCallback(Part* part, InputEvent event) {
  if (part->ptrData != NULL) {
    float value = part->vecData.x;
    ScrollState* state = (ScrollState*) part->ptrData;
    state->target = state->maxTarget * value;
  }
  return true;
}

bool scrollCallback(Part* part, InputEvent event) {
  float hiddenSpace = part->vecData.y;
  float value = part->vecData.x;

  float actualScroll = value * hiddenSpace;
  actualScroll -= event.scroll.y*c(CScrollSpeed);
  if (actualScroll < 0) {
    actualScroll = 0;
  } else if (actualScroll > hiddenSpace) {
    actualScroll = hiddenSpace;
  }

  if (part->ptrData != NULL) {
    ScrollState* state = (ScrollState*) part->ptrData;
    state->target = actualScroll;
  }
  return true;
}

Part* scrollbox(vec2 start, vec2 size) {
  Part* result = panel(line(vec3(start,0), vec3(size,0)));
  result->renderMode = RenderTransparent;
  result->padding = 0.25;
  return result;
}

Part* scrollboxFrame(vec2 start, vec2 size, float innerSpace,
  ScrollState* state, Part* scrollbox, bool inverted) {

  float innerSize = innerSpace;
  innerSize += scrollbox->padding*2;
  float hiddenSpace = std::max(innerSize - size.y, 0.f);
  state->maxTarget = hiddenSpace;
  if(state->target != state->target) {
    state->target = 0;
  } else if(state->target > hiddenSpace) {
    state->target = hiddenSpace;
  } else if(state->target < 0) {
    state->target = 0;
  }

  // Smooth scrolling
  float time = getCameraTime();
  state->amount = mix(state->amount, state->target,
      5*clamp(time - state->lastCameraTime, 0.f, 0.1f));
  if (abs(state->amount - state->target) < 0.02f) state->amount = state->target;

  state->lastCameraTime = time;
  float scrollFrac = state->amount/hiddenSpace;
  scrollbox->dim.start.y -= state->amount;

  Part* result = panel(start, size);
  result->onScroll = scrollCallback;
  result->vecData = vec3(scrollFrac, hiddenSpace, 0);
  result->flags |= _partLowered | _partClip;
  result->flags &= ~_partFreePtr;
  result->ptrData = (void*)state;
  result->dim.end.z = -state->amount;

  r(result, scrollbox);

  if(hiddenSpace > 0) {
    float sliderX = inverted ? 0 : size.x - 0.5;
    Part* sldr = vslider(vec2(sliderX, 0), vec2(0.51, size.y),
      scrollFrac, scrollClickCallback);
    sldr->dim.start.z += 10;
    sldr->ptrData = (void*)state;
    state->slider = sldr;
    r(result, sldr);
  } else {
    state->slider = 0;
  }

  return result;
}

Part* scrollboxFrame(vec2 start, vec2 size, ScrollState* state,
    Part* scrollbox, bool inverted) {

  float innerSize = 0;
  for (int i = 0; i < scrollbox->numContents; i++) {
    Part* p = scrollbox->contents[i];
    float bottom = p->dim.start.y + p->dim.end.y;
    if (p->flags & _partIsPanel) {
      bottom += 0.1;
    }
    if (innerSize < bottom) {
      innerSize = bottom;
    }
  }

  return scrollboxFrame(start, size, innerSize, state, scrollbox, inverted);
}

Part* scrollboxFrame(vec2 start, vec2 size, ScrollState* state,
    Part* scrollbox) {
  return scrollboxFrame(start, size, state, scrollbox, false);
}

Part* scrollboxFrame(vec2 start, vec2 size, float innerSpace,
  ScrollState* state, Part* scrollbox) {
  return scrollboxFrame(start, size, innerSpace, state, scrollbox, false);
}
