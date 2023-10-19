//-----------------------------------------------------------------------------
// steamws_core - Contains all the NewCity specific API hooks to 
// interact with the Steam Workshop and UGC
//-----------------------------------------------------------------------------

#include "../platform/file.hpp"
#include "steamwrapper.hpp"
#include "steamws_core.hpp"
#include <stdlib.h>

#ifdef __linux__
#include <linux/limits.h>
#elif WIN32
#include <Windows.h>
#endif


// The steamws_core API should have everything needed for NewCity Steam Workshop
// The only publicly exposed functions should be the steamws_core API functions
// The steamws_core class and associated methods shouldn't ever need to be directly called from outside

const std::string defaultPreviewImgPath = "";
const double wsTimeout = 10000.0; // Millieconds

void steamws_CreateNewItem();
void steamws_UpdateItem(std::array<std::string, steamws_itemField::Num_Fields> itemFields, std::array<std::string, steamws_maxTags> itemTags, std::string changeTxt = "");
bool steamws_StageItemData(UGCUpdateHandle_t handle, PublishedFileId_t id);
void steamws_SubscribeToItem(PublishedFileId_t id);
void steamws_UnsubscribeFromItem(PublishedFileId_t id);

class steamws_core {
private:
  // CreateNewItem
  void OnCreateNewItem(CreateItemResult_t *callback, bool failed);
  CCallResult<steamws_core, CreateItemResult_t> CreateNewItemCallResult;
  // DownloadSubscribedItem
  void OnDownloadSubscribedItem(DownloadItemResult_t *callback, bool failed);
  CCallResult<steamws_core, DownloadItemResult_t> DownloadSubscribedItemResult;
  // InstallSubscribedItem
  void OnInstallSubscribedItem(ItemInstalled_t *callback, bool failed);
  CCallResult<steamws_core, ItemInstalled_t> InstallSubscribedItemResult;
  // ReceiveUGCQuery
  void OnReceiveUGCQuery(SteamUGCQueryCompleted_t *callback, bool failed);
  CCallResult<steamws_core, SteamUGCQueryCompleted_t> ReceiveUGCQueryResult;
  // SubscribeToItem
  void OnSubscribeToItem(RemoteStorageSubscribePublishedFileResult_t *callback, bool failed);
  CCallResult<steamws_core, RemoteStorageSubscribePublishedFileResult_t> SubscribeToItemCallResult;
  // UnsubscribeFromItem
  void OnUnsubscribeFromItem(RemoteStorageUnsubscribePublishedFileResult_t *callback, bool failed);
  CCallResult<steamws_core, RemoteStorageUnsubscribePublishedFileResult_t> UnsubscribeFromItemCallResult;
  // UpdateItem
  void OnUpdateItem(SubmitItemUpdateResult_t *callback, bool failed);
  CCallResult<steamws_core, SubmitItemUpdateResult_t> UpdateItemCallResult;
  
  // Helper methods
  void PrepareMoveWorkshopItems(PublishedFileId_t *items, uint32_t numItems);

  // Instance vars
  bool _busy;
  bool _failed;
  bool _queryReturned;

  double _timer;
  SteamAPICall_t _downloadResult;
  SteamAPICall_t _installResult;

public:
  steamws_core();
  void init() {
    // Initialize instance vars
    this->reset(); 
    _downloadResult = k_uAPICallInvalid;
    _installResult = k_uAPICallInvalid;

    // Set callbacks for downloading and installing subscribed items
    DownloadSubscribedItemResult.Set(_downloadResult, this, &steamws_core::OnDownloadSubscribedItem);
    InstallSubscribedItemResult.Set(_installResult, this, &steamws_core::OnInstallSubscribedItem);
  }
  void reset() { 
    SPDLOG_INFO("Resetting Steam Workshop ... ");
    this->clearBusy();
    this->clearFailed();
    this->clearQueryReturned();
    _timer = 0.0;
  }
  bool failed() { return _failed; }
  void setFailed() { _failed = true; }
  void clearFailed() { _failed = false; }
  bool busy() { return _busy; }
  void setBusy() { _busy = true; }
  void clearBusy() { _busy = false; }
  bool queryReturned() { return _queryReturned; }
  void setQueryReturned() { _queryReturned = true; }
  void clearQueryReturned() { _queryReturned = false; }
  double timer() { return _timer; }
  void addToTimer(double time) { _timer += time; }

  void CreateNewItem();
  void InitSubscribedItems(PublishedFileId_t *items, uint32_t numItems);
  void MoveSubbedItemsToGameDir();
  void SubscribeToItem(PublishedFileId_t id);
  void UnsubscribeFromItem(PublishedFileId_t id);
  void UpdateItem(std::array<std::string, steamws_itemField::Num_Fields> itemFields, std::array<std::string, steamws_maxTags> itemTags, std::string changeTxt = "");

};

static steamws_core* steamws_core_obj;
static steamws_state_struct steamws_state; // Static variable for state

//------------------------------------------------------------------------------
// steamws_core class method definitions
//------------------------------------------------------------------------------
steamws_core::steamws_core() {
  this->clearBusy();
  this->clearFailed();
  this->clearQueryReturned();
  _timer = 0.0;
}

