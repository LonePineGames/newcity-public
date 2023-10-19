#include "../draw/camera.hpp"
#include "../error.hpp"
#include "../game/feature.hpp"
#include "../game/game.hpp"
#include "../graph/stop.hpp"
#include "../graph/transit.hpp"
#include "../heatmap.hpp"
#include "../label.hpp"
#include "../lot.hpp"
#include "../person.hpp"
#include "../renderGraph.hpp"
#include "../vehicle/renderVehicle.hpp"
#include "../selection.hpp"
#include "../sound.hpp"
#include "../zone.hpp"

#include "../parts/block.hpp"
#include "../parts/root.hpp"
#include "../parts/slider.hpp"
#include "../parts/textBox.hpp"
#include "../parts/tooltip.hpp"

#include "tool.hpp"

#include "spdlog/spdlog.h"

static item currentLabel = 0;
static item lastHoverLabel = 0;
static item lastLabelFlags = _labelExists | (6 << _labelSizeShift);
TextBoxState labelTbState;

void correctCameraDistance() {
  //float fontSize = getLabelFontSize(currentLabel);
  Label* label = getLabel(currentLabel);
  if (label->entity == 0) return;
  Entity* e = getEntity(label->entity);
  float minDist = e->simpleDistance*2.f;
  float maxDist = e->maxCameraDistance*.02f;
  float dist = getCameraDistance();
  if (dist < minDist) {
    setCameraDistance(minDist);
  } else if (dist > maxDist) {
    setCameraDistance(maxDist);
  }
}

void label_mouse_button_callback(InputEvent event) {
  int button = event.button;
  int action = event.action;

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    playSound(_soundClickCancel);

    labelTbState.flags |= _textBoxEditing;
    currentLabel = intersectLabel(event.mouseLine);
    if (currentLabel == 0) {
      currentLabel = addLabel(landIntersect(event.mouseLine),
          lastLabelFlags);
    }

    Label* lab = getLabel(currentLabel);
    setCameraTarget(lab->location);
    correctCameraDistance();
  }
}

void label_mouse_move_callback(InputEvent event) {
  if (lastHoverLabel != 0) {
    setLabelHighlight(lastHoverLabel, true);
    lastHoverLabel = 0;
  }

  lastHoverLabel = intersectLabel(event.mouseLine);
  setLabelHighlight(lastHoverLabel, false);
}

void label_select() {
  setLabelsVisible(true);
}

void label_reset() {
  currentLabel = 0;
  labelTbState.flags &= ~_textBoxEditing;

  if (lastHoverLabel != 0) {
    setLabelHighlight(lastHoverLabel, true);
    lastHoverLabel = 0;
  }
}

bool finishEditingLabel(Part* part, InputEvent event) {
  SPDLOG_INFO("finishEditingLabel");
  labelTbState.flags &= ~_textBoxEditing;
  currentLabel = 0;
  focusTextBox(0);
  return true;
}

bool adjustLabelSize(Part* part, InputEvent event) {
  int size = round(part->vecData.x * 15);
  Label* lab = getLabel(currentLabel);
  lab->flags &= ~_labelSizeMask;
  lab->flags |= size << _labelSizeShift;
  correctCameraDistance();
  return true;
}

bool setLabelRed(Part* part, InputEvent event) {
  Label* lab = getLabel(currentLabel);
  lab->flags |= _labelRed;
  lab->flags &= ~_labelBlue;
  return true;
}

bool setLabelBlue(Part* part, InputEvent event) {
  Label* lab = getLabel(currentLabel);
  lab->flags |= _labelBlue;
  lab->flags &= ~_labelRed;
  return true;
}

bool setLabelWhite(Part* part, InputEvent event) {
  Label* lab = getLabel(currentLabel);
  lab->flags &= ~_labelBlue;
  lab->flags &= ~_labelRed;
  return true;
}

bool removeLabel(Part* part, InputEvent event) {
  removeLabel(currentLabel);
  labelTbState.flags &= ~_textBoxEditing;
  currentLabel = 0;
  focusTextBox(0);
  return true;
}

Part* label_render(Line dim) {
  Part* result = panel(dim);

  r(result, label(vec2(0,0), 1, strdup_s("Label")));
  //r(result, hr(vec2(0,1.0), dim.end.x-toolPad*2));

  if (currentLabel == 0) {
    r(result, icon(vec2(4.f,2.5f), vec2(1,1), iconPointer));

  } else {
    if (labelTbState.flags & _textBoxEditing) {

      renderLabel(currentLabel);
      Label* lab = getLabel(currentLabel);
      lastLabelFlags = lab->flags;
      labelTbState.flags |= _textBoxEditing;
      labelTbState.text = &(lab->text);
      focusTextBox(&labelTbState);

      r(result, label(vec2(0,0), 1, strdup_s("Label")));

      Part* tb = r(result, textBoxLabel(vec2(0.5f, 2.f), vec2(9.f, 1.f),
          &labelTbState));
      tb->onCustom = finishEditingLabel;

      r(result, button(vec2(.5f, 4.f), iconNo, removeLabel));
      r(result, button(vec2(2.f, 4.f), iconRed, setLabelRed));
      //r(result, button(vec2(2.5f, 5.f), iconZoneColor[RetailZone],
            //setLabelBlue));
      r(result, button(vec2(3.f, 4.f), iconWhite, setLabelWhite));

      float val = (lab->flags & _labelSizeMask) >> _labelSizeShift;
      val /= 15;
      r(result, slider(vec2(5.f, 4.f), vec2(4.5f, 1.f), val, adjustLabelSize));

      return result;

    } else {
      Label* lab = getLabel(currentLabel);
      if (strlen(lab->text) == 0) {
        removeLabel(currentLabel);
      }
      currentLabel = 0;
    }
  }

  return result;
}

bool label_visible() {
  return true;
}

Tool toolLabel = {
  label_mouse_button_callback,
  label_mouse_move_callback,
  label_select,
  label_reset,
  label_visible,
  iconMessage,
  label_render,
  queryInstructionPanel,
};

