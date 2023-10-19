#include "transitPanel.hpp"

#include "block.hpp"
#include "button.hpp"
#include "colorPicker.hpp"
#include "hr.hpp"
#include "icon.hpp"
#include "label.hpp"
#include "leftPanel.hpp"
#include "panel.hpp"
#include "scrollbox.hpp"
#include "slider.hpp"
#include "textBox.hpp"
#include "transitDesignerPanel.hpp"

#include "../color.hpp"
#include "../economy.hpp"
#include "../icons.hpp"
#include "../game/feature.hpp"
#include "../graph/stop.hpp"
#include "../graph/transit.hpp"
#include "../selection.hpp"
#include "../string_proxy.hpp"
#include "../time.hpp"
#include "../tutorial.hpp"
#include "../tools/road.hpp"

#include "spdlog/spdlog.h"

static ScrollState linesScroll;
static ScrollState stopsScroll;
static ScrollState systemsScroll;
static TextBoxState transitNameText;
static item lastHoveredCursor = -1;
static item lastSelectedCursor = -1;
static bool isEditingTransitBrand = false;

const float tpHeight = 18.5;
const float tpWidth = 15;
const float tpPad = 0.25;

bool selectStop(Part* part, InputEvent event) {
  setSelection(SelectionStop, part->itemData);
  return true;
}

bool setTicketPrice(Part* part, InputEvent event) {
  if (getCurrentTransitSystem() == 0) return true;
  float result = part->vecData.x;
  result = int(result*50)/50.f;
  result *= getInflationMultiplier();
  TransitSystem* system = getTransitSystem(getCurrentTransitSystem());
  if (part->itemData == 0) {
    system->ticketPrice = result;
  } else {
    system->transferPrice = result;
  }
  reportTutorialUpdate(ChangedTransitPrice);
  return true;
}

bool setLineHeadway(Part* part, InputEvent event) {
  TransitLine* line = getTransitLine(getCurrentLine());
  float result = part->vecData.x;
  result = (result*59+1)/60.f/24.f;
  line->headway[part->itemData] = result;
  reportTutorialUpdate(ChangedHeadway);
  return true;
}

bool toggleLineEnabled(Part* part, InputEvent event) {
  TransitLine* line = getTransitLine(getCurrentLine());
  if (line->flags & _transitEnabled) {
    line->flags &= ~_transitEnabled;
  } else {
    line->flags |= _transitEnabled;
  }
  paintTransit();
  return true;
}

bool setTransitLineColor(Part* part, InputEvent event) {
  setTransitLineColor(getCurrentLine(), part->itemData);
  return true;
}

bool setCurrentTransitSystem(Part* part, InputEvent event) {
  setCurrentTransitSystem(part->itemData);
  setLeftPanel(TransitPanel);
  return true;
}

bool setCurrentLine(Part* part, InputEvent event) {
  setCurrentLine(part->itemData);
  return true;
}

bool removeLine(Part* part, InputEvent event) {
  removeTransitLine(part->itemData);
  return true;
}

bool removeStop(Part* part, InputEvent event) {
  removeStopFromLineByIndex(getCurrentLine(), part->itemData);
  return true;
}

bool focusStop(Part* part, InputEvent event) {
  if (getCurrentLine() == 0) return true;

  TransitLine* tl = getTransitLine(getCurrentLine());
  item stopNdx = tl->legs[part->itemData].stop;
  setSelection(SelectionStop, stopNdx);
  setLeftPanel(TransitPanel);
  //Stop* stop = getStop(stopNdx);
  //setCameraTarget(stop->location);

  return true;
}

bool hoverCursor(Part* part, InputEvent event) {
  lastHoveredCursor = part->itemData;
  return true;
}

bool setCursor(Part* part, InputEvent event) {
  setTransitCursor(part->itemData);
  return true;
}

bool closeTransitPanel(Part* part, InputEvent event) {
  setLeftPanel(NoPanel);
  return true;
}

bool newTransitSystem(Part* part, InputEvent event) {
  setCurrentTransitSystem(addTransitSystem());
  setLeftPanel(TransitPanel);
  return true;
}

bool removeTransitSystem(Part* part, InputEvent event) {
  removeTransitSystem(getCurrentTransitSystem());
  return true;
}

bool toggleEditingTransitBrand(Part* part, InputEvent event) {
  isEditingTransitBrand = !isEditingTransitBrand;
  return true;
}

