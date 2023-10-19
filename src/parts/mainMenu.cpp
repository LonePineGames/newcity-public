#include "mainMenu.hpp"

#include "../compass.hpp"
#include "../draw/camera.hpp"
#include "../icons.hpp"
#include "../game/feature.hpp"
#include "../game/game.hpp"
#include "../game/version.hpp"
#include "../land.hpp"
#include "../option.hpp"
#include "../platform/file.hpp"
#include "../platform/lookup.hpp"
#include "../platform/mod.hpp"
#include "../string_proxy.hpp"
#include "../selection.hpp"
#include "../serialize.hpp"

#include "blank.hpp"
#include "block.hpp"
#include "button.hpp"
#include "error.hpp"
#include "hr.hpp"
#include "label.hpp"
#include "loadPreviewPanel.hpp"
#include "panel.hpp"
#include "root.hpp"
#include "scrollbox.hpp"
#include "textBox.hpp"
#include "toolbar.hpp"

#include "spdlog/spdlog.h"

//#include "../steam/steamwrapper.hpp"

#include <algorithm>

InputCallback afterSaveConfirm = NULL;
bool doSaveConfirm = true;
char* saveFilename = strdup_s("");
ScrollState saveScroll;
ScrollState loadScroll;
ScrollState modsScroll;
TextBoxState searchTB;
std::string selectedLoadFile;
char* searchText = 0;
bool showModpackDesigns = true;
bool showBaseDesigns = true;
bool saveTestAsCity = false;
uint32_t loadMenuLookupFlags = _lookupForceMod;
item saveLoadModeTab = ModeGame;

const vec2 mainMenuLoc=vec2(-5, 5);
const vec2 mainMenuSize=vec2(14, 22.5);

void setDoSaveConfirm(bool val) {
  doSaveConfirm = true;
}

void clearSaveConfirm() {
  afterSaveConfirm = NULL;
}

void setSaveLoadModeTab(item mode) {
  saveLoadModeTab = mode;
}

uint32_t getLoadMenuLookupFlags() {
  if (saveLoadModeTab == ModeBuildingDesigner) {
    return loadMenuLookupFlags;
  } else {
    return 0;
  }
}

bool openOptionsMenu(Part* part, InputEvent event) {
  setMenuMode(OptionsMenu);
  return true;
}

bool openAboutPage(Part* part, InputEvent event) {
  setMenuMode(AboutPage);
  return true;
}

bool openSteamWorkshop(Part* part, InputEvent event) {
  setMenuMode(MenuMode::SteamWorkshop);
  return true;
}

void setSaveFilename(char* name) {
  free(saveFilename);
  saveFilename = name;
}

const char* getSaveFilename() {
  return saveFilename;
}

bool inputSaveFilenameKey(Part* part, InputEvent event) {
  if (event.key == GLFW_KEY_BACKSPACE) {
    int length = strlength(saveFilename);
    if (length == 0) {
      return true;
    }
    saveFilename[length-1] = '\0';
    return true;
  } else if (event.key == GLFW_KEY_ENTER) {
    if (saveGame(saveFilename, saveTestAsCity)) {
      closeMenus();
    }
    return true;
  }
  return false;
}

bool inputSaveFilenameText(Part* part, InputEvent event) {
  if (event.unicode > 0) {
    char* saveFilenameSwap = sprintf_o("%s%c",
        saveFilename, (char)event.unicode);
    free(saveFilename);
    saveFilename = saveFilenameSwap;
  }
  return true;
}

bool openLoadMenu(Part* part, InputEvent event) {
  saveLoadModeTab = getGameMode();
  setMenuMode(LoadGameMenu);
  return true;
}

bool openSaveMenu(Part* part, InputEvent event) {
  saveLoadModeTab = getGameMode();
  setMenuMode(SaveGameMenu);
  return true;
}

bool openSelectMod(Part* part, InputEvent event) {
  setMenuMode(ModsMenu);
  return true;
}

bool newGame(Part* part, InputEvent event) {
  closeMenus();
  setMenuMode(NewGameMenu);
  // Randomize land seeds
  randomizeNextLandConfig();
  //gameLoad(LoadTargetNew, ModeGame, 0);
  doSaveConfirm = false;
  return true;
}

bool returnFromDesigner(Part* part, InputEvent event) {
  closeMenus();
  gameLoad(LoadTargetAutosave, wasTestMode() ? ModeTest : ModeGame, 0);
  return true;
}

