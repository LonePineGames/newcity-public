#include "file.hpp"

#include "mod.hpp"
#include "fcaseopen.hpp"

#include "../error.hpp"
#include "../string_proxy.hpp"
#include "../steam/steamws_const.hpp"

#include "spdlog/spdlog.h"
#include <algorithm>
#include <stdio.h>
#include <set>
#include <sys/stat.h>

namespace fs = std::experimental::filesystem;

#if defined (__linux__) || defined (__APPLE__)
  #include <dirent.h>
#else
  #include <windows.h>
#endif

#ifdef WIN32
const int maxPathLen = _MAX_PATH;
#else
const int maxPathLen = 256;
#endif


bool compareStrings(const char* a, const char* b) {
  return strcmpi_s(a, b) < 0;
}

bool fileExists(const char* filename) {
  // fopen "r" mode throws an exception and crashes the program if
  // the file doesn't exist in certain situations. Switching to
  // stat() to check if the file exists per the following:
  // https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c
  
  struct stat buffer;
  return (stat(filename, &buffer) == 0);
  
  // Old approach
  /*
  FILE* file;
  if(file = fopen(filename, "r")) {
    fclose(file);
    return true;
  } else {
    return false;
  }
  */
}

bool fileExists(string filename) {
  return fileExists(filename.c_str());
}

char* getModFilePath(const char* filename) {
  return sprintf_o("%s%s", modDirectory(), filename);
}

char* getModFilePathNonNull(const char* filename) {
  return sprintf_o("%s%s", modDirectoryNonNull(), filename);
}

char* getWorkshopFilePath(const char* filename) {
  return sprintf_o("%s/%s", steamws_rootPath.c_str(), filename);
}

char* getFilenameInBase(const char* filename) {
  return fixFileCase(strdup_s(filename));
}

char* getFilenameInMod(const char* filename, bool nonNull) {
  char* modFilename = nonNull ?
    getModFilePathNonNull(filename) : getModFilePath(filename);
  modFilename = fixFileCase(modFilename);
  if (fileExists(modFilename)) {
    return modFilename;
  } else {
    free(modFilename);
    return getFilenameInBase(filename);
  }
}

char* getFilenameInWorkshop(const char* filename) {
  char* workshopFilename = getWorkshopFilePath(filename);
  if(fileExists(workshopFilename)) {
    return workshopFilename;
  }
  free(workshopFilename);
  return getFilenameInBase(filename); // Failed to find workshop file, search base
}

char* getFilenameInAll(const char* filename, bool nonNull) {
  // Test Steam Workshop first
  char* workshopFilename = getWorkshopFilePath(filename);
  if(fileExists(workshopFilename)) {
    return workshopFilename;
  }
  free(workshopFilename); // Free when done

  // Test Mods next
  char* modFilename = getFilenameInMod(filename, false);
  if(fileExists(modFilename)) {
    return modFilename;
  }
  free(modFilename); // Free when done

  // Assume base exists
  return getFilenameInBase(filename);
}

char* getFilenameInMod(const char* filename) {
  return getFilenameInMod(filename, false);
}

char* getFilenameInAll(const char* filename) {
  return getFilenameInAll(filename, false);
}

char* getFilenameInAllNonNull(const char* filename) {
  return getFilenameInAll(filename, true);
}

char* getFilenameInModNonNull(const char* filename) {
  return getFilenameInMod(filename, true);
}

vector<char*> readDirectory(const char* dirName, const char* ext) {
  vector<char*> files;
  int extLength = strlength(ext);

  #if defined (__linux__) || defined (__APPLE__)
    DIR* d;
    struct dirent *dir;
    d = opendir(dirName);
    while ((dir = readdir(d)) != NULL) {
      char *name = strdup_s(dir->d_name);
      if (name != 0 && name[0] != '.' && endsWith(name, ext)) {
        name[strlength(name)-extLength] = '\0';
        files.push_back(name);
      } else if (name != 0) {
        free(name);
      }
    }
    closedir(d);

  #else
    char* findPattern = sprintf_o("%s\\*", dirName);
    WIN32_FIND_DATA data;
    HANDLE hFind;
    if ((hFind = FindFirstFile(findPattern, &data)) !=
        INVALID_HANDLE_VALUE) {
      do {
        char *name = strdup_s(data.cFileName);
        if (name[0] != '.' && endsWith(name, ext)) {
          name[strlength(name)-extLength] = '\0';
          files.push_back(name);
        } else {
          free(name);
        }
      } while (FindNextFile(hFind, &data) != 0);
      FindClose(hFind);
    }
  #endif

  sort(files.begin(), files.end(), compareStrings);
  return files;
}

struct less_char_star : binary_function <char*,char*,bool> {
  bool operator() (char* const& x, char* const& y) const {
    int cmp = strcmpi_s(x, y);
    return cmp < 0;
  }
};

