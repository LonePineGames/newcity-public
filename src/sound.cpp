#ifndef BOOST_ALL_NO_LIB
	#define BOOST_ALL_NO_LIB
#endif

#include "sound.hpp"

#include "item.hpp"
#include "error.hpp"
#include "option.hpp"
#include "platform/mod.hpp"
#include "parts/optionsPanel.hpp"
#include "platform/file.hpp"
#include "string_proxy.hpp"
#include "util.hpp"
#include "vorbis_proxy.hpp"
#include "thread.hpp"

#include "spdlog/spdlog.h"
#include <stdio.h>
#include <time.h>
#include <queue>
#include "AL/al.h"
#include "AL/alc.h"

#include <boost/lockfree/spsc_queue.hpp>
using namespace boost::lockfree;

struct SoundData {
  int uiSounds = 0;
  float volume[numVolumes] = {0.5, 0.5, 0.5, 0.5};
  bool muted[numVolumes] = {false, false, false, false};
  bool updateVolumes = true;
  item environmentSound = -1;
  bool paused = false;
  vec3 envSoundLoc;
  vec3 cameraLoc;
  vec3 cameraLook;
  float cameraDistance;
};

const float constAmplification[numVolumes] = {1.0, 0.5, 1.0, 1.0};
const std::string songDefaultDir = "sound/music";
const int songMaxErrors = 3;

const char* soundFiles[numSounds] = {
  "Error", "Click Primary", "Click Complete",
  "Click Cancel", "Hover", "Notification", "SaveLoad",
  "Startup", "Thunder", "Fireworks"
};

const char* volumeNames[numVolumes] = {
  "Master", "Music", "Sound Effects", "Environment"
};

spsc_queue<SoundData, capacity<1024>> soundDataQueue;
std::queue<SoundData> toEnqueue;
SoundData soundData, nextSoundData;
static std::atomic<bool> shouldContinue(true);
static std::atomic<bool> soundResetting(false);
static vector<string> songFiles;

static ALuint soundBuffers[numSounds];
static vector<ALuint> sources;
static vector<uint8_t> sourceIsUiVector;
static int numSources = 0;
static int nextSource = 0;
static ALuint errorSource;

static VorbisStream* musicStream = 0;
static const int numMusicShorts = 4096*2;
static const int queueTarget = 400;
static short* musicData = 0;
static ALuint musicSource;
static item songSeed = 1000001;
static const int numPrevSongs = 4;
static int prevSongs[numPrevSongs] = {-1};
static float songTimer = 0.0f;
static int songErrorCount = 0;
static bool songsEnabled = true;

void loadSound(ALuint buffer, char* filename);
void updateSound();
#include "soundEnvironment.cpp"

bool getSongsEnabled() {
  return songsEnabled;
}

void setSongsEnabled(bool enabled) {
  songsEnabled = enabled;
}

void resetSongs() {
  SPDLOG_INFO("Resetting song files...");
  songFiles.clear();
  for (int i = 0; i < numPrevSongs; i++) {
    prevSongs[i] = -1;
  }
}

void loadSongsFromDir(std::string dir) {
  songFiles = lookupDirectory(dir, ".ogg", 0);
}

void resetSound() {
  if (!c(CEnableSounds)) return;

  SPDLOG_INFO("Resetting Sound");
  shouldContinue = false;
  soundResetting = true;
  resetSoundEnvironment();

  // Fade out
  float origMaster = soundData.volume[MasterVolume];
  float fadeOutTime = 1;
  float fadeOutStepLength = 1.0/60.0;
  auto sleepTime = std::chrono::microseconds(int(fadeOutStepLength * 1000000));
  for (float i = 0; i < fadeOutTime; i += fadeOutStepLength) {
    soundData.volume[MasterVolume] = origMaster * (1-i/fadeOutTime);
    soundData.updateVolumes = true;
    updateSound();
    std::this_thread::sleep_for(sleepTime);
  }
  SPDLOG_INFO("Fade Out Done");

  ALCcontext* context =  alcGetCurrentContext();
  ALCdevice* device = alcGetContextsDevice(context);
  alcMakeContextCurrent(NULL);
  alcDestroyContext(context);
  alcCloseDevice(device);

  std::queue<SoundData> empty;
  std::swap(toEnqueue, empty);
  soundDataQueue.reset();

  for (int i = 0; i < numSounds; i++) {
    soundBuffers[i] = 0;
  }

  sources.clear();
  sourceIsUiVector.clear();
  numSources = 0;
  nextSource = 0;
  errorSource = 0;

  if (musicStream != 0) {
    stopOggStream(musicStream);
    musicStream = 0;
  }

  if (musicData != 0) free(musicData);
  musicData = 0;
  musicSource = 0;
  songTimer = 0.0f;

  resetSongs();

  soundResetting = false;
  SPDLOG_INFO("Sound Reset");
}

