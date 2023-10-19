#include "tutorialPopup.hpp"

#include "../icons.hpp"
#include "../option.hpp"
#include "../string_proxy.hpp"

#include "button.hpp"
#include "hr.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "root.hpp"

bool setTutorialMode(Part* part, InputEvent event) {
  setHasCompletedTutorial(part->itemData);
  writeOptions();
  return true;
}

Part* tutorialPopup(float aspectRatio) {
  float tutPad = 0.25;
  float tutWidth = 16;
  float tutHeight = 5;
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

  Part* noTutButt = button(vec2(0,y), iconCheck,
    vec2(tutWidth, 1),
    strdup_s("I am familiar with city games."), setTutorialMode, 0);
  r(result, noTutButt);
  y += 1.5;

  Part* tutButt = button(vec2(0,y), iconCheck,
    vec2(tutWidth, 1),
    strdup_s("I haven't played city games before."), setTutorialMode, 1);
  r(result, tutButt);
  y += 1.5;

  r(result, hr(vec2(0, y), tutWidth));
  y += .25;
  r(result, labelCenter(vec2(0, y), vec2(tutWidth, .75f),
        strdup_s("You can change your answer in the options menu.")));
  y += 1;

  result->dim.start.y = (uiGridSizeY-y-tutPad)*.5f;
  result->dim.end.y = y + tutPad*2;
  return result;
}