vector<char*> readDirectoryMod(const char* dirName, const char* ext,
    bool nonNull) {
  vector<char*> mainFiles = readDirectory(dirName, ext);
  vector<char*> result;

  const char* mod = nonNull ? modDirectoryNonNull() : modDirectory();
  char* modDir = sprintf_o("%s%s", mod, dirName);
  #ifdef __linux__
    if (!fileExists(modDir)) return mainFiles;
  #endif
  vector<char*> modFiles = readDirectory(modDir, ext);
  free(modDir);

  if(modFiles.size() > 0) {
    set<char*,less_char_star> dedupe(modFiles.begin(),modFiles.end());
    dedupe.insert(mainFiles.begin(),mainFiles.end());
    result = vector<char*>(dedupe.begin(),dedupe.end());
  } else {
    result = mainFiles;
  }

  return result;
}

vector<char*> readDirectoryMod(const char* dirName, const char* ext) {
  return readDirectoryMod(dirName, ext, false);
}

vector<char*> readDirectoryModNonNull(const char* dirName, const char* ext) {
  return readDirectoryMod(dirName, ext, true);
}

vector<char*> readDirectoryModOnly(const char* dirName,
    const char* ext, bool nonNull) {
  const char* mod = nonNull ? modDirectoryNonNull() : modDirectory();
  char* modDir = sprintf_o("%s%s", mod, dirName);
  #ifdef __linux__
    if (!fileExists(modDir)) return vector<char*>();
  #endif
  vector<char*> modFiles = readDirectory(modDir, ext);
  free(modDir);

  return modFiles;
}

vector<char*> readDirectoryModOnlyNonNull(const char* dirName,
    const char* ext) {
  return readDirectoryModOnly(dirName, ext, true);
}

vector<char*> readDirectoryModOnly(const char* dirName,
    const char* ext) {
  return readDirectoryModOnly(dirName, ext, false);
}

vector<char*> readDirectoryWorkshopOnly(const char* dirName, const char* ext) {
  std::string relativeDir = steamws_rootPath + "/" + dirName;
  if(!fileExists(relativeDir.c_str())) return vector<char*>();

  vector<char*> workshopFiles = readDirectory(relativeDir.c_str(), ext);

  return workshopFiles;
}

vector<char*> readDirectoryModAndWorkshopOnly(const char* dirName, const char* ext, bool nonNull) {
  vector<char*> modFiles = readDirectoryModOnly(dirName, ext, nonNull);
  vector<char*> workshopFiles = readDirectoryWorkshopOnly(dirName, ext);
  vector<char*> combinedFiles;

  if(workshopFiles.size() > 0) {
    set<char*, less_char_star> dedupe(workshopFiles.begin(), workshopFiles.end());
    dedupe.insert(modFiles.begin(), modFiles.end());
    combinedFiles = vector<char*>(dedupe.begin(), dedupe.end());
  } else {
    combinedFiles = modFiles;
  }

  return combinedFiles;
}

vector<char*> readDirectoryModAndWorkshopOnly(const char* dirName, const char* ext) {
  return readDirectoryModAndWorkshopOnly(dirName, ext, false);
}

vector<char*> readDirectoryModAndWorkshopOnlyNonNull(const char* dirName, const char* ext) {
  return readDirectoryModAndWorkshopOnly(dirName, ext, true);
}

vector<char*> readDirectoryAll(const char* dirName, const char* ext, bool nonNull) {
  vector<char*> baseAndModFiles = readDirectoryMod(dirName, ext, nonNull);
  vector<char*> workshopFiles = readDirectoryWorkshopOnly(dirName, ext);
  vector<char*> combinedFiles;

  if(workshopFiles.size() > 0) {
    set<char*, less_char_star> dedupe(workshopFiles.begin(), workshopFiles.end());
    dedupe.insert(baseAndModFiles.begin(), baseAndModFiles.end());
    combinedFiles = vector<char*>(dedupe.begin(), dedupe.end());
  } else {
    combinedFiles = baseAndModFiles;
  }

  return combinedFiles;
}

vector<char*> readDirectoryAll(const char* filename, const char* ext) {
  return readDirectoryAll(filename, ext, false);
}

vector<char*> readDirectoryAllNonNull(const char* filename, const char* ext) {
  return readDirectoryAll(filename, ext, true);
}

/*
void create_directories(fs::path p) {
  if (!fs::is_directory(p.parent_path())) {
    create_directories(p.parent_path());
  }

  #if defined(_WIN32)
    if (!CreateDirectory(p.c_str(), NULL)) {
      int nError = GetLastError();
      if (nError != ERROR_ALREADY_EXISTS) {
        handleError("Couldn't make directory %s (Error %d during creation)",
            dirName, nError);
      }
    }
  #else
    int nError = 0;
    mode_t nMode = 0733; // UNIX style permissions
    nError = mkdir(p.c_str(), nMode);

    if (nError != 0 && nError != -1) { // -1 is exists
      handleError("Couldn't make directory %s (error %d)", dirName, nError);
    }
  #endif
}
*/

void makeDirectory(const char* dirName) {
  fs::path p(dirName);
  fs::create_directories(p);
}

