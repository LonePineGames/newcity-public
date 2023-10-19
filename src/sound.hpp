#pragma once

const int _soundError         = 1 << 0;
const int _soundClickPrimary  = 1 << 1;
const int _soundClickComplete = 1 << 2;
const int _soundClickCancel   = 1 << 3;
const int _soundHover         = 1 << 4;
const int _soundNotification  = 1 << 5;
const int _soundSaveLoad      = 1 << 6;
const int _soundStartup       = 1 << 7;
const int _soundThunder       = 1 << 8;
const int _soundWin           = 1 << 9;
const int numSounds = 10;

enum VolumeControl {
  MasterVolume, MusicVolume, UIVolume, EnvironmentVolume,
  numVolumes
};

void initSound();
void swapSound();
void soundLoop();
void resetSound();
void updateSoundEnvironment();

void playSound(int soundNum);
void playErrorSound();
void tryPlayRandomSong(bool playNow);
void tryPlaySong(int songNum, bool playNow);
void displaySongDebug();
void vehicleAudible_g(int modelNdx);

bool getSongsEnabled();
void setSongsEnabled(bool enabled);
float getVolume(VolumeControl ndx);
void setVolume(VolumeControl ndx, float volume);
bool getMuted(VolumeControl ndx);
void setMuted(VolumeControl ndx, bool muted);
const char* getVolumeName(VolumeControl ndx);

