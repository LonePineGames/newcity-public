//-----------------------------------------------------------------------------
// steamws - Internal API for interacting with the
// Steam API for Steam Workshop functionality
//-----------------------------------------------------------------------------

#include "steamws.hpp"

#include "../building/design.hpp"
#include "../platform/file.hpp"
#include "../platform/lookup.hpp"

//-----------------------------------------------------------------------------
// Steam Workshop API
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Steam Workshop state struct
//-----------------------------------------------------------------------------
// Function definitions for steamws_state_struct
steamws_state_struct::steamws_state_struct() {
  resetSteamWSState();
}

void steamws_state_struct::resetSteamWSState() {
  SPDLOG_INFO("Resetting Steam Workshop state struct ... ");
  _activeAction = steamws_action::NoAction;
  _nextAction = steamws_action::NoAction;
  _activeItem = k_PublishedFileIdInvalid;
  _activeItemFile = WSFile();
  std::fill(_activeItemFields.begin(), _activeItemFields.end(), "");
  std::fill(_activeItemTags.begin(), _activeItemTags.end(), "");
  _activeItemChangeTxt = "";

  if (_activeQueryHandle != k_UGCQueryHandleInvalid) {
    steamws_queryReleaseUGCRequest(_activeQueryHandle);
  }
  _activeQueryHandle = k_UGCQueryHandleInvalid;

  _activeQueryNumResults = 0;
  
  // Not released like a query
  _activeUpdateHandle = k_UGCUpdateHandleInvalid;
}

//-----------------------------------------------------------------------------
// Basic item management
//-----------------------------------------------------------------------------
SteamAPICall_t steamws_itemCreate(EWorkshopFileType type) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to create Steam Workshop item when Steam was inactive");
    return k_uAPICallInvalid;
  }

  return SteamUGC()->CreateItem(NewCityAppID, type);
}

SteamAPICall_t steamws_itemDelete(PublishedFileId_t id) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to delete Steam Workshop item when Steam was inactive");
    return k_uAPICallInvalid;
  }

  return SteamUGC()->DeleteItem(id);
}

bool steamws_itemDownload(PublishedFileId_t id, bool highPriority) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to download Steam Workshop item when Steam was inactive");
    return NULL;
  }

  return SteamUGC()->DownloadItem(id, highPriority);
}

SteamAPICall_t steamws_itemSubscribe(PublishedFileId_t id) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to subscribe to Steam Workshop item when Steam was inactive");
    return k_uAPICallInvalid;
  }

  return SteamUGC()->SubscribeItem(id);
}

SteamAPICall_t steamws_itemUnsubscribe(PublishedFileId_t id) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to unsubscribe to Steam Workshop item when Steam was inactive");
    return k_uAPICallInvalid;
  }

  return SteamUGC()->UnsubscribeItem(id);
}


//-----------------------------------------------------------------------------
// Updating items, with things like tags and metadata (UGCUpdate)
// Max 100 changes per item update (Possibly just for key value tags? Better to be safe...)
// Max char lengths handled internally by Steam? I don't think we need to manually check for them. 
// Anytime a Steam API language is involved, if no language is set then "english" is assumed
//-----------------------------------------------------------------------------

// Use to get UGCUpdateHandle for making changes and submitting update; call SubmitItemUpdate when done
UGCUpdateHandle_t steamws_itemStartUpdate(PublishedFileId_t id) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to start Steam Workshop item update when Steam was inactive");
    return k_uAPICallInvalid;
  }

  return SteamUGC()->StartItemUpdate(NewCityAppID, id);
}

bool steamws_itemAddKeyValueTag(UGCUpdateHandle_t handle, std::string key, std::string value) {

  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to add Steam Workshop item key value tag when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to add Steam Workshop item key value tag with null UGCUpdateHandle");
    return false;
  }

  if(key.length() > steamws_keyValueTagMaxLen) {
    SPDLOG_ERROR("Attempted to add Steam Workshop item key value tag with key longer than {}", steamws_keyValueTagMaxLen);
    return false;
  }

  if(value.length() > steamws_keyValueTagMaxLen) {
    SPDLOG_ERROR("Attempted to add Steam Workshop item key value tag with value longer than {}", steamws_keyValueTagMaxLen);
    return false;
  }

  return SteamUGC()->AddItemKeyValueTag(handle, key.c_str(), value.c_str());
}

