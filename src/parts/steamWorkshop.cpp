//-----------------------------------------------------------------------------
// steamWorkshop - Defines and renders the UI element for the Steam Workshop
//-----------------------------------------------------------------------------

#include "button.hpp"
#include "icon.hpp"
#include "image.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "root.hpp"
#include "scrollbox.hpp"
#include "steamWorkshop.hpp"
#include "textBox.hpp"
#include "../building/design.hpp"
#include "../platform/file.hpp"
#include "../platform/lookup.hpp"
#include "../platform/mod.hpp"
#include "../icons.hpp"
#include "../string.hpp"
#include "../steam/steamws_core.hpp"

enum wsExtEnum {
  ExtDesign = 0,
  ExtGif,
  ExtJpg,
  ExtJpeg,
  ExtLua,
  ExtPng,
  ExtObj,
  ExtOgg,
  Dir,
  Num_Exts
};

// Workshop str consts
const std::string wsTabUnk = "Unknown Tab";
const std::string wsNameUnk = "Unknown Name";
const std::string wsBaseStr = "Base";
const std::string wsModStr = "Active Mod";
const std::string wsDesignStr = "Designs";
const std::string wsSoundStr = "Sounds/Music";
const std::string wsTextureStr = "Textures";
const std::string wsBlank = "This space intentionally left blank";

// Browse tab str consts
const std::string wsBrowseStr = "Browse NewCity Workshop content on Steam:";
const std::string wsClickBrowseStr1 = "Click here to browse the Workshop";
const std::string wsClickBrowseStr2 = "through the Steam Overlay";

// Upload tab str consts
const std::string wsUploadStr = "Upload an item to the Steam Workshop:";
const std::string wsClickUploadStr = "Click to open file browser";
const std::string wsDescStr = "Description: ";
const std::string wsSelectedStr = "Selected file: ";
const std::string wsButtonBackStr = "Back";
const std::string wsButtonUploadStr = "Upload";

// Workshop general size and pos consts
const float wsPad = 0.25f;
const float wsPanelCoeff = 0.5f;
const float wsWidth = 40;
const float wsHeight = 24;
const float wsTitleSize = 1.25f;
const float wsTxtSize = 0.85f;
const vec2 wsSize = vec2(wsWidth+wsPad*2, wsHeight+wsPad*2);
const vec2 wsFileBrowserLoc=vec2(-5, 5);

// Workshop status consts
const std::string wsBusyStr = "Busy...";
const vec2 wsStatusSize = vec2(16.0f, 9.0f);

// Submission window consts
const std::string wsDefDescStr = "Item for use with NewCity; refer to file extension for specific handling behavior";
const vec2 wsSubSize = vec2(16.0f, 9.0f);

// File Browser modifiers
//static bool wsBaseFiles = false; // Unchecked by default
//static bool wsModFiles = true;

// File Browser state
static int32_t wsFileIndex = -1;
static std::string wsFilePath = "";
static steamws_itemTag wsActiveFileTag = steamws_itemTag::DesignLegacy;
static std::array<std::string, wsExtEnum::Num_Exts> wsActiveFileExts = { steamws_extDesign, "", "", "", "", "", "", "" };
static char* wsFileSearchTxt = 0;
static vector<WSFile> files;
static ScrollState wsFileBrowserScroll;
static TextBoxState wsFileBrowserTB;

// Submission window state
static bool wsSubWindow = false;
static std::string wsSubName = "";
static std::string wsSubDesc = wsDefDescStr;
static std::string wsSubChange = "";
static char* wsSubNamePtr = 0;
static char* wsSubDescPtr = 0;
static char* wsSubChangePtr = 0;
static TextBoxState wsSubNameTB;
static TextBoxState wsSubDescTB;
static TextBoxState wsSubChangeTB;


// Workshop state
static WSTab wsActiveTab = WSTab::Browse;
static std::string wsStatusTxt = wsBusyStr;


void workshopDeselectFile() {
  wsFileIndex = -1;
  wsFilePath = "";
}

void workshopResetActiveFileVars() {
  wsActiveFileTag = steamws_itemTag::Generic;
  wsActiveFileExts = { "", "", "", "", "", "", "", "" };
}

