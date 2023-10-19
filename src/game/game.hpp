#pragma once

#include "../item.hpp"
#include "../serialize.hpp"

enum GameDifficulty {
  DifficultyEasy = 0, DifficultyMedium, DifficultyHard,
  DifficultyVeryHard, numDifficulties
};

const char* const difficultyName[] = {
  "Easy", "Medium", "Hard", "Very Hard"
};

enum GameMode {
  ModeGame, ModeTest, ModeBuildingDesigner,
  ModeDesignOrganizer,
  numGameModes
};

enum LoadMode {
  LoadTargetAutosave, LoadTargetFilename, LoadTargetNew,
  GameLoading, GameUnrendered, GameSimulating, GameReady,
  ErrorLoad, Autosaving,
  numLoadModes
};

void initGame();
void updateGame();
void resetAll();
void resetRender();

const char* saveDirectory();
const char* saveDirectory(item gm);
const char* designDirectory();
const char* designExtension();
const char* fileExtension();
const char* fileExtension(item gm);
const char* dataDirectory();
const char* gameRootDirectory();
const char* imagesDirectory();
const char* modelsDirectory();
const char* modpacksDirectory();
const char* soundDirectory();
const char* texturesDirectory();

void gameLoad(LoadMode lm, GameMode gm, char* filename);
bool crashAutosave();
bool autosave();
bool autosave(FileBuffer* file);
void updateAutosave(double duration);
void doAutosave(double duration);
bool testDoAutosave(double duration);
bool nextFrameWillAutosave();
bool saveGame(char* name);
bool saveGame(char* filename, bool saveTestAsCity);
bool readHeader(FileBuffer* file);
const char* getCityName();
char** getCityNamePtr();
char** getNextCityNamePtr();
char* getSaveFilename(const char* name);
void asyncCompressAndWrite(FileBuffer* buffer, char* filename,
    char* autosaveFilename);

GameMode getGameMode();
void setIsPresimulating(bool val);
bool isPresimulating();
bool getGameAccelerated();
bool getNextGameAccelerated();
void setGameAccelerated(bool accel);
void setNextGameAccelerated(bool accel);
bool getNextLeftHandTraffic();
void setNextLeftHandTraffic(bool val);
void suggestGameMode(item gameMode);
void initAutosave();
void setAutosaveInterval(float time);
float getAutosaveInterval();
GameDifficulty getGameDifficulty();
GameDifficulty getNextGameDifficulty();
void setGameDifficulty(GameDifficulty diff);
void setNextGameDifficulty(GameDifficulty diff);
const char* getLoaderText();
LoadMode getLoadMode();
void toggleGamePause();
bool isGamePaused();
bool isGameLoaded();
bool isGameLoading();
bool isGameWaitingForSwap();
bool isGameSaving();
void setGameSpeed(item speed);
void setGameMenuPause(bool pause);
item getGameSpeed();
item getEffectiveGameSpeed();
item getUnpausedSpeed();
void setGameLoaded(bool val);
void doErrorLoad();
void doErrorLoadNewGame();
void clearErrorLoad();
void newBuildingDesigner();
void winGame(item building);