// Can call max 100 times per item update
bool steamws_itemRemoveKeyValueTag(UGCUpdateHandle_t handle, std::string key) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to remove Steam Workshop item key value tag when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to remove Steam Workshop item key value tag with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->RemoveItemKeyValueTags(handle, key.c_str());
}

// Tag max 255 chars, and can only include printable characters
bool steamws_itemSetTags(UGCUpdateHandle_t handle, const SteamParamStringArray_t *tags) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item key value tag array when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item key value tag array with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->SetItemTags(handle, tags);
}

bool steamws_itemSetMetadata(UGCUpdateHandle_t handle, std::string metadata) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item metadata when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item metadata with null UGCUpdateHandle");
    return false;
  }

  if(metadata.length() > k_cchDeveloperMetadataMax) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item metadata string longer than k_cchDeveloperMetadataMax");
    return false;
  }

  return SteamUGC()->SetItemMetadata(handle, metadata.c_str());
}

// Should be .jpg, .png, or .gif
bool steamws_itemSetPreviewImg(UGCUpdateHandle_t handle, std::string path) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item preview img when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item preview img with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->SetItemPreview(handle, path.c_str());
}

// Adds an additional preview image
bool steamws_itemAddAdditionalPreviewFile(UGCUpdateHandle_t handle, std::string path, EItemPreviewType type) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to add additional Steam Workshop item preview img when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to add additional Steam Workshop item preview img with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->AddItemPreviewFile(handle, path.c_str(), type);
}

bool steamws_itemRemoveItemPreviewImg(UGCUpdateHandle_t handle, uint32_t index) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to remove Steam Workshop item preview img when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to remove Steam Workshop item preview img with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->RemoveItemPreview(handle, index);
}

// Max 128 chars per k_cchPublishedDocumentTitleMax
bool steamws_itemSetTitle(UGCUpdateHandle_t handle, std::string title) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item title when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item title with null UGCUpdateHandle");
    return false;
  }

  if(title.length() > k_cchPublishedDocumentTitleMax) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item title longer than k_cchPublishedDocumentTitleMax length");
    return false;
  }

  return SteamUGC()->SetItemTitle(handle, title.c_str());
}

// Set folder that represents item content
bool steamws_itemSetContent(UGCUpdateHandle_t handle, std::string path) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item content folder when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item content folder with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->SetItemContent(handle, path.c_str());
}

// Max 5000 chars per k_cchDeveloperMetadataMax
bool steamws_itemSetDescription(UGCUpdateHandle_t handle, std::string desc) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item description when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item description with null UGCUpdateHandle");
    return false;
  }

  if(desc.length() > k_cchDeveloperMetadataMax) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item description longer than k_cchDeveloperMetadataMax length");
    return false;
  }

  return SteamUGC()->SetItemDescription(handle, desc.c_str());
}

bool steamws_itemSetVisibility(UGCUpdateHandle_t handle, ERemoteStoragePublishedFileVisibility visibility) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item visibility when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item visibility with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->SetItemVisibility(handle, visibility);
}

// See Steam API language codes - https://partner.steamgames.com/doc/store/localization#supported_languages
bool steamws_itemSetUpdateLanguage(UGCUpdateHandle_t handle, std::string language) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item update language when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set Steam Workshop item update language with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->SetItemUpdateLanguage(handle, language.c_str());
}

