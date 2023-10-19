#include "serialize.hpp"

enum WeatherType {
  NORMAL = 0,
  RAIN,
  SNOW,
  STROBE,
  NUM_WEATHER_TYPES
};

enum WeatherIcon {
  WeatherNull, WeatherSnow, WeatherLightning, WeatherRain, WeatherCloudy,
  WeatherMoon, WeatherSun, WeatherCold
};

struct Weather {
  float temp;
  float pressure;
  float clouds;
  float percipitation;
  float snow;
  float drought;
  vec3 wind;
};

const float snowTemp = 3;

Weather getWeather();
WeatherIcon getWeatherIcon();
vec3 getWeatherIconVec3(item ico);
vec3 getWeatherIconVec3();
void setWeather(Weather nW);
void setStrobeClouds(bool strobe);
void resetWeather();
void updateWeather(double duration);
void renderWeather();
void renderWeatherBox();
void initWeatherEntities();
void moveRain(vec2 amount);
vec3 getRainPos();
float getCurrentRain();
float getLightning();
float getWaterTime();
float getIceFade();
float getWaterSnow();

void readWeather(FileBuffer* file, int version);
void writeWeather(FileBuffer* file);
