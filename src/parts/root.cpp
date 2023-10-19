#include "root.hpp"

#include "../compass.hpp"
#include "../console/conDisplay.hpp"
#include "../draw/camera.hpp"
#include "../game/feature.hpp"
#include "../game/game.hpp"
#include "../graph/transit.hpp"
#include "../input.hpp"
#include "../option.hpp"
#include "../platform/file.hpp"
#include "../platform/mod.hpp"
#include "../string_proxy.hpp"
#include "../selection.hpp"
#include "../serialize.hpp"
#include "../steam/steamwrapper.hpp"
#include "../tools/road.hpp"
#include "../tutorial.hpp"
#include "../util.hpp"

#include "aboutPanel.hpp"
#include "blank.hpp"
#include "blueprintsList.hpp"
#include "button.hpp"
#include "changelogPanel.hpp"
#include "console.hpp"
#include "designOrganizerPanel.hpp"
#include "economyPanel.hpp"
#include "error.hpp"
#include "hr.hpp"
#include "label.hpp"
#include "leftPanel.hpp"
#include "loader.hpp"
#include "mainMenu.hpp"
#include "menuBar.hpp"
#include "messageBoard.hpp"
#include "newGamePanel.hpp"
#include "optInPopup.hpp"
#include "optionsPanel.hpp"
#include "panel.hpp"
#include "scrollbox.hpp"
#include "selectionPanel.hpp"
#include "statusBar.hpp"
#include "steamWorkshop.hpp"
#include "textBox.hpp"
#include "toolbar.hpp"
#include "tutorialPanel.hpp"
#include "tutorialPopup.hpp"

#include <algorithm>

MenuMode menuMode = MainMenu;
SideBarMode sideBarMode = MessageBoardSideBar;
SideBarMode prevSideBarMode = MessageBoardSideBar;
char* errorMessage = NULL;

MenuMode getMenuMode() {
  return menuMode;
}

void setMenuMode(MenuMode mode) {
  if (menuMode == OptionsMenu) closeOptionsMenu();
  if (mode == OptionsMenu) detectKeyCollisionsAll(); // If switching to the options menu, do a check for collisions in preparation
  menuMode = mode;
  setDoSaveConfirm(true);
  setCompassVisible(mode == HiddenMenu);
  setFreezeCamera(mode != HiddenMenu && mode != HideUI);
  setGameMenuPause(mode != HiddenMenu && mode != HideUI);
}

SideBarMode getSideBarMode() {
  return sideBarMode;
}

void setSideBarMode(SideBarMode mode) {
  if (mode == sideBarMode) return;
  SideBarMode prevMode = sideBarMode;
  if (prevMode != BlueprintsListSideBar) prevSideBarMode = prevMode;
  sideBarMode = mode;
  if (prevMode == BlueprintsListSideBar) {
    setTool(0);
  } else if (mode == BlueprintsListSideBar) {
    setBlueprintTool();
  }
}

void closeBlueprintSidebar() {
  setSideBarMode(prevSideBarMode);
}

bool toggleSideBarMode(Part* part, InputEvent event) {
  SideBarMode mode = (SideBarMode) part->itemData;
  if (mode == sideBarMode) {
    setSideBarMode(NoSideBar);
  } else {
    setSideBarMode(mode);
  }
  return true;
}

void openMainMenu() {
  if(menuMode == MenuMode::ModsMenu) {
    selectMod(getMod());
  }

  setMenuMode(MainMenu);
  clearSaveConfirm();
}

bool openMainMenu(Part* part, InputEvent event) {
  openMainMenu();
  return true;
}

bool closeMenus() {
  stopBlinkingFeature(FPlay);
  errorMessage = NULL;
  clearErrorState();
  setCompassVisible(true);
  setMenuMode(HiddenMenu);
  reportTutorialUpdate(TutorialUpdateCode::ClosedMainMenu);
  setFixBuildingMode(false);
  return true;
}

bool closeMenus(Part* part, InputEvent event) {
  closeMenus();
  return true;
}

/*
bool tildePressed(Part* part, InputEvent event) {
  consoleToggle();
  return true;
}
*/

bool escapePressed(Part* part, InputEvent event) {
  if (getErrorMessage() != 0) {
    clearErrorState();
    //endGame();

  } else if (menuMode == OptionsMenu || menuMode == AboutPage ||
      menuMode == NewGameMenu) {
    openMainMenu();

  } else if (menuMode != HiddenMenu) {
    closeMenus();

  } else if (isFixBuildingMode()) {
    setFixBuildingMode(false);

  } else if (isTransitTool()) {
    if (getCurrentLine() != 0) {
      setCurrentLine(0);
    } else {
      setTool(0);
    }

  } else if (getLeftPanel() != NoPanel) {
    setLeftPanel(NoPanel);

  } else if (getSelectionType() != NoSelection) {
    clearSelection();

  } else if (getCurrentTool() != 0) {
    setTool(0);

  } else if (menuMode == HiddenMenu) {
    openMainMenu();
  }

  // Close console if open
  consoleSetOpen(false);

  return true;
}

