#ifndef TIME_H
#define TIME_H

#include "serialize.hpp"

struct LightInformation {
  double level;
  vec3 direction;
  vec3 cameraSpace;
  vec3 color;
  vec3 skyColor = vec3(1,1,1);
};

const double oneHour = 1.f/24;
const double maxCommuteTime = 2.f/24;
const double workDayLength = 8.f/24;
const double earlyWakeTime = 6.f/24;
const double lateWakeTime = 9.f/24;
const double sleepTime = 22.f/24;
const double startTime = lateWakeTime;
const double gameDayInRealSeconds = 24*60; // One irl sec = One game minute
  // (at speed 1)
const double dawn = 6.f/24;
const double dusk = 18.f/24;
const int oneSeason = 1;
const int oneYear = oneSeason*4;
const int startDay = 0; //oneSeason;
const int inflationBaseYear = 2019;

double getCurrentTime();
int getCurrentDate();
int getCurrentYear();
int getBaseYear();
double getCurrentDateTime();
float getAnimationTime();
item getPaletteAnimationFrame();
item getAnimationSeason();
double getLightTime();
double getLightSeason();
LightInformation getLightInformation();
double getLightLevel();
bool isDay();
void setBrightness(float val);
float getBrightness();

char* printDurationString(double durationInRealSeconds);
char* printNiceDurationString(double duration);
char* printDateTimeString(double dateTime);
char* printDateString(double dateTime);
char* printDateTimeStringSanYear(double dateTime);
char* printTimeString(double time);
char* printSeasonString(double time);

void setLightTime(double time);
void setLightSeason(double time);
void setTime(double time);
void updateTime(double duration);
void updateAnimation(double duration);
void resetTime();
void resetSkybox();
void renderSkybox();
void setSkyVisible();

void writeTime(FileBuffer* file);
void readTime(FileBuffer* file, int version);

#endif