bool toggleDesignOrganizer(Part* part, InputEvent event) {
  closeMenus();
  gameLoad(LoadTargetAutosave, saveLoadModeTab == ModeDesignOrganizer ?
      ModeBuildingDesigner : ModeDesignOrganizer, 0);
  return true;
}

bool returnToGame(Part* part, InputEvent event) {
  closeMenus();
  gameLoad(LoadTargetAutosave, ModeGame, 0);
  return true;
}

bool newTest(Part* part, InputEvent event) {
  closeMenus();
  setMenuMode(NewGameMenu);
  // Randomize land seeds
  randomizeNextLandConfig();
  //gameLoad(LoadTargetNew, ModeGame, 0);
  doSaveConfirm = false;
  return true;
  //gameLoad(LoadTargetNew, ModeTest, 0);
}

bool startTest(Part* part, InputEvent event) {
  closeMenus();
  gameLoad(LoadTargetAutosave, ModeTest, 0);
  return true;
}

bool saveConfirm(Part* part, InputEvent event) {
  if (doSaveConfirm && getGameMode() != ModeDesignOrganizer) {
    setMenuMode(SaveConfirm);
    afterSaveConfirm = (InputCallback) part->ptrData;
    return true;

  } else {
    return ((InputCallback)part->ptrData)(part, event);
  }
}

Part* saveConfirmButton(vec2 loc, vec2 size, char* text,
  InputCallback afterCallback
) {
  Part* result = buttonCenter(loc, size, text, saveConfirm);
  result->ptrData = (void*) afterCallback;
  result->flags &= ~_partFreePtr;
  return result;
}

bool startDesigner(Part* part, InputEvent event) {
  closeMenus();
  gameLoad(LoadTargetAutosave, ModeBuildingDesigner, 0);
  return true;
}

bool newBuildingDesigner(Part* part, InputEvent event) {
  closeMenus();
  gameLoad(LoadTargetNew, ModeBuildingDesigner, 0);
  return true;
}

bool quitGame(Part* part, InputEvent event) {
  endGame();
  return true;
}

bool restartGame(Part* part, InputEvent event) {
  restartGame();
  return true;
}

bool selectLoadFileCallback(Part* part, InputEvent event) {
  selectedLoadFile = part->text;
  return true;
}

bool loadGameCallback(Part* part, InputEvent event) {
  closeMenus();
  gameLoad(LoadTargetFilename, (GameMode)saveLoadModeTab,
      strdup_s(selectedLoadFile.c_str()));
  return true;
}

bool selectSaveLoadModeTab(Part* part, InputEvent event) {
  saveLoadModeTab = part->itemData;
  return true;
}

bool selectNullMod(Part* part, InputEvent event) {
  selectMod(0);
  return true;
}

bool selectModCallback(Part* part, InputEvent event) {
  selectMod(part->text);
  return true;
}

bool saveGameCallback(Part* part, InputEvent event) {
  if (saveGame(saveFilename, saveTestAsCity)) {
    if (afterSaveConfirm != NULL) {
      afterSaveConfirm(part, event);
      afterSaveConfirm = NULL;
    } else {
      closeMenus();
    }
  }
  return true;
}

bool setSaveFilename(Part* part, InputEvent event) {
  free(saveFilename);
  saveFilename = strdup_s(part->text);
  return true;
}

bool focusSearchBox(Part* part, InputEvent event) {
  focusTextBox(&searchTB);
  return true;
}

bool unfocusSearchBox(Part* part, InputEvent event) {
  focusTextBox(0);
  return true;
}

bool toggleShowDesignsDir(Part* part, InputEvent event) {
  bool val = (loadMenuLookupFlags & part->itemData);
  if (val) {
    loadMenuLookupFlags &= ~part->itemData;
  } else {
    loadMenuLookupFlags |= part->itemData;
  }

  /*
  if (part->itemData) {
    showModpackDesigns = !showModpackDesigns;
    if (!showModpackDesigns) showBaseDesigns = true;
  } else {
    showBaseDesigns = !showBaseDesigns;
    if (!showBaseDesigns) showModpackDesigns = true;
  }
  */
  return true;
}

bool toggleSaveTestAsCity(Part* part, InputEvent event) {
  saveTestAsCity = !saveTestAsCity;
  return true;
}

bool crashImmediately(Part* part, InputEvent event) {
  handleError("You didn't say the magic word.");
  return true;
}

