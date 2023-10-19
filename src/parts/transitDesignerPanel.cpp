#include "transitDesignerPanel.hpp"

#include "block.hpp"
#include "button.hpp"
#include "colorPicker.hpp"
#include "hr.hpp"
#include "icon.hpp"
#include "label.hpp"
#include "leftPanel.hpp"
#include "loader.hpp"
#include "panel.hpp"
#include "scrollbox.hpp"
#include "slider.hpp"
#include "textBox.hpp"
#include "transitPanel.hpp"

#include "../color.hpp"
#include "../icons.hpp"
#include "../graph/config.hpp"
#include "../graph/stop.hpp"
#include "../graph/transit.hpp"
#include "../graph/transitText.hpp"
#include "../selection.hpp"
#include "../time.hpp"
#include "../string_proxy.hpp"
#include "../tools/road.hpp"

static ScrollState transitScroll;
static TextBoxState systemNameText;
const float tpHeight = 18;
const float tpWidth = 15;
const float tpPad = 0.25;

bool closeTransitDesignerPanel(Part* part, InputEvent event) {
  setLeftPanel(NoPanel);
  return true;
}

bool setTransitSystemType(Part* part, InputEvent event) {
  addTransitSystemFeature(getCurrentTransitSystem(), part->itemData);
  TransitSystem* system = getTransitSystem(getCurrentTransitSystem());
  system->type = part->itemData;
  free(system->name);
  system->name = strdup_s(getGraphFeature(system->type)->name);
  transitScroll.target = 0;
  return true;
}

bool setTransitSystemPower(Part* part, InputEvent event) {
  addTransitSystemFeature(getCurrentTransitSystem(), part->itemData);
  TransitSystem* system = getTransitSystem(getCurrentTransitSystem());
  system->power = part->itemData;
  transitScroll.target = 0;
  return true;
}

bool setTransitSystemAutomation(Part* part, InputEvent event) {
  addTransitSystemFeature(getCurrentTransitSystem(), part->itemData);
  TransitSystem* system = getTransitSystem(getCurrentTransitSystem());
  system->automation = part->itemData;
  transitScroll.target = 0;
  focusTextBox(&systemNameText);
  return true;
}

bool setTransitSystemLogo(Part* part, InputEvent event) {
  TransitSystem* system = getTransitSystem(getCurrentTransitSystem());
  system->logo = part->itemData;
  return true;
}

bool setTransitSystemColor(Part* part, InputEvent event) {
  int k = part->vecData.x;
  setTransitSystemColor(getCurrentTransitSystem(), k, part->itemData);
  return true;
}

bool callForTransitBids(Part* part, InputEvent event) {
  callForTransitBids(getCurrentTransitSystem());
  return true;
}

bool selectTransitBid(Part* part, InputEvent event) {
  selectTransitBid(getCurrentTransitSystem(), part->itemData);
  return true;
}

bool acceptTransitDesign(Part* part, InputEvent event) {
  acceptTransitDesign(getCurrentTransitSystem());
  setLeftPanel(TransitPanel);
  return true;
}

bool rejectTransitDesign(Part* part, InputEvent event) {
  removeTransitSystem(getCurrentTransitSystem());
  return true;
}

