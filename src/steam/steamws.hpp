//-----------------------------------------------------------------------------
// steamws - Internal API for interacting with the
// Steam API for Steam Workshop functionality
//-----------------------------------------------------------------------------

#pragma once

#include "steamwrapper.hpp"
#include "steamws_const.hpp"

const int steamws_langNum = 28;

// Only include and call these functions directly if absolutely necessary
// steamws_core.hpp should have everything needed for NewCity
// When passing in a char* to retrieve some string data, also pass in buffer size

//-----------------------------------------------------------------------------
// Steam Workshop API 
//-----------------------------------------------------------------------------
// Static array of valid API language values per https://partner.steamgames.com/doc/store/localization#supported_languages
const std::array<std::string, steamws_langNum> steamws_langArr ={
  "arabic",
  "bulgarian",
  "schinese",
  "tchinese",
  "czech",
  "danish",
  "dutch",
  "english",
  "finnish",
  "french",
  "german",
  "greek",
  "hungarian",
  "italian",
  "japanese",
  "koreana",
  "norwegian",
  "polish"
  "portuguese",
  "brazilian",
  "romanian",
  "russian",
  "spanish",
  "latam",
  "swedish",
  "thai",
  "turkish",
  "ukrainian",
  "vietnamese"
};

enum steamws_itemTag {
  PreviewImg = -1,
  Generic = 0,
  DesignLegacy,
  DesignPack,
  Lua,
  LuaPack,
  Model,
  ModelDecoration,
  ModelVehicle,
  ModelPack,
  Mod,
  Sound,
  SoundEnvironment,
  SoundMusic,
  SoundPack,
  Texture,
  TextureBuilding,
  TextureVehicle,
  TexturePack,
  Num_Tags
};

enum steamws_itemField {
  Title = 0,
  Desc,
  PrimaryTag,
  PreviewImgPath,
  Metadata,
  Num_Fields
};

enum steamws_action {
  NoAction = 0,
  CreateNewItem,
  UpdateItem,
  FetchItems,
  AwaitUGCQuery,
  MoveSubbedItemsToGameDir,
  InitSubscribedItems,
  SubscribeItem,
  UnsubscribeItem,
  VoteItem,
  Num_Actions
};


const std::string steamws_tagGeneric = "Generic Item";
const std::string steamws_tagDesign = "Design";
const std::string steamws_tagDesignPack = "Design Pack";
const std::string steamws_tagData = "Data";
const std::string steamws_tagDataPack = "Data Pack";
const std::string steamws_tagModel = "Model";
const std::string steamws_tagModelDecoration = "Decoration";
const std::string steamws_tagModelVehicle = "Vehicle Model";
const std::string steamws_tagModelPack = "Model Pack";
const std::string steamws_tagModpack = "Modpack";
const std::string steamws_tagSound = "Sound";
const std::string steamws_tagSoundEnvironment = "Environment Sound";
const std::string steamws_tagSoundMusic = "Music Track";
const std::string steamws_tagSoundPack = "Sound Pack";
const std::string steamws_tagTexture = "Texture";
const std::string steamws_tagTextureBuilding = "Building Texture";
const std::string steamws_tagTextureVehicle = "Vehicle Texture";
const std::string steamws_tagTexturePack = "Texture Pack";
const std::string steamws_tagKey = "tags";
const std::string steamws_defaultTagValue = steamws_tagGeneric;
const std::string steamws_tagUnknown = "Unk";
const int steamws_keyValueTagMaxLen = 255;
const int steamws_maxTags = 8; // Maximum number of unique tags a single Workshop Item can have

//-----------------------------------------------------------------------------
// Steam Workshop typedefs
//-----------------------------------------------------------------------------
struct WSFile {
  std::string dir;
  std::string name;
  std::string ext;
  std::string desc;
  steamws_itemTag tag;

  WSFile() {
    dir = "";
    name = "";
    ext = "";
    desc = "";
    tag = steamws_itemTag::Generic;
  }

  WSFile(std::string di, std::string n, std::string e, std::string de, steamws_itemTag t) {
    dir = di;
    name = n;
    ext = e;
    desc = de;
    tag = t;
  }

  std::string fullPath();

  std::string dirAndName() {
    return dir + name;
  }

  std::string nameAndExt() {
    return name + ext;
  }