void steamws_core::OnCreateNewItem(CreateItemResult_t *callback, bool failed) {
  if(callback == 0) {
    SPDLOG_ERROR("OnCreateNewItem callback was null");
    this->setFailed();
    return;
  }

  if(failed) {
    SPDLOG_ERROR("CreateNewItem reported failed in OnCreateNewItem");
    this->setFailed();
    return;
  }

  EResult result = callback->m_eResult;
  if(result != k_EResultOK) {
    SPDLOG_ERROR("OnCreateNewItem result was not OK ({})", result);
    this->setFailed();
    return;
  }

  PublishedFileId_t id = callback->m_nPublishedFileId;
  SPDLOG_INFO("Item successfully created with id {}", id);
  steamws_state.setActiveItem(id);

  if(callback->m_bUserNeedsToAcceptWorkshopLegalAgreement) {
    // Url per https://partner.steamgames.com/doc/features/workshop/implementation#Legal
    std::string itemUrl = "steam://url/CommunityFilePage/" + std::to_string(id);
    SPDLOG_INFO("User needs to accept Steam Workshop legal agreement");
    steam_openOverlayWebPage(itemUrl);
  }

  // Completed action, no longer busy
  this->clearBusy();
}

void steamws_core::OnDownloadSubscribedItem(DownloadItemResult_t *callback, bool failed) {
  if(callback == 0) {
    SPDLOG_ERROR("OnDownloadSubscribedItem callback was null");
    // Don't set failed, this may not be directly in response to an invoked steamws_action
    return;
  }

  if(failed) {
    SPDLOG_ERROR("DownloadSubscribedItem reported failed in OnDownloadSubscribedItem");
    // Don't set failed, this may not be directly in response to an invoked steamws_action
    return;
  }

  AppId_t app = callback->m_unAppID;
  if(app != NewCityAppID) {
    // Quietly ignore; not an item we care about
    return;
  }

  EResult result = callback->m_eResult;
  if(result != k_EResultOK) {
    SPDLOG_ERROR("OnDownloadSubscribedItem result was not OK ({})", result);
    // Don't set failed, this may not be directly in response to an invoked steamws_action
    return;
  }

  PublishedFileId_t id = callback->m_nPublishedFileId;
  if(id == k_PublishedFileIdInvalid) {
    SPDLOG_ERROR("Somehow downloaded an invalid or null Steam Workshop item, according to the id");
    // Don't set failed, this may not be directly in response to an invoked steamws_action
    return;
  }

  SPDLOG_INFO("Successfully downloaded Steam Workshop item {}", id);
}

void steamws_core::OnInstallSubscribedItem(ItemInstalled_t *callback, bool failed) {
  if(callback == 0) {
    SPDLOG_ERROR("OnInstallSubscribedItem callback was null");
    // Don't set failed, this may not be directly in response to an invoked steamws_action
    return;
  }

  if(failed) {
    SPDLOG_ERROR("InstallSubscribedItem reported failed in OnInstallSubscribedItem");
    // Don't set failed, this may not be directly in response to an invoked steamws_action
    return;
  }

  AppId_t app = callback->m_unAppID;
  if(app != NewCityAppID) {
    // Quietly ignore; not an item we care about
    return;
  }

  PublishedFileId_t id = callback->m_nPublishedFileId;
  if(id == k_PublishedFileIdInvalid) {
    SPDLOG_ERROR("Somehow installed an invalid or null Steam Workshop item, according to the id");
    // Don't set failed, this mayh not be directly in response to an invoked steamws_action
    return;
  }

  SPDLOG_INFO("Successfully installed Steam Workshop item {}", id);
}

void steamws_core::OnReceiveUGCQuery(SteamUGCQueryCompleted_t *callback, bool failed) {
  if(steamws_state.getActiveAction() == steamws_action::AwaitUGCQuery) {
    // Callback was invoked, the action is complete
    steamws_state.clearActiveAction();
    this->clearBusy();
  }

  if(callback == 0) {
    SPDLOG_ERROR("OnReceiveUGCQuery callback was null");
    this->setFailed();
    return;
  }

  if(failed) {
    SPDLOG_ERROR("ReceiveUGCQuery reported failed in OnReceiveUGCQuery");
    this->setFailed();
    return;
  }

  EResult result = callback->m_eResult;
  if(result != k_EResultOK) {
    SPDLOG_ERROR("OnReceiveUGCQuery result was not OK ({})", result);
    this->setFailed();
    return;
  }

  SPDLOG_INFO("Received UGCQuery results for {} items", callback->m_unNumResultsReturned);

  // Store handle to UGCQuery in the steamws_state struct for use elsewhere
  steamws_state.setActiveQueryHandle(callback->m_handle, callback->m_unNumResultsReturned);
  this->setQueryReturned();
}

void steamws_core::OnSubscribeToItem(RemoteStorageSubscribePublishedFileResult_t *callback, bool failed) {
  if(callback == 0) {
    SPDLOG_ERROR("OnSubscribeToItem callback was null");
    this->setFailed();
    return;
  }

  if(failed) {
    SPDLOG_ERROR("SubscribeToItem reported failed in OnSubscribeToItem");
    this->setFailed();
    return;
  }

  EResult result = callback->m_eResult;
  if(result != k_EResultOK) {
    SPDLOG_ERROR("OnSubscribeToItem result was not OK ({})", result);
    this->setFailed();
    return;
  }

  SPDLOG_INFO("User successfully subscribed to item ({})", callback->m_nPublishedFileId);

  // Completed action, no longer busy
  this->clearBusy();
}

void steamws_core::OnUnsubscribeFromItem(RemoteStorageUnsubscribePublishedFileResult_t *callback, bool failed) {
  if(callback == 0) {
    SPDLOG_ERROR("OnUnsubscribeFromItem callback was null");
    this->setFailed();
    return;
  }

  if(failed) {
    SPDLOG_ERROR("UnsubscribeFromItem reported failed in OnUnsubscribeFromItem");
    this->setFailed();
    return;
  }

  EResult result = callback->m_eResult;
  if(result != k_EResultOK) {
    SPDLOG_ERROR("OnUnsubscribeFromItem result was not OK ({})", result);
    this->setFailed();
    return;
  }

  SPDLOG_INFO("User successfully unsubscribed from item ({})", callback->m_nPublishedFileId);

  // Completed action, no longer busy
  this->clearBusy();
}

