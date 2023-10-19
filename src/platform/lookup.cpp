#include "lookup.hpp"

#include "mod.hpp"
#include "fcaseopen.hpp"
#include "file.hpp"

#include "../string_proxy.hpp"

#include "spdlog/spdlog.h"

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
#include <set>
#include <algorithm>

using namespace std::experimental::filesystem;

string fixFileCase(string in) {
  char* result = fixFileCase(strdup_s(in.c_str()));
  string resultStr = result;
  free(result);
  return resultStr;
}

bool endsWith(std::string const &fullString, std::string const &ending) {
  int fsLength = fullString.length();
  int endLength = ending.length();
  if (fsLength >= endLength) {
    return (0 == fullString.compare(fsLength - endLength, endLength, ending));
  } else {
    return false;
  }
}

vector<string> lookupFileCandidates(string filename, uint32_t flags) {
  vector<string> candidates;
  bool ws = !(flags & _lookupExcludeWorkshop);
  bool anyMod = (flags & _lookupForceMod) || (
      isModsEnabled() && (
       !(flags & _lookupExcludeBaseMods) ||
       !(flags & _lookupExcludeWorkshopMods)));

  if (!(flags & _lookupExcludeWorkshop)) {
    candidates.push_back("workshop/" + filename);
  }

  if (anyMod) {
    string modFile = modDirectoryNonNull() + filename;
    if (!(flags & _lookupExcludeWorkshopMods)) {
      candidates.push_back("workshop/" + modFile);
    }
    if (!(flags & _lookupExcludeBaseMods)) {
      candidates.push_back(modFile);
    }
  }

  if (!(flags & _lookupExcludeBase)) candidates.push_back(filename);

  for (int i = 0; i < candidates.size(); i++) {
    candidates[i] = fixFileCase(candidates[i]);
  }

  return candidates;
}

vector<string> lookupFileVersions(string filename, uint32_t flags) {
  vector<string> results;
  vector<string> candidates = lookupFileCandidates(filename, flags);

  for (int i = 0; i < candidates.size(); i++) {
    if (fileExists(candidates[i])) {
      results.push_back(candidates[i]);
    }
  }

  return results;
}

string lookupFile(string filename, uint32_t flags) {
  vector<string> versions = lookupFileVersions(filename, flags);
  if (versions.size() == 0) return filename;
  return versions[0];
}

/*
bool compareStrings(string a, string b) {
  return strcmpi_s(a.c_str(), b.c_str()) < 0;
}
*/

bool compareStrings(const string& lhs,const string& rhs){

   string::size_type common_length = std::min(lhs.length(),rhs.length());

   for(string::size_type i=0;i<common_length;++i){
      if(toupper(lhs[i]) < toupper(rhs[i]))return true;
      if(toupper(lhs[i]) > toupper(rhs[i]))return false;
   }

   if(lhs.length()<rhs.length())return true;
   if(lhs.length()>rhs.length())return false;//can ignore

   return false;//equal should return false
}

struct lessString : binary_function <string, string, bool> {
  bool operator() (string const& x, string const& y) const {
    return compareStrings(x, y);
  }
};

vector<string> lookupDirectory(string dir, string ext, uint32_t flags) {
  vector<string> dirVersions = lookupFileVersions(dir, flags);
  int extLength = ext.length();
  lessString helper;

  set<string, lessString> resultSet;
  for (int i = 0; i < dirVersions.size(); i++) {
    std::error_code err;
    for(auto f: directory_iterator(dirVersions[i], err)) {
      string filename = f.path().filename().string();
      if (endsWith(filename, ext)) {
        string filenameWOExt =
          filename.substr(0, filename.length() - extLength);
        resultSet.insert(filenameWOExt);
      }
    }
  }

  vector<string> result(resultSet.begin(), resultSet.end());
  sort(result.begin(), result.end(), helper);
  return result;
}

vector<string> lookupSubDirectories(string dir, uint32_t flags) {
  vector<string> dirVersions = lookupFileVersions(dir, flags);
  lessString helper;

  set<string, lessString> resultSet;
  for (int i = 0; i < dirVersions.size(); i++) {
    std::error_code err;
    for(auto f: directory_iterator(dirVersions[i], err)) {
      if (is_directory(f.path().string())) {
        string filename = f.path().filename().string();
        resultSet.insert(filename);
      }
    }
  }

  vector<string> result(resultSet.begin(), resultSet.end());
  sort(result.begin(), result.end(), helper);
  return result;
}

string lookupSave(string filename) {
  return modDirectoryNonNull() + filename;
}

vector<string> listModpacks(uint32_t flags) {
  return lookupSubDirectories("modpacks",
      flags | _lookupExcludeBaseMods | _lookupExcludeWorkshopMods);
}

/// Returns all existing designs, including design packages and legacy designs,
/// sorted alphabetically, with no duplicates, case insensitive.
/// Design packages end with '/', legacy designs do not.
/// The filenames returned do not have the file extension.
vector<string> lookupDesigns(uint32_t flags) {
  // legacy
  vector<string> result = lookupDirectory("designs", ".design", flags);

  // packages
  vector<string> packages = lookupSubDirectories("designs", flags);
  for (int i = 0; i < packages.size(); i++) {
    string package = packages[i];
    string canonDesignFile = lookupFile("designs/" + package + "/design.design", flags);
    if (!fileExists(canonDesignFile)) continue;
    package += "/";
    result.push_back(package);
  }

  // sort
  lessString helper;
  sort(result.begin(), result.end(), helper);
  return result;
}