  WSFile &operator=(WSFile const &other) {
    dir = other.dir;
    name = other.name;
    ext = other.ext;
    desc = other.desc;
    tag = other.tag;
    return *this;
  }
  operator const char*() { return name.c_str(); }
  operator char*() { return strdup_s(name.c_str()); }
};

struct steamws_state_struct {
private:
  steamws_action _activeAction;
  steamws_action _nextAction;
  PublishedFileId_t _activeItem;
  WSFile _activeItemFile;
  std::array<std::string, steamws_itemField::Num_Fields> _activeItemFields;
  std::array<std::string, steamws_maxTags> _activeItemTags;
  std::string _activeItemChangeTxt;
  UGCQueryHandle_t _activeQueryHandle;
  uint32_t _activeQueryNumResults;
  UGCUpdateHandle_t _activeUpdateHandle;


public:
  steamws_state_struct();
  void resetSteamWSState();

  steamws_action getActiveAction() { return _activeAction; }
  steamws_action setActiveAction(steamws_action act) { 
    _activeAction = act;
    SPDLOG_INFO("Steam Workshop state action set to {}", _activeAction); 
    return _activeAction; 
  }
  void clearActiveAction() { setActiveAction(steamws_action::NoAction); }
  steamws_action getNextAction() { return _nextAction; }
  steamws_action setNextAction(steamws_action act) { _nextAction = act; return _nextAction; }
  void setActions(steamws_action act, steamws_action next) { setActiveAction(act); setNextAction(next); }
  PublishedFileId_t getActiveItem() { return _activeItem; }
  PublishedFileId_t setActiveItem(PublishedFileId_t id) { _activeItem = id; return _activeItem; }
  WSFile setActiveItemFile(WSFile file) { _activeItemFile = file; return _activeItemFile; }
  WSFile getActiveItemFile() { return _activeItemFile; }
  void clearActiveItemInfo() { _activeItem = k_PublishedFileIdInvalid; _activeItemFile = WSFile(); }
  void setActiveItemField(steamws_itemField field, std::string value) { _activeItemFields[field] = value; }
  void setActiveItemFields(std::array<std::string, steamws_itemField::Num_Fields> valueArr) { _activeItemFields = valueArr; }
  std::string getActiveItemField(steamws_itemField field) { return _activeItemFields[field]; }
  std::array<std::string, steamws_itemField::Num_Fields> getActiveItemFields() { return _activeItemFields; }
  void setActiveItemTag(int index, std::string tag) { if(index >= 0 && index < _activeItemTags.size()) { _activeItemTags[index] = tag; } }
  void setActiveItemTags(std::array<std::string, steamws_maxTags> tagArr) { _activeItemTags = tagArr; }
  std::string getActiveItemField(int index) { if(index >= 0 && index < _activeItemTags.size()) { return _activeItemTags[index]; } return ""; }
  std::array<std::string, steamws_maxTags> getActiveItemTags() { return _activeItemTags; }
  void setActiveItemChangeTxt(std::string txt) { _activeItemChangeTxt = txt; }
  std::string getActiveItemChangeTxt() { return _activeItemChangeTxt; }
  UGCQueryHandle_t getActiveQueryHandle() { return _activeQueryHandle; }
  UGCQueryHandle_t setActiveQueryHandle(UGCQueryHandle_t handle, uint32_t numResults) { _activeQueryHandle = handle; _activeQueryNumResults = numResults; return _activeQueryHandle; }
  uint32_t getActiveQueryNumResults() { return _activeQueryNumResults; }
  void resetActiveQuery() { _activeQueryHandle = k_UGCQueryHandleInvalid; _activeQueryNumResults = 0; }
  UGCUpdateHandle_t getActiveUpdateHandle() { return _activeUpdateHandle; }
  UGCUpdateHandle_t setActiveUpdateHandle(UGCUpdateHandle_t handle) { _activeUpdateHandle = handle; return _activeUpdateHandle; }
};


//-----------------------------------------------------------------------------
// Basic item management
//-----------------------------------------------------------------------------

SteamAPICall_t steamws_itemCreate(EWorkshopFileType type);
SteamAPICall_t steamws_itemDelete(PublishedFileId_t id);
bool steamws_itemDownload(PublishedFileId_t id, bool highPriority);
SteamAPICall_t steamws_itemSubscribe(PublishedFileId_t id);
SteamAPICall_t steamws_itemUnsubscribe(PublishedFileId_t id);


