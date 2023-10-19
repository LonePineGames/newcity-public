#include "loader.hpp"

#include "hr.hpp"
#include "icon.hpp"
#include "label.hpp"
#include "panel.hpp"

#include "../draw/camera.hpp"
#include "../game/game.hpp"
#include "../string.hpp"
#include "../string_proxy.hpp"
#include "../icons.hpp"
#include "../util.hpp"

#include "spdlog/spdlog.h"

vector<float> loaderCars;

Part* loaderBar(vec2 loc, vec2 size) {
  return loaderBar(loc, size, iconCar);
}

Part* loaderBar(vec2 loc, vec2 size, vec3 ico) {
  Part* cars = panel(loc, size);
  float lsx = size.x;
  cars->flags |= _partLowered | _partClip;

  int numCars = loaderCars.size();
  float dur = getFrameDuration()*10;
  dur = std::min(dur, 1.f);
  for (int i = 0; i < numCars; i++) {
    r(cars, icon(vec2(loaderCars[i], 0), vec2(1,1), ico));
    loaderCars[i] += dur;
  }

  if (numCars == 0 ||
      (loaderCars[numCars-1] > 1 && randFloat() < getFrameDuration()*2)) {
    loaderCars.push_back(-1);
  }

  if (loaderCars[0] > lsx) {
    loaderCars.erase(loaderCars.begin());
  }

  return cars;
}

Part* loader() {
  float loadPad = 0.25;
  float lsx = 10; //stringWidth(message) + 0 + loadPad;
  float lsy = 2.25;
  vec2 loaderSize = vec2(lsx+loadPad*2, lsy+loadPad*2);
  float uiX = uiGridSizeX * getAspectRatio();

  //Part* container = panel(line(vec3(0,0,0),
    //vec3(uiX, uiGridSizeY, 0)));
  //container->renderMode = RenderShadow;

  Part* loader = panel(vec2(uiX,uiGridSizeY)*0.5f - loaderSize*0.5f,
      loaderSize);
  loader->padding = loadPad;
  //r(container, loader);

  if (getCameraTime() < splashScreenTime) {
    float iconSize = 1.5;
    float iconY = (lsy-1.25)*.5f - 0.05;
    float textSize = 1.5;
    float textY = (lsy-textSize)*.5f;
    r(loader, icon(vec2((lsx-iconSize)*.5f, iconY),
          vec2(iconSize, iconSize), iconTree));
    r(loader, labelRight(vec2(0, textY), vec2((lsx-iconSize)*.5f,textSize),
          strdup_s("Lone")));
    r(loader, label(vec2((lsx+iconSize)*.5f-0.1f, textY), textSize,
          strdup_s("Pine")));

  } else {
    r(loader, labelCenter(vec2(0, 0), vec2(lsx,lsy-1), strdup_s(getLoaderText())));
    r(loader, loaderBar(vec2(0,1.25), vec2(lsx,1)));
  }

  return loader;
}

