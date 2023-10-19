#include "icons.hpp"
#include "land.hpp"
#include "option.hpp"
#include "renderUtils.hpp"
#include "time.hpp"
#include "util.hpp"
#include "weather.hpp"

#include "draw/entity.hpp"
#include "draw/texture.hpp"

#include "game/game.hpp"

#include "spdlog/spdlog.h"
#include <algorithm>

const float weatherTick = 1.f/24.f/60.f;
static float weatherDurToGo = 0;
static Weather w;
static item weatherEntity = 0;
static vec3 percipitationPos;
static double waterTime = 0;
bool weatherRendered = false;
bool strobeClouds = false;

void resetWeather() {
  weatherDurToGo = 0;
  percipitationPos = vec3(0,0,0);
  weatherEntity = 0;
  weatherRendered = false;
  waterTime = 0;
  w.temp = 15;
  w.pressure = 0;
  w.clouds = 0;
  w.drought = 0;
  w.snow = 0;
  w.percipitation = 0;
  w.wind = vec3(10, 0, 0);
}

Weather getWeather() {
  return w;
}

void setWeather(Weather nW) {
  w = nW;
}

void setStrobeClouds(bool strobe) {
  strobeClouds = strobe;
}

bool showSnow() {
  return c(CRenderSnow) && isShowWeather();
}

float getIceFade() {
  return showSnow() ? clamp(w.temp/-5,0.f,1.f) : 0;
}

float getWaterSnow() {
  return showSnow() ? w.snow * clamp((w.temp+5)/-5,0.f,1.f) : 0;
}

float getLightning() {
  if (w.percipitation < .7) return 0;
  if (w.temp < snowTemp && w.percipitation < .9) return 0;

  float lightning = 0;
  float time = getCameraTime();
  int lightningIters = 8;
  for (int i = 0; i < lightningIters; i++) {
    lightning += sin(time*i*i*.02f)/lightningIters;
  }

  return clamp(lightning*8-2.9f, 0.f, 1.f);
}

void updateWeatherInner(double duration) {
  float time = getCurrentDateTime();
  float season = time/4 - .5;
  float adur = 24 * duration;
  float light = getLightLevel(); //getLightLevel()*.1f + .9f;
  float seasonFactor = (.5f-.5f*sin(season*pi_o*2))*(.75f+.25f*light);
  bool desert = getLandConfig().flags & _landDesert;

  float seasonTemp = desert ?
    mix(10, 50, pow(seasonFactor,1)) :
    mix(-10, 40, pow(seasonFactor,1));
  float targetTemp = seasonTemp - w.percipitation*8;
  w.temp = mix(w.temp, targetTemp, adur*0.1f);
  w.temp += randFloat(-1,1)*adur*0.1f;
  //w.temp = seasonTemp;

  //w.temp = mix(w.temp, seasonTemp, adur*4);
  //w.temp = w.temp*(1-adur*0.05f) + seasonTemp*adur*0.05f;
  //w.temp += randFloat(-1,1)*adur*5;
  //w.temp -= w.percipitation * adur * 1.f;
  //w.temp -= (1-w.pressure) * adur * 1.f;
  //w.temp = clamp(w.temp, -40.f, 50.f);

  float targetClouds = randFloat(-1,1) + seasonFactor*.1f - w.pressure;
  float clouds = mix(w.clouds, targetClouds, adur);
  clouds = mix(w.clouds, clouds, 0.25f);
  w.clouds = clamp(clouds, 0.f, 1.f);

  float targetPercip = pow(w.clouds - w.pressure, 2.f);
  if (desert) targetPercip *= targetPercip;
  targetPercip += adur*4*randFloat(-1,1);
  w.percipitation = mix(w.percipitation, targetPercip, adur);
  w.percipitation = clamp(w.percipitation, 0.f, w.clouds);
  w.drought = mix(w.drought, 1-w.percipitation, adur);
  w.drought = clamp(w.drought, 0.f, 1.f);
  if (w.temp < snowTemp) {
    w.snow += w.percipitation * adur * c(CSnowCollectionRate);
  }
  if (w.temp > 0) {
    w.snow *= (1-adur*w.temp*0.02f);
  }
  if (time > 0 && time < startTime) {
    w.snow -= adur;
  }
  w.snow = clamp(w.snow, 0.f, 1.f);

  int pressureIters = 4;
  float targetPress = 0;
  for (int i = 0; i < pressureIters; i++) {
    targetPress += 1.25f*sin(time*i*i)/pressureIters;
  }
  targetPress += (seasonFactor*1-.6f)*.15f;
  targetPress = clamp(targetPress, -1.f, 1.f);
  w.pressure = mix(w.pressure, targetPress, adur);
  w.pressure += randFloat(-1,1)*adur;
  w.pressure = clamp(w.pressure, -1.f, 1.f);

  if (strobeClouds) {
    w.clouds = sin(getCurrentDateTime()*24*5)*.5f+.5f;
    w.pressure = 2-2*w.clouds;
    w.percipitation = w.clouds*1.5-.5f;
  }

  //SPDLOG_INFO("{} adur\n{} light, {} temp, {} pressure",
      //adur, light, w.temp, w.pressure);
  //SPDLOG_INFO("{} clouds\n{} percip, {} percipitation, {} snow",
      //w.clouds, percip, w.percipitation, w.snow);

  float ang = randFloat()*pi_o*2;
  float windSpeed = length(w.wind);
  float targetSpeed = pow(1-w.pressure,2) * 8.f;
  float nextWindSpeed = mix(windSpeed, targetSpeed, adur);
  nextWindSpeed += randFloat(-1,1)*adur*4;
  nextWindSpeed = clamp(nextWindSpeed, 0.1f, 20.f);
  vec3 change = vec3(cos(ang), sin(ang), 0);
  change.y += 0.001;
  change *= 24*60*duration * 0.1;
  w.wind = normalize(w.wind + change) * nextWindSpeed;
}