// Sends item changes to the server; must be called after all changed made to handle
SteamAPICall_t steamws_itemSubmitUpdate(UGCUpdateHandle_t handle, std::string changeNote) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to submit Steam Workshop item update when Steam was inactive");
    return k_uAPICallInvalid;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to submit Steam Workshop item update with null UGCUpdateHandle");
    return k_uAPICallInvalid;
  }

  return SteamUGC()->SubmitItemUpdate(handle, changeNote.length() > 0 ? changeNote.c_str() : NULL);
}


//-----------------------------------------------------------------------------
// Checking item state and info
//-----------------------------------------------------------------------------

// Returns true if info available, ref args to get byte info
bool steamws_itemGetDownloadInfo(PublishedFileId_t id, uint64_steam *bytesDownloaded, uint64_steam *bytesTotal) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to get Steam Workshop item download info when Steam was inactive");
    return false;
  }

  return SteamUGC()->GetItemDownloadInfo(id, bytesDownloaded, bytesTotal);
}

bool steamws_itemGetInstallInfo(PublishedFileId_t id, uint64_steam *bytesOnDisk, char *folderPath, uint32_t folderSize, uint32_t *timeUpdated) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to get Steam Workshop item install info when Steam was inactive");
    return false;
  }

  return SteamUGC()->GetItemInstallInfo(id, bytesOnDisk, folderPath, folderSize, timeUpdated);
}

// Returns the item state; use EItemState flags to check state specifics
uint32_t steamws_itemGetState(PublishedFileId_t id) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to get Steam Workshop item state when Steam was inactive");
    return 0;
  }

  return SteamUGC()->GetItemState(id);
}

EItemUpdateStatus steamws_itemGetUpdateProgress(UGCUpdateHandle_t handle, uint64_steam *bytesUploaded, uint64_steam *bytesTotal) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to get Steam Workshop item update progress when Steam was inactive");
    return k_EItemUpdateStatusInvalid;
  }

  return SteamUGC()->GetItemUpdateProgress(handle, bytesUploaded, bytesTotal);
}


//-----------------------------------------------------------------------------
// Creating item queries (UGCQuery)
//-----------------------------------------------------------------------------

// Use to list all available UGC
UGCQueryHandle_t steamws_queryAllUGCRequest(EUGCQuery queryType, EUGCMatchingUGCType fileType, uint32_t pageNum) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to create Steam Workshop query for all UGC when Steam was inactive");
    return k_uAPICallInvalid;
  }

  return SteamUGC()->CreateQueryAllUGCRequest(queryType, fileType, NewCityAppID, NewCityAppID, pageNum);
}

// Used to get details for specific UGC items
UGCQueryHandle_t steamws_queryUGCDetailsRequest(PublishedFileId_t *id, uint32_t numPublishedFileIds) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to create Steam Workshop query for UGC details when Steam was inactive");
    return k_uAPICallInvalid;
  }

  return SteamUGC()->CreateQueryUGCDetailsRequest(id, numPublishedFileIds);
}

// Used to query UGC associated with a user
UGCQueryHandle_t steamws_queryUserUGCRequest(AccountID_t id, EUserUGCList listType, EUGCMatchingUGCType fileType, EUserUGCListSortOrder sortOrder, uint32_t pageNum) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to create Steam Workshop query for user-specific UGC when Steam was inactive");
    return k_uAPICallInvalid;
  }

  return SteamUGC()->CreateQueryUserUGCRequest(id, listType, fileType, sortOrder, NewCityAppID, NewCityAppID, pageNum);
}

bool steamws_queryAddRequiredKeyValueTag(UGCQueryHandle_t handle, std::string key, std::string value) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to add required key/value tag for Steam Workshop query when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to add required key/value tag for Steam Workshop query with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->AddRequiredKeyValueTag(handle, key.c_str(), value.c_str());
}

bool steamws_queryAddRequiredTag(UGCQueryHandle_t handle, std::string tag) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to add required tag for Steam Workshop query when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to add required tag for Steam Workshop query with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->AddRequiredTag(handle, tag.c_str());
}

bool steamws_queryAddExcludedTag(UGCQueryHandle_t handle, std::string tag) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to add excluded tag for Steam Workshop query when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to add excluded tag for Steam Workshop query with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->AddExcludedTag(handle, tag.c_str());
}