void workshopResetSubmissionWindow() {
  wsSubWindow = false;
  wsSubName = "";
  wsSubDesc = wsDefDescStr;
  wsSubChange = "";
  if(wsSubNamePtr != 0) {
    free(wsSubNamePtr);
  }
  if(wsSubDescPtr != 0) {
    free(wsSubDescPtr);
  }
  if(wsSubChangePtr != 0) {
    free(wsSubChangePtr);
  }
  wsSubNamePtr = 0;
  wsSubDescPtr = 0;
  wsSubChangePtr = 0;
}

void workshopResetFileSources() {
  const char* mod = getMod();
  int modLen = 0;

  if(mod != NULL) {
    modLen = strlen(mod);
  }

  //wsBaseFiles = modLen <= 0;
  //wsModFiles = modLen > 0;
}

void workshopResetFileBrowser() {
  // Reset File Browser
  workshopDeselectFile();
  if(wsFileSearchTxt != 0) {
    free(wsFileSearchTxt);
    wsFileSearchTxt = 0;
  }
  std::vector<WSFile>().swap(files);
  workshopResetFileSources();
  workshopResetActiveFileVars();
  workshopResetSubmissionWindow();
}

bool workshopResetFileBrowser(Part* part, InputEvent event) {
  workshopResetFileBrowser();
  return true;
}

bool workshopResetSubmissionWindow(Part* part, InputEvent event) {
  workshopResetSubmissionWindow();
  return true;
}

void workshopSetBusyTxt(std::string txt) {
  wsStatusTxt = txt;
}

void workshopResetFileVector() {
  SPDLOG_INFO("Resetting file vector ... ");
  files.clear(); // Reset size to zero, leave capacity
}

void workshopPopulateFileVector(steamws_itemTag tag, std::string ext) {
  // Are we fetching mods?
  bool isMods = tag == steamws_itemTag::Mod;

  // Get dir from tag
  std::string dir = steamws_getRelativeDirForTag(tag);

  // If we're working with mods
  if (isMods) {
    vector<string> mods = listModpacks(0);
    for(int b = 0; b < mods.size(); b++) {
      files.push_back(WSFile(dir, mods[b], ext, wsBaseStr, tag));
    }

    // Early return
    return;
  }

  // For designs, iterate over design directories
  if (tag == steamws_itemTag::DesignLegacy || tag == steamws_itemTag::DesignPack) {
    vector<string> designDirs = lookupSubDirectories(dir, _lookupOnlyMods);
    std::string subdirs = "";
    for (int d = 0; d < designDirs.size(); d++) {
      subdirs += designDirs[d] + " ";
      vector<string> filenames = lookupDirectory(dir + designDirs[d], ext, _lookupOnlyMods);
      for (int m = 0; m < filenames.size(); m++) {
        string filePath = lookupFile(dir + designDirs[d] + "/" + filenames[m] + ext,
          _lookupExcludeBase);
        string parentDir = getParentDirectory(filePath) + "/";
        files.push_back(WSFile(parentDir, designDirs[d],
          ext, "", tag));
        // SPDLOG_INFO("parentDir {} for file {}", parentDir, designDirs[d] + "/" + filenames[m]);
      }
    }
    SPDLOG_INFO("Found design dirs: {}", subdirs);
  } else {
    // For anything not Designs, just get files in dir
    vector<string> filenames = lookupDirectory(dir, ext, _lookupOnlyMods);
    for (int m = 0; m < filenames.size(); m++) {
      string filePath = lookupFile(dir + filenames[m] + ext,
        _lookupExcludeBase);
      string parentDir = getParentDirectory(filePath) + "/";
      files.push_back(WSFile(parentDir, filenames[m],
        ext, "", tag));
    }
  }

  /*
  vector<string> baseFiles;
  vector<string> modFiles;

  // TODO: Search and read subdirectories as well
  if(wsBaseFiles && mods) {
    baseFiles = listModpacks(0);
  }

  if(!mods && wsModFiles) {
    modFiles = lookupDirectory(dir, ext, _lookupOnlyMods);
  }

  // Combine the char* vectors into the WSFile vector and sort
  for(int b = 0; b < baseFiles.size(); b++) {
    files.push_back(WSFile(dir, baseFiles[b], ext, wsBaseStr, tag));
  }
  for(int m = 0; m < modFiles.size(); m++) {
    files.push_back(WSFile(std::string(modDirectory()) + dir, modFiles[m], ext, wsModStr, tag));
  }
  if(baseFiles.size() > 0 && modFiles.size() > 0) {
    sort(files.begin(), files.end(), compareStrings);
  }

  SPDLOG_INFO("Populated file vector for ext {}, tag {}, from dir {}, files found: {}", ext, tag, dir, baseFiles.size() + modFiles.size());
  */
}

