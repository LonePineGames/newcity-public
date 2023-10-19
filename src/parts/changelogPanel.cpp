#include "changelogPanel.hpp"

#include "../icons.hpp"
#include "../option.hpp"
#include "../string_proxy.hpp"

#include "button.hpp"
#include "hr.hpp"
#include "icon.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "scrollbox.hpp"

// Constant size values for Changelog
const float clHeight = 16.1f; //9.44f; // 24.472 //16.1f;
const float clWidth = 14.2f;
const float clPadding = 0.25f;
const float clTxtTitleSize = 1.0f;
const float clTxtSize = 0.85f;
const float clSubTxtSize = 0.75f;
const float clHrOffset = 1.25f;
const float clEleSpace = 0.5f;
const float clStartX = 0;
const float clStartY = 1.5f;

static ScrollState scrollState;
static int openVersion = 0;

bool toggleChangelog(Part* part, InputEvent event) {
  setChangelogClosed(!isChangelogClosed());
  return true;
}

bool changeOpenVersion(Part* part, InputEvent event) {
  int newVal = part->itemData;
  if(newVal == openVersion) {
    openVersion = -1;
  } else {
    openVersion = newVal;
  }

  scrollState.target = 0;

  return true;
}

Part* changelogPanel() {
  float y = 0.0f;
  bool closed = isChangelogClosed();
  float panelHeight = closed ? clTxtTitleSize + (clPadding*2) : clHeight;
  float clInnerWidth = clWidth - clPadding*2;
  Part* result = panel(vec2(clStartX, clStartY), vec2(clWidth, panelHeight));
  result->padding = clPadding;

  // Title (Always show, even when minimized)
  r(result, button(vec2(0, y), vec2(clInnerWidth,clTxtTitleSize),
    strdup_s("Changelog"), toggleChangelog));
  r(result, icon(vec2(clInnerWidth-1, y), closed ? iconPlus : iconMinus));
  y += 1;

  if(closed) {
    return result;
  }

  // Horizontal rule
  //y += clHrOffset;
  //r(result, hr(vec2(clPadding, y), clWidth - (clPadding * 4)));
  //y += clEleSpace;
  y += clPadding;

  // Scrollbox and changelog content
  float scrlY = 0.0f;
  float clPanelWidth = clInnerWidth-2;
  Part* clScrollbox = scrollbox(vec2(0, 0),
    vec2(clInnerWidth, clHeight - y));

  for(int i = 0; i < *(&changelogs+1)-changelogs; i++) {
    float panelY = 0;
    Part* clPanel = panel(vec2(0, scrlY), vec2(clPanelWidth, 1));
    clPanel->renderMode = RenderTransparent;
    Part* clVersionButt = button(vec2(0, panelY),
      i == openVersion ? iconMinus : iconPlus,
      vec2(clPanelWidth, clTxtSize),
      strdup_s(changelogs[i].version), changeOpenVersion, i);
    //clVersionButt->padding = clPadding;
    r(clPanel, clVersionButt);
    panelY ++;

    if(i == openVersion) { // Render full text
      float multiY = 0.0f;
      r(clPanel, multiline(vec2(clTxtSize, clTxtSize+clPadding),
        vec2(clPanelWidth, clSubTxtSize),
        strdup_s(changelogs[i].text), &multiY));
      panelY += multiY;
    }

    clPanel->dim.end.y = panelY;
    r(clScrollbox, clPanel);
    scrlY += panelY;
  }

  Part* clScrollboxFrame = scrollboxFrame(vec2(0, y), 
    vec2(clWidth - (clPadding * 2), clHeight - (y + (clPadding * 2))), 
    &scrollState, clScrollbox);

  r(result, clScrollboxFrame);
  return result;
}