Part* transitDesignerPanel() {
  TransitSystem* system = getTransitSystem(getCurrentTransitSystem());

  vec2 tpLoc = vec2(0.1, 2-tpPad);
  vec2 tpSize = vec2(tpWidth+tpPad*2, tpHeight+tpPad*2);
  Part* result = panel(tpLoc, tpSize);
  result->padding = tpPad;
  float y = 0;
  vec2 buttSize = vec2(tpWidth-3.,0.75);
  vec2 selectButtSize = vec2(tpWidth-3.,2);
  float slideWidth = (tpWidth-1)*.5f;
  bool complete = system->flags & _transitComplete;

  r(result, icon(vec2(0,0), vec2(1,1), iconPlus));
  r(result, label(vec2(1,0), 1.f, strdup_s("Designing Transit System")));
  r(result, button(vec2(tpWidth-1,0), iconX, vec2(1,1),
        closeTransitDesignerPanel, 0));
  y += 1.5;

  InputCallback callback;
  InputCallback goBack;
  bool skip = false;
  vector<item> features;

  if (system->type == 0) {
    features = getGraphFeatures(TractionFeature);
    callback = setTransitSystemType;
    goBack = 0;

  } else if (system->power == 0) {
    features = getGraphFeatures(PowerFeature);
    callback = setTransitSystemPower;
    goBack = setTransitSystemType;

  } else if (system->automation == 0) {
    features = getGraphFeatures(AutomationFeature);
    callback = setTransitSystemAutomation;
    goBack = setTransitSystemPower;

  } else {
    skip = true;
  }

  if (!skip) {
    Part* innerPanel = scrollbox(vec2(0,0), vec2(tpWidth,tpHeight-y));
    //Part* innerPanel = r(result, panel(vec2(0,y), vec2(tpWidth, tpHeight-y)));
    innerPanel->flags |= _partLowered;
    innerPanel->padding = tpPad;
    float iy = 0;
    float selectionWidth = tpWidth - tpPad*2-.5f;

    if (goBack == 0) {
      r(innerPanel, button(vec2(0,iy), iconLeft, vec2(tpWidth-tpPad*2,0.85),
            strdup_s("All Transit Systems"), rejectTransitDesign, 0));
      iy += 1.25;

    } else {
      r(innerPanel, button(vec2(0,iy), iconLeft, vec2(tpWidth-tpPad*2, 0.85f),
            strdup_s("Back"), goBack, 0));
      iy += 0.85f+tpPad;
    }

    for (int i = 0; i < features.size(); i++) {
      GraphFeature* feature = getGraphFeature(features[i]);

      Part* selection = r(innerPanel, panel(vec2(0,iy),
            vec2(selectionWidth, 1)));
      selection->onClick = callback;
      selection->itemData = features[i];
      selection->flags |= _partHover;

      r(selection, icon(vec2(0,0), feature->icon));
      r(selection, label(vec2(1,0), 0.85f, strdup_s(feature->name)));

      float ty = 0;
      if (feature->text != 0) {
        r(selection, multiline(vec2(tpPad,0.85f),
              vec2(selectionWidth-tpPad*2, 0.75f),
              strdup_s(feature->text), &ty));
      }

      ty += 0.85f;
      selection->dim.end.y = ty;
      iy += ty + tpPad;
    }

    r(result, scrollboxFrame(vec2(0,y), vec2(tpWidth, tpHeight-y),
          &transitScroll, innerPanel));

  } else if (complete || !(system->flags & _transitBidding)) {
    if (!complete) {
      r(result, button(vec2(0,y), iconLeft, vec2(tpWidth, 0.85f),
            strdup_s("Back"), setTransitSystemAutomation, 0));
      y += 0.85f+tpPad;
    }

    // Name Text Box
    systemNameText.text = &system->name;
    //focusTextBox(&systemNameText);
    r(result, label(vec2(0,y), 0.75, strdup_s("System Name")));
    y += 0.75;
    Part* nameTB = r(result,
        textBoxLabel(vec2(0,y), vec2(tpWidth, 1.25), &systemNameText));
    //if (complete) {
      //nameTB->onClick = toggleEditingTransitBrand;
      //nameTB->onCustom = toggleEditingTransitBrand;
    //}
    y += 1.5f;

    r(result, labelRight(vec2(0,y), vec2(6,0.75), strdup_s("Primary Color")));
    r(result, label(vec2(9,y), 0.75, strdup_s("Secondary Color")));

    Part* background = r(result, gradientBlock(vec2(6.25, y+.25),
          vec2(2.5, 3.5), colorDarkGray, colorDarkGray));
    background->dim.start.z -= 0.1;
    vec3 color = getColorInPalette(system->color[0]);
    r(result, gradientBlock(vec2(6.5, y+.5), vec2(2, 1.5),
          color, color));
    color = getColorInPalette(system->color[1]);
    r(result, gradientBlock(vec2(6.5, y + 2), vec2(2, 1.5),
          color, color));

    y += 1;


    for (int j = 0; j < 2; j++) {
      r(result, colorPicker(vec2(j*9, y), 6, system->color[j],
            setTransitSystemColor, j));

      /*
      for (int i = 0; i < numColors; i++) {
        float lx = j*9 + 0.75*(i/4);
        float ly = y + 0.75*(i%4);

        color = getColorInPalette(i);
        Part* colorButt = r(result, panel(vec2(lx,ly), vec2(0.75,0.75)));
        colorButt->renderMode = RenderTransparent;
        r(colorButt, gradientBlock(vec2(0.125,0.125),
              vec2(0.5,0.5), color, color));
        colorButt->onClick = setTransitSystemColor;
        colorButt->itemData = i;
        colorButt->vecData.x = j;
        item current = system->color[j];
        if (i == current) {
          colorButt->flags |= _partHighlight;
        }
      }
      */
    }

    y += 3;

    r(result, label(vec2(0,y), 0.75, strdup_s("Logo")));
    y += 0.75;

    const int numPerRow = 13;
    for (int i = 0; i < numTransitLogos; i++) {
      float lx = (i % numPerRow)*1.1f;
      float ly = (i / numPerRow) + y;
      Part* logoButt = r(result, button(vec2(lx, ly),
            iconTransitLogos[i], setTransitSystemLogo, i));
      if (i == system->logo) {
        logoButt->flags |= _partHighlight;
      }
    }

    y += ceil(float(numTransitLogos)/numPerRow);

    if (complete) {
      r(result, button(vec2(0,tpHeight-1), iconCheck, vec2(tpWidth, 1.f),
            strdup_s("Done"), toggleEditingTransitBrand, 0));

    } else {
      r(result, button(vec2(0,tpHeight-1), iconCheck, vec2(tpWidth, 1.f),
            strdup_s("Call for Bids"), callForTransitBids, 0));
    }

  } else if (!(system->flags & _transitDesigning)) {
    r(result, label(vec2(0,y), 1.f, strdup_s("Bids")));
    y += 1;

    Part* innerPanel = r(result, panel(vec2(0,y), vec2(tpWidth, tpHeight-y)));
    innerPanel->flags |= _partLowered;
    innerPanel->padding = tpPad;
    float iy = 0;
    float selectionWidth = tpWidth - tpPad*2;

    for (int i = 0; i < system->bids.size(); i++) {
      TransitSystemBid bid = system->bids[i];
      Part* selection = r(innerPanel, panel(vec2(0,iy),
            vec2(selectionWidth, 1)));
      selection->onClick = selectTransitBid;
      selection->itemData = i;
      selection->flags |= _partHover;

      r(selection, label(vec2(0,0), 0.85f, strdup_s(bid.firmName)));

      float ty = 0;
      //r(selection, multiline(vec2(tpPad,0.85f),
            //vec2(selectionWidth-tpPad*2, 0.75f), strdup_s(desc[i]), &ty));
      selection->dim.end.y = ty + 0.85f;
      iy += selection->dim.end.y + tpPad;
    }

    if (system->bids.size() < c(CNumTransitBids)) {
      r(innerPanel, label(vec2(0,iy), 0.85f, strdup_s("Collecting Bids...")));
      iy += 0.85f;
      r(innerPanel, loaderBar(vec2(0,iy), vec2(selectionWidth, 1), iconDocument));
    }

  } else if (!(system->flags & _transitDesigned)) {
    r(result, label(vec2(0,y), 1.f, strdup_s("Designing...")));
    y += 1.f;
    r(result, loaderBar(vec2(0,y), vec2(tpWidth, 1), iconDocument));
    y += 1.25f;
    char* dateStr = printDateString(system->designDate);
    r(result, label(vec2(0,y), 0.85f,
          sprintf_o("Estimated Completion: %s", dateStr)));
    free(dateStr);

  } else { //if (!(system->flags & _transitComplete)) {
    r(result, label(vec2(0,y), 0.85f, strdup_s("Design Complete")));

    r(result, button(vec2(0,tpHeight-2), iconCheck, vec2(tpWidth, 1.f),
          sprintf_o("Accept Design"), acceptTransitDesign, 0));
    r(result, button(vec2(0,tpHeight-1), iconX, vec2(tpWidth, 1.f),
          strdup_s("Reject Design"), rejectTransitDesign, 0));
  }

  return result;
}