//-----------------------------------------------------------------------------
// Updating items, with things like tags and metadata (UGCUpdate)
// Max 100 changes per item update (Possibly just for key value tags? Better to be safe...)
//-----------------------------------------------------------------------------

UGCUpdateHandle_t steamws_itemStartUpdate(PublishedFileId_t id); // Use to get UGCUpdateHandle for making changes and submitting update; call SubmitItemUpdate when done
bool steamws_itemAddKeyValueTag(UGCUpdateHandle_t handle, std::string key, std::string value);
bool steamws_itemRemoveKeyValueTag(UGCUpdateHandle_t handle, std::string key);
bool steamws_itemSetTags(UGCUpdateHandle_t handle, const SteamParamStringArray_t *tags); // Tag max 255 chars, and can only include printable characters
bool steamws_itemSetMetadata(UGCUpdateHandle_t handle, std::string metadata);
bool steamws_itemSetPreviewImg(UGCUpdateHandle_t handle, std::string path); // Should be .jpg, .png, or .gif
bool steamws_itemAddAdditionalPreviewFile(UGCUpdateHandle_t handle, std::string path, EItemPreviewType type); // Adds an additional preview image
bool steamws_itemRemoveItemPreviewImg(UGCUpdateHandle_t handle, uint32_t index);
bool steamws_itemSetTitle(UGCUpdateHandle_t handle, std::string title); // Max 128 chars per k_cchPublishedDocumentTitleMax
bool steamws_itemSetContent(UGCUpdateHandle_t handle, std::string path); // Set folder that represents item content
bool steamws_itemSetDescription(UGCUpdateHandle_t handle, std::string desc); // Max 5000 chars per k_cchDeveloperMetadataMax
bool steamws_itemSetVisibility(UGCUpdateHandle_t handle, ERemoteStoragePublishedFileVisibility visibility);
bool steamws_itemSetUpdateLanguage(UGCUpdateHandle_t handle, std::string language); // See Steam API language codes - https://partner.steamgames.com/doc/store/localization#supported_languages
SteamAPICall_t steamws_itemSubmitUpdate(UGCUpdateHandle_t handle, std::string changeNote); // Sends item changes to the server; must be called after all changed made to handle


//-----------------------------------------------------------------------------
// Checking item state and info
//-----------------------------------------------------------------------------
bool steamws_itemGetDownloadInfo(PublishedFileId_t id, uint64_steam *bytesDownloaded, uint64_steam *bytesTotal); // Returns true if info available, ref args to get byte info
bool steamws_itemGetInstallInfo(PublishedFileId_t id, uint64_steam *bytesOnDisk, char *folderPath, uint32_t folderSize, uint32_t *timeUpdated);
uint32_t steamws_itemGetState(PublishedFileId_t id); // Returns the item state; use EItemState flags to check state specifics
EItemUpdateStatus steamws_itemGetUpdateProgress(UGCUpdateHandle_t handle, uint64_steam *bytesUploaded, uint64_steam *bytesTotal);