void workshopPopulateFileVector(steamws_itemTag tag, std::array<std::string, wsExtEnum::Num_Exts> exts) {
  // Clear file vector
  workshopResetFileVector();
  // Deselect any file, because we're repopulating the vector
  workshopDeselectFile();
  for(int i = 0; i < exts.size(); i++) {
    if(exts[i].length() == 0) continue;
    workshopPopulateFileVector(tag, exts[i]);
  }
}

bool workshopFocusSearchBox(Part* part, InputEvent event) {
  focusTextBox(&wsFileBrowserTB);
  return true;
}

bool workshopUnfocusTextBox(Part* part, InputEvent event) {
  focusTextBox(0);
  return true;
}

bool workshopFocusNameBox(Part* part, InputEvent event) {
  focusTextBox(&wsSubNameTB);
  return true;
}

bool workshopFocusDescBox(Part* part, InputEvent event) {
  focusTextBox(&wsSubDescTB);
  return true;
}

bool workshopFocusChangeBox(Part* part, InputEvent event) {
  focusTextBox(&wsSubChangeTB);
  return true;
}

bool workshopToggleDesigns() {
  workshopResetActiveFileVars(); // Must reset before setting up new active exts
  wsActiveFileTag = steamws_itemTag::DesignLegacy;
  wsActiveFileExts[wsExtEnum::ExtDesign] = steamws_extDesign;
  workshopPopulateFileVector(wsActiveFileTag, wsActiveFileExts);
  return true;
}

bool workshopToggleDesigns(Part* part, InputEvent event) {
  workshopToggleDesigns();
  return true;
}

bool workshopToggleLua(Part* part, InputEvent event) {
  workshopResetActiveFileVars(); // Must reset before setting up new active exts
  wsActiveFileTag = steamws_itemTag::Lua;
  wsActiveFileExts[wsExtEnum::ExtLua] = steamws_extLua;
  workshopPopulateFileVector(wsActiveFileTag, wsActiveFileExts);
  return true;
}

bool workshopToggleModels(Part* part, InputEvent event) {
  workshopResetActiveFileVars(); // Must reset before setting up new active exts
  wsActiveFileTag = steamws_itemTag::Model;
  wsActiveFileExts[wsExtEnum::ExtObj] = steamws_extObj;
  workshopPopulateFileVector(wsActiveFileTag, wsActiveFileExts);
  return true;
}

bool workshopToggleModelsDecorations(Part* part, InputEvent event) {
  workshopResetActiveFileVars(); // Must reset before setting up new active exts
  wsActiveFileTag = steamws_itemTag::ModelDecoration;
  wsActiveFileExts[wsExtEnum::ExtObj] = steamws_extObj;
  workshopPopulateFileVector(wsActiveFileTag, wsActiveFileExts);
  return true;
}

bool workshopToggleModpacks(Part* part, InputEvent event) {
  workshopResetActiveFileVars(); // Must reset before setting up new active exts
  wsActiveFileTag = steamws_itemTag::Mod;
  wsActiveFileExts[wsExtEnum::Dir] = steamws_extDir;
  workshopPopulateFileVector(wsActiveFileTag, wsActiveFileExts);
  return true;
}

bool workshopToggleModelsVehicles(Part* part, InputEvent event) {
  workshopResetActiveFileVars(); // Must reset before setting up new active exts
  wsActiveFileTag = steamws_itemTag::ModelVehicle;
  wsActiveFileExts[wsExtEnum::ExtObj] = steamws_extObj;
  workshopPopulateFileVector(wsActiveFileTag, wsActiveFileExts);
  return true;
}

bool workshopToggleSounds(Part* part, InputEvent event) {
  workshopResetActiveFileVars(); // Must reset before setting up new active exts
  wsActiveFileTag = steamws_itemTag::Sound;
  wsActiveFileExts[wsExtEnum::ExtOgg] = steamws_extOgg;
  workshopPopulateFileVector(wsActiveFileTag, wsActiveFileExts);
  return true;
}

bool workshopToggleSoundsEnvironment(Part* part, InputEvent event) {
  workshopResetActiveFileVars(); // Must reset before setting up new active exts
  wsActiveFileTag = steamws_itemTag::SoundEnvironment;
  wsActiveFileExts[wsExtEnum::ExtOgg] = steamws_extOgg;
  workshopPopulateFileVector(wsActiveFileTag, wsActiveFileExts);
  return true;
}