// Can only be used with steamws_queryAllUGCRequest
bool steamws_querySetText(UGCQueryHandle_t handle, std::string text) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set search text for Steam Workshop query when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set search text for Steam Workshop query with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->SetSearchText(handle, text.c_str());
}

// Can only be used with steamws_queryAllUGCRequest
bool steamws_querySetMatchAnyTag(UGCQueryHandle_t handle, bool anyTag) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set any tag flag for Steam Workshop query when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set any tag flag for Steam Workshop query with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->SetMatchAnyTag(handle, anyTag);
}

bool steamws_querySetAllowCachedResponse(UGCQueryHandle_t handle, uint32_t maxAgeSeconds) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set allow cached response flag for Steam Workshop query when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set allow cached response flag for Steam Workshop query with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->SetAllowCachedResponse(handle, maxAgeSeconds);
}

// Set to only return results that have a specific filename
// Can only be used with steamws_queryUserUGCRequest
bool steamws_querySetCloudFilenameFilter(UGCQueryHandle_t handle, std::string filename) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set cloud filename filter for Steam Workshop query when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set cloud filename filter for Steam Workshop query with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->SetCloudFileNameFilter(handle, filename.c_str());
}

bool steamws_querySetReturnTotalOnly(UGCQueryHandle_t handle, bool totalOnly) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set return total only flag for Steam Workshop query when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set return total only for Steam Workshop query with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->SetReturnTotalOnly(handle, totalOnly);
}

// Only return item Ids, like when checking items a user has in favorites
bool steamws_querySetReturnOnlyIds(UGCQueryHandle_t handle, bool idsOnly) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set return UGC ids flag for Steam Workshop query when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set return UGC ids flag for Steam Workshop query with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->SetReturnOnlyIDs(handle, idsOnly);
}

bool steamws_querySetReturnKeyValueTags(UGCQueryHandle_t handle, bool returnKeyValueTags) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set return key/value tags flag for Steam Workshop query when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set return key/value tags flag for Steam Workshop query with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->SetReturnKeyValueTags(handle, returnKeyValueTags);
}

bool steamws_querySetReturnMetadata(UGCQueryHandle_t handle, bool metadata) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set return metadata flag for Steam Workshop query when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set return metadata flag for Steam Workshop query with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->SetReturnMetadata(handle, metadata);
}

// Set whether to return the full description for items; otherwise summary truncated at 255 bytes
bool steamws_querySetReturnLongDescription(UGCQueryHandle_t handle, bool longDesc) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set return long description flag for Steam Workshop query when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set return long description flag for Steam Workshop query with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->SetReturnLongDescription(handle, longDesc);
}

bool steamws_querySetReturnAdditionalPreviews(UGCQueryHandle_t handle, bool additionalPreviews) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set return additional previews flag for Steam Workshop query when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set return additional previews flag for Steam Workshop query with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->SetReturnAdditionalPreviews(handle, additionalPreviews);
}

// uint32_t arg sets number of days to rank items over, valid values are 1-365 inclusive
bool steamws_querySetRankedByTrendDays(UGCQueryHandle_t handle, uint32_t daysToRank) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set rank by trend days for Steam Workshop query when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set rank by trend days for Steam Workshop query with null UGCUpdateHandle");
    return false;
  }

  if(daysToRank < 1 || daysToRank > 365) {
    SPDLOG_ERROR("Attempted to set ranking by trend days for Steam Workshop query to invalid value");
    return false;
  }

  return SteamUGC()->SetRankedByTrendDays(handle, daysToRank);
}