void playSound(int soundNum) {
  nextSoundData.uiSounds |= soundNum;
}

void playErrorSound() {
  alSourcePlay(errorSource);
}

std::string getALErrorStr(int err) {
  std::string errStr = "";

  switch(err) {
    case AL_NO_ERROR:
      errStr = "No error";
      break;
    case AL_INVALID_NAME:
      errStr = "Invalid name/ID";
      break;
    case AL_INVALID_ENUM:
      errStr = "Invalid enum value";
      break;
    case AL_INVALID_VALUE:
      errStr = "Invalid value";
      break;
    case AL_INVALID_OPERATION:
      errStr = "Requested an invalid op";
      break;
    case AL_OUT_OF_MEMORY:
      errStr = "Out of memory";
      break;
    default:
      errStr = "Unknown error";
      break;
  }

  return errStr;
}

void testSoundError(const char* message) {
  int err = alGetError();
  if (err != AL_NO_ERROR) {
    char* errMsg = sprintf_o("%s: %d (%s) %s", message, err,
        getALErrorStr(err).c_str(), alGetString(err));
    SPDLOG_ERROR(errMsg);
    free(errMsg);
  }
}

void setSourceIsUI(int num, bool val) {
  uint8_t byte = sourceIsUiVector[num/8];
  if (val) {
    byte |= (1 << (num%8));
  } else {
    byte &= ~(1 << (num%8));
  }
  sourceIsUiVector[num/8] = byte;
  alSourcef(sources[num], AL_GAIN,
      soundData.volume[val ? UIVolume : EnvironmentVolume]);
}

bool sourceIsUI(int num) {
  return sourceIsUiVector[num/8] & (1 << (num%8));
}

void loadSound(ALuint buffer, char* filename) {
  int channels;
  int sampleRate;
  short* data;
  string modFilename = lookupFile(filename, 0);
  int numSamples = loadOgg(modFilename.c_str(), &channels, &sampleRate, &data);
  //SPDLOG_INFO("loadOgg {}: channels {}, rate {}, samples {}", filename,
      //channels, sampleRate, numSamples);

  if(numSamples < 0)
    return;

  std::string errTxt = "Error w/ alBufferData loadSound for file: ";
  errTxt += filename;

  ALuint format = channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
  alBufferData(buffer, format, data,
      numSamples*channels*sizeof(short), sampleRate);
  testSoundError(errTxt.c_str());
  free(filename);
  free(data);
}

void playSong(string songName) {
  if (!songsEnabled) return;

  string songPath = "sound/music/" + songName + ".ogg";
  string filename = lookupFile(songPath, 0);
  SPDLOG_INFO("Playing song {} @ {}", songName, filename);

  int streamError = 0;
  musicStream = startOggStream(&streamError, filename.c_str());

  if (streamError) {
    SPDLOG_ERROR("Error in startOggStream: {}", filename);
    resetSongs();
    loadSongsFromDir(songDefaultDir);
    songErrorCount++;

    // If we reached max song error count, something's going wrong
    // so disable the songs for this session
    if (songErrorCount >= songMaxErrors) {
      SPDLOG_ERROR("Max song errors reached; disabling music");
      songsEnabled = false;
    }
    return;
  }

  songErrorCount = 0; // Reset, successfully played a song
  setOptionsSongName(songName);
}

void tryPlayRandomSong(bool playNow) {
  if (!songsEnabled) return;

  // Add delay before starting next song, if we shouldn't play now
  if(!playNow) {
    if(songTimer < c(CSongDelay)) {
      songTimer += getFrameDuration();
      return;
    }

    // Reset song timer
    songTimer = 0.0f;
  }

  if (musicStream != 0) {
    stopOggStream(musicStream);
    musicStream = 0;
  }

  item songNum = randItem(songFiles.size());

  // Don't play the same song twice in a row
  for (int i = 0; i < numPrevSongs; i++) {
    if (prevSongs[i] == songNum) {
      return;
    }
  }

  tryPlaySong(songNum, false);
}

