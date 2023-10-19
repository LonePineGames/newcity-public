#include "building/building.hpp"
#include "building/design.hpp"
#include "draw/camera.hpp"
#include "game/game.hpp"
#include "graph.hpp"
#include "heatmap.hpp"
#include "land.hpp"
#include "platform/lookup.hpp"
#include "time.hpp"
#include "util.hpp"
#include "weather.hpp"
#include "vehicle/laneLoc.hpp"
#include "vehicle/model.hpp"
#include "zone.hpp"

const int _soundEnvWater    = 1 << 1;
const int _soundEnvWind     = 1 << 2;
const int _soundEnvSnow     = 1 << 3;
const int _soundEnvSummer   = 1 << 4;
const int _soundEnvNight    = 1 << 5;
const int _soundEnvUpgraded = 1 << 6;
const int _soundEnvDay      = 1 << 7;
const int _soundEnvBirds    = 1 << 8;

struct EnvironmentSound {
  char* file;
  ALuint buffer;
  int flags;
  item zone;
  const char* designName;
  float prosperity;
  float crime;
  float traffic;
  float rain;
  float lastPlayTime;
  vector<item> vehicleModels;
  vector<item> vehicleModelMins;
};

vector<EnvironmentSound> environmentSounds;
vector<item> audibleVehicleModels_g;
float lastSoundTime = -200;
const float minTimeBetweenPlays = oneHour/2;
const double soundDistance = 1000;

void resetSoundEnvironment() {
  for (int i = 0; i < environmentSounds.size(); i++) {
    free(environmentSounds[i].file);
  }
  environmentSounds.clear();
  audibleVehicleModels_g.clear();
  lastSoundTime = -200;
}

void updateSoundEnvironment() {
  const float environmentSoundRate =
    c(CEnvironmentSoundRate) / gameDayInRealSeconds;
  if (environmentSounds.size() == 0) return;

  float time = getAnimationTime();
  float timeSinceSound = time - lastSoundTime;
  if (timeSinceSound < environmentSoundRate) return;

  float ang = randFloat(0, pi_o*2);
  float mag = pow(randFloat(0, 1), 0.5) * soundDistance;
  vec3 loc = pointOnLand(getCameraTarget() +
      vec3(sin(ang)*mag, cos(ang)*mag, 0));
  if (!validate(loc)) return;
  float prosperity = heatMapGet(Prosperity, loc)*10-4.9;
  float crime = heatMapGet(Crime, loc)*10+.1;

  Weather w = getWeather();
  bool water = loc.z < beachLine && w.temp > 0;
  float rain = w.temp < snowTemp ? 0 : w.percipitation*10;
  bool wind = length(w.wind) > 5;
  float timeInDay = getCurrentTime();
  bool night = timeInDay < 4*oneHour || timeInDay > 22*oneHour;
  bool summer = w.temp > 30;
  bool snow = w.snow > 0.2;

  Box b = box(vec2(loc), 10);
  vector<item> buildings = collideBuilding(b, 0);
  item zone = 0;
  const char* designName = 0;
  bool upgraded = false;
  if (buildings.size() > 0) {
    Building* b = getBuilding(buildings[randItem(buildings.size())]);
    if (b->flags & _buildingLights) {
      Design* d = getDesign(b->design);
      zone = b->zone;
      designName = d->name;
      bool isRes = zone == ResidentialZone || zone == MixedUseZone;
      upgraded = (isRes && d->minDensity > .25) ||
        (zone == FactoryZone);
    }
  }

  float traffic = 0;
  int numCars = 0;
  Box gb = box(vec2(loc), tileSize*getChunkSize()*2.f);
  vector<item> elems = getGraphCollisions(gb);
  int numElems = elems.size();
  for (int i = 0; i < numElems && i < 100; i++) {
    item elem = elems[numElems - i - 1];
    if (elem <= 0) continue;
    Edge* e = getEdge(elem);
    if (!(e->flags & _graphComplete)) continue;
    numCars += numVehiclesInBlock(e->laneBlocks[0]);
    if (e->laneBlocks[1] != 0) {
      numCars += numVehiclesInBlock(e->laneBlocks[1]);
    }
  }
  traffic = round((numCars+9.5)/40.);

  nextSoundData.environmentSound = -1;
  for (int i = 0; i < 5; i++) {
    item num = randItem(environmentSounds.size());
    EnvironmentSound e = environmentSounds[num];
    if (time - e.lastPlayTime < minTimeBetweenPlays) continue;

    if (e.zone != 0) {
      if (e.zone != zone &&
          (e.zone != ResidentialZone || zone != MixedUseZone)) continue;
      if (zone == GovernmentZone &&
          !endsWith(designName, e.designName)) continue;
    }

    if (traffic < e.traffic) continue;
    if (prosperity < e.prosperity) continue;
    if (crime < e.crime) continue;
    if (rain < e.rain) continue;
    if (!water    &&  (e.flags & _soundEnvWater))    continue;
    if (!snow     &&  (e.flags & _soundEnvSnow))     continue;
    if (!wind     &&  (e.flags & _soundEnvWind))     continue;
    if (!summer   &&  (e.flags & _soundEnvSummer))   continue;
    if (!night    &&  (e.flags & _soundEnvNight))    continue;
    if (night     &&  (e.flags & _soundEnvDay))      continue;
    if (!upgraded &&  (e.flags & _soundEnvUpgraded)) continue;
    if (upgraded  && !(e.flags & _soundEnvUpgraded)) continue;
    if (timeSinceSound < environmentSoundRate*2 &&
        (e.flags & _soundEnvBirds)) continue;

    if (e.vehicleModels.size() > 0) {
      bool vehicleFound = false;
      for (int m = 0; m < e.vehicleModels.size(); m++) {
        item modelNdx = e.vehicleModels[m];
        if (audibleVehicleModels_g.size() > modelNdx &&
            audibleVehicleModels_g[modelNdx] > e.vehicleModelMins[m]) {
          vehicleFound = true;
          break;
        }
      }
      if (!vehicleFound) continue;
    }

    //SPDLOG_INFO("playing environment sound {} {} {}", e.file,
     // traffic, numCars);
    nextSoundData.environmentSound = num;
    environmentSounds[num].lastPlayTime = time;
    nextSoundData.envSoundLoc = loc;
    lastSoundTime = time;
    break;
  }

  for (int i = 0; i < audibleVehicleModels_g.size(); i++) {
    audibleVehicleModels_g[i] = 0;
  }
}