bool workshopToggleSoundsMusic(Part* part, InputEvent event) {
  workshopResetActiveFileVars(); // Must reset before setting up new active exts
  wsActiveFileTag = steamws_itemTag::SoundMusic;
  wsActiveFileExts[wsExtEnum::ExtOgg] = steamws_extOgg;
  workshopPopulateFileVector(wsActiveFileTag, wsActiveFileExts);
  return true;
}

bool workshopToggleTextures(Part* part, InputEvent event) {
  workshopResetActiveFileVars();
  wsActiveFileTag = steamws_itemTag::Texture;
  // wsActiveFileExts[wsExtEnum::ExtGif] = wsExtGif;
  // wsActiveFileExts[wsExtEnum::ExtJpg] = wsExtJpg;
  // wsActiveFileExts[wsExtEnum::ExtJpeg] = wsExtJpeg;
  wsActiveFileExts[wsExtEnum::ExtPng] = steamws_extPng;
  workshopPopulateFileVector(wsActiveFileTag, wsActiveFileExts);
  return true;
}

bool workshopToggleTexturesBuildings(Part* part, InputEvent event) {
  workshopResetActiveFileVars();
  wsActiveFileTag = steamws_itemTag::TextureBuilding;
  // wsActiveFileExts[wsExtEnum::ExtGif] = wsExtGif;
  // wsActiveFileExts[wsExtEnum::ExtJpg] = wsExtJpg;
  // wsActiveFileExts[wsExtEnum::ExtJpeg] = wsExtJpeg;
  wsActiveFileExts[wsExtEnum::ExtPng] = steamws_extPng;
  workshopPopulateFileVector(wsActiveFileTag, wsActiveFileExts);
  return true;
}

bool workshopToggleTexturesVehicles(Part* part, InputEvent event) {
  workshopResetActiveFileVars();
  wsActiveFileTag = steamws_itemTag::TextureVehicle;
  // wsActiveFileExts[wsExtEnum::ExtGif] = wsExtGif;
  // wsActiveFileExts[wsExtEnum::ExtJpg] = wsExtJpg;
  // wsActiveFileExts[wsExtEnum::ExtJpeg] = wsExtJpeg;
  wsActiveFileExts[wsExtEnum::ExtPng] = steamws_extPng;
  workshopPopulateFileVector(wsActiveFileTag, wsActiveFileExts);
  return true;
}

/*
bool workshopToggleBaseFiles(Part* part, InputEvent event) {
  if(getMod() == 0) {
    // If no mod is loaded, we never want to be able to toggle base files off
    wsBaseFiles = true;
    workshopPopulateFileVector(wsActiveFileTag, wsActiveFileExts);
    return true;
  }

  wsBaseFiles = !wsBaseFiles;
  workshopPopulateFileVector(wsActiveFileTag, wsActiveFileExts);
  return true;
}

bool workshopToggleModFiles(Part* part, InputEvent event) {
  if(getMod() == 0) {
    // If no mod is loaded, we never want to be able to toggle mod files on
    wsModFiles = false;
    return true;
  }

  wsModFiles = !wsModFiles;
  workshopPopulateFileVector(wsActiveFileTag, wsActiveFileExts);
  return true;
}
*/

bool workshopSetFilePath(Part* part, InputEvent event) {
  if(part == 0) {
    return false;
  }

  wsFileIndex = part->itemData;
  wsFilePath = files[wsFileIndex].fullPath();
  return true;
}

bool workshopStartUpload(Part* part, InputEvent event) {
  workshopResetSubmissionWindow(); // Refresh window for new submission

  WSFile file = files[wsFileIndex];
  wsSubName = file.name; // Set initial name to file.name field
  wsSubDesc = file.desc;
  wsSubNamePtr = strdup_s(wsSubName.c_str());
  wsSubDescPtr = strdup_s(wsSubDesc.c_str());
  wsSubWindow = true;
  return true;
}