void tryPlaySong(int songNum, bool playNow) {
  if(songNum < 0 || songNum >= songFiles.size()) {
    SPDLOG_ERROR("Tried to play song by invalid index ({}), songFiles.size ({})",
        songNum, songFiles.size());
    return;
  }

  // Add delay before starting next song, if we shouldn't play now
  if(!playNow) {
    if(songTimer < c(CSongDelay)) {
      songTimer += getFrameDuration();
      return;
    }

    // Reset song timer
    songTimer = 0.0f;
  }

  if(musicStream != 0) {
    stopOggStream(musicStream);
    musicStream = 0;
  }

  // Update the history of songs played
  for (int i = 0; i < numPrevSongs-1; i++) {
    if (i < numPrevSongs - 1) {
      prevSongs[i] = prevSongs[i+1];
    }
  }
  prevSongs[numPrevSongs-1] = songNum;

  std::string songName = songFiles[songNum];
  playSong(songName);
}

void displaySongDebug() {
  for(int i = 0; i < songFiles.size(); i++) {
    SPDLOG_INFO("Song {} loaded at index {}", songFiles[i], i);
  }
}

void initSound() {
  if (!c(CEnableSounds)) return;

  SPDLOG_INFO("Initialzing Sound");
  while(soundResetting) {
    SPDLOG_INFO("Resetting Sound");
    auto sleepTime = std::chrono::milliseconds(10);
    std::this_thread::sleep_for(sleepTime);
  }
  //testSoundError("Error while initializing/resetting OpenAL");
  // alGetError();
  ALCdevice* device = alcOpenDevice(NULL);
  //testSoundError("Error while calling alcOpenDevice");
  if (device) {
    auto context=alcCreateContext(device,NULL);
    testSoundError("Error while creating OpenAL context");
    if (!alcMakeContextCurrent(context)) {
      SPDLOG_ERROR("Failed to make ALCcontext current");
      logStacktrace();
      shouldContinue = false; // Failed to initialize
      return;
    }
    // Successfully initialized, set shouldContinue true
    shouldContinue = true;
  } else {
    SPDLOG_ERROR("Could not open ALCdevice");
    logStacktrace();
    shouldContinue = false; // Failed to initialize
    return;
  }
  testSoundError("Unexpected OpenAL device/context error");
  SPDLOG_INFO("Sound Context Created");

  alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);

  alGenSources((ALuint)1, &errorSource);
  testSoundError("Error w/ alGenSources errorSource");
  alGenSources((ALuint)1, &musicSource);
  testSoundError("Error w/ alGenSources musicSource");

  int mono_count = 0, stereo_count = 0;
  alcGetIntegerv(device, ALC_MONO_SOURCES, 1, &mono_count);
  testSoundError("Error w/ alcGetIntegerv mono_count");
  alcGetIntegerv(device, ALC_STEREO_SOURCES, 1, &stereo_count);
  testSoundError("Error w/ alcGetIntegerv stereo_count");
  numSources = mono_count + stereo_count - 5;
  SPDLOG_INFO("Using {} sound effect sources", numSources);
  sources.resize(numSources);
  alGenSources((ALuint)numSources, sources.data());
  testSoundError("Error w/ alGenSources for mono/stereo sources");

  sourceIsUiVector.resize(numSources/8+1);
  for (int i = 0; i < numSources/8+1; i++) {
    sourceIsUiVector[i] = 0;
  }

  alGenBuffers((ALuint)numSounds, soundBuffers);
  testSoundError("Error w/ alGenSources soundBuffers");

  for (int i = 0; i < numSounds; i++) {
    loadSound(soundBuffers[i], sprintf_o("sound/%s.ogg", soundFiles[i]));
  }
  loadEnvironmentSounds();

  alSourcei(errorSource, AL_BUFFER, soundBuffers[0]);
  testSoundError("Error w/ alSourcei for errorSource");

  loadSongsFromDir(songDefaultDir);

  musicData = (short*) malloc(numMusicShorts*sizeof(short));
  nextSoundData.uiSounds |= _soundStartup;
  soundData.updateVolumes = true;
  nextSoundData.updateVolumes = true;
  songTimer = c(CSongDelay);
  if (!hasCompletedTutorial()) {
    tryPlaySong(9, true); // Should be Tourist Trap.ogg
  }
  SPDLOG_INFO("Finished Initilazing Sound");
}

