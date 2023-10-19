#include "color.hpp"
#include "draw/entity.hpp"
#include "draw/image.hpp"
#include "draw/texture.hpp"
#include "game/game.hpp"
#include "heatmap.hpp"
#include "land.hpp"
#include "option.hpp"
#include "renderUtils.hpp"
#include "string_proxy.hpp"
#include "time.hpp"
#include "util.hpp"
#include "weather.hpp"

#include "spdlog/spdlog.h"
#include <algorithm>

//const vec3 white   = vec3(1  , 1  , 0.9);
//const vec3 gold    = vec3(1  , 0.5, 0.1);
//const vec3 blue    = vec3(0.2, 0.4, 0.7);
//const vec3 orange  = vec3(0.9, 0.4, 0.1);
//const vec3 skyBlue = vec3(.224, .31, .357);
const vec3 skyColor[4] = {
  vec3(1.0, 0.1, 0.1),
  vec3(1.0, 0.5, 0.1),
  vec3(0.1, 0.3, 1.0),
  vec3(0.0, 0.0, 0.2)
};

const double latitude = 0.3;

static double currentTime = startTime + startDay;
static double lightTime = 0.4;
static double lightSeason = 1.5;
static item animationSeason = 0;
static double animationTime = 0;
static double paletteAnimationFrameFrac = 0;
static item paletteAnimationFrame = 0;
static double lightLevel;
static item skyboxEntity = 0;
static double brightness = .5f;

const char* weekdays[] = {
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat",
  "Sun"
};

const char* seasons[] = {
  "Spring",
  "Summer",
  "Autumn",
  "Winter"
};

const char* ampm[] = {
  "AM",
  "PM"
};

double getCurrentDateTime() {
  return currentTime;
}

double getCurrentTime() {
  return currentTime - (int)currentTime;
}

int getCurrentDate() {
  return (int)currentTime;
}

int getCurrentYear() {
  return getBaseYear() + c(CStartYear);
}

int getBaseYear() {
  return int(currentTime)/oneYear;
}

void setTime(double time) {
  currentTime = time;
  double clockTheta = (getLightTime()-0.5)*pi_o*2;
  double seasonTheta = ((getLightSeason()-1.5)/4)*pi_o*2;
  double sunHeight = cos(clockTheta) + cos(seasonTheta)/4;
  lightLevel = sunHeight*0.5+0.5 - getWeather().clouds*.5f;
  if (isPresimulating()) lightLevel = 1;
}

float getAnimationTime() {
  return animationTime;
}

item getAnimationSeason() {
  return animationSeason;
}

item getPaletteAnimationFrame() {
  return paletteAnimationFrame;
}

void updateAnimation(double duration) {
  float dayFrac = duration / gameDayInRealSeconds;
  animationTime += clamp(dayFrac, 0.f, .25f/24/60);
  if (animationTime > 4) animationTime -= 4;

  paletteAnimationFrameFrac += duration * c(CPaletteAnimationFramerate);
  if (paletteAnimationFrameFrac > c(CPaletteFrames)) {
    paletteAnimationFrameFrac -= c(CPaletteFrames);
  }
  paletteAnimationFrame = floor(paletteAnimationFrameFrac);

  if (isShowWeather()) {
    if (getGameMode() == ModeGame) {
      animationSeason = floor(currentTime);
    } else {
      animationSeason = floor(lightSeason);
    }
    animationSeason %= 4;
  } else {
    animationSeason = 1; // summer
  }
}

void updateTime(double duration) {
  float dayFrac = duration / gameDayInRealSeconds;
  setTime(currentTime + dayFrac);
}

void setLightTime(double time) {
  lightTime = time;
}

double getLightTime() {
  DaylightMode dayMode = getDaylightMode();
  if (getGameMode() != ModeGame) {
    return lightTime;

  } else if (!c(CRenderNight) || dayMode == DaylightAlwaysDay ||
      isHeatMapIntense()) {
    return 0.4;

  } else if (!c(CRenderNight) || dayMode == DaylightAlwaysNight ||
      isHeatMapIntense()) {
    return 0;

  } else if (dayMode == DaylightSystemTime) {
    time_t rawtime = time(NULL);
    struct tm * timeinfo;
    timeinfo = localtime ( &rawtime );
    float hour = timeinfo->tm_hour;
    float minute = timeinfo->tm_hour;
    float second = timeinfo->tm_hour;
    return (((second/60.f) + minute)/60.f + hour)/24.f;

  } else if (isPresimulating()) {
    return startTime;

  } else {
    return getCurrentTime();
  }
}