// See Steam API language codes - https://partner.steamgames.com/doc/store/localization#supported_languages
bool steamws_querySetLanguage(UGCQueryHandle_t handle, std::string lang) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set language for Steam Workshop query when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to set language for Steam Workshop query with null UGCUpdateHandle");
    return false;
  }

  // Ensure string is lowercase before testing
  std::string lowerLang = "";
  for(int i = 0; i < lang.length(); i++) {
    lowerLang += tolower(lang[i]);
  }

  if(!steamws_langIsValid(lowerLang)) {
    SPDLOG_ERROR("Attempted to set language for Steam Workshop query with unrecognized language ({})", lowerLang);
    return false;
  }

  return SteamUGC()->SetLanguage(handle, lowerLang.c_str());
}

// Send the created UGC request to Steam
SteamAPICall_t steamws_querySendUGCRequest(UGCQueryHandle_t handle) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to send Steam Workshop query when Steam was inactive");
    return k_uAPICallInvalid;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to send Steam Workshop query with null UGCUpdateHandle");
    return k_uAPICallInvalid;
  }

  return SteamUGC()->SendQueryUGCRequest(handle);
}


//-----------------------------------------------------------------------------
// Processing item queries (UGCQuery)
//-----------------------------------------------------------------------------
bool steamws_queryGetUGCAdditionalPreview(UGCQueryHandle_t handle, uint32_t index, uint32_t previewIndex, char *urlOrVideoId, uint32_t urlSize, char *originalFilename, uint32_t filenameSize, EItemPreviewType *type) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to get a Steam Workshop item additional preview when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to get a Steam Workshop item additional preview with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->GetQueryUGCAdditionalPreview(handle, index, previewIndex, urlOrVideoId, urlSize, originalFilename, filenameSize, type);
}

bool steamws_queryGetUGCKeyValueTag(UGCQueryHandle_t handle, uint32_t index, uint32_t keyValueTagIndex, char *key, uint32_t keySize, char *value, uint32_t valueSize) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to get a Steam Workshop item key/value tag when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to get a Steam Workshop item key/value tag with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->GetQueryUGCKeyValueTag(handle, index, keyValueTagIndex, key, keySize, value, valueSize);
}

bool steamws_queryGetUGCMetadata(UGCQueryHandle_t handle, uint32_t index, char *metadata, uint32_t metadataSize) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to get a Steam Workshop item's metadata when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to get a Steam Workshop item's metadata with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->GetQueryUGCMetadata(handle, index, metadata, metadataSize);
}

uint32_t steamws_queryGetQueryUGCNumAdditionalPreviews(UGCQueryHandle_t handle, uint32_t index) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to get the number of additional previews for a Steam Workshop item when Steam was inactive");
    return 0;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to get the number of additional previews for a Steam Workshop item with null UGCUpdateHandle");
    return 0;
  }

  return SteamUGC()->GetQueryUGCNumAdditionalPreviews(handle, index);
}

uint32_t steamws_queryGetUGCNumKeyValueTags(UGCQueryHandle_t handle, uint32_t index) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to get the number of key/value tags for a Steam Workshop item when Steam was inactive");
    return 0;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to get the number of key/value tags for a Steam Workshop item with null UGCUpdateHandle");
    return 0;
  }

  return SteamUGC()->GetQueryUGCNumKeyValueTags(handle, index);
}

bool steamws_queryGetUGCPreviewURL(UGCQueryHandle_t handle, uint32_t index, char *url, uint32_t urlSize) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to get the URL for a Steam Workshop item preview image when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to get the URL for a Steam Workshop item preview image with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->GetQueryUGCPreviewURL(handle, index, url, urlSize);
}

bool steamws_queryGetUGCResult(UGCQueryHandle_t handle, uint32_t index, SteamUGCDetails_t *details) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to get detailed result for a Steam Workshop item when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to get detailed result for a Steam Workshop item with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->GetQueryUGCResult(handle, index, details);
}