bool workshopSendUpload(Part* part, InputEvent event) {
  if(wsFileIndex < 0 || wsFileIndex >= files.size()) {
    handleError("Unable to upload file; invalid index");
    return false;
  }

  WSFile file = files[wsFileIndex];

  if(file.dir.length() <= 0) {
    handleError("Unable to upload file; invalid directory in WSFile");
    return false;
  }

  if(file.name.length() <= 0) {
    handleError("Unable to upload file; invalid filename in WSFile");
    return false;
  }

  workshopSetBusyTxt("Uploading...");

  // Pull char* data into strings
  wsSubName = std::string(wsSubNamePtr);
  wsSubDesc = std::string(wsSubDescPtr);
  wsSubChange = std::string(wsSubChangePtr != 0 ? wsSubChangePtr : "");

  // TODO: Make this aware via a member of WSFile what type it actually is
  std::string primaryTag = steamws_getStringForTag(file.tag);
  SPDLOG_INFO("Primary tag for upload is {}", primaryTag);

  // TODO: Scan for preview image
  std::string previewImgPath = steamws_getImageForItem(file);

  std::array<std::string, steamws_itemField::Num_Fields> itemFields = {
    wsSubName,
    wsSubDesc,
    primaryTag,
    previewImgPath,
    "",
  };

  std::array<std::string, steamws_maxTags> itemTags ={
    primaryTag,
    "",
    "",
    "",
    "",
    "",
    "",
    ""
  };

  std::string changeTxt = wsSubChange;

  steamws_state_struct* state = steamws_GetStateStructPtr();
  state->setActiveItemFile(file);
  state->setActiveItemFields(itemFields);
  state->setActiveItemTags(itemTags);
  state->setActiveItemChangeTxt(changeTxt);
  state->setActions(steamws_action::CreateNewItem, steamws_action::UpdateItem);

  // Reset Submission Window
  workshopResetSubmissionWindow();

  return true;
}

std::string workshopGetStringForTab(WSTab tab) {
  switch(tab) {
    case WSTab::Home:
      return "Home";
    case WSTab::Search:
      return "Search";
    case WSTab::Browse:
      return "Browse";
    case WSTab::Upload:
      return "Upload";
    case WSTab::Subscribed:
      return "Subscribed";
    default:
      return wsTabUnk;
  }
}

bool workshopSetActiveTab(item newTab) {
  int oldTab = wsActiveTab;

  if(newTab >= 0 && newTab < WSTab::NumTabs) {
    wsActiveTab = (WSTab)newTab;
    if(wsActiveTab == WSTab::Upload) {
      if(wsActiveTab != oldTab) {
        workshopResetFileBrowser(); // Only reset file browser state if we're newly switching to the Upload tab
      }
      workshopToggleDesigns();
    }
  } else {
    wsActiveTab = WSTab::Browse;
  }
  return true;
}

bool workshopSetActiveTab(Part* part, InputEvent event) {
  if(part == 0) return false;

  int newTab = part->itemData;
  workshopSetActiveTab(newTab);
  return true;
}

bool workshopOpenOverlay(Part* part, InputEvent event) {
  if(part == 0) return false;

  steamws_OpenOverlayToWorkshop();
  return true;
}

bool workshopOpenMainMenu(Part* part, InputEvent event) {
  openMainMenu(part, event);
  return true;
}

bool workshopClearFiles(Part* part, InputEvent event) {
  steamws_clearAllLocal();
  return true;
}