double getLightSeason() {
  DaylightMode dayMode = getDaylightMode();
  if (getGameMode() != ModeGame) {
    return lightSeason;

  } else if (!c(CRenderNight) || dayMode == DaylightAlwaysDay ||
      dayMode == DaylightAlwaysNight || isHeatMapIntense()) {
    return 1.5;

  } else if (dayMode == DaylightSystemTime) {
    time_t rawtime = time(NULL);
    struct tm * timeinfo;
    timeinfo = localtime ( &rawtime );
    float day = timeinfo->tm_yday;
    return day*4.f/365.f;

  } else {
    return fmod(getCurrentDateTime(), 4);
  }
}

void setLightSeason(double val) {
  lightSeason = val;
}

char* printDurationString(double duration) {
  const char* neg = duration < 0 ? "-" : "";
  duration = abs(duration);
  int years = duration/4;
  float fdays = (duration - years*4);
  int days = fdays;
  float fhours = (fdays - days)*24;
  int hours = fhours;
  float fminutes = (fhours - hours)*60;
  int minutes = fminutes;
  float seconds = (fminutes - minutes)*60;
  if (years == 0 && days == 0 && hours == 0 && minutes == 0 && seconds == 0) neg = "";

  if (years > 100000) {
    return sprintf_o("%s%dKy", neg, years/1000);
  } else if (years > 1000) {
    return sprintf_o("%s%.1fKy", neg, years/1000.f);
  } else if (years > 10) {
    return sprintf_o("%s%dy", neg, years);
  } else if (years > 0 && days == 0) {
    return sprintf_o("%s%dy", neg, years);
  } else if (years > 0) {
    return sprintf_o("%s%dy%dd", neg, years, days);
  } else if (days > 0 && hours == 0) {
    return sprintf_o("%s%dd", neg, days);
  } else if (days > 0) {
    return sprintf_o("%s%dd%02dh", neg, days, hours);
  } else if (hours > 0 && minutes == 0) {
    return sprintf_o("%s%dh", neg, hours);
  } else if (hours > 0) {
    return sprintf_o("%s%dh%02dm", neg, hours, minutes);
  } else if (minutes > 0 && seconds == 0) {
    return sprintf_o("%s%dm", neg, minutes);
  } else if (minutes > 0) {
    return sprintf_o("%s%dm%02ds", neg, minutes, int(seconds));
  } else {
    return sprintf_o("%s%02.2fs", neg, seconds);
  }
}

char* printNiceDurationString(double duration) {
  const char* neg = duration < 0 ? "-" : "";
  duration = abs(duration);
  float fdays = duration;
  int days = fdays;
  float fhours = (fdays - days)*24;
  int hours = fhours;
  float fminutes = (fhours - hours)*60;
  int minutes = fminutes;
  float seconds = (fminutes - minutes)*60;
  if (days == 0 && hours == 0 && minutes == 0 && seconds == 0) neg = "";

  if (days > 1000000) {
    int millions = days / 1000000;
    int thousands = (days / 1000) % 1000;
    return sprintf_o("%s%d,%03d,%03d days", neg, millions, thousands, days%1000);
  } else if (days > 1000) {
    return sprintf_o("%s%d,%03d days", neg, days/1000, days%1000);
  } else if (days > 0 && (hours == 0 || days > 10)) {
    return sprintf_o("%s%d days", neg, days);
  } else if (days > 0) {
    return sprintf_o("%s%d days, %d hours", neg, days, hours);
  } else if (hours > 0 && minutes == 0) {
    return sprintf_o("%s%d hours", neg, hours);
  } else if (hours > 0) {
    return sprintf_o("%s%d hours, %dmin", neg, hours, minutes);
  } else if (minutes > 0) {
    return sprintf_o("%s%d minutes", neg, minutes);
  } else if (seconds > 0) {
    return sprintf_o("%s%d seconds", neg, int(seconds));
  } else {
    return sprintf_o("%s%.2f seconds", neg, seconds);
  }
}