ALuint getNextSource(bool isUi) {
  for (int i = 0; i < numSources; i++) {
    int num = nextSource;
    nextSource = (nextSource + 1) % numSources;

    ALuint source = sources[num];
    int sourceState = 0;
    alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
    testSoundError("Error w/ alGetSourcei");
    if (sourceState == AL_INITIAL || sourceState == AL_STOPPED) {
      setSourceIsUI(num, isUi);
      return source;
    }
  }
  return 0;
}

void playSoundInternal(ALuint buffer) {
  ALuint source = getNextSource(true);
  if (source == 0) return;
  alSourcei(source, AL_BUFFER, buffer);
  testSoundError("Error w/ alSourcei");
  alSourcei(source, AL_SOURCE_RELATIVE, 1);
  testSoundError("Error w/ alSourcei");
  alSource3f(source, AL_POSITION, 0, 0, 0);
  testSoundError("Error w/ AL_SOURCE_POSITION");
  alSourcePlay(source);
  testSoundError("Error w/ alSourcePlay");
}

void playSoundEnvironment(int num, vec3 loc) {
  EnvironmentSound e = environmentSounds[num];
  if (e.buffer == 0) {
    alGenBuffers((ALuint)1, &e.buffer);
    environmentSounds[num].buffer = e.buffer;
    loadSound(e.buffer, sprintf_o("sound/environment/%s.ogg", e.file));
  }

  ALuint source = getNextSource(false);
  if (source == 0) return;
  alSourcei(source, AL_BUFFER, e.buffer);
  testSoundError("Error w/ alSourcei");
  alSourcei(source, AL_SOURCE_RELATIVE, 0);
  testSoundError("Error w/ AL_SOURCE_RELATIVE");

  alSource3f(source, AL_POSITION, loc.x, loc.y, loc.z);
  testSoundError("Error w/ AL_SOURCE_POSITION");
  alSourcef(source, AL_GAIN, 1);
  alSourcef(source, AL_REFERENCE_DISTANCE, 100);
  alSourcef(source, AL_MAX_DISTANCE, 1000);
  alSourcef(source, AL_ROLLOFF_FACTOR, 0.9);

  alSourcePlay(source);
  testSoundError("Error w/ alSourcePlay");
}

void updateMusic() {
  if (!c(CEnableSounds)) return;

  if (getCameraTime() < splashScreenTime) return;
  bool muted = soundData.muted[MusicVolume];
  int musicBuffersQueued = 0;
  int musicBuffersProcessed = 0;
  alGetSourcei(musicSource, AL_BUFFERS_QUEUED, &musicBuffersQueued);
  alGetSourcei(musicSource, AL_BUFFERS_PROCESSED, &musicBuffersProcessed);

  if (musicBuffersProcessed > 0) {
    vector<ALuint> processedBuffers;
    processedBuffers.resize(musicBuffersProcessed);
    alSourceUnqueueBuffers(musicSource, musicBuffersProcessed,
        processedBuffers.data());
    alDeleteBuffers(musicBuffersProcessed, processedBuffers.data());
  }

  // Songs disabled due to errors
  if (!songsEnabled) return;

  if (muted) {
    alSourcePause(musicSource);
    testSoundError("Error w/ alSourcePause");
    return;
  } else if (musicStream == 0) {
    tryPlayRandomSong(true);
    return;
  }

  int numToQueue = queueTarget - musicBuffersQueued + musicBuffersProcessed + 1;
  for (int i = 0; i < numToQueue; i++) {
    int numSamples = getNextOggFrame(musicStream, musicData, numMusicShorts);
    if (!muted && numSamples == 0) {
      if (shouldContinue) tryPlayRandomSong(false);
      return;
    }

    ALuint format = musicStream->channels == 2 ?
      AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    ALuint musicBuffer = 0;
    alGenBuffers(1, &musicBuffer);
    testSoundError("Error w/ alGenSources");
    int dataSize = numSamples * musicStream->channels * sizeof(short);

    alBufferData(musicBuffer, format, musicData, dataSize,
        musicStream->sampleRate);
    testSoundError("Error w/ alBufferData");

    alSourceQueueBuffers(musicSource, 1, &musicBuffer);
    testSoundError("Error w/ alSourceQueueBuffers");
  }

  int sourceState = 0;
  alGetSourcei(musicSource, AL_SOURCE_STATE, &sourceState);
  testSoundError("Error w/ alGetSourcei");
  if (muted && sourceState == AL_PLAYING) {
    alSourcePause(musicSource);
    testSoundError("Error w/ alSourcePause");
  } else if (!muted && sourceState != AL_PLAYING) {
    SPDLOG_INFO("Playing music");
    alSourcePlay(musicSource);
    testSoundError("Error w/ alSourcePlay");
  }
}