void steamws_core::OnUpdateItem(SubmitItemUpdateResult_t *callback, bool failed) {
  if(callback == 0) {
    SPDLOG_ERROR("OnUpdateItem callback was null");
    this->setFailed();
    return;
  }

  if(failed) {
    SPDLOG_ERROR("UpdateItem reported failed in OnUpdateItem");
    this->setFailed();
    return;
  }

  EResult result = callback->m_eResult;
  if(result != k_EResultOK) {
    SPDLOG_ERROR("OnSubmitItemUpdate result was not OK ({})", result);
    this->setFailed();
    return;
  }

  SPDLOG_INFO("Update for item {} successfully submitted to Steam Workshop", steamws_state.getActiveItem());

  if(callback->m_bUserNeedsToAcceptWorkshopLegalAgreement) {
    // Url per https://partner.steamgames.com/doc/features/workshop/implementation#Legal
    std::string itemUrl = "steam://url/CommunityFilePage/" + std::to_string(steamws_state.getActiveItem());
    SPDLOG_INFO("User needs to accept Steam Workshop legal agreement");
    steam_openOverlayWebPage(itemUrl);
  }

  // Completed action, no longer busy
  this->clearBusy();
  // Reset steamws_state because we finished updating an item
  steamws_state.resetSteamWSState();
}

void steamws_core::CreateNewItem() {
  if(steamws_state.getActiveItem() != k_PublishedFileIdInvalid) {
    SPDLOG_ERROR("Tried to create a new Steam Workshop item when an item was already active");
    // Didn't fail necessarily; at least, we don't want to reset the steamws_state
    return;
  }

  SPDLOG_INFO("Creating new Steam Workshop item");

  // Beginning task, now busy
  this->setBusy();

  // Default to Community type file when creating item from ingame
  SteamAPICall_t createCallResult = steamws_itemCreate(k_EWorkshopFileTypeCommunity);
  // Set SteamAPI CallResult callback
  CreateNewItemCallResult.Set(createCallResult, this, &steamws_core::OnCreateNewItem);
}

void steamws_core::PrepareMoveWorkshopItems(PublishedFileId_t *items, uint32_t numItems) {
  if(items == 0) {
    SPDLOG_ERROR("Null Steam Workshop item array passed; can't move items to game dir");
    this->setFailed();
    return;
  }

  if(numItems == 0) {
    SPDLOG_INFO("No files to be moved from Workshop to Game dir");
    this->setFailed();
    return;
  }

  SPDLOG_INFO("Searching for Steam Workshop items to be moved to NewCity directory...");

  std::string workshopNewCityContentPath = steamws_contentPath + "/" + std::to_string(NewCityAppID);
  vector<std::string> workshopItemSubfolders = getSubfoldersInDir(workshopNewCityContentPath.c_str(), false);
  uint32_t numSubfolders = workshopItemSubfolders.size();

  if(numSubfolders > 0) {
    std::string workshopItemStr = "Item(s) found (by folder): ";
    for(int i = 0; i < numSubfolders; i++) {
      if(i < numSubfolders - 1) {
        workshopItemStr += workshopItemSubfolders[i] + ", ";
      } else {
        workshopItemStr += workshopItemSubfolders[i];
      }
    }
    SPDLOG_INFO(workshopItemStr);
  } else {
    SPDLOG_INFO("No items found in Steam Workshop content folder for NewCity");
    this->setFailed();
    return;
  }

  // Start the query to get subscribed item types
  UGCQueryHandle_t subItemQuery = steamws_queryUGCDetailsRequest(items, numItems);
  SteamAPICall_t queryCallResult = steamws_querySendUGCRequest(subItemQuery);
  // Set SteamAPI CallResult callback
  ReceiveUGCQueryResult.Set(queryCallResult, this, &steamws_core::OnReceiveUGCQuery);

  // Update the active action to await the UGCQuery, then to move the items
  steamws_state.setActions(steamws_action::AwaitUGCQuery, steamws_action::MoveSubbedItemsToGameDir);
}