//-----------------------------------------------------------------------------
// Creating item queries (UGCQuery)
//-----------------------------------------------------------------------------
UGCQueryHandle_t steamws_queryAllUGCRequest(EUGCQuery queryType, EUGCMatchingUGCType fileType, uint32_t pageNum); // Use to list all available UGC
UGCQueryHandle_t steamws_queryUGCDetailsRequest(PublishedFileId_t *id, uint32_t numPublishedFileIds); // Used to get details for specific UGC items
UGCQueryHandle_t steamws_queryUserUGCRequest(AccountID_t id, EUserUGCList listType, EUGCMatchingUGCType fileType, EUserUGCListSortOrder sortOrder, uint32_t pageNum); // Used to query UGC associated with a user
bool steamws_queryAddRequiredKeyValueTag(UGCQueryHandle_t handle, std::string key, std::string value);
bool steamws_queryAddRequiredTag(UGCQueryHandle_t handle, std::string tag);
bool steamws_queryAddExcludedTag(UGCQueryHandle_t handle, std::string tag);
bool steamws_querySetText(UGCQueryHandle_t handle, std::string text); // Can only be used with steamws_queryAllUGCRequest
bool steamws_querySetMatchAnyTag(UGCQueryHandle_t handle, bool anyTag); // Can only be used with steamws_queryAllUGCRequest
bool steamws_querySetAllowCachedResponse(UGCQueryHandle_t handle, uint32_t maxAgeSeconds);
bool steamws_querySetCloudFilenameFilter(UGCQueryHandle_t handle, std::string filename); // Set to only return results that have a specific filename; Can only be used with steamws_queryUserUGCRequest
bool steamws_querySetReturnTotalOnly(UGCQueryHandle_t handle, bool totalOnly);
bool steamws_querySetReturnOnlyIds(UGCQueryHandle_t handle, bool idsOnly); // Only return item Ids, like when checking items a user has in favorites
bool steamws_querySetReturnKeyValueTags(UGCQueryHandle_t handle, bool returnKeyValueTags);
bool steamws_querySetReturnMetadata(UGCQueryHandle_t handle, bool metadata);
bool steamws_querySetReturnLongDescription(UGCQueryHandle_t handle, bool longDesc); // Set whether to return the full description for items; otherwise summary truncated at 255 bytes
bool steamws_querySetReturnAdditionalPreviews(UGCQueryHandle_t handle, bool additionalPreviews);
bool steamws_querySetRankedByTrendDays(UGCQueryHandle_t handle, uint32_t daysToRank); // uint32_t arg sets number of days to rank items over, valid values are 1-365 inclusive
bool steamws_querySetLanguage(UGCQueryHandle_t handle, std::string lang); // See Steam API language codes - https://partner.steamgames.com/doc/store/localization#supported_languages
SteamAPICall_t steamws_querySendUGCRequest(UGCQueryHandle_t handle); // Send the created UGC request to Steam


//-----------------------------------------------------------------------------
// Processing item queries (UGCQuery)
//-----------------------------------------------------------------------------
bool steamws_queryGetUGCAdditionalPreview(UGCQueryHandle_t handle, uint32_t index, uint32_t previewIndex, char *urlOrVideoId, uint32_t urlSize, char *originalFilename, uint32_t filenameSize, EItemPreviewType *type);
bool steamws_queryGetUGCKeyValueTag(UGCQueryHandle_t handle, uint32_t index, uint32_t keyValueTagIndex, char *key, uint32_t keySize, char *value, uint32_t valueSize);
bool steamws_queryGetUGCMetadata(UGCQueryHandle_t handle, uint32_t index, char *metadata, uint32_t metadataSize);
uint32_t steamws_queryGetQueryUGCNumAdditionalPreviews(UGCQueryHandle_t handle, uint32_t index);
uint32_t steamws_queryGetUGCNumKeyValueTags(UGCQueryHandle_t handle, uint32_t index);
bool steamws_queryGetUGCPreviewURL(UGCQueryHandle_t handle, uint32_t index, char *url, uint32_t urlSize);
bool steamws_queryGetUGCResult(UGCQueryHandle_t handle, uint32_t index, SteamUGCDetails_t *details);
bool steamws_queryGetUGCStatistic(UGCQueryHandle_t handle, uint32_t index, EItemStatistic statType, uint64_steam *statValue);


//-----------------------------------------------------------------------------
// Cleaning up queries (UGCQuery)
//-----------------------------------------------------------------------------
bool steamws_queryReleaseUGCRequest(UGCQueryHandle_t handle); // Call to free up memory when finished with a UGCQuery


//-----------------------------------------------------------------------------
// User experience
//-----------------------------------------------------------------------------
SteamAPICall_t steamws_userAddItemToFavorites(PublishedFileId_t id);
SteamAPICall_t steamws_userRemoveItemFromFavorites(PublishedFileId_t id);
SteamAPICall_t steamws_userGetItemVote(PublishedFileId_t id);
SteamAPICall_t steamws_userSetItemVote(PublishedFileId_t id, bool voteUp);
uint32_t steamws_userGetNumSubscribedItems();
uint32_t steamws_userGetSubscribedItems(PublishedFileId_t *itemArr, uint32_t maxEntries);


//-----------------------------------------------------------------------------
// Util
//-----------------------------------------------------------------------------
bool steamws_langIsValid(std::string lang);
std::array<std::string, steamws_langNum> steamws_getLangArray();
std::string steamws_getStringForTag(steamws_itemTag type);
std::string steamws_getImageForItem(WSFile file);
steamws_itemTag steamws_getTagForString(std::string str);