void updateSound() {
  if (!c(CEnableSounds)) return;

  //if (soundData.updateVolumes) {
    for (int i = 0; i < numVolumes; i++) {
      if (soundData.muted[i] || soundData.muted[MasterVolume]) {
        soundData.volume[i] = 0;
      } else {
        soundData.volume[i] *= constAmplification[i];
      }
    }

    for (int k = 0; k < numSources; k++) {
      bool isUI = sourceIsUI(k);
      alSourcef(sources[k], AL_GAIN,
          soundData.volume[isUI ? UIVolume : EnvironmentVolume]);

      int sourceState = 0;
      alGetSourcei(sources[k], AL_SOURCE_STATE, &sourceState);
      if (sourceState == AL_PLAYING && !isUI && soundData.paused) {
        alSourcePause(sources[k]);
      } else if (sourceState == AL_PAUSED && (isUI || !soundData.paused)) {
        alSourcePlay(sources[k]);
      }
    }

    alSourcef(musicSource, AL_GAIN, soundData.volume[MusicVolume]);
    alListenerf(AL_GAIN, soundData.volume[MasterVolume]);
  //}

  if (soundData.muted[MasterVolume]) {
    return;
  }

  if (!soundData.muted[UIVolume]) {
    for (int i = 0; i < numSounds; i++) {
      if (soundData.uiSounds & (1 << i)) {
        //SPDLOG_INFO("playing {}", soundFiles[i]);
        playSoundInternal(soundBuffers[i]);
      }
    }
  }

  if (!soundData.muted[EnvironmentVolume] && soundData.environmentSound >= 0) {
    playSoundEnvironment(soundData.environmentSound, soundData.envSoundLoc);
  }

  vec3 loc = soundData.cameraLoc;
  alListener3f(AL_POSITION, loc.x, loc.y, loc.z);
  testSoundError("Error w/ alListenerfv");
  vec3 look = soundData.cameraLook;
  float orien[6] = {look.x, look.y, look.z, 0, 0, 1};
  alListenerfv(AL_ORIENTATION, &orien[0]);
  testSoundError("Error w/ alListenerfv");

  updateMusic();
}

void swapSound() {
  if (!c(CEnableSounds)) return;

  Camera camera = getMainCamera();
  nextSoundData.cameraLoc = vec3(camera.target -
    normalize(camera.direction) * soundDistance);
  if (!validate(nextSoundData.cameraLoc)) {
    nextSoundData.cameraLoc = vec3(0,0,0);
  }
  nextSoundData.cameraLook = -camera.direction;
  nextSoundData.paused = isGamePaused();

  toEnqueue.push(nextSoundData);
  while (toEnqueue.size() > 0) {
    SoundData data = toEnqueue.front();
    if (soundDataQueue.push(data)) {
      toEnqueue.pop();
    } else {
      break;
    }
  }

  nextSoundData.uiSounds = 0;
  nextSoundData.environmentSound = -1;
  nextSoundData.updateVolumes = getCameraTime() < splashScreenTime;
}

void soundLoop() {
  if (!c(CEnableSounds)) return;

  initSound();
  auto sleepTime = std::chrono::milliseconds(2);
  while(shouldContinue) {
    SoundData data;
    if (soundDataQueue.pop(&data, 1)) {
      soundData = data;
      updateSound();
    }
    std::this_thread::sleep_for(sleepTime);
  }
}

float getVolume(VolumeControl ndx) {
  return nextSoundData.volume[ndx];
}

void setVolume(VolumeControl ndx, float volume) {
  nextSoundData.volume[ndx] = volume;
  nextSoundData.updateVolumes = true;
}

bool getMuted(VolumeControl ndx) {
  return nextSoundData.muted[ndx];
}

void setMuted(VolumeControl ndx, bool muted) {
  nextSoundData.muted[ndx] = muted;
  nextSoundData.updateVolumes = true;
}

const char* getVolumeName(VolumeControl ndx) {
  return volumeNames[ndx];
}