void steamws_core::MoveSubbedItemsToGameDir() {

  SPDLOG_INFO("Moving Steam Workshop items to NewCity directory...");

  std::string workshopNewCityContentPath = steamws_contentPath + "/" + std::to_string(NewCityAppID);
  vector<std::string> workshopItemSubfolders = getSubfoldersInDir(workshopNewCityContentPath.c_str(), false);
  uint32_t numSubfolders = workshopItemSubfolders.size();

  if(numSubfolders > 0) {
    std::string workshopItemStr = "Item(s) found (by folder): ";
    for(int i = 0; i < numSubfolders; i++) {
      if(i < numSubfolders - 1) {
        workshopItemStr += workshopItemSubfolders[i] + ", ";
      } else {
        workshopItemStr += workshopItemSubfolders[i];
      }
    }
    SPDLOG_INFO(workshopItemStr);
  } else {
    SPDLOG_INFO("No items found in Steam Workshop content folder for NewCity");
    this->setFailed();
    return;
  }

  UGCQueryHandle_t returnedHandle = steamws_state.getActiveQueryHandle();
  if(returnedHandle == k_uAPICallInvalid) {
    SPDLOG_ERROR("Error with active UGCQuery result handle; cannot determine tagged type of Steam Workshop content");
    this->setFailed();
    return;
  }

  uint32_t returnedItems = steamws_state.getActiveQueryNumResults();
  if(returnedItems == 0) {
    SPDLOG_INFO("UGCQuery for moving subbed items returned 0 items; won't be able to get necessary data to move");
    this->setFailed();
    return;
  }

  // Temporary storage for SteamUGC item details
  SteamUGCDetails_t itemDetails;

  // Get the returned item ids
  PublishedFileId_t *items = new PublishedFileId_t[returnedItems];
  steamws_itemTag *itemTags = new steamws_itemTag[returnedItems];
  // Freeing itemNames as array throws a fatal error, so it's a vector
  std::vector<std::string> itemNames;
  std::vector<std::string>().swap(itemNames);

  for(int r = 0; r < returnedItems; r++) {
    if(!steamws_queryGetUGCResult(returnedHandle, r, &itemDetails)) {
      SPDLOG_ERROR("Error fetching UGCQuery results for index {}", r);
      continue;
    }
    items[r] = itemDetails.m_nPublishedFileId;

    // Get tag
    std::string tempTagStr = itemDetails.m_rgchTags;
    size_t firstCommaPos = tempTagStr.find_first_of(',');
    itemTags[r] = steamws_getTagForString(tempTagStr.substr(0, firstCommaPos));

    // Get human-readable name
    std::string tempNameStr = itemDetails.m_rgchTitle;
    itemNames.push_back(tempNameStr);
  }

  // Iterate through the subfolders and compare their names to the known subscribed item ids.
  // If one is found, move any items within the Steam staged subfolder for the item to the 
  // appropriate game directory workshop folder according to its tag.
  steamws_pathEnum tagPath = steamws_pathEnum::PathRoot;
  std::string sourceDir = "";
  std::string localDirForTag = "";
  for(int f = 0; f < numSubfolders; f++) {
    for(int i = 0; i < returnedItems; i++) {
      if(!strcmp(workshopItemSubfolders[f].c_str(), std::to_string(items[i]).c_str())) {
        tagPath = steamws_getLocalPathEnumFromTag(itemTags[i]);
        sourceDir = workshopNewCityContentPath + "/" + workshopItemSubfolders[f];
        localDirForTag = steamws_getLocalRelativePath(tagPath) + "/" + itemNames[i];
        if(!copyDir(sourceDir, localDirForTag, std::experimental::filesystem::copy_options::overwrite_existing 
          | std::experimental::filesystem::copy_options::recursive)) {
          SPDLOG_ERROR("Error copying Steam Workshop item dir from source ({}) to game dir staging ({})", sourceDir, localDirForTag);
        } else {
          SPDLOG_INFO("Successfully copied Steam Workshop item dir from source ({}) to game dir staging ({})", sourceDir, localDirForTag);
        }
      }
    }
  }

  // Release query handle when done
  steamws_queryReleaseUGCRequest(returnedHandle);

  // Clean up arrays
  free(items);
  free(itemTags);
  std::vector<std::string>().swap(itemNames);

  // Completed action, no longer busy
  this->clearBusy();
}

void steamws_core::InitSubscribedItems(PublishedFileId_t *items, uint32_t numItems) {
  // Beginning task, now busy
  this->setBusy();

  // List subscribed NewCity items
  std::string itemStr = "";
  for(int i = 0; i < numItems; i++) {
    if(i < numItems-1) {
      itemStr += std::to_string(items[i]) + ", ";
    } else {
      itemStr += std::to_string(items[i]);
    }
  }
  SPDLOG_INFO("User is subscribed to {} NewCity Steam Workshop item(s): {}", numItems, itemStr);

  // Must verify the items first, then the UGCQuery result will be used to move the items to the appropriate folder
  PrepareMoveWorkshopItems(items, numItems);

  // Busy will be unset in the function that moves workshop items to the game directory
}

void steamws_core::SubscribeToItem(PublishedFileId_t id) {
  if(id == k_PublishedFileIdInvalid) {
    SPDLOG_ERROR("Tried to subscribe to Steam Workshop item with invalid PublishedFileId");
    this->setFailed();
    return;
  }

  SPDLOG_INFO("Attempting to subscribe to Steam Workshop item ({})", id);

  // Beginning task, now busy
  this->setBusy();

  SteamAPICall_t subscribeItemCallResult = steamws_itemSubscribe(id);
  // Set SteamAPI CallResult callback
  SubscribeToItemCallResult.Set(subscribeItemCallResult, this, &steamws_core::OnSubscribeToItem);
}

void steamws_core::UnsubscribeFromItem(PublishedFileId_t id) {
  if(id == k_PublishedFileIdInvalid) {
    SPDLOG_ERROR("Tried to unsubscribe from Steam Workshop item with invalid PublishedFileId");
    this->setFailed();
    return;
  }

  SPDLOG_INFO("Attempting to unsubscribe from Steam Workshop item ({})", id);

  // Beginning task, now busy
  this->setBusy();

  SteamAPICall_t unsubscribeItemCallResult = steamws_itemUnsubscribe(id);
  // Set SteamAPI CallResult callback
  UnsubscribeFromItemCallResult.Set(unsubscribeItemCallResult, this, &steamws_core::OnUnsubscribeFromItem);
}