void makeDirectoryForFilename(const char* filename) {
  fs::path p(filename);
  if (!fs::is_directory(p.string())) {
    p = p.parent_path();
  }
  fs::create_directories(p);
}

string getParentDirectory(string filepath) {
  fs::path p(filepath);
  return p.parent_path().string();
}

bool copyDir(std::string dirPath, std::string destPath, fs::copy_options copyRule) {
  if(!fileExists(dirPath.c_str())) {
    SPDLOG_ERROR("Could not copy dir ({}), dir does not exist...", dirPath);
    return false;
  }

  SPDLOG_INFO("Copying directory {} to {}", dirPath, destPath);
  fs::copy(dirPath, destPath, copyRule);

  // Return success if destPath directory now exists
  return fileExists(destPath.c_str());
}

bool copyFile(std::string filePath, std::string destPath) {
  makeDirectoryForFilename(destPath.c_str());
  std::error_code err;
  fs::copy(fs::path(filePath), fs::path(destPath), err);
  if (err) {
    string msg = err.message();
    handleError("Could not copy %s to %s: %s", filePath.c_str(), destPath.c_str(), msg.c_str());
  }
  return true;

  /*
  bool success;
  #if defined (__linux__) || defined (__APPLE__)
    // Hey Lone Pine, wanna make this pretty and better? -supersoup
    std::string cmd = "cp " + filePath + " " + destPath;
    // 0 - success, 1 - failed
    success = !system(cmd.c_str());
    return success;
  #elif WIN32
    success = CopyFileA(filePath.c_str(), destPath.c_str(), false);
    return success;
  #endif
  */
}

std::string getAbsolutePath(std::string relPath) {
  SPDLOG_INFO("getAbsolutePath {} {}", maxPathLen, relPath);
  std::string absPath = "";
  #if defined (__linux__) || defined (__APPLE__)
    absPath = realpath(relPath.c_str(), NULL);
  #else
    char pathBuffer[maxPathLen]; // Used as a staging area when retrieving absolute paths
    absPath = _fullpath(pathBuffer, relPath.c_str(), maxPathLen);
  #endif
  return absPath;
}

bool deleteFile(string filePath) {
  std::error_code err;
  fs::remove(fs::path(filePath), err);
  if (err) {
    string msg = err.message();
    handleError("Could not delete %s: %s", filePath.c_str(), msg.c_str());
    return false;
  } else {
    return true;
  }
}

bool deleteDir(string filePath) {
  std::error_code err;
  fs::remove_all(fs::path(filePath), err);
  if (err) {
    string msg = err.message();
    handleError("Could not delete %s: %s", filePath.c_str(), msg.c_str());
    return false;
  } else {
    return true;
  }
}

bool deleteFilesInDir(std::string dir, bool subDirs) {
  if(!fileExists(dir.c_str())) {
    SPDLOG_ERROR("Could not delete files within dir ({}), dir does not exist...", dir);
    return false;
  }

  SPDLOG_INFO("Deleting files in {} ...", dir);

  std::error_code err;
  bool isDir;
  for(auto f: fs::directory_iterator(dir, err)) {
    isDir = fs::is_directory(f.path().string());
    if(isDir && subDirs) {
      SPDLOG_INFO("Found dir {}, removing contained files and subdirs ... ", f.path().string());
      fs::remove_all(f.path(), err);
    } else {
      SPDLOG_INFO("Found file {}, removing ... ", f.path().string());
      fs::remove(f.path(), err);
    }
  }

  return true;
}

vector<char*> getSubfoldersInDir(std::string dir) {
  if(!fileExists(dir.c_str())) {
    SPDLOG_ERROR("Could not get subfolders as char* within dir ({}), dir does not exist...", dir);
    return vector<char*>();
  }

  vector<char*> subfolders = vector<char*>();

  SPDLOG_INFO("Getting subfolders as char* in {}", dir);

  char *element = "";
  std::error_code err;
  bool isDir;
  for(auto f: fs::directory_iterator(dir, err)) {
    isDir = fs::is_directory(f.path().string());
    if(isDir) {
      element = strdup_s(f.path().filename().string().c_str());
      SPDLOG_INFO("Found subfolder {}", element);
      subfolders.push_back(element);
    }
  }

  return subfolders;
}

vector<std::string> getSubfoldersInDir(std::string dir, bool fullPath) {
  if(!fileExists(dir.c_str())) {
    SPDLOG_ERROR("Could not get subfolders within dir ({}), dir does not exist...", dir);
    return vector<std::string>();
  }

  vector<std::string> subfolders = vector<std::string>();

  SPDLOG_INFO("Getting subfolders in {}", dir);

  std::string element = "";
  std::error_code err;
  bool isDir;
  for(auto f: fs::directory_iterator(dir, err)) {
    isDir = fs::is_directory(f.path().string());
    if(isDir) {
      if(fullPath) {
        element = f.path().string();
      } else {
        element = f.path().filename().string();
      }
      SPDLOG_INFO("Found subfolder {}", element);
      subfolders.push_back(element);
    }
  }

  return subfolders;
}