void updateWeather(double duration) {
  if (getGameMode() != ModeGame) return;

  weatherDurToGo += duration / gameDayInRealSeconds;
  while (weatherDurToGo > weatherTick) {
    weatherDurToGo -= weatherTick;
    updateWeatherInner(weatherTick);
  }

  if (w.temp >= snowTemp) {
    percipitationPos.y += duration*.8;
  } else {
    percipitationPos.y += duration*.05;
    percipitationPos.x -= duration*.01;
  }

  float dayFrac = duration / gameDayInRealSeconds;
  waterTime += clamp(dayFrac, 0.f, .25f/24/60) * (1-getIceFade());
  if (waterTime > 4) waterTime -= 4;

  renderWeather();
}

float getWaterTime() {
  return waterTime;
}

float getCurrentRain() {
  return clamp(w.percipitation - w.snow, 0.f, 1.f);
}

void moveRain(vec2 amount) {
  percipitationPos.x += amount.x;
  percipitationPos.y += amount.y;
}

void initWeatherEntities() {
  if (weatherEntity == 0) {
    weatherEntity = addEntity(RainShader);
  }
  createMeshForEntity(weatherEntity);
}

void renderWeatherBox() {
  initWeatherEntities();
  Mesh* mesh = getMeshForEntity(weatherEntity);
  weatherRendered = true;

  const int numSides = 32;
  const int numZ = 32;
  vec3 up = vec3(0,0,1);

  for (int y = 5; y > 2; y--) {
    for (int x = 0; x < numSides; x++) {
      for (int z = 2; z < numZ-1; z++) {
        float size = y * 100.f + 700.f;
        float zUp = 500.f;
        float t1 = (pi_o*2*(x+1))/numSides;
        float t0 = (pi_o*2*(x  ))/numSides;
        float zt0 = (pi_o*.5f*(z+1))/numZ;
        float zt1 = (pi_o*.5f*(z+2))/numZ;
        vec3 z0 = vec3(0,0, -cos(zt0)*size*2.f + zUp);
        vec3 z1 = vec3(0,0, -cos(zt1)*size*2.f + zUp);
        float cz0 = sin(zt0);
        float cz1 = sin(zt1);
        float xScl = y*4.5f;
        float zScl = y*1.5f;
        if (z < 8) {
          xScl = 1.2f*(z+2);
          zScl = 0.4f*(z+2);
        }
        xScl *= 0.75f;
        zScl *= 0.75f;
        float xShft = y*pi_o;

        vec3 dir0 = vec3(cos(t0), sin(t0), 0) * size;
        vec3 dir1 = vec3(cos(t1), sin(t1), 0) * size;
        vec3 p0 = dir0*cz0 + z0;
        vec3 p1 = dir1*cz0 + z0;
        vec3 p2 = dir0*cz1 + z1;
        vec3 p3 = dir1*cz1 + z1;
        vec3 x0 = vec3(xShft + xScl* x   /numSides, zScl* z   /numZ, y);
        vec3 x1 = vec3(xShft + xScl*(x+1)/numSides, zScl* z   /numZ, y);
        vec3 x2 = vec3(xShft + xScl* x   /numSides, zScl*(z+1)/numZ, y);
        vec3 x3 = vec3(xShft + xScl*(x+1)/numSides, zScl*(z+1)/numZ, y);

        makeQuad(mesh, p0, p1, p2, p3, up, up, up, up, x0, x1, x2, x3);
      }
    }
  }

  bufferMeshForEntity(weatherEntity);
}