void steamws_core::UpdateItem(std::array<std::string, steamws_itemField::Num_Fields> itemFields, std::array<std::string, steamws_maxTags> itemTags, std::string changeTxt) {
  SPDLOG_INFO("Starting a Steam Workshop item update");

  PublishedFileId_t id = steamws_state.getActiveItem();
  if(id == k_PublishedFileIdInvalid) {
    SPDLOG_ERROR("Tried to update Steam Workshop item when id was invalid");
    this->setFailed();
    return;
  }

  UGCUpdateHandle_t handle = steamws_itemStartUpdate(id);
  bool error = false;

  for(int i = 0; i < steamws_itemField::Num_Fields; i++) {
    if(error) {
      break;
    }

    // Braces for cases needed because of strange init error on previewPath var
    switch(i) {
      case steamws_itemField::Title:
      {
        if(!steamws_itemSetTitle(handle, itemFields[i])) {
          SPDLOG_ERROR("Error setting title for Steam Workshop item update");
          error = true;
        }
        break;
      }
      case steamws_itemField::Desc:
      {
        if(!steamws_itemSetDescription(handle, itemFields[i])) {
          SPDLOG_ERROR("Error setting description for Steam Workshop item update");
          error = true;
        }
        break;
      }
      case steamws_itemField::PrimaryTag:
      {
        // TODO: Support multiple tags
        const char* strings[1];
        strings[0] = itemTags[0].c_str();
        SteamParamStringArray_t *mainTag = new SteamParamStringArray_t();
        mainTag->m_ppStrings = strings;
        mainTag->m_nNumStrings = 1;

        if(!steamws_itemSetTags(handle, mainTag)) {
          SPDLOG_ERROR("Error setting param string arr tag(s) for Steam Workshop item update");
          error = true;
          break;
        }

        for(int t = 0; t < steamws_maxTags; t++) {
          if(error) {
            break;
          }

          if(itemTags[i].length() > 0) {
            if(!steamws_itemAddKeyValueTag(handle, steamws_tagKey, itemTags[i])) {
              SPDLOG_ERROR("Error setting key value tag for Steam Workshop item update");
              error = true;
            }
          }
        }
        break;
      }
      case steamws_itemField::PreviewImgPath:
      {
        std::string previewPath = "";

        // To resolve absolute file path Linux uses realpath() and Windows uses _fullpath
        if(itemFields[i].length() <= 0) {
          SPDLOG_INFO("No preview img path specified for Steam Workshop item update, setting default...");
          previewPath = getAbsolutePath(defaultPreviewImgPath);
        } else if(!fileExists(itemFields[i].c_str())) {
          SPDLOG_INFO("Preview img path ({}) does not exist or is not a path to a valid file, setting default...", itemFields[i]);
          previewPath = getAbsolutePath(defaultPreviewImgPath);
        } else {
          SPDLOG_INFO("Preview img path ({}) verified to exist", itemFields[i]);
          previewPath = getAbsolutePath(itemFields[i]);
        }

        if(!steamws_itemSetPreviewImg(handle, previewPath)) {
          SPDLOG_ERROR("Error setting preview img for Steam Workshop item update");
          error = true;
        }

        // Free char array memory
        // free(pathBuffer);
        break;
      }
      case steamws_itemField::Metadata:
      {
        if(!steamws_itemSetMetadata(handle, itemFields[i])) {
          SPDLOG_ERROR("Error setting metadata for Steam Workshop item update");
          error = true;
        }
        break;
      }
    }
  }

  if(!steamws_StageItemData(handle, steamws_state.getActiveItem())) {
    SPDLOG_ERROR("Error staging item data for Steam Workshop item update");
    error = true;
  }

  // TODO: Handle different visibilities
  if(!steamws_itemSetVisibility(handle, k_ERemoteStoragePublishedFileVisibilityPublic)) {
    SPDLOG_ERROR("Error setting visibility for Steam Workshop item update");
    error = true;
  }

  if(error) {
    SPDLOG_ERROR("Aborting Steam Workshop item update...");
    this->setFailed();
    return;
  }

  SPDLOG_INFO("Item update successful; uploading to Steam Workshop...");

  // Beginning task, now busy
  this->setBusy();

  SteamAPICall_t updateItemCallResult = steamws_itemSubmitUpdate(handle, changeTxt);
  // Set SteamAPI CallResult callback
  UpdateItemCallResult.Set(updateItemCallResult, this, &steamws_core::OnUpdateItem);
}


//------------------------------------------------------------------------------
// steamws_core API function definitions
//------------------------------------------------------------------------------
// Will set the next steamws_state action to check for subscribed items and download those 
// that aren't present after completion
void steamws_initWorkshop() {  
  // Create steamws_core obj
  steamws_core_obj = new steamws_core();
  // Set callbacks, and handle other initialization
  steamws_core_obj->init();

  steamws_ensureDirs();

  // First action should be to handle subscribed items
  steamws_state.setActiveAction(steamws_action::InitSubscribedItems);
}

// Checks for and creates Steam Workshop dirs if they do not exist/cleans up staging area
void steamws_ensureDirs() {
  for (int i = 0; i < steamws_pathEnum::Num_Paths; i++) {
    // Our fileExists() function can also test paths because of the underlying stat() implementation
    std::string wsPath = steamws_getLocalRelativePath((steamws_pathEnum)i);
    if (!fileExists(wsPath.c_str())) {
      SPDLOG_INFO("Workshop dir {} did not exist, creating...", wsPath);
      makeDirectory(wsPath.c_str());
    } else {
      SPDLOG_INFO("Workshop dir {} found!", wsPath);

      if (i == steamws_pathEnum::PathStaging) {
        // Clean out staging area if it previously existed
        deleteFilesInDir(wsPath, true);
      }
    }
  }
}

