#include "menuBar.hpp"

#include "../compass.hpp"
#include "../draw/camera.hpp"
#include "../game/feature.hpp"
#include "../game/game.hpp"
#include "../icons.hpp"
#include "../newspaper/newspaper.hpp"
#include "../option.hpp"
#include "../string_proxy.hpp"
#include "../util.hpp"

#include "button.hpp"
#include "label.hpp"
#include "leftPanel.hpp"
#include "messageBoard.hpp"
#include "panel.hpp"
#include "root.hpp"
#include "root.hpp"
#include "slider.hpp"
#include "tooltip.hpp"

#include <time.h>
#include "spdlog/spdlog.h"

FPSMode fpsMode = ShowNoFPS; //LP_DEBUG ? ShowFPS : ShowNoFPS;
float lastUIToggleHover = 0;

int getFPSMode() {
  return fpsMode;
}

void setFPSMode(int mode) {
  fpsMode = (FPSMode)mode;
}

bool toggleFPS(Part* part, InputEvent event) {
  fpsMode = (FPSMode) ((fpsMode+1) % numFPSModes);
  writeOptions();
  return true;
}

bool toggleMessageBoard(Part* part, InputEvent event) {
  lastUIToggleHover = getCameraTime();
  toggleMessageBoard();
  return true;
}

bool toggleUIHover(Part* part, InputEvent event) {
  bool dimClip = isInDim(event.mouseLoc, line(vec3(0,0,0), part->dim.end));
  if (dimClip) {
    lastUIToggleHover = getCameraTime();
  }
  return false;
}

bool openNewspaper(Part* part, InputEvent event) {
  setLeftPanel(NewspaperPanel);
  return false;
}

bool toggleUI(Part* part, InputEvent event) {
  item key = event.key;
  lastUIToggleHover = getCameraTime();
  if (getMenuMode() == HideUI) {
    setMenuMode(HiddenMenu);
  } else {
    setMenuMode(HideUI);
    setCompassVisible(false);
  }
  return true;
}

Part* menuBar(float uiX, float uiY, bool showUI) {
  float padding = 0.1;

  float xsize = 8+padding*2;
  Part* menuBar = panel(vec2(uiX-xsize, 0), vec2(xsize, 1+padding*2));
  menuBar->padding = padding;

  if (showUI) {
    if (isFeatureEnabled(FNoteObject)) {
      Part* showMsgButt = button(vec2(0,0), iconPin,
          toggleSideBarMode, MessageBoardSideBar);
      setPartTooltipValues(showMsgButt,
        TooltipType::TbMsg);
      if (getSideBarMode() == MessageBoardSideBar) {
        showMsgButt->flags |= _partHighlight;
      }
      r(menuBar, showMsgButt);
    }

    if (getGameMode() != ModeBuildingDesigner &&
        isFeatureEnabledGlobal(FBlueprint)) {
      Part* bpButt = button(vec2(1,0), iconBlueprint,
          toggleSideBarMode, BlueprintsListSideBar);
      setPartTooltipValues(bpButt,
        TooltipType::TbBlu);
      bpButt->inputAction = InputAction::ActBlueprintTool;
      if (getSideBarMode() == BlueprintsListSideBar) {
        bpButt->flags |= _partHighlight;
      }
      r(menuBar, bpButt);
    }

    if (getGameMode() != ModeBuildingDesigner &&
        isFeatureEnabledGlobal(FNewspaper) && doesNewspaperExist()) {
      Part* npButt = button(vec2(2,0), iconNewspaper,
          openNewspaper, 0);
      npButt->inputAction = InputAction::ActNewspaperPanel;
      setPartTooltipValues(npButt, TooltipType::TbNewspaper);
      ///if (getSideBarMode() == BlueprintsListSideBar) {
        ///npButt->flags |= _partHighlight;
      ///}
      r(menuBar, npButt);
    }

    //r(menuBar, label(vec2(0,0), 1, sprintf_o("%.0f FPS", getFPS())));
    char* fpsMsg = 0;
    if (fpsMode == ShowFPS) {
      fpsMsg = sprintf_o("%3.ffps", getFPS());
    } else if (fpsMode == ShowMS) {
      fpsMsg = sprintf_o("%2.0fms", getFrameDuration()*1000);

    } else if (fpsMode == ShowTime12 || fpsMode == ShowTime24) {
      time_t rawtime = time(NULL);
      struct tm * timeinfo;
      timeinfo = localtime ( &rawtime );
      int hour = timeinfo->tm_hour;
      if (fpsMode == ShowTime12) {
        fpsMsg = sprintf_o("%d:%02d%s", (hour+11)%12+1, timeinfo->tm_min,
            hour >= 12 ? "pm" : "am");
      } else {
        fpsMsg = sprintf_o("%d:%02d", hour, timeinfo->tm_min);
      }

    } else {
      fpsMsg = strdup_s("");
    }

    Part* fpsButt = button(vec2(3.125,0), vec2(3.75,1), fpsMsg, toggleFPS);
    fpsButt->flags |= _partAlignRight;
    setPartTooltipValues(fpsButt,
      TooltipType::TbFPS);

    r(menuBar, fpsButt);
    r(menuBar, button(vec2(7,0), iconMenu, openMainMenu, 0));

  } else {
    menuBar->renderMode = RenderTransparent;
  }

  if (isFeatureEnabledGlobal(FQueryTool)) {
    Part* eyeButt = button(vec2(7+padding,uiY-1),
        showUI ? iconEye : iconEyeClosed, toggleUI, 0);
    if (!showUI && lastUIToggleHover + 1 < getCameraTime()) {
      eyeButt->renderMode = RenderTransparent;
    }
    eyeButt->onHover = toggleUIHover;
    eyeButt->inputAction = ActHideUI;
    setPartTooltipValues(eyeButt,
      TooltipType::TbEye);
    if (getCameraTime() - lastUIToggleHover > 0.1) {
      eyeButt->flags |= _partTransparent;
    }
    r(menuBar, eyeButt);
  }

  return menuBar;
}

