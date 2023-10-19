//-----------------------------------------------------------------------------
// tutorialPanel - The left-panel popup for the tutorial
// (c) 2021 Lone Pine Games, LLC.
//-----------------------------------------------------------------------------

#include "tutorialPanel.hpp"

#include "article.hpp"
#include "citipediaPanel.hpp"
#include "icon.hpp"
#include "leftPanel.hpp"
#include "messageBoard.hpp"
#include "root.hpp"
#include "scrollbox.hpp"
#include "span.hpp"

#include "../color.hpp"
#include "../draw/camera.hpp"
#include "../icons.hpp"
#include "../option.hpp"
#include "../selection.hpp"
#include "../string.hpp"
#include "../time.hpp"

const float tpPadding = 0.25f;
const vec2 tutorialPanelLoc = vec2(0.1, 1.5);
static vec2 lastTutorialPanelLoc = tutorialPanelLoc;
const float tpWidth = 13.0f;
const float tpHeight = 18.5f;
const float tpTitleY = 2.0f;
const float tpTextY = 0.75f;
const vec2 tutorialPanelSize = vec2(tpWidth + tpPadding*2,
  tpHeight + tpPadding*2);
const float buttonWidth = 3.0f;
const float buttonHeight = 1.0f;
const float buttonStartX = (tpWidth/2.0f)-(buttonWidth/2.0f);
char* lastArticle = 0;
bool showTutorial = true;

// static std::string tpStrCache[tutorialMaxStrings];
static std::string tpStatusStr = "";
static ScrollState tutorialScroll;

bool isTutorialPanelOpen() {
  return showTutorial;
}

void setTutorialPanelOpen(bool open) {
  showTutorial = open;
}

bool closeTutorialPanel(Part* part, InputEvent event) {
  setTutorialPanelOpen(false);
  return true;
}

bool tutorialBack(Part* part, InputEvent event) {
  getTutorialStatePtr()->tutorialBack();
  return true;
}

bool selectTutorialMenu(Part* part, InputEvent event) {
  selectTutorial(TutorialCategory::TutorialMenu00);
  return true;
}

Part* tutorialArticle(vec2 loc, vec2 size, char** title) {
  Part* scroll = scrollbox(vec2(0,0), size);

  ArticleRenderConfig config;
  config.flags = 0;
  config.textColor = PickerPalette::White;
  config.captionTextColor = PickerPalette::White;
  config.linkTextColor = PickerPalette::CyanLight;
  config.captionBackground = line(colorDarkGrad0, colorDarkGrad1);
  config.linkHandler = followLink;
  config.chartDate = getCurrentDateTime();

  char* currentArticle = strdup_s(getTutorialArticleForCurrentState().c_str());

  r(scroll, article(vec2(0,0), vec2(size.x-1, tpTextY),
        &currentArticle, title, config));

  if (streql(currentArticle, lastArticle)) {
    free(currentArticle);
  } else {
    if (lastArticle != 0) free(lastArticle);
    lastArticle = currentArticle;
    resetScrollState(&tutorialScroll);
  }

  Part* scrollFrame = scrollboxFrame(loc,
      size,
      &tutorialScroll, scroll);

  return scrollFrame;
}