void loadEnvironmentSounds() {
  vector<string> files = lookupDirectory("sound/environment", ".ogg", 0);
  for (int i = 0; i < files.size(); i ++) {
    EnvironmentSound e;
    e.file = strdup_s(files[i].c_str());
    e.flags = 0;
    e.traffic = 0;
    e.prosperity = 0;
    e.crime = 0;
    e.rain = 0;
    e.designName = 0;
    e.zone = 0;
    e.lastPlayTime = -100;
    e.buffer = 0;
    e.vehicleModels.clear();
    e.vehicleModelMins.clear();

    char* conditions = strdup_s(e.file);
    char* condptr = NULL;
    char* condition = strtok_r(conditions, "_", &condptr);

    while (condition != NULL) {
      char* dup = strdup_s(condition);
      char* typeptr = NULL;
      char* type = strtok_r(dup, "-", &typeptr);
      char* param = strtok_r(NULL, "-", &typeptr);
      int paramVal = param ? strtol(param, NULL, 10) : -1;

      if (streql(type, "traffic")) {
        e.traffic = paramVal;
      } else if (streql(type, "prosperity")) {
        e.prosperity = paramVal;
        e.flags |= _soundEnvDay;
      } else if (streql(type, "crime")) {
        e.crime = paramVal;
      } else if (streql(type, "rain")) {
        e.rain = paramVal;
      } else if (streql(type, "water")) {
        e.flags |= _soundEnvWater;
      } else if (streql(type, "snow")) {
        e.flags |= _soundEnvSnow;
      } else if (streql(type, "wind")) {
        e.flags |= _soundEnvWind;
      } else if (streql(type, "summer")) {
        e.flags |= _soundEnvSummer;
      } else if (streql(type, "night")) {
        e.flags |= _soundEnvNight;
      } else if (streql(type, "birds")) {
        e.flags |= _soundEnvBirds;
        e.flags |= _soundEnvDay;

      } else if (streql(type, "house")) {
        e.zone = ResidentialZone;
      } else if (streql(type, "apartment")) {
        e.zone = ResidentialZone;
        e.flags |= _soundEnvUpgraded;
      } else if (streql(type, "shop")) {
        e.zone = RetailZone;
      } else if (streql(type, "farm")) {
        e.zone = FarmZone;
      } else if (streql(type, "factory")) {
        e.zone = FactoryZone;
        e.flags |= _soundEnvUpgraded;
      } else if (streql(type, "gov")) {
        e.zone = GovernmentZone;
        e.designName = strdup_s(param);

      } else {
        item modelNdx = vehicleModelByCode(type);
        if (modelNdx > 0) {
          item min = 1;
          if (paramVal > 0) min = paramVal;
          e.vehicleModels.push_back(modelNdx);
          e.vehicleModelMins.push_back(modelNdx);
        }
      }

      condition = strtok_r(NULL, "_", &condptr);
      free(dup);
    }

    free(conditions);

    if (e.flags & _soundEnvNight) {
      e.flags &= ~_soundEnvDay;
    }

    environmentSounds.push_back(e);
  }

  SPDLOG_INFO("num environment sounds {}/{}",
      environmentSounds.size(), files.size());
}

void vehicleAudible_g(item modelNdx) {
  if (audibleVehicleModels_g.size() <= modelNdx) {
    audibleVehicleModels_g.resize(modelNdx+1);
  }
  audibleVehicleModels_g[modelNdx] ++;
}