bool steamws_queryGetUGCStatistic(UGCQueryHandle_t handle, uint32_t index, EItemStatistic statType, uint64_steam *statValue) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to get statistics for a Steam Workshop item when Steam was inactive");
    return false;
  }

  if(handle == 0) {
    SPDLOG_ERROR("Attempted to get statistics for a Steam Workshop item with null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->GetQueryUGCStatistic(handle, index, statType, statValue);
}


//-----------------------------------------------------------------------------
// Cleaning up queries (UGCQuery)
//-----------------------------------------------------------------------------

// Call to free up memory when finished with a UGCQuery
bool steamws_queryReleaseUGCRequest(UGCQueryHandle_t handle) {
  if(handle == 0) {
    SPDLOG_ERROR("Attempted to release UGCQueryHandle for null UGCUpdateHandle");
    return false;
  }

  return SteamUGC()->ReleaseQueryUGCRequest(handle);
}


//-----------------------------------------------------------------------------
// User experience
//-----------------------------------------------------------------------------
SteamAPICall_t steamws_userAddItemToFavorites(PublishedFileId_t id) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to add a Steam Workshop item to user favorites when Steam was inactive");
    return k_uAPICallInvalid;
  }

  return SteamUGC()->AddItemToFavorites(NewCityAppID, id);
}

SteamAPICall_t steamws_userRemoveItemFromFavorites(PublishedFileId_t id) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to remove a Steam Workshop item from user favorites when Steam was inactive");
    return k_uAPICallInvalid;
  }

  return SteamUGC()->RemoveItemFromFavorites(NewCityAppID, id);
}

SteamAPICall_t steamws_userGetItemVote(PublishedFileId_t id) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to fetch user vote on a Steam Workshop item when Steam was inactive");
    return k_uAPICallInvalid;
  }

  return SteamUGC()->GetUserItemVote(id);
}

SteamAPICall_t steamws_userSetItemVote(PublishedFileId_t id, bool voteUp) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to set a user vote on a Steam Workshop item when Steam was inactive");
    return k_uAPICallInvalid;
  }

  return SteamUGC()->SetUserItemVote(id, voteUp);
}

uint32_t steamws_userGetNumSubscribedItems() {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to fetch the number of subscribed Steam Workshop items for a user when Steam was inactive");
    return 0;
  }

  return SteamUGC()->GetNumSubscribedItems();
}

uint32_t steamws_userGetSubscribedItems(PublishedFileId_t *itemArr, uint32_t maxEntries) {
  if(!steam_isActive()) {
    SPDLOG_ERROR("Attempted to get a list of the subscribed Steam Workshop items for a user when Steam was inactive");
    return 0;
  }

  if(itemArr == 0) {
    SPDLOG_ERROR("Attempted to get a list of the subscribed Steam Workshop items for a user with a null itemArr");
    return 0;
  }

  return SteamUGC()->GetSubscribedItems(itemArr, maxEntries);
}


//-----------------------------------------------------------------------------
// Util
//-----------------------------------------------------------------------------
bool steamws_langIsValid(std::string lang) {
  bool valid = false;

  for(int i = 0; i < steamws_langNum; i++) {
    if(lang == steamws_langArr[i]) {
      valid = true;
      break;
    }
  }

  return valid;
}

std::array<std::string, steamws_langNum> steamws_getLangArray() {
  return steamws_langArr;
}

std::string steamws_getStringForTag(steamws_itemTag type) {
  switch(type) {
    case steamws_itemTag::Generic:
      return steamws_tagGeneric;
    case steamws_itemTag::DesignLegacy:
      return steamws_tagDesign;
    case steamws_itemTag::DesignPack:
      return steamws_tagDesignPack;
    case steamws_itemTag::Lua:
      return steamws_tagData;
    case steamws_itemTag::LuaPack:
      return steamws_tagDataPack;
    case steamws_itemTag::Model:
      return steamws_tagModel;
    case steamws_itemTag::ModelDecoration:
      return steamws_tagModelDecoration;
    case steamws_itemTag::ModelVehicle:
      return steamws_tagModelVehicle;
    case steamws_itemTag::ModelPack:
      return steamws_tagModelPack;
    case steamws_itemTag::Mod:
      return steamws_tagModpack;
    case steamws_itemTag::Sound:
      return steamws_tagSound;
    case steamws_itemTag::SoundEnvironment:
      return steamws_tagSoundEnvironment;
    case steamws_itemTag::SoundMusic:
      return steamws_tagSoundMusic;
    case steamws_itemTag::SoundPack:
      return steamws_tagSoundPack;
    case steamws_itemTag::Texture:
      return steamws_tagTexture;
    case steamws_itemTag::TextureBuilding:
      return steamws_tagTextureBuilding;
    case steamws_itemTag::TextureVehicle:
      return steamws_tagTextureVehicle;
    case steamws_itemTag::TexturePack:
      return steamws_tagTexturePack;
    default:
      return steamws_defaultTagValue;
  }
}