Part* tutorialPanel() {
  vec2 locAdj = tutorialPanelLoc;
  if ((getLeftPanel() != NoPanel || getGameMode() == ModeBuildingDesigner || getGameMode() == ModeDesignOrganizer || getSelectionType() != NoSelection) && getMenuMode() == HiddenMenu) {
    locAdj.x = getAspectRatio()*uiGridSizeX - messageWidthOuter
      - tutorialPanelSize.x - 1;
  }

  locAdj = .5f*(locAdj + lastTutorialPanelLoc);
  lastTutorialPanelLoc = locAdj;

  TutorialState* ptr = getTutorialStatePtr();

  Part* result = panel(locAdj, tutorialPanelSize);
  result->padding = tpPadding;

  float yVal = 0.0f;
  yVal += tpTitleY + tpPadding; // for title

  // Horizontal rule
  //r(result, hr(vec2(tpPadding, yVal), tpWidth-(tpPadding*2.0f)));
  //yVal += tpTitleY;

  // Handle tutorial text
  float yFloat = 0.0f;

  char* title = 0;
  vec2 innerSize = tutorialPanelSize - vec2(tpPadding*2, tpPadding*2+yVal);
  if (ptr->category() != TutorialMenu00) {
    innerSize.y -= tpPadding+buttonHeight*2;
  }
  r(result, tutorialArticle(vec2(0.f, yVal), innerSize, &title));

  float widthForTitle = tpWidth;
  float titleWidth = stringWidth(title);
  float titleSpacing = widthForTitle/titleWidth;
  titleSpacing = clamp(titleSpacing, tpTitleY*.5f, tpTitleY);
  //if (titleSpacing > tpTitleY) titleSpacing = tpTitleY;
  //if (titleSpacing < tpTitleY*.5f) titleSpacing = tpTitleY*.5f;
  //Part* titlePart = r(result, labelCenter(vec2(1,0),
        //vec2(widthForTitle, titleSpacing),
        //title == 0 ? strdup_s("Tutorial") : title));

  vec2 titleEnd(0,0);
  Part* titlePart = r(result, spanCenter(vec2(1,0), 1,
      vec2(tpWidth-1, titleSpacing),
      title == 0 ? strdup_s("Tutorial") : title, &titleEnd));

  r(result, icon(vec2(0,0), iconQuestion));

  Part* tutExit = button(vec2(tpWidth-1.0f, 0.0f), iconX, closeTutorialPanel);
  r(result, tutExit);

  if (ptr == 0 || !ptr->tutorialActive()) {
    // Show error
    std::string error = "";

    if (ptr == 0) {
      error = "Invalid TutorialState pointer";
    } else if (!ptr->tutorialActive()) {
      error = "Tutorial has been disabled";
    } else {
      error = "Unknown Tutorial error";
    }

    Part* tutError = labelCenter(vec2(tpPadding, tutorialPanelSize.y-buttonHeight-(tpPadding*2.0f)),
      vec2(tpWidth-(tpPadding*2.0f), tpTextY),
      strdup_s(tpStatusStr.c_str()));
    tutError->foregroundColor = PickerPalette::YellowMedD;

    return result;
  }

  if (ptr->tutorialOKWait()) {
    /*
    std::string* strArr = getTutorialStringsForCurrentState();
    if (strArr != 0)
    {
      for (int i = 0; i < tutorialMaxStrings; i++) {
        tpStrCache[i] = strArr[i];
      }
    }
    */

    // Ok button
    r(result, labelCenter(vec2(tpPadding, tutorialPanelSize.y-(buttonHeight*2.0f)-(tpPadding*2.0f)),
      vec2(tpWidth-(tpPadding*2.0f), tpTextY),
      strdup_s("Click \"OK\" to continue")));

    Part* tutButt = buttonCenter(vec2(buttonStartX, tutorialPanelSize.y-buttonHeight-(tpPadding*2.0f)), vec2(buttonWidth, buttonHeight),
      strdup_s("Ok"), acknowledgeTutorialInfo);
    // tutButt->inputAction = ActTutorialOK;
    r(result, tutButt);
  } else {
    // Show status 
    tpStatusStr = ptr->waitString();
    Part* tutStatus = labelCenter(vec2(tpPadding, tutorialPanelSize.y-buttonHeight-(tpPadding*2.0f)),
      vec2(tpWidth-(tpPadding*2.0f), tpTextY),
      strdup_s(tpStatusStr.c_str()));

    // Render green if actions completed, otherwise render yellow
    if (ptr->actionsCompleted()) {
      tutStatus->foregroundColor = PickerPalette::GreenLight;
    } else {
      tutStatus->foregroundColor = PickerPalette::YellowLight;
    }

    r(result, tutStatus);
  }

  if (ptr->category() != TutorialMenu00) {
    if (ptr->category() != Welcome00) {
      Part* tutBack = r(result, button(vec2(0,1), iconLeft, tutorialBack));
    }

    Part* homeButt = r(result, button(vec2(tpWidth-1,1), iconMenu, vec2(1,1),
          selectTutorialMenu, 0));
  }


  return result;
}
