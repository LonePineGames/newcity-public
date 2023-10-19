#pragma once

#include <cstdint>
#include <vector>
#include <string>
using namespace std;

const uint32_t _lookupForceMod = 1 << 0;
const uint32_t _lookupExcludeWorkshop = 1 << 1;
const uint32_t _lookupExcludeWorkshopMods = 1 << 2;
const uint32_t _lookupExcludeBaseMods = 1 << 3;
const uint32_t _lookupExcludeBase = 1 << 4;

const uint32_t _lookupOnlyBase =
  _lookupExcludeWorkshop | _lookupExcludeWorkshopMods | _lookupExcludeBaseMods;

const uint32_t _lookupOnlyMods =
  _lookupExcludeWorkshop | _lookupExcludeBase;

const uint32_t _lookupExcludeMods =
  _lookupExcludeWorkshopMods | _lookupExcludeBaseMods;

/// Returns all candidate locations for a file,
/// including some that may not exist.
vector<string> lookupFileCandidates(string filename, uint32_t flags);

/// Returns all locations a file can be found,
/// and only those that actually exist.
vector<string> lookupFileVersions(string filename, uint32_t flags);

/// Returns an existing canonical location for a file.
/// If the file doesn't exist anywhere, returns filename.
string lookupFile(string filename, uint32_t flags);

/// Returns all existing files in every version of a directory,
/// sorted alphabetically, with no duplicates, case insensitive.
/// The filenames returned do not have the file extension.
vector<string> lookupDirectory(string dir, string ext, uint32_t flags);

/// Returns all existing designs, including design packages and legacy designs,
/// sorted alphabetically, with no duplicates, case insensitive.
/// Design packages end with '/', legacy designs do not.
/// The filenames returned do not have the file extension.
vector<string> lookupDesigns(uint32_t flags);

/// Returns all existing subdirectories in every version of a directory,
/// sorted alphabetically, with no duplicates, case insensitive.
/// The filenames returned do not have the file extension.
vector<string> lookupSubDirectories(string dir, uint32_t flags);

/// Returns the preferred save location for a file.
/// This is always the player's current modpack,
/// and never in the workshop directory.
string lookupSave(string filename);

/// Returns all existing modpacks.
vector<string> listModpacks(uint32_t flags);