Part* steamWorkshopSubWindow(float uiX) {
  float xPos = 0.0f;
  float yPos = 0.0f;
  float wsSubPanelHalf = wsSubSize.x/2.0f;
  float wsSubButtonX = 4.0f;
  vec2 subStart = vec2(uiX, uiGridSizeY)*wsPanelCoeff - wsSubSize*wsPanelCoeff;
  Part* subPanel = panel(subStart, wsSubSize);

  // Title
  r(subPanel, label(vec2(xPos, yPos), wsTitleSize, strdup_s("Submission Details:")));

  yPos += 1.0f+wsPad;

  // Textboxes
  r(subPanel, label(vec2(xPos, yPos), wsTxtSize, strdup_s("Name")));
  yPos += 1.0f;
  wsSubNameTB.text = &wsSubNamePtr;
  Part* tbName = r(subPanel, textBox(vec2(xPos+wsPad, yPos),
    vec2(wsSubPanelHalf-(wsPad*2.0f), 1.0f), &wsSubNameTB));
  tbName->onClick = workshopFocusNameBox;
  tbName->onCustom = workshopUnfocusTextBox;
  if(wsSubNamePtr == 0 || strlength(wsSubNamePtr) == 0) {
    Part* searchName = r(subPanel, label(vec2(xPos+wsPad, yPos+0.1f), wsTxtSize, strdup_s("Enter name...")));
    searchName->foregroundColor = PickerPalette::GrayDark;
  }
  yPos += 1.0f+wsPad;
  r(subPanel, label(vec2(xPos, yPos), wsTxtSize, strdup_s("Description")));
  yPos += 1.0f;
  wsSubDescTB.text = &wsSubDescPtr;
  Part* tbDesc = r(subPanel, textBox(vec2(xPos+wsPad, yPos),
    vec2(wsSubPanelHalf-(wsPad*2.0f), 1.0f), &wsSubDescTB));
  tbDesc->onClick = workshopFocusDescBox;
  tbDesc->onCustom = workshopUnfocusTextBox;
  if(wsSubDescPtr == 0 || strlength(wsSubDescPtr) == 0) {
    Part* searchDesc = r(subPanel, label(vec2(xPos+wsPad, yPos+0.1f), wsTxtSize, strdup_s("Enter description...")));
    searchDesc->foregroundColor = PickerPalette::GrayDark;
  }
  yPos += 1.0f+wsPad;
  r(subPanel, label(vec2(xPos, yPos), wsTxtSize, strdup_s("Change Text")));
  yPos += 1.0f;
  wsSubChangeTB.text = &wsSubChangePtr;
  Part* tbChange = r(subPanel, textBox(vec2(xPos+wsPad, yPos),
    vec2(wsSubPanelHalf-(wsPad*2.0f), 1.0f), &wsSubChangeTB));
  tbChange->onClick = workshopFocusChangeBox;
  tbChange->onCustom = workshopUnfocusTextBox;
  if(wsSubChangePtr == 0 || strlength(wsSubChangePtr) == 0) {
    Part* searchChange = r(subPanel, label(vec2(xPos+wsPad, yPos+0.1f), wsTxtSize, strdup_s("Enter change...")));
    searchChange->foregroundColor = PickerPalette::GrayDark;
  }

  // Nav buttons
  r(subPanel, buttonCenter(vec2(0.0f, wsSubSize.y-1.0f), vec2(wsSubButtonX, 1.0f), strdup_s(wsButtonBackStr.c_str()), workshopResetSubmissionWindow));
  r(subPanel, buttonCenter(wsSubSize-vec2(wsSubButtonX, 1.0f), vec2(wsSubButtonX, 1.0f), strdup_s(wsButtonUploadStr.c_str()), workshopSendUpload));

  return subPanel;
}

bool openDesignInWorkshop(Part* part, InputEvent event) {
  setMenuMode(MenuMode::SteamWorkshop);
  workshopToggleDesigns();
  workshopSetActiveTab(WSTab::Upload);

  if (wsFileSearchTxt != 0) {
    free(wsFileSearchTxt);
    wsFileSearchTxt = 0;
  }

  if (part->itemData <= 0) {
    return true;
  }

  auto design = getDesign(part->itemData);

  if (design->name == 0) {
    return true;
  }

  std::string name = design->name;
  name = "designs/" + name + "/design.design";

  wsFilePath = lookupFile(name, _lookupExcludeBase);
  wsFileIndex = -1;

  return true;
}