char* printTimeString(double time) {
  time -= int(time);
  int hour = time*24;
  int minute = (time*24 - hour)*60;
  bool pm = hour >= 12;
  if (pm) {
    hour -= 12;
  }
  if (hour == 0) {
    hour = 12;
  }

  return sprintf_o("%2d:%02d%s", hour, minute, ampm[pm]);
}

char* printSeasonString(double time) {
  int season = int(time)%4;
  return strdup_s(seasons[season]);
}

char* printDateTimeStringSanYear(double dateTime) {
  int day = (int)dateTime;
  double time = dateTime - day;
  int hour = time*24;
  int minute = (time*24 - hour)*60;
  bool pm = hour >= 12;
  if (pm) {
    hour -= 12;
  }
  if (hour == 0) {
    hour = 12;
  }
  int season = day%4;

  return sprintf_o("%2d:%02d%s, %s,", hour, minute, ampm[pm], seasons[season]);
}

char* printDateTimeString(double dateTime) {
  int day = (dateTime < 0?-1:0) + (int)dateTime;
  double time = dateTime - day;
  int hour = time*24;
  int minute = (time*24 - hour)*60;
  bool pm = hour >= 12;
  if (pm) {
    hour -= 12;
  }
  if (hour == 0) {
    hour = 12;
  }
  int season = day%4;
  if (season < 0) season += 4;
  int year = dateTime/4 + c(CStartYear);

  return sprintf_o("%2d:%02d%s, %s, %d", hour, minute, ampm[pm],
    seasons[season], year);
}

char* printDateString(double dateTime) {
  int day = (int)dateTime;
  int season = day%4;
  int year = day/4 + c(CStartYear);

  return sprintf_o("%s, %d", seasons[season], year);
}

void writeTime(FileBuffer* file) {
  fwrite_double(file, currentTime);
  fwrite_double(file, lightTime);
}

void readTime(FileBuffer* file, int version) {
  setTime(version >= 23 ? fread_double(file) : fread_float(file));
  if (version >= 50) {
    setLightTime(fread_double(file));
  }
  //setTime(173.7085); //5PM Summer 1993
}

void resetTime() {
  if (getGameMode() == ModeGame) {
    setTime(startTime + startDay);
  } else {
    setTime(c(CDefaultLightTime));
  }
}

void resetSkybox() {
  if (skyboxEntity != 0) {
    removeEntityAndMesh(skyboxEntity);
    skyboxEntity = 0;
  }
}