bool rootKeyPressed(Part* part, InputEvent event) {
  if(getKeyBind((int)InputAction::ActRootEscape).active) {
    escapePressed(part, event);
    return true;
  } else if (getKeyBind((int)InputAction::ActToggleConsole).active) {
    consoleToggle();
    return true;
  } else if (getKeyBind((int)InputAction::ActPauseGame).active && menuMode == HideUI) {
    toggleGamePause();
    return true;
  } else { // Not a key we wanted
    return false;
  }
}

bool nullCallback(Part* part, InputEvent event) {
  return true;
}

char* getErrorMessage() {
  return errorMessage;
}

void setErrorMessage(char* message) {
  setMenuMode(ErrorMode);
  errorMessage = message;
}

bool steam_button_callback(Part* part, InputEvent event) {
  #ifdef INCLUDE_STEAM
    steam_openOverlay(STEAM_OVERLAY_ENUM::OFFICIALGAMEGROUP);
  #endif
  return true;
}

Part* root(float aspectRatio) {
  float uiX = uiGridSizeX * aspectRatio;
  if(menuMode == ErrorMode) {
    return errorPanel(aspectRatio, errorMessage);
  }

  Part* container = panel(line(vec3(0, 0, 0),
    vec3(uiX, uiGridSizeY, 0)));
  item mode = getGameMode();
  container->onKeyDown = rootKeyPressed;

  if (getCameraTime() < 5 || isGameLoading()) {
    r(container, loader());
    container->renderMode = RenderShadow;
    return container;
  }

  // Tutorial panel
  bool tutorial = false;
  if (getCameraTime() > 6 && isTutorialPanelOpen() &&
      (menuMode == HiddenMenu || menuMode == MainMenu) &&
      (getLeftPanel() != NewspaperPanel || menuMode != HiddenMenu)) {
    tutorial = true;
    r(container, tutorialPanel());
  }

  if(consoleIsOpen()) {
    float conSizeX = uiX / 2.0f;
    float conSizeY = uiGridSizeY / 2.0f;
    vec2 conSizeNow = consoleSize();

    if(conSizeNow.x != conSizeX && conSizeX >= conMinSizeX) {
      consoleSetSize(conSizeX, conSizeY);
    }
    //consoleSizeEnforce(conSizeX, conSizeY);

    Part* con = console();
    r(container, con);
  }

  if(menuMode == HiddenMenu || menuMode == HideUI) {
    r(container, menuBar(uiX, uiGridSizeY, menuMode != HideUI));
    r(container, statusBar(uiX));
    r(container, toolbar());
    r(container, leftPanel());
    if(sideBarMode == MessageBoardSideBar) {
      r(container, messageBoard(uiX, uiGridSizeY));
    } else if(sideBarMode == BlueprintsListSideBar) {
      r(container, blueprintsList(uiX, uiGridSizeY));
    }
    container->renderMode = RenderTransparent;
    if(menuMode == HideUI) {
      for(int i = 1; i < container->numContents; i++) {
        Part* part = container->contents[i];
        part->renderMode = RenderHidden;
      }
    }
    return container;
  }

  container->onKeyDown = rootKeyPressed; // Duplicate assignment of onKeyDown? 
  //Commenting out instead of deleting in case I'm mistaken about something...

  //container->onScroll = nullCallback;
  container->onHover = nullCallback;
  container->renderMode = RenderShadow;

  //if(!seenTutorialPopup()) {
    //r(container, tutorialPopup(aspectRatio));
    //return container;

  //} else if (!hasSeenLogUploadMessage()) {
    //r(container, optInPopup(aspectRatio));
    //return container;

  //} else 
  if (menuMode == OptionsMenu) {
    r(container, optionsPanel(aspectRatio));
    return container;

  } else if (menuMode == NewGameMenu) {
    r(container, newGamePanel(aspectRatio));
    return container;

  } else if (menuMode == AboutPage) {
    r(container, aboutPanel(aspectRatio));
    return container;
  } else if (menuMode == SteamWorkshop) {
    r(container, steamWorkshop(aspectRatio));
    return container;
  }

  #ifdef INCLUDE_STEAM
    // Even if Steam's included, only show buttons if it has an active connection
    if (steam_isActive()) {
      std::string wsTxt = "Steam Workshop";
      float steamXPad = 0.5f;
      float xPos = 1.0f;
      float wsTxtLen = stringWidth(wsTxt.c_str());

      // Container for buttons
      Part* steamWSContainer = panel(vec2(xPos, uiGridSizeY-2.0f), vec2(1.0f+wsTxtLen, 1.0f));

      // Steam button
      Part* steam = button(vec2(0.0f, 0.0f), iconSteam,
          steam_button_callback);
      r(steamWSContainer, steam);

      // Steam workshop button
      Part* steamWS = button(vec2(1.0f+(steamXPad*2.0f), 0.0f), vec2(wsTxtLen-1.0f, 1.0f), strdup_s(wsTxt.c_str()), openSteamWorkshop);

      r(steamWSContainer, steamWS);
      r(container, steamWSContainer);
    }
  #endif

  if (!tutorial) {
    r(container, changelogPanel());
  }
  r(container, mainMenu(aspectRatio));
  r(container, statusBar(uiX));
  return container;
}