// Checks local workshop dirs and clears folders found
void steamws_clearAllLocal() {
  // Start at 1, don't clear root
  for (int i = 1; i < steamws_pathEnum::Num_Paths; i++) {
    // Our fileExists() function can also test paths because of the underlying stat() implementation
    std::string wsPath = steamws_getLocalRelativePath((steamws_pathEnum)i);
    if (fileExists(wsPath.c_str())) {
      SPDLOG_INFO("Clearing local Workshop dir {}...", wsPath);
      deleteFilesInDir(wsPath, true);
    }
  }
}

void steamws_InitSubscribedItems() {
  if(steamws_core_obj == 0) {
    SPDLOG_ERROR("Tried to init subscribed Steam Workshop items when steamws_core_obj was null");
    return;
  }

  uint32_t numSubItems = steamws_userGetNumSubscribedItems();

  if(numSubItems == 0) {
    SPDLOG_INFO("User is not subscribed to any Steam Workshop items");
    return;
  }

  PublishedFileId_t *items = new PublishedFileId_t[numSubItems];
  uint32_t numFetchedItems = steamws_userGetSubscribedItems(items, numSubItems);

  if(numFetchedItems == 0) {
    SPDLOG_ERROR("Error fetching subscribed Steam Workshop items; returned 0");
    return;
  }

  if(items == 0) {
    SPDLOG_ERROR("Error fetching subscribed Steam Workshop items; item array is null");
    return;
  }

  if(numFetchedItems != numSubItems) {
    SPDLOG_WARN("Number of fetched Steam Workshop items differs from number subbed ({}/{})", numFetchedItems, numSubItems);
  }

  steamws_core_obj->InitSubscribedItems(items, numFetchedItems);
}

steamws_state_struct* steamws_GetStateStructPtr() {
  return &steamws_state;
}

bool steamws_IsBusy() {
  if(steamws_core_obj == 0) {
    return false;
  }
  return steamws_core_obj->busy();
}

void steamws_CreateNewItem() {
  if(steamws_core_obj == 0) {
    SPDLOG_ERROR("Tried to create new Steam Workshop item when steamws_core_obj was null");
    return;
  }

  // Reset active item, as we're making a new one and it's easy to get file ids back
  steamws_state.setActiveItem(k_PublishedFileIdInvalid);
  steamws_core_obj->CreateNewItem();
}

void steamws_MoveSubbedItemsToGameDir() {
  if(steamws_core_obj == 0) {
    SPDLOG_ERROR("Tried to move Steam Workshop items to game dir when steamws_core_obj was null");
    return;
  }

  steamws_core_obj->MoveSubbedItemsToGameDir();
}

void steamws_OpenOverlayToWorkshop() {
  std::string url = "http://steamcommunity.com/app/" + std::to_string(NewCityAppID) + "/workshop/";
  steam_openOverlayWebPage(url);
}

void steamws_UpdateItem(std::array<std::string, steamws_itemField::Num_Fields> itemFields, std::array<std::string, steamws_maxTags> itemTags, std::string changeTxt) {
  if(steamws_core_obj == 0) {
    SPDLOG_ERROR("Tried to update Steam Workshop item when steamws_core_obj was null");
    return;
  }

  UGCUpdateHandle_t handle = steamws_state.getActiveUpdateHandle();
  // If there's an update handle in progress, prevent a new update
  if(handle != k_UGCUpdateHandleInvalid) {
    SPDLOG_ERROR("Tried to initate a new item update when there was an active update handle");
    return;
  }

  steamws_core_obj->UpdateItem(itemFields, itemTags, changeTxt);
}

bool steamws_StageItemData(UGCUpdateHandle_t handle, PublishedFileId_t id) {
  if(id == k_PublishedFileIdInvalid) {
    SPDLOG_ERROR("Attempted to stage Steam Workshop item data for invalid item");
    return false;
  }

  SPDLOG_INFO("Staging Steam Workshop item data for update...");

  // Verify that the active workshop file exists
  WSFile file = steamws_state.getActiveItemFile();
  bool isMod = file.tag == steamws_itemTag::Mod;
  bool isDesign = file.tag == steamws_itemTag::DesignLegacy || file.tag == steamws_itemTag::DesignPack;
  std::string itemPath = "";
  if (isMod) {
    itemPath = file.dirAndName();
  } else {
    if (isDesign) {
      // If it's a design or design pack, we want the whole directory
      itemPath = file.dir;
    } else {
      itemPath = file.fullPath();
    }
  }
  if (itemPath.length() == 0 || !fileExists(itemPath.c_str())) {
    SPDLOG_ERROR("File to move ({}) does not exist", itemPath);
    return false;
  }

  // Our fileExists() function can also test paths because of the underlying stat() implementation
  // Verify that the root staging path exists
  std::string stagePath = steamws_getLocalRelativePath(steamws_pathEnum::PathStaging);
  if(!fileExists(stagePath.c_str())) {
    SPDLOG_INFO("Workshop dir {} did not exist, creating...", stagePath);
    makeDirectory(stagePath.c_str());
  } 

  // Create the item data dir in /staging, if it doesn't exist
  std::string itemStagePath = stagePath + "/" + file.name;
  if(!fileExists(itemStagePath.c_str())) {
    SPDLOG_INFO("Creating staging dir {} for item {}...", itemStagePath, id);
    makeDirectory(itemStagePath.c_str());
  }

  // Copy file to staging
  std::string itemDestPath = itemStagePath + "/";
  if(!isMod) {
    // itemDestPath += file.nameAndExt();
    if (isDesign) {
      if (!copyDir(itemPath, itemDestPath, std::experimental::filesystem::copy_options::overwrite_existing |
        std::experimental::filesystem::copy_options::recursive)) {
        SPDLOG_ERROR("Error copying design (Src: {}) (Dest: {})", itemPath, itemDestPath);
        return false;
      }
    }
  } else {
    // We want to copy over the content into the mod name dir beneath the item dir
    std::string modDestPath = itemDestPath + file.name + "/";
    makeDirectory(modDestPath.c_str());

    if(!copyDir(itemPath, modDestPath, std::experimental::filesystem::copy_options::overwrite_existing |
      std::experimental::filesystem::copy_options::recursive)) {
      SPDLOG_ERROR("Error copying mod (Src: {}) (Dest: {})", itemPath, modDestPath);
      return false;
    }
  }

  /*
  // THIS IS ALREADY COVERED UNDER THE DIR COPY ABOVE -supersoup
  // Verify that image file exists, then copy file to staging
  std::string imgPath = steamws_state.getActiveItemField(steamws_itemField::PreviewImgPath);
  std::string imgExt = imgPath.substr(imgPath.find_last_of("."), std::string::npos);
  std::string imgDestPath = itemStagePath + "/" + "design.design" + imgExt;
  if(!copyFile(imgPath, imgDestPath)) {
    SPDLOG_ERROR("Error copying file (Src: {}) (Dest: {})", imgPath, imgDestPath);
    return false;
  }
  */
  
  SPDLOG_INFO("All files successfully staged for Steam Workshop update!");

  // Set update handle content to the absolute path of the item staging folder
  std::string absPath = getAbsolutePath(itemStagePath);
  steamws_itemSetContent(handle, absPath);

  return true;
}