Part* steamWorkshop(float aspectRatio) {
  float uiX = uiGridSizeX * aspectRatio;
  float xPos = 0.0f;
  float yPos = 0.0f;

  // If we're actively doing something workshop related
  if(steamws_IsBusy()) {
    // Show status panel
    vec2 statusStart = vec2(uiX, uiGridSizeY)*wsPanelCoeff - wsStatusSize*wsPanelCoeff;
    Part* statusPanel = panel(statusStart, wsStatusSize);
    r(statusPanel, labelCenter(vec2(0.0f, (wsStatusSize.y*0.5f)-1.0f), vec2(wsStatusSize.x, 1.0f), strdup_s(wsStatusTxt.c_str())));

    return statusPanel;
  }

  // If we're handling an item submission, but haven't sent it yet
  if(wsSubWindow) {
    return steamWorkshopSubWindow(uiX);
  }

  vec2 resultStart = vec2(uiX, uiGridSizeY)*wsPanelCoeff - wsSize*wsPanelCoeff;

  // Main panel
  Part* result = panel(resultStart, wsSize);
  result->padding = wsPad;
  result->flags |= _partLowered;

  // Title
  r(result, label(vec2(0, 0), wsTitleSize, strdup_s("Steam Workshop")));

  // Close button
  r(result, button(vec2(wsWidth-1, 0), iconX, workshopOpenMainMenu));

  yPos += wsTitleSize + wsPad;

  // Tabs
  float tabBtnXSize = wsWidth / WSTab::NumTabs;
  for(int i = 0; i < WSTab::NumTabs; i++) {
    Part* tabBtn = button(vec2(xPos+wsPad, yPos), vec2(tabBtnXSize-(wsPad*2.0f), 1.5f), strdup_s(workshopGetStringForTab((WSTab)i).c_str()), workshopSetActiveTab);
    tabBtn->itemData = i;
    tabBtn->flags |= _partAlignCenter;
    if(i == wsActiveTab) {
      tabBtn->flags |= _partRaised;
    }
    r(result, tabBtn);
    xPos += tabBtnXSize;
  }

  xPos = 0.0f;
  yPos += 1.5f;
  //yPos += (1.0f + wsPad)*2.0f;

  vec2 browserSize = vec2(wsSize.x - (wsPad*2.0f), wsSize.y - yPos - (wsPad*2.0f));
  Part* browserPanel = panel(vec2(xPos, yPos), browserSize);
  browserPanel->padding = 0.5f;
  r(result, browserPanel);

  if(wsActiveTab == WSTab::Upload) {
    float eleSizeX = browserSize.x - 1;
    float top;

    r(browserPanel, labelCenter(vec2(0, 0), vec2(eleSizeX, wsTitleSize),
      strdup_s("Select File to Upload")));
    top = 4.0f;

    // Checkboxes to filter by directory
    float pad = 0.25f;
    float eleWidth = eleSizeX*0.166f;
    float halfWidth = eleSizeX*.55f;
    float bx = eleSizeX-halfWidth-pad;
    float sclTitle = 0.8f;
    float sclTxt = 0.65f;
    float rowCoeff = 1.0f;

    // Which directory to look in
    /*
    r(browserPanel, label(vec2(0.0f, 4.125f), sclTitle,
      strdup_s("Reading files from")));
    Part* btnBase = button(vec2(eleWidth*rowCoeff, 3.75f), wsBaseFiles ? iconCheck : iconNull,
      vec2(eleWidth, sclTxt), strdup_s(wsBaseStr.c_str()), workshopToggleBaseFiles, 0);
    btnBase->flags |= _partHighlight;
    r(browserPanel, btnBase);
    if(wsActiveFileTag != steamws_itemTag::Mod) { // Can mods even have sub mods as of yet? - 10/29/2020 supersoup
      const char* modName = getMod();
      if(modName != NULL) { // Don't show mod file toggle if no mod loaded
        Part* btnMod = button(vec2(eleWidth*rowCoeff, 4.5f), wsModFiles ? iconCheck : iconNull,
          vec2(eleWidth, sclTxt), strdup_s(wsModStr.c_str()), workshopToggleModFiles, 0);
        btnMod->flags |= _partHighlight;
        r(browserPanel, btnMod);
      }
    }
    */

    // Which file types to consider
    rowCoeff = 2.0f;
    r(browserPanel, button(vec2(0, 2.f),
      wsActiveFileTag == steamws_itemTag::DesignLegacy ? iconCheck : iconNull,
      vec2(eleWidth, wsTxtSize), strdup_s("Designs"),
      workshopToggleDesigns, 0));
    r(browserPanel, button(vec2(eleWidth+1, 2.f),
      wsActiveFileTag == steamws_itemTag::Mod ? iconCheck : iconNull,
      vec2(eleWidth, wsTxtSize), strdup_s("Modpacks"),
      workshopToggleModpacks, 0));

    //top += 1.75;

    wsFileBrowserTB.text = &wsFileSearchTxt;
    float searchX = eleSizeX*.5f;
    Part* tb = r(browserPanel, textBox(vec2(searchX, 2),
      vec2(eleSizeX*.5f, 1.5), &wsFileBrowserTB));
    tb->onClick = workshopFocusSearchBox;
    tb->onCustom = workshopUnfocusTextBox;
    if(wsFileSearchTxt == 0 || strlength(wsFileSearchTxt) == 0) {
      Part* search = r(browserPanel, label(vec2(searchX + 0.25, 2.25), 1, strdup_s("Search")));
      search->foregroundColor = PickerPalette::GrayDark;
    }

    vec2 scrollSize = browserSize - vec2(1, top+2.5);
    Part* scroll = scrollbox(vec2(0, 0), scrollSize);
    float sy = 0;

    for(int f = 0; f < files.size(); f++) {
      std::string search = wsFileSearchTxt;

      if (wsFileIndex < 0 && files[f].fullPath() == wsFilePath) {
        wsFileIndex = f;
      }

      if(search.length() > 0) {
        std::string fileStr = files[f].name;
        std::string searchStr = search;
        std::transform(fileStr.begin(), fileStr.end(), fileStr.begin(),
          [](unsigned char c) { return std::tolower(c); });
        std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(),
          [](unsigned char c) { return std::tolower(c); });
        if(fileStr.find(searchStr) == std::string::npos) continue;
      }


      Part* fileButt = button(vec2(0, sy), vec2(eleSizeX*.5f-1, 1),
          strdup_s(files[f].name.c_str()),
          /*
          wsActiveFileTag != steamws_itemTag::Mod 
          ? sprintf_o("%s %s", files[f].desc.c_str(), files[f].name.c_str())
          : sprintf_o("%s", files[f].name.c_str()), // We don't want the desc when searching for mods
          */
          workshopSetFilePath);
      fileButt->itemData = f;
      if(wsFileIndex == f) {
        fileButt->flags |= _partHighlight;
      }
      r(scroll, fileButt);
      sy++;
    }

    float bottomButtonX = 4.0f;
    Part* scrollFrame = scrollboxFrame(vec2(0, top), scrollSize,
      &wsFileBrowserScroll, scroll);
    r(browserPanel, scrollFrame);

    /*
    Part* backButt = r(browserPanel, button(
      vec2(0.0f, browserSize.y-2.0f),
      vec2(4.0f, 1.0f),
      strdup_s(wsButtonBackStr.c_str()), workshopResetFileBrowser));
    backButt->flags |= _partAlignCenter;
    */

    if(wsFilePath.length() > 0) {
      Part* uploadButt = r(browserPanel, button(
        vec2(browserSize.x-bottomButtonX-(wsPad*4.0f), browserSize.y-2.0f),
        vec2(bottomButtonX, 1.0f),
        strdup_s(wsButtonUploadStr.c_str()), workshopStartUpload));
      uploadButt->flags |= _partAlignCenter;

      std::string previewImgPath = steamws_getImageForItem(files[wsFileIndex]);
      float y;
      Part* img = r(browserPanel, image(
            vec2(scrollSize.x*.5f, top+1),
            scrollSize.x*.5f-1, sprintf_o(previewImgPath.c_str()), &y));
    }

  } else {
    r(browserPanel, labelCenter(vec2(xPos, yPos), vec2(wsWidth, wsTxtSize),
          strdup_s(wsBrowseStr.c_str())));
    yPos += 2.0f + wsPad;

    float buttWidth = 12;
    float buttCenter = xPos + .5f*(wsWidth-buttWidth-wsPad*2);
    Part* butt = r(browserPanel, button(vec2(buttCenter, yPos),
          vec2(buttWidth+wsPad*2, wsTxtSize*3+wsPad*2),
          strdup_s(""), workshopOpenOverlay));
    butt->flags |= _partIsPanel;
    butt->padding = wsPad;
    yPos += 1.0f + wsTxtSize*3;

    float by = 0;
    float icoCenter = .5f*(buttWidth-wsTxtSize);
    r(butt, icon(vec2(icoCenter,by), vec2(wsTxtSize, wsTxtSize), iconSteam));
    by += wsTxtSize;
    r(butt, labelCenter(vec2(0.5,by), vec2(buttWidth-1, wsTxtSize),
          strdup_s(wsClickBrowseStr1.c_str())));
    by += wsTxtSize;
    r(butt, labelCenter(vec2(0.5,by), vec2(buttWidth-1, wsTxtSize),
          strdup_s(wsClickBrowseStr2.c_str())));
    by += wsTxtSize;
    yPos += 1.0f + wsTxtSize + wsPad;

    Part* buttClearAll = button(vec2(buttCenter, yPos),
      vec2(buttWidth+1.15f, wsTxtSize),
      strdup_s("  Clear local NewCity Workshop files"), workshopClearFiles);
    Part* buttClearXLeft = r(buttClearAll, icon(vec2(buttCenter, yPos), vec2(wsTxtSize, wsTxtSize), iconNo));
    Part* buttClearXRight = r(buttClearAll, icon(vec2(buttCenter+buttWidth+wsPad, yPos), vec2(wsTxtSize, wsTxtSize), iconNo));
    r(browserPanel, buttClearAll);
  }

  return result;
}

