#ifndef PART_MENU_H
#define PART_MENU_H

#include "part.hpp"

enum MenuMode {
  HiddenMenu, MainMenu, NewGameMenu, SaveGameMenu, LoadGameMenu, SaveConfirm,
  HideUI, ErrorMode, OptionsMenu, AboutPage, ModsMenu, SteamWorkshop, numMenuModes
};

enum SideBarMode {
  MessageBoardSideBar, BlueprintsListSideBar, NoSideBar,
  numSideBarModes
};

Part* root(float aspectRatio);
bool openMainMenu(Part* part, InputEvent event);
MenuMode getMenuMode();
void setMenuMode(MenuMode mode);
SideBarMode getSideBarMode();
void setSideBarMode(SideBarMode mode);
bool toggleSideBarMode(Part* part, InputEvent event);
void closeBlueprintSidebar();
void setErrorMessage(char* message);
char* getErrorMessage();
bool closeMenus();
bool closeMenus(Part* part, InputEvent event);
bool quitGame(Part* part, InputEvent event);
bool rootKeyPressed(Part* part, InputEvent event);
bool escapePressed(Part* part, InputEvent event);

#endif