void steamws_SubscribeToItem(PublishedFileId_t id) {
  if(steamws_core_obj == 0) {
    SPDLOG_ERROR("Tried to subscribe to Steam Workshop item when steamws_core_obj was null");
    return;
  }

  steamws_core_obj->SubscribeToItem(id);
}

void steamws_UnsubscribeFromItem(PublishedFileId_t id) {
  if(steamws_core_obj == 0) {
    SPDLOG_ERROR("Tried to unsubscribe from Steam Workshop item when steamws_core_obj was null");
    return;
  }

  steamws_core_obj->UnsubscribeFromItem(id);
}

void steamws_tick() { 
  if(steamws_core_obj->failed()) {
    steamws_state.resetSteamWSState();
    steamws_core_obj->reset();
    return;
  }

  if(steamws_core_obj->busy()) {
    /* steamws_core_obj->addToTimer(duration);
    SPDLOG_INFO("WS Busy, Timer: {}", steamws_core_obj->timer());
    if(steamws_core_obj->timer() >= wsTimeout) {
      SPDLOG_ERROR("Steam Workshop action {} timed out", activeAct);
      steamws_core_obj->forceFailed();
    }*/
    return;
  }

  steamws_action activeAct = steamws_state.getActiveAction();
  // If no active action
  if(activeAct == steamws_action::NoAction) {
    // Check next action, and set if present
    steamws_action next = steamws_state.getNextAction();
    if(next != steamws_action::NoAction) {
      steamws_state.setActiveAction(next);
      steamws_state.setNextAction(steamws_action::NoAction);
    }
    return;
  }

  // Handle actions
  if(activeAct == steamws_action::CreateNewItem) {
    steamws_CreateNewItem();
  } else if(activeAct == steamws_action::UpdateItem) {
    steamws_UpdateItem(steamws_state.getActiveItemFields(), steamws_state.getActiveItemTags(), steamws_state.getActiveItemChangeTxt());
  } else if(activeAct == steamws_action::InitSubscribedItems) {
    steamws_InitSubscribedItems();
  } else if(activeAct == steamws_action::MoveSubbedItemsToGameDir) {
    steamws_MoveSubbedItemsToGameDir();
  }

  // Some actions will imply we don't want to clear the active action
  activeAct = steamws_state.getActiveAction(); // The active action could have changed; refresh
  if(activeAct == steamws_action::AwaitUGCQuery) {
    // Waiting on the callback
  } else {
    steamws_state.clearActiveAction();
  }
}

void steamws_shutdown() {
  // TODO: Handle steamws_core obj cleanup
  steamws_state.resetSteamWSState();
}

//------------------------------------------------------------------------------
// steamws_core API utility function definitions
//------------------------------------------------------------------------------

steamws_pathEnum steamws_getLocalPathEnumFromTag(steamws_itemTag tag) {
  switch(tag) {
    case steamws_itemTag::PreviewImg:
    case steamws_itemTag::Generic:
    case steamws_itemTag::DesignLegacy:
    case steamws_itemTag::DesignPack:
      return steamws_pathEnum::PathDesigns;
    case steamws_itemTag::Lua:
    case steamws_itemTag::LuaPack:
      return steamws_pathEnum::PathData;
    case steamws_itemTag::Model:
    case steamws_itemTag::ModelPack:
      return steamws_pathEnum::PathModelsRoot;
    case steamws_itemTag::Mod:
      return steamws_pathEnum::PathModpacks;
    case steamws_itemTag::Sound:
    case steamws_itemTag::SoundPack:
      return steamws_pathEnum::PathSoundRoot;
    case steamws_itemTag::Texture:
    case steamws_itemTag::TexturePack:
      return steamws_pathEnum::PathTexturesRoot;
      break;
    default:
      return steamws_pathEnum::PathRoot;
  }

  // Something went wrong fetching local path enum
  SPDLOG_ERROR("Something went wrong fetching local path enum for item tag, returning game root ...");
  return steamws_pathEnum::PathRoot;
}