std::string WSFile::fullPath() {
  string result;
  if (tag == steamws_itemTag::DesignLegacy || tag == steamws_itemTag::DesignPack) {
    result = fixDesignerPath(name);
  } else {
    result = dir + name + ext;
  }

  return result;
}

std::string steamws_getImageForItem(WSFile file) {
  std::string filename = file.dir + file.name;
  std::string result = filename;
  if (file.tag == steamws_itemTag::Mod) {
    result = filename + "/preview.png";
  } else if (file.tag == steamws_itemTag::DesignLegacy || file.tag == steamws_itemTag::DesignPack) {

    result = lookupFile("designs/" + file.name + "/design.design.png", 0);

  } else {
    filename += file.ext;
    result = filename + ".png";
  }

  if (!fileExists(result)) {
    return "textures/house_icon.png";
  } else {
    return result;
  }
}

steamws_itemTag steamws_getTagForString(std::string str) {
  const char* strC = str.c_str();

  if(!(strcmp(strC, steamws_tagDesign.c_str()))) {
    return steamws_itemTag::DesignLegacy;
  } else if(!(strcmp(strC, steamws_tagDesignPack.c_str()))) {
    return steamws_itemTag::DesignPack;
  } else if(!(strcmp(strC, steamws_tagData.c_str()))) {
    return steamws_itemTag::Lua;
  } else if(!(strcmp(strC, steamws_tagDataPack.c_str()))) {
    return steamws_itemTag::LuaPack;
  } else if (!(strcmp(strC, steamws_tagModpack.c_str()))) {
    return steamws_itemTag::Mod;
  } else if(!(strcmp(strC, steamws_tagModel.c_str()))) {
    return steamws_itemTag::Model;
  } else if(!(strcmp(strC, steamws_tagModelPack.c_str()))) {
    return steamws_itemTag::ModelPack;
  } else if(!(strcmp(strC, steamws_tagModelDecoration.c_str()))) {
    return steamws_itemTag::ModelDecoration;
  } else if(!(strcmp(strC, steamws_tagModelVehicle.c_str()))) {
    return steamws_itemTag::ModelVehicle;
  } else if(!(strcmp(strC, steamws_tagSound.c_str()))) {
    return steamws_itemTag::Sound;
  } else if(!(strcmp(strC, steamws_tagSoundEnvironment.c_str()))) {
    return steamws_itemTag::SoundEnvironment;
  } else if(!(strcmp(strC, steamws_tagSoundMusic.c_str()))) {
    return steamws_itemTag::SoundMusic;
  } else if(!(strcmp(strC, steamws_tagSoundPack.c_str()))) {
    return steamws_itemTag::SoundPack;
  } else if(!(strcmp(strC, steamws_tagTexture.c_str()))) {
    return steamws_itemTag::Texture;
  } else if(!(strcmp(strC, steamws_tagTextureBuilding.c_str()))) {
    return steamws_itemTag::TextureBuilding;
  } else if(!(strcmp(strC, steamws_tagTextureVehicle.c_str()))) {
    return steamws_itemTag::TextureVehicle;
  } else if(!(strcmp(strC, steamws_tagTexturePack.c_str()))) {
    return steamws_itemTag::TexturePack;
  }
  
  // No match, return generic
  return steamws_itemTag::Generic;
}