LightInformation getLightInformation() {
  LightInformation result;
  Weather w = getWeather();
  float lightning = 0;
  double dateTime = getLightSeason();
  double timeTheta = (getLightTime()-0.5)*pi_o*2;
  double seasonTheta = ((dateTime-1.5)/4)*pi_o*2;
  double sunHeight = cos(timeTheta) + cos(seasonTheta)/4 + 0.1;
  double goldAmount = 1-pow(sunHeight,2);
  double goldA4 = pow(goldAmount,4)*.85f;
  double moonTheta = (dateTime/4-0.5)*pi_o*2;
  double cloudDim = 1-w.clouds*.8;
  if (isPresimulating()) cloudDim = 1;
  double lightPower0 = clamp(sunHeight * cloudDim, 0., 1.);
  double lightPower1 = 1 - lightPower0;
  double dirFactor = std::min(std::max(0.0, sunHeight + goldAmount), 0.8);
  double skyBright = std::max(0.1, cloudDim*(1+cos(timeTheta))*.4f);
  double cloudEffect = (1-w.clouds);
  cloudEffect = 1-cloudEffect*cloudEffect;
  result.level = lightLevel;
  if (isPresimulating()) result.level = 1;

  /*
  for (int i = 0; i < 3; i++) {
    double pos = (i-1) * (goldA4+0.5) + 2 * (1-goldA4);
    //if (pos > 2.5) pos = 2.5;
    vec3 color = vec3(0,0,0);
    for (int j = 0; j < 4; j++) {
      color += skyColor[j] * clamp(float(1-abs(j - pos)), 0.f, 1.f);
    }
    color = color * float(1-cloudEffect) + vec3(1,1,1)*float(cloudEffect);
    color *= skyBright;
    color += vec3(1,1,1)*float(lightning*.5f);
    result.skyColor[i] = color;
  }
  */

  result.skyColor = vec3(goldAmount, skyBright, w.clouds);
  result.skyColor = clamp(result.skyColor, 0.f, 1.f);

  /*
  result.color = (dvec3(1,1,1)*lightPower0 + dvec3(0.1, 0.1, 0.8)*lightPower1)
    *(1-goldA4) + dvec3(1.0, 0.3, 0.1)*goldA4;
    */

  float x = 5 - 4*skyBright;
  float y = 49 + 8*clamp(goldAmount, 0., 1.);
  result.color = vec3(
      samplePixel(getPaletteImage(), x/paletteSize, y/paletteSize));
  result.color += vec3(1,1,1)*float(lightning*.2f);

  result.direction =
    dirFactor*dvec3(sin(timeTheta), cos(seasonTheta+pi_o)+latitude,
        sunHeight*.5) +
    (1-dirFactor)*dvec3(sin(moonTheta), latitude, .5);
  if (lightning > 0.1) {
    double dirEffect = lightning*.5f;
    double angle = moonTheta + pi_o*.5f;
    dvec3 randVec = vec3(sin(angle), cos(angle), 0.5);
    result.direction = result.direction * float(1-dirEffect) +
      vec3(randVec*dirEffect);
  }

  // Adjust for bad sun angle at sunset
  double rx = result.direction.x;
  double ry = result.direction.y;
  double rz = result.direction.z;
  result.color *= clamp(2 - rz / (sqrt(rx*rx + ry*ry) + 1.f), .5, 2.);
  result.color *= brightness*2.4+.1f;
  //result.color *= 0.5f;

  return result;
}

void setBrightness(float val) {
  brightness = val;
}

float getBrightness() {
  return brightness;
}

double getLightLevel() {
  return lightLevel;
}

void setSkyVisible() {
  if (skyboxEntity == 0) return;
  setEntityVisible(skyboxEntity, !isUndergroundView());
}

vec3 getSkyboxUV(float turn, float z) {
  float power = mix(1.f, 3.f, getFOV()/2.1f);
  float zAdj = 1-pow(z/6.f, power);
  return vec3(turn, zAdj, 0);
}

float getSkyboxY(float z) {
  return 3.f*(1 - 2*abs(.5f - z/6.f)) + 0.f;
}