std::string steamws_getLocalRelativePath(steamws_pathEnum path) {
  switch(path) {
    case steamws_pathEnum::PathRoot:
      return steamws_rootPath;
    case steamws_pathEnum::PathData:
      return steamws_dataPath;
    case steamws_pathEnum::PathDesigns:
      return steamws_designsPath;
    case steamws_pathEnum::PathDocs:
      return steamws_docsPath;
    case steamws_pathEnum::PathFonts:
      return steamws_fontsPath;
    case steamws_pathEnum::PathLocale:
      return steamws_localePath;
    case steamws_pathEnum::PathModelsRoot:
      return steamws_modelsRootPath;
    case steamws_pathEnum::PathModelsDecorations:
      return steamws_modelsDecorationsPath;
    case steamws_pathEnum::PathModelsVehicles:
      return steamws_modelsVehiclesPath;
    case steamws_pathEnum::PathModpacks:
      return steamws_modpacksPath;
    case steamws_pathEnum::PathNewspaper:
      return steamws_newspaperPath;
    case steamws_pathEnum::PathShaders:
      return steamws_shadersPath;
    case steamws_pathEnum::PathSoundRoot:
      return steamws_soundRootPath;
    case steamws_pathEnum::PathSoundEnvironment:
      return steamws_soundEnvironmentPath;
    case steamws_pathEnum::PathSoundMusic:
      return steamws_soundMusicPath;
    case steamws_pathEnum::PathTexturesRoot:
      return steamws_texturesRootPath;
    case steamws_pathEnum::PathTexturesBuildings:
      return steamws_texturesBuildingsPath;
    case steamws_pathEnum::PathTexturesVehicles:
      return steamws_texturesVehiclesPath;
    case steamws_pathEnum::PathStaging:
      return steamws_stagingPath;
    default:
      return steamws_rootPath;
  }
}

std::string steamws_getRelativeDirForTag(steamws_itemTag tag) {
  std::string dir = "";

  switch(tag) {
    case steamws_itemTag::PreviewImg:
      dir = imagesDirectory();
      break;
    case steamws_itemTag::Generic:
      dir = gameRootDirectory();
      break;
    case steamws_itemTag::DesignLegacy:
    case steamws_itemTag::DesignPack:
      dir = designDirectory();
      break;
    case steamws_itemTag::Lua:
    case steamws_itemTag::LuaPack:
      dir = dataDirectory();
      break;
    case steamws_itemTag::Model:
    case steamws_itemTag::ModelPack:
      dir = modelsDirectory();
      break;
    case steamws_itemTag::ModelDecoration:
      dir = modelsDirectory() + steamws_modelsDecorationsAddPath;
      break;
    case steamws_itemTag::ModelVehicle:
      dir = modelsDirectory() + steamws_modelsVehiclesAddPath;
      break;
    case steamws_itemTag::Mod:
      dir = modpacksDirectory();
      break;
    case steamws_itemTag::Sound:
    case steamws_itemTag::SoundPack:
      dir = soundDirectory();
      break;
    case steamws_itemTag::SoundEnvironment:
      dir = soundDirectory() + steamws_soundEnvironmentAddPath;
      break;
    case steamws_itemTag::SoundMusic:
      dir = soundDirectory() + steamws_soundMusicAddPath;
      break;
    case steamws_itemTag::Texture:
    case steamws_itemTag::TexturePack:
      dir = texturesDirectory();
      break;
    case steamws_itemTag::TextureBuilding:
      dir = texturesDirectory() + steamws_texturesBuildingsAddPath;
      break;
    case steamws_itemTag::TextureVehicle:
      dir = texturesDirectory() + steamws_texturesVehiclesAddPath;
      break;
    default:
      // Something went wrong fetching directory
      SPDLOG_ERROR("Something went wrong fetching directory for item tag, returning game root ...");
      dir = gameRootDirectory();
      break;
  }

  return dir;
}

// Returns Generic if a tag doesn't match
steamws_itemTag steamws_getTagForExt(std::string ext) {
  if(ext.length() == 0) {
    SPDLOG_ERROR("Null string passed to steamws_getTagFromExt, cannot evaluate; returning Generic ...");
    return steamws_itemTag::Generic;
  }

  if(ext[0] != '.') {
    ext.insert(0,1,'.');
  }

  for(int i = 1; i < ext.length(); i++) {
    ext[i] = tolower(ext[i]);
  }

  SPDLOG_INFO("Getting item tag for ext {}", ext);

  const char* extC = ext.c_str();
  if(!strcmp(extC, steamws_extDesign.c_str())) {
    return steamws_itemTag::DesignLegacy;
  } else if(!strcmp(extC, steamws_extGif.c_str())) {
    return steamws_itemTag::PreviewImg;
  } else if(!strcmp(extC, steamws_extJpg.c_str())) {
    return steamws_itemTag::PreviewImg;
  } else if(!strcmp(extC, steamws_extJpeg.c_str())) {
    return steamws_itemTag::PreviewImg;
  } else if(!strcmp(extC, steamws_extLua.c_str())) {
    return steamws_itemTag::Lua;
  } else if(!strcmp(extC, steamws_extPng.c_str())) {
    return steamws_itemTag::Texture;
  } else if(!strcmp(extC, steamws_extObj.c_str())) {
    return steamws_itemTag::Model;
  } else if(!strcmp(extC, steamws_extOgg.c_str())) {
    return steamws_itemTag::Sound;
  } else {
    SPDLOG_ERROR("Unrecognized file extension {}, returning Generic", extC);
    return steamws_itemTag::Generic;
  }

  // How did we get here...
  SPDLOG_ERROR("Something went wrong fetching an item tag from file ext {}, returning Generic", extC);
  return steamws_itemTag::Generic;
}