//bool openSteamOverlay(Part* part, InputEvent event) {
  //steam_openOverlay("steamid");
  //return true;
//}

Part* mainMenu(float aspectRatio) {
  float uiX = uiGridSizeX * aspectRatio;
  MenuMode menuMode = getMenuMode();
  GameMode mode = getGameMode();
  bool designer = mode == ModeBuildingDesigner;
  bool organizer = mode == ModeDesignOrganizer;
  bool test = mode == ModeTest;

  if (menuMode == SaveConfirm) {
    vec2 center = vec2(uiX*.5f, uiGridSizeY*.5f);
    vec2 size = vec2(12, 5);
    Part* saveConfirmPanel = panel(center - size*.5f, size);
    saveConfirmPanel->padding = 0.5f;

    r(saveConfirmPanel, label(vec2(0,0), 1.25,
          strdup_s("Do you want to save?")));
    r(saveConfirmPanel, button(vec2(0,3), vec2(3,1),
      strdup_s("Yes"), openSaveMenu));
    r(saveConfirmPanel, button(vec2(4,3), vec2(3,1),
      strdup_s("No"), afterSaveConfirm));
    r(saveConfirmPanel, button(vec2(8,3), vec2(3,1),
      strdup_s("Cancel"), openMainMenu));

    return saveConfirmPanel;
  }

  Part* menu = panel(mainMenuLoc + vec2(uiX-mainMenuSize.x,0), mainMenuSize);
  menu->padding = 0.5;

  float bs = mainMenuSize.x - 1;
  float buttonx = 0;
  float buttonxs = bs;
  float buttys = 1.25;
  float y = 0.25;

  if (menuMode == MainMenu) {
    r(menu, labelCenter(vec2(0,y), vec2(bs, 3),
          strdup_s("NewCity")));
    y += 3.5;
    r(menu, hr(vec2(0,y), bs));
    y += .25;
    Part* versionLabel =
      labelCenter(vec2(0,y), vec2(bs,0.75), strdup_s(versionString()));
    y += 1.25;
    //#ifdef LP_DEBUG
     versionLabel->onClick = crashImmediately;
     //supersoup - Lol, I don't think we want this in production.
     //Yes we do, it's hilarious! -LP
     //supersoup - When people ask me what working at Lone Pine Games was like,
     //I'm going to show them a screenshot of this comment chain
    //#endif
    r(menu, versionLabel);
    float buttStart = y+2;
    Part* playButt = buttonCenter(vec2(buttonx,buttStart-2),
        vec2(buttonxs, buttys), strdup_s("Play"), closeMenus);
    if (blinkFeature(FPlay)) {
      playButt->flags |= _partBlink;
    }
    r(menu, playButt);

    if (organizer) {
      if (isFeatureEnabledGlobal(FBuildingDesigner)) {
        r(menu, saveConfirmButton(vec2(buttonx,buttStart+4),
          vec2(buttonxs,buttys), strdup_s("Building Designer"), startDesigner));
      }
      r(menu, saveConfirmButton(vec2(buttonx,buttStart+5.5),
        vec2(buttonxs,buttys), strdup_s("Return to City"), returnToGame));

    } else if (designer) {
      r(menu, saveConfirmButton(vec2(buttonx,buttStart-.5),
        vec2(buttonxs,buttys), strdup_s("New Design"), newBuildingDesigner));
      r(menu, saveConfirmButton(vec2(buttonx,buttStart+1),
        vec2(buttonxs,buttys), strdup_s("Load Design"), openLoadMenu));
      r(menu, buttonCenter(vec2(buttonx,buttStart+2.5),
        vec2(buttonxs,buttys), strdup_s("Save Design"), openSaveMenu));
      if (c(CEnableDesignOrganizer)) {
        r(menu, saveConfirmButton(vec2(buttonx,buttStart+4),
          vec2(buttonxs,buttys), strdup_s("Organize Designs"),
            toggleDesignOrganizer));
      }
      r(menu, saveConfirmButton(vec2(buttonx,buttStart+5.5),
        vec2(buttonxs,buttys), strdup_s(wasTestMode() ? "Return to Test" :
          "Return to City"), returnFromDesigner));

    } else if (test) {
      r(menu, saveConfirmButton(vec2(buttonx,buttStart-.5),
        vec2(buttonxs,buttys), strdup_s("New Scenario"), newTest));
      r(menu, saveConfirmButton(vec2(buttonx,buttStart+1),
        vec2(buttonxs,buttys), strdup_s("Load Scenario"), openLoadMenu));
      r(menu, buttonCenter(vec2(buttonx,buttStart+2.5),
        vec2(buttonxs,buttys), strdup_s("Save Scenario"), openSaveMenu));

      if (isFeatureEnabledGlobal(FBuildingDesigner)) {
        r(menu, saveConfirmButton(vec2(buttonx,buttStart+4),
          vec2(buttonxs,buttys), strdup_s("Building Designer"), startDesigner));
      }

      r(menu, saveConfirmButton(vec2(buttonx,buttStart+5.5),
        vec2(buttonxs,buttys), strdup_s("Return to City"), returnToGame));

    } else {
      if (isFeatureEnabledGlobal(FNewGame)) {
        r(menu, saveConfirmButton(vec2(buttonx,buttStart-.5),
          vec2(buttonxs,buttys), strdup_s("New City"), newGame));
        r(menu, saveConfirmButton(vec2(buttonx,buttStart+1),
          vec2(buttonxs,buttys), strdup_s("Load City"), openLoadMenu));
        r(menu, buttonCenter(vec2(buttonx,buttStart+2.5),
          vec2(buttonxs,buttys), strdup_s("Save City"), openSaveMenu));
      }

      if (isFeatureEnabledGlobal(FBuildingDesigner)) {
        r(menu, saveConfirmButton(vec2(buttonx,buttStart+4),
          vec2(buttonxs,buttys), strdup_s("Building Designer"), startDesigner));
      }

      if (isFeatureEnabledGlobal(FTestMode)) {
        r(menu, saveConfirmButton(vec2(buttonx,buttStart+5.5),
          vec2(buttonxs,buttys), strdup_s("Scenario Editor"), startTest));
      }
    }

    if (isFeatureEnabledGlobal(FMods)) {
      r(menu, buttonCenter(vec2(buttonx,buttStart+8.25),
        vec2(buttonxs,buttys), strdup_s("Select Modpack"), openSelectMod));
    }

    r(menu, buttonCenter(vec2(buttonx, buttStart+9.75),
          vec2(buttonxs, buttys), strdup_s("Options"), openOptionsMenu));
    r(menu, buttonCenter(vec2(buttonx, buttStart+11.25),
          vec2(buttonxs, buttys), strdup_s("About"), openAboutPage));
    r(menu, buttonCenter(vec2(buttonx, buttStart+12.75),
          vec2(buttonxs, buttys), strdup_s("Quit"), quitGame));

  } else if (menuMode == SaveGameMenu || menuMode == LoadGameMenu ||
      menuMode == ModsMenu) {
    float top;
    InputCallback callback;
    bool mods = menuMode == ModsMenu;
    ScrollState* scrollState;

    if (menuMode == SaveGameMenu) {
      r(menu, labelCenter(vec2(0,0), vec2(bs,2),
        strdup_s(designer?"Save Design":test?"Save Scenario":"Save City")));
      r(menu, textBox(vec2(0,2), vec2(mainMenuSize.x-1, 1.5),
            strdup_s(saveFilename)));
      top = 4;

      if (designer) {
        r(menu, label(vec2(0,3.5), 0.85,
          sprintf_o("Files are saved in\n%sdesigns/", modDirectoryNonNull())));
        top += 1.5;

      } else if (test) {
        r(menu, button(vec2(0, 3.75),
              saveTestAsCity ? iconCheck : iconNull,
              vec2(mainMenuSize.x-1, 1), strdup_s("Save as City"),
              toggleSaveTestAsCity, 0));
        top += 1.5;
      }

      scrollState = &saveScroll;
      callback = setSaveFilename;
      menu->onText = inputSaveFilenameText;
      menu->onKeyDown = inputSaveFilenameKey;
      float halfWidth = (bs-0.5)*.5f;
      Part* saveButt = r(menu, button(vec2(halfWidth+0.5,mainMenuSize.y-2),
              vec2(halfWidth, 1), strdup_s("Save"), saveGameCallback));
      saveButt->flags |= _partAlignCenter;

    } else if (menuMode == LoadGameMenu) {
      r(menu, labelCenter(vec2(0,0), vec2(bs,2), strdup_s(
              saveLoadModeTab == ModeBuildingDesigner ? "Load Design" :
              saveLoadModeTab == ModeTest ? "Load Scenario" :
              "Load City")));
      top = 2;
      callback = selectLoadFileCallback;
      scrollState = &loadScroll;
      float modeTabPad = 0.5f;
      float modeTabWidth = (mainMenuSize.x-modeTabPad*2)/3;

      for (int i = ModeGame; i <= ModeBuildingDesigner; i++) {
        Part* modeTab = r(menu, buttonCenter(vec2(i*(modeTabWidth+modeTabPad)-0.5, -1.5), vec2(modeTabWidth, 1), strdup_s(
              i == ModeBuildingDesigner ? "Design" :
              i == ModeTest ? "Scenario" : "City"),
              selectSaveLoadModeTab));
        modeTab->itemData = i;
        if (i == saveLoadModeTab) {
          modeTab->flags |= _partRaised;
          modeTab->flags &= ~_partHover;
        } else {
          modeTab->flags |= _partHighlight;
        }
      }
      //top += 1;

      //r(menu, gradientBlock(vec2(0,-.75), vec2(bs,0.25), colorPanelGrad0, colorLoweredGrad1));
      //top += 0.5;

      // Checkboxes to filter by directory
      if (saveLoadModeTab == ModeBuildingDesigner) {
        float bx = 0;
        float scl = 0.8;
        float lfpY = 0;
        float pad = 0.25;
        float lfpWidth = bs;
        float lfpLength = scl*5+1;
        vec2 loc = vec2(-lfpWidth-1-menu->padding,
          menu->dim.end.y*.5f-4-lfpLength-menu->padding*2);
        Part* lookupFlagsPanel = r(menu, panel(loc, vec2(lfpWidth, lfpLength)));
        lookupFlagsPanel->padding = pad;

        r(lookupFlagsPanel, label(vec2(0,lfpY), scl,
            strdup_s("Reading files from")));
        lfpY += scl;

        vector<string> directories = lookupFileCandidates("designs", 0);
        vector<string> selectedDirs = lookupFileCandidates("designs",
            getLoadMenuLookupFlags());

        int k = 0;
        for (int i = 0; i < directories.size(); i++) {
          bool match = false;
          if (k < selectedDirs.size() &&
              strcmpi_s(directories[i].c_str(), selectedDirs[k].c_str()) == 0) {
            k ++;
            match = true;
          }

          r(lookupFlagsPanel, button(vec2(bx, lfpY),
                match ? iconCheck : iconNull,
                vec2(bs-1, scl), strdup_s(directories[i].c_str()),
                toggleShowDesignsDir, 1 << (i+1)));
          lfpY += scl;
        }
      }

      searchTB.text = &searchText;
      Part* tb = r(menu, textBox(vec2(0,top),
            vec2(mainMenuSize.x-1, 1.5), &searchTB));
      tb->onClick = focusSearchBox;
      tb->onCustom = unfocusSearchBox;
      if (searchText == 0 || strlength(searchText) == 0) {
        Part* search = r(menu, label(vec2(0.25,top+0.25), 1, strdup_s("Search")));
        search->foregroundColor = PickerPalette::GrayDark;
      }
      top += 2;

      if (selectedLoadFile.length() > 0) {
        float halfWidth = (bs-0.5)*.5f;
        Part* loadButt = r(menu, button(vec2(halfWidth+0.5,mainMenuSize.y-2),
                vec2(halfWidth, 1), strdup_s("Load"), loadGameCallback));
        loadButt->flags |= _partAlignCenter;
      }

    } else if (mods) {
      r(menu, labelCenter(vec2(0,0), vec2(bs,2), strdup_s("Select Modpack")));
      r(menu, hr(vec2(0,2), bs));
      r(menu, label(vec2(0,2.25), 1,
        strdup_s("You will need to restart.")));
      top = 4;
      callback = selectModCallback;
      scrollState = &modsScroll;
      float halfWidth = (bs-0.5)*.5f;
      Part* restartButt = r(menu, button(vec2(halfWidth+0.5,mainMenuSize.y-2),
              vec2(halfWidth, 1), strdup_s("Restart"), restartGame));
      restartButt->flags |= _partAlignCenter;

    } else {
      handleError("bad menuMode");
    }

    vector<string> files;
    bool forceCity = test && saveTestAsCity && menuMode == SaveGameMenu;
    bool isDesigns = (menuMode != LoadGameMenu && designer) ||
      (menuMode == LoadGameMenu && saveLoadModeTab == ModeBuildingDesigner);
    const char* ext = menuMode == ModsMenu ? "" :
      forceCity ? ".city" :
      menuMode == SaveGameMenu ? fileExtension() :
      menuMode == LoadGameMenu ? (
          saveLoadModeTab == ModeBuildingDesigner ? ".design" :
          saveLoadModeTab == ModeTest ? ".test" :
          saveLoadModeTab == ModeGame ? ".city" : ".err") : ".err";
    uint32_t lookupFlags = 0;

    if (menuMode == ModsMenu) {
      files = listModpacks(0);

    } else {
      if (menuMode == LoadGameMenu) lookupFlags = getLoadMenuLookupFlags();
      if (isDesigns) {
        files = lookupDesigns(lookupFlags);
      } else {
        files = lookupDirectory(saveDirectory(saveLoadModeTab), ext, lookupFlags);
      }
    }

    vec2 scrollSize = mainMenuSize - vec2(1,top+2.5);
    Part* scroll = scrollbox(vec2(0,0), scrollSize);
    float sy = 0;

    if (mods) {
      Part* btn = button(vec2(0,0), getMod() == 0 ? iconCheck : iconNull,
          vec2(bs-1, 1), strdup_s("Disable Mods"), selectNullMod, 0);
      if (getNextMod() == 0) {
        btn->flags |= _partHighlight;
      }
      r(scroll, btn);
      sy ++;
    }

    string previewFile = "invalid";
    if (menuMode == ModsMenu && getNextMod() != 0) {
      previewFile = getNextMod();
    } else if (menuMode == LoadGameMenu) {
      previewFile = selectedLoadFile;
    } else if (menuMode == SaveGameMenu && saveFilename != 0) {
      previewFile = saveFilename;
    }

    bool anyMatches = false;
    char* search = menuMode == SaveGameMenu ? saveFilename :
      menuMode == ModsMenu ? 0 : searchText;
    if (search != 0 && strlength(search) > 0) {
      for (int i=0; i < files.size(); i++) {
        if (stringContainsCaseInsensitive(files[i], search)) {
          anyMatches = true;
          break;
        }
      }
    }

    for (int i=0; i < files.size(); i++) {
      if (anyMatches && !stringContainsCaseInsensitive(files[i], search)) {
        continue;
      }

      char* filename = strdup_s(files[i].c_str());
      if (menuMode == ModsMenu) {
        bool isCurrent = streql(filename, getMod());
        Part* btn = button(vec2(0,i+1), isCurrent ? iconCheck : iconNull,
            vec2(bs-1, 1), filename, callback, 0);
        if (streql(filename, getNextMod())) {
          btn->flags |= _partHighlight;
        }
        r(scroll, btn);
        sy ++;

      } else {
        Part* btn = r(scroll, button(vec2(0,sy), vec2(bs-1, 1),
              filename, callback));
        if (streql(filename, previewFile.c_str())) {
          btn->flags |= _partHighlight;
        }
        sy ++;
      }
    }

    Part* scrollFrame = scrollboxFrame(vec2(0,top), scrollSize,
      scrollState, scroll);
    r(menu, scrollFrame);
    Part* backButt = r(menu, button(
          vec2(0, mainMenuSize.y-2), vec2((bs-0.5)*.5f, 1),
          strdup_s("Back"), openMainMenu));
    backButt->flags |= _partAlignCenter;

    string previewFilename = menuMode == LoadGameMenu ? saveDirectory(saveLoadModeTab) : saveDirectory();
    if (menuMode != ModsMenu) {
      previewFilename = previewFilename + previewFile;
      if (endsWith(previewFilename.c_str(), "/")) {
        previewFilename = previewFilename + "design";
      }
      previewFilename = previewFilename + ext;
      previewFilename = lookupFile(previewFilename, lookupFlags);
    } else if (getNextMod() != 0) {
      previewFilename = "modpacks/";
      previewFilename = lookupFile(previewFilename + getNextMod()
          + "/preview.png", 0);
    }

    // Preview Panel
    if (fileExists(previewFilename)) {
      vec2 size = vec2(16, 8);
      vec2 loc = vec2(-size.x-1-menu->padding,
          menu->dim.end.y*.5f-size.y*.5f);
      r(menu, loadPreviewPanel(loc, size, previewFilename.c_str()));
    }
  }

  return menu;
}