void renderWeather() {
  if (!c(CRenderWeather)) return;

  if (!isShowWeather()) {
    if (weatherEntity != 0) {
      setEntityVisible(weatherEntity, false);
    }
    return;
  }

  if (!weatherRendered) {
    renderWeatherBox();
  }

  Entity* entity = getEntity(weatherEntity);
  if (w.temp < snowTemp) {
    entity->texture = snowTexture;
  } else {
    entity->texture = rainTexture;
  }

  if (w.percipitation > 0.1f) {
    setEntityVisible(weatherEntity, true);
    float opacity = clamp((w.percipitation-0.1f), 0.f, 1.f);
    //opacity = 1 - opacity*opacity;
    entity->flags &= ~(255 << 23);
    entity->flags |= (int(255*opacity) << 23);
  } else {
    setEntityVisible(weatherEntity, false);
  }
}

vec3 getRainPos() {
  return percipitationPos;
}

WeatherIcon getWeatherIcon(Weather w) {
  if (w.percipitation > 0.15 && w.temp < snowTemp) {
    return WeatherSnow;
  } else if (w.clouds > 0.7) {
    return WeatherLightning;
  } else if (w.percipitation > 0.15) {
    return WeatherRain;
  } else if (w.clouds > 0.2) {
    return WeatherCloudy;
  } else if (getLightLevel() < 0.3) {
    return WeatherMoon;
  } else if (w.temp > 30) {
    return WeatherSun;
  } else if (w.temp < 0) {
    return WeatherCold;
  } else {
    return WeatherNull;
  }
}

WeatherIcon getWeatherIcon() {
  return getWeatherIcon(getWeather());
}

vec3 getWeatherIconVec3(item ico) {
  switch (ico) {
    case WeatherSnow: return iconWeatherSnow;
    case WeatherLightning: return iconWeatherLightning;
    case WeatherRain: return iconWeatherRain;
    case WeatherCloudy: return iconWeatherCloudy;
    case WeatherMoon: return iconWeatherMoon;
    case WeatherSun: return iconWeatherSun;
    case WeatherCold: return iconWeatherCold;
    case WeatherNull: // fallthrough
    default: return iconNull;
  }
}

vec3 getWeatherIconVec3() {
  return getWeatherIconVec3(getWeatherIcon());
}

void readWeather(FileBuffer* file, int version) {
  if (version >= 39) {
    w.temp = fread_float(file);
    w.pressure = fread_float(file);
  }
  w.clouds = fread_float(file);
  w.snow = fread_float(file);
  w.percipitation = fread_float(file);
  w.drought = fread_float(file);
  w.wind = fread_vec3(file);
}

void writeWeather(FileBuffer* file) {
  fwrite_float(file, w.temp);
  fwrite_float(file, w.pressure);
  fwrite_float(file, w.clouds);
  fwrite_float(file, w.snow);
  fwrite_float(file, w.percipitation);
  fwrite_float(file, w.drought);
  fwrite_vec3(file, w.wind);
}

