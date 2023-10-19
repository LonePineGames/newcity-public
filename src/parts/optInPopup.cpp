#include "optInPopup.hpp"

#include "../icons.hpp"
#include "../option.hpp"
#include "../string_proxy.hpp"

#include "button.hpp"
#include "hr.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "root.hpp"

bool setLogUpload(Part* part, InputEvent event) {
  setLogUpload(part->itemData);
  writeOptions();
  setSeenLogUploadMessage();
  return true;
}

Part* optInPopup(float aspectRatio) {
  float tutPad = 0.25;
  float tutWidth = 16;
  float tutHeight = 7;
  vec2 tutPanelSizePadded =
    vec2(tutWidth+tutPad*2, tutHeight+tutPad*2);
  float uiX = uiGridSizeX * aspectRatio;

  Part* result = panel(
      vec2(uiX,uiGridSizeY)*0.5f - tutPanelSizePadded*0.5f,
      tutPanelSizePadded);
  result->padding = tutPad;
  float halfWidth = (tutWidth - tutPad)*.5f;
  float y = 0;
  float scale = 1;

  r(result, multiline(vec2(0,y), vec2(tutWidth, 1),
        strdup_s("NewCity occasionally uploads log files to a server. "
          "This information helps improve the game."), &y));

  Part* noTutButt = button(vec2(0,y), iconCheck,
    vec2(tutWidth, 1),
    strdup_s("Great, I want to help!"), setLogUpload, 1);
  r(result, noTutButt);
  y += 1.5;

  Part* tutButt = button(vec2(0,y), iconNull,
    vec2(tutWidth, 1),
    strdup_s("I do not consent."), setLogUpload, 0);
  r(result, tutButt);
  y += 1.5;

  r(result, hr(vec2(0, y), tutWidth));
  y += .25;
  r(result, labelCenter(vec2(0, y), vec2(tutWidth, .75f),
        strdup_s("You can change your answer in the options menu.")));
  y += .75;
  r(result, labelCenter(vec2(0, y), vec2(tutWidth, .75f),
        strdup_s("See EULA for details.")));
  y += 1;

  result->dim.start.y = (uiGridSizeY-y-tutPad)*.5f;
  result->dim.end.y = y + tutPad*2;
  return result;
}