Part* transitPanel() {
  item currentLine = getCurrentLine();
  item currentSystem = getCurrentTransitSystem();

  if (currentSystem != 0 &&
      (!(getTransitSystem(currentSystem)->flags & _transitComplete) ||
       isEditingTransitBrand)) {
    return transitDesignerPanel();
  }

  vec2 tpLoc = vec2(0.1, 1.5);
  vec2 tpSize = vec2(tpWidth+tpPad*2, tpHeight+tpPad*2);
  Part* result = panel(tpLoc, tpSize);
  result->padding = tpPad;
  float y = 0;
  vec2 buttSize = vec2(tpWidth-3.,0.75);
  float slideWidth = (tpWidth-1)*.5f;

  r(result, button(vec2(tpWidth-1,0), iconX, vec2(1,1), closeTransitPanel, 0));

  if (currentSystem == 0) {
    r(result, icon(vec2(0,y), vec2(1,1), iconBus));
    r(result, label(vec2(1,y), 1, strdup_s("Transit")));
    y += 1;
    r(result, hr(vec2(0,y), tpWidth));
    y += 0.25;

    if (isFeatureEnabled(FTransitSystems)) {
      r(result, button(vec2(0,y), iconPlus, buttSize,
        strdup_s("New Transit System"), newTransitSystem, 0));
    }

    // Space for "All Transit Systems" button
    y += 1.25;

    // Space for enabled button on line view
    y += 1;

    // Space for headway label
    y += 0.75;

    // Space for price/headway sliders
    y += 2;

    float scrollBoxHeight = tpHeight - y;
    Part* sbox = scrollbox(vec2(0,0), vec2(tpWidth-1, scrollBoxHeight));

    float ys = 0;
    for (int i=1; i <= sizeTransitSystems(); i++) {
      TransitSystem* system = getTransitSystem(i);
      if (!(system->flags & _transitExists)) continue;

      r(sbox, button(vec2(0,ys), iconTransitLogos[system->logo], buttSize,
        strdup_s(system->name), setCurrentTransitSystem, i));
      vec3 xColor = getColorInPalette(system->color[0]);
      r(sbox, gradientBlock(vec2(tpWidth-1.5,ys+.125),
            vec2(.5,.5), xColor, xColor));
      xColor = getColorInPalette(system->color[1]);
      r(sbox, gradientBlock(vec2(tpWidth-1.0,ys+.125),
            vec2(.5,.5), xColor, xColor));
      //r(sbox, button(vec2(tpWidth-1.5,ys), iconTrash, vec2(1,1),
            //removeLine, i));
      ys ++;
    }

    r(result, scrollboxFrame(vec2(0,y),
          vec2(tpWidth, scrollBoxHeight), &systemsScroll, sbox));

  } else if (currentLine == 0) {
    TransitSystem* system = getTransitSystem(currentSystem);
    r(result, icon(vec2(0,y), vec2(1,1), iconTransitLogos[system->logo]));
    r(result, label(vec2(1,y), 1, strdup_s(system->name)));
    r(result, button(vec2(tpWidth-2,y), iconPencil, vec2(1,1),
          toggleEditingTransitBrand, 0));
    y += 1;
    r(result, hr(vec2(0,y), tpWidth));
    y += 0.25;

    r(result, button(vec2(0,y), iconLeft, vec2(tpWidth,0.75),
          strdup_s("All Transit Systems"), setCurrentTransitSystem, 0));
    y += 1.25;

    //r(result, button(vec2(0,y), iconX, vec2(tpWidth,0.75),
          //strdup_s("Shutdown"), removeTransitSystem, 0));
    y += 1;

    // Space for headway label
    y += 0.75;

    for (int i = 0; i < 2; i++) {
      r(result, icon(vec2(0,y), vec2(0.75,0.75), iconCash));
      float val = i ? system->transferPrice : system->ticketPrice;
      float sliderVal = val / getInflationMultiplier();
      Part* slide = r(result, slider(vec2(0.75,y), vec2(slideWidth, 0.75),
            sliderVal, setTicketPrice));
      slide->itemData = i;
      r(result, label(vec2(slideWidth+1,y), 0.75,
            sprintf_o("$%2.2f %s", val, i ? "Transfer" : "Ticket")));
      y += 1;
    }

    float scrollBoxHeight = tpHeight - y;
    Part* sbox = scrollbox(vec2(0,0), vec2(tpWidth-1, scrollBoxHeight));

    float ys = 0;
    for (int i=1; i <= sizeTransitLines(); i++) {
      TransitLine* line = getTransitLine(i);
      if (!(line->flags & _transitExists)) continue;
      if (line->system != currentSystem) continue;
      r(sbox, button(vec2(0,ys), iconTransitLogos[system->logo], buttSize,
        strdup_s(line->name), setCurrentLine, i));
      vec3 xColor = getTransitLineColorInPalette(i);
      r(sbox, gradientBlock(vec2(tpWidth-3.625,ys+.125),
            vec2(.5,.5), xColor, xColor));
      r(sbox, button(vec2(tpWidth-1.5,ys), iconTrash, vec2(1,1),
            removeLine, i));
      ys ++;
    }

    r(result, scrollboxFrame(vec2(0,y),
          vec2(tpWidth, scrollBoxHeight), &linesScroll, sbox));

  } else {
    TransitSystem* system = getTransitSystem(currentSystem);
    TransitLine* line = getTransitLine(currentLine);
    transitNameText.text = &line->name;

    r(result, icon(vec2(0,y), vec2(1,1), iconTransitLogos[system->logo]));
    r(result, textBoxLabel(vec2(1,y), vec2(tpWidth-2,1), &transitNameText));
    y += 1;
    r(result, hr(vec2(0,y), tpWidth));
    y += 0.25;

    // Color Picker
    float colorPickerWidth = 4;
    r(result, colorPicker(vec2(tpWidth - colorPickerWidth, y),
          colorPickerWidth, line->color, setTransitLineColor, 0));
    vec3 xColor = getColorInPalette(system->color[0]);

    // Color Picker Default Button
    Part* defaultButt = r(result, button(vec2(tpWidth-colorPickerWidth,y+2),
          iconNull, vec2(colorPickerWidth,0.7),
          strdup_s("Use Default"), setTransitLineColor, 255));
    defaultButt->flags |= _partIsPanel;
    if (line->color == 255) {
      defaultButt->flags |= _partHighlight;
    }
    r(defaultButt, gradientBlock(vec2(0,0), vec2(0.5, 0.5), xColor, xColor));

    // Color Picker Background
    //Part* background = r(result,
        //gradientBlock(vec2(tpWidth-colorPickerWidth, y),
          //vec2(colorPickerWidth, 2.7), colorDarkGray, colorDarkGray));
    //background->dim.start.z -= 0.1;

    r(result, button(vec2(0,y), iconLeft, vec2(tpWidth-colorPickerWidth-1,0.75),
          strdup_s(system->name), setCurrentLine, 0));
    y += 1.25;

    r(result, button(vec2(0,y),
          line->flags & _transitEnabled ? iconCheck : iconNull,
          vec2(tpWidth-colorPickerWidth-1, 0.75),
          strdup_s("Enabled"), toggleLineEnabled, 0));
    y += 1;

    r(result, label(vec2(0,y), 0.75, strdup_s("Headway")));
    y += 0.75;
    for (int i = 0; i < 2; i++) {
      r(result, icon(vec2(0,y), vec2(0.75,0.75),
            i ? iconWeatherMoon : iconWeatherSun));
      float minutes = line->headway[i]*60*24;
      float val = (minutes-1)/59.f;
      Part* slide = r(result, slider(vec2(0.75,y), vec2(slideWidth, 0.75),
            val, setLineHeadway));
      slide->itemData = i;
      r(result, label(vec2(slideWidth+1,y), 0.75,
            sprintf_o("%d Minutes", int(minutes))));
      y += 1;
    }

    float scrollBoxHeight = tpHeight - y;
    Part* sbox = scrollbox(vec2(0,0), vec2(tpWidth-1, scrollBoxHeight));
    float sy = 0;
    float time = 0;
    int tlS = line->legs.size();
    float buttX = 2.25;
    buttSize.x -= buttX;
    float cursorWidth = buttSize.x+2;
    item cursor = getTransitCursor();
    if (getLastLegAdded() >= 0 && cursor > 0) cursor --;

    for (int i=0; i <= tlS; i++) {
      Part* cursorHandle = r(sbox, block(vec2(buttX,sy),
            vec2(cursorWidth,0.2)));
      cursorHandle->onHover = hoverCursor;
      cursorHandle->onClick = setCursor;
      cursorHandle->itemData = i;

      if (i == cursor || i == lastHoveredCursor) {
        r(sbox, block(vec2(buttX,sy-0.1), vec2(0.1,0.4)));
        r(sbox, block(vec2(buttX+cursorWidth-0.1,sy-0.1), vec2(0.1,0.4)));
      } else {
        cursorHandle->renderMode = RenderTransparent;
      }

      if (i == getTransitCursor() && getTransitCursor() != lastSelectedCursor) {
        stopsScroll.target = sy-scrollBoxHeight*.5f;
      }
      sy += 0.2;

      if (i == tlS) break;

      TransitLeg leg = line->legs[i];
      item stopNdx = leg.stop;
      Stop* stop = getStop(stopNdx);
      Part* butt = button(vec2(buttX,sy), iconBusSign, buttSize,
        strdup_s(stop->name), selectStop, stopNdx);
      r(sbox, butt);
      if (i == getLastLegAdded()) {
        butt->flags |= _partHighlight;
      }

      r(sbox, button(vec2(tpWidth-2.75,sy), iconEye,
            vec2(0.75,0.75), focusStop, i));

      r(sbox, button(vec2(tpWidth-1.75,sy), iconTrash,
            vec2(0.75,0.75), removeStop, i));

      if (getSelectionType() == SelectionStop && stopNdx == getSelection()) {
        Part* arrow = r(sbox, icon(vec2(tpWidth-3.75,sy),
              vec2(0.75,0.75), iconLeft));
        arrow->foregroundColor = PickerPalette::RedLight;
      }

      if (i < line->legs.size()-1) {
        time += leg.timeEstimate;
        char* dur = printDurationString(time);
        Part* timeLabel = r(sbox,
            label(vec2(-0.1,sy+.45), 0.75, sprintf_o("+%s", dur)));
        timeLabel->dim.end.x = 3;
        timeLabel->itemData = i+1;
        timeLabel->onHover = hoverCursor;
        timeLabel->onClick = setCursor;
        free(dur);
      }

      sy += 0.75;
    }

    lastHoveredCursor = -1;
    lastSelectedCursor = cursor;

    r(result, scrollboxFrame(vec2(0,y),
          vec2(tpWidth, scrollBoxHeight), &stopsScroll, sbox));
  }

  return result;
}