void renderSkybox() {
  if (!c(CRenderSky)) return;
  if (skyboxEntity == 0) {
    skyboxEntity = addEntity(SkyboxShader);
  }

  createMeshForEntity(skyboxEntity);
  Entity* entity = getEntity(skyboxEntity);
  entity->texture = cloudTexture;
  Mesh* mesh = getMeshForEntity(skyboxEntity);

  const int numSides = 24;
  const float skyboxSize = 500;
  vec3 up = vec3(0,0,1);

  for (int z = 0; z < 6; z++) {
    float z0 = z;
    float z1 = z+1;
    vec3 bottom = vec3(0, 0, (z0-3)*skyboxSize);
    vec3 top = vec3(0, 0, (z1-3)*skyboxSize);
    vec3 *points0 = (vec3*)alloca(sizeof(vec3)*numSides);
    vec3 *points1 = (vec3*)alloca(sizeof(vec3)*numSides);
    vec3 *tex0 = (vec3*)alloca(sizeof(vec3)*(numSides+1));
    vec3 *tex1 = (vec3*)alloca(sizeof(vec3)*(numSides+1));
    float y0 = getSkyboxY(z0);
    float y1 = getSkyboxY(z1);

    for (int i = 0; i < numSides; i ++) {
      float theta = pi_o * 2 * i / numSides;
      vec3 dir = vec3(cos(theta), sin(theta), 0) *
        (-2.f*skyboxSize);
      points0[i] = dir*y0 + bottom;
      points1[i] = dir*y1 + top;
    }

    for (int i = 0; i <= numSides; i ++) {
      float turn = float(i)/numSides;
      tex0[i] = getSkyboxUV(turn, z0);
      tex1[i] = getSkyboxUV(turn, z1);
    }

    for (int i = 0; i < numSides; i ++) {
      int j = (i+1)%numSides;
      int k = i+1;
      vec3 p0 = points0[j];
      vec3 p1 = points0[i];
      vec3 p2 = points1[j];
      vec3 p3 = points1[i];
      vec3 xp0 = tex0[k];
      vec3 xp1 = tex0[i];
      vec3 xp2 = tex1[k];
      vec3 xp3 = tex1[i];

      /*
      if (z0 == 0) {
        Triangle t0 = {{
          {p0, up, xp0},
          {p1, up, xp1},
          {bottom, up, xp0},
        }};
        pushTriangle(mesh, t0);
      }
      */

      Triangle t1 = {{
        {p2, up, xp2},
        {p1, up, xp1},
        {p0, up, xp0},
      }};
      pushTriangle(mesh, t1);

      Triangle t2 = {{
        {p2, up, xp2},
        {p3, up, xp3},
        {p1, up, xp1},
      }};
      pushTriangle(mesh, t2);
    }
  }

  /*
  vec3 bottom = vec3(0, 0, -skyboxSize*1.f);
  vec3 middle = vec3(0,0, skyboxSize*1.f);
  vec3 top = vec3(0,0, skyboxSize*2.f);
  vec3 *points0 = (vec3*)alloca(sizeof(vec3)*numSides);
  vec3 *points1 = (vec3*)alloca(sizeof(vec3)*numSides);
  vec3 *points2 = (vec3*)alloca(sizeof(vec3)*numSides);
  vec3 *tex0 = (vec3*)alloca(sizeof(vec3)*(numSides+1));
  vec3 *tex1 = (vec3*)alloca(sizeof(vec3)*(numSides+1));
  vec3 *tex2 = (vec3*)alloca(sizeof(vec3)*(numSides+1));

  for (int i = 0; i < numSides; i ++) {
    float theta = pi_o * 2 * i / numSides;
    vec3 dir = vec3(cos(theta), sin(theta), 0) *
      (-2.f*skyboxSize);
    points0[i] = dir + bottom;
    points1[i] = dir*2.f + middle;
    points2[i] = dir*.5f + top;
  }

  for (int i = 0; i <= numSides; i ++) {
    tex0[i] = vec3(double(i)/numSides, 0, 0);
    tex1[i] = vec3(double(i)/numSides, .5f, 0);
    tex2[i] = vec3(double(i)/numSides, 1, 0);
  }

  for (int i = 0; i < numSides; i ++) {
    int j = (i+1)%numSides;
    int k = i+1;
    vec3 p0 = points0[j];
    vec3 p1 = points0[i];
    vec3 p2 = points1[j];
    vec3 p3 = points1[i];
    vec3 p4 = points2[j];
    vec3 p5 = points2[i];
    vec3 xp0 = tex0[k];
    vec3 xp1 = tex0[i];
    vec3 xp2 = tex1[k];
    vec3 xp3 = tex1[i];
    vec3 xp4 = tex2[k];
    vec3 xp5 = tex2[i];

    Triangle t0 = {{
      {p0, up, xp0},
      {p1, up, xp1},
      {bottom, up, xp0},
    }};
    pushTriangle(mesh, t0);

    Triangle t1 = {{
      {p2, up, xp2},
      {p1, up, xp1},
      {p0, up, xp0},
    }};
    pushTriangle(mesh, t1);

    Triangle t2 = {{
      {p2, up, xp2},
      {p3, up, xp3},
      {p1, up, xp1},
    }};
    pushTriangle(mesh, t2);

    Triangle t3 = {{
      {p4, up, xp4},
      {p3, up, xp3},
      {p2, up, xp2},
    }};
    pushTriangle(mesh, t3);

    Triangle t4 = {{
      {p4, up, xp4},
      {p5, up, xp5},
      {p3, up, xp3},
    }};
    pushTriangle(mesh, t4);
  }
  */

  bufferMeshForEntity(skyboxEntity);
}

