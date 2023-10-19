#include "root.hpp"

#include "button.hpp"
#include "hr.hpp"
#include "icon.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "root.hpp"
#include "span.hpp"

#include "../game/game.hpp"
#include "../icons.hpp"
#include "../string.hpp"
#include "../string_proxy.hpp"

#include <stdio.h>
#include <algorithm>

static const char* errorTitle = "Error";
static const char* errorLoadTitle = "LOADING PAUSED";
static const char* errorLoadMessage = "NewCity did not exit normally.";
static const char* errorLoadMessage2 = "Would you like to start a new game?";
static const char* errorLoadMessage3 =
  "(We'll copy the previous game to a new file,";
static const char* errorLoadMessage4 = "so you can try recovering it later.)";

bool errorLoadNewGame(Part* part, InputEvent event) {
  doErrorLoadNewGame();
  return true;
}

bool clearErrorLoad(Part* part, InputEvent event) {
  clearErrorLoad();
  return true;
}

Part* errorPanel(float aspectRatio, const char* message) {
  //float titleWidth = stringWidth(errorTitle);
  float messageWidth = stringWidth(message);
  //vec2 panelSize = vec2(std::max(20.f, std::max(titleWidth, messageWidth) + 1), 8);
  float uiX = uiGridSizeX * aspectRatio;
  vec2 panelSize = vec2(clamp(messageWidth, 20.f, uiX-4), 8.f);

  Part* container = panel(line(vec3(0,0,0),
    vec3(uiX, uiGridSizeY, 0)));
  container->renderMode = RenderShadow;
  container->onKeyDown = escapePressed;

  float sizeX = panelSize.x - 1;
  Part* errorPanel = panel(vec2(uiX,uiGridSizeY)*0.5f - panelSize*0.5f,
      panelSize);
  errorPanel->padding = 0.5;
  r(container, errorPanel);

  r(errorPanel, icon(vec2(sizeX*.5f-1, 1), vec2(2,2), iconCrash));
  r(errorPanel, labelCenter(vec2(0,0), vec2(sizeX, 1),
        strdup_s(errorTitle)));
  r(errorPanel, hr(vec2(0,1.9), (sizeX-2)*.5f));
  r(errorPanel, hr(vec2(sizeX*.5f+1,1.9), (sizeX-2)*.5f));

  //r(errorPanel, labelCenter(vec2(0,3), vec2(sizeX, 1),
        //strdup_s(message)));
  float y = 3;
  r(errorPanel, spanCenter(vec2(0,3), 0, vec2(sizeX,1), strdup_s(message), &y));
  y ++;

  //r(errorPanel, label(vec2(0,5.5), 1,
        //strdup_s("Your game was saved as \"crashAutosave\"")));
  r(errorPanel, buttonCenter(vec2(0,y),vec2(sizeX*.5f,1),
        strdup_s("Continue"), closeMenus));
  r(errorPanel, buttonCenter(vec2(sizeX*.5,y),vec2(sizeX*.5f,1),
        strdup_s("Quit Game"), quitGame));
  y +=2;

  errorPanel->dim.end.y = y;
  errorPanel->dim.start.y = (uiGridSizeY-y)*.5f;

  return container;
}

Part* errorLoadPanel(float aspectRatio) {
  float titleWidth = stringWidth(errorLoadTitle);
  float messageWidth = stringWidth(errorLoadMessage);
  vec2 panelSize = vec2(20.f, 13.25f);

  float uiX = uiGridSizeX * aspectRatio;
  Part* container = panel(line(vec3(0,0,0),
    vec3(uiX, uiGridSizeY, 0)));
  container->renderMode = RenderShadow;
  container->onKeyDown = escapePressed;

  float sizeX = panelSize.x - 1;
  Part* errorPanel = panel(vec2(uiX,uiGridSizeY)*0.5f - panelSize*0.5f,
      panelSize);
  errorPanel->padding = 0.5;
  r(container, errorPanel);

  r(errorPanel, icon(vec2(sizeX/2-1, 0), vec2(2,2), iconCrash));
  r(errorPanel, labelCenter(vec2(0,2), vec2(sizeX, 1),
        strdup_s(errorLoadTitle)));
  r(errorPanel, hr(vec2(0,3.2), panelSize.x - 1));
  r(errorPanel, labelCenter(vec2(0,4), vec2(sizeX, 1),
        strdup_s(errorLoadMessage)));
  r(errorPanel, labelCenter(vec2(0,5), vec2(sizeX, 1),
        strdup_s(errorLoadMessage2)));
  r(errorPanel, labelCenter(vec2(0,6.2), vec2(sizeX, 0.8),
        strdup_s(errorLoadMessage3)));
  r(errorPanel, labelCenter(vec2(0,7.0), vec2(sizeX, 0.8),
        strdup_s(errorLoadMessage4)));

  //r(errorPanel, label(vec2(0,5.5), 1,
        //strdup_s("Your game was saved as \"crashAutosave\"")));
  r(errorPanel, buttonCenter(vec2(0, 9), vec2(sizeX,1),
        strdup_s("Continue as Normal"), clearErrorLoad));
  r(errorPanel, buttonCenter(vec2(0, 10), vec2(sizeX,1),
        strdup_s("Start a New Game"), errorLoadNewGame));
  r(errorPanel, buttonCenter(vec2(0, 11), vec2(sizeX,1),
        strdup_s("Quit Game"), quitGame));

  return container;
}

