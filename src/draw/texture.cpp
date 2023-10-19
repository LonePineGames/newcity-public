#include "texture.hpp"

#include "../pipeline.hpp"
#include "../pool.hpp"
#include "../platform/file.hpp"
#include "../platform/lookup.hpp"
#include "../util.hpp"

#include "camera.hpp"
#include "framebuffer.hpp"
#include "image.hpp"
#include "shader.hpp"

#include "spdlog/spdlog.h"
#include <stdio.h>
#include <unordered_map>
#include <string>
using namespace std;

unordered_map<string, item> textureByFile;
unordered_map<string, item> textureByData;
unordered_map<item, item> makeTextureCache;
Pipeline<item> texturesToLoad;
Pool<Texture>* textures = Pool<Texture>::newPool(40);
Cup<GLuint> textureIDPool;
Cup<GLuint> spareTextures;
int lastTexture = -1;
const item animatedPaletteNdx = numTextures;
item numAnimatedPalettes = 0;
Image paletteImage;
Image blueNoiseImage;

const char* filenames[] = {
  "textures/gray.png",
  "textures/car.png",
  "textures/car.png",
  "textures/car.png",
  "textures/car.png",
  "textures/font_white.png",
  "textures/icons.png",
  "textures/road.png",
  "textures/expressway.png",
  "textures/rail.png",
  "textures/buildings/illum-9pane_norm-alumsiding_albedo_residential_value-0_size-3-4-4_0.png",
  "textures/buildings/illum-9pane_norm-alumsiding_albedo_residential_value-0_size-3-4-4_0.png", // abandoned
  "textures/palette.png",
  "textures/rain.png",
  "textures/snow.png",
  "textures/palette.png",
  "textures/land-grain.png",
  "textures/blue-noise.png",
  "textures/clouds.png",

  "textures/stoplights/stoplight0.png",
  "textures/stoplights/stoplight1.png",
  "textures/stoplights/stoplight2.png",
  "textures/stoplights/stoplight3.png",
  "textures/stoplights/stoplight4.png",
  "textures/stoplights/stoplight5.png",
  "textures/stoplights/stoplight6.png",
  "textures/stoplights/stoplight7.png",
  "textures/stoplights/stoplight8.png",
  "textures/stoplights/stoplight9.png",
  "textures/stoplights/stoplight10.png",
  "textures/stoplights/stoplight11.png"
};

const char* illuminationFilenames[] = {
  "textures/null_illumination.png",
  "textures/null_illumination.png",
  "textures/car_illum_night.png",
  "textures/car_illum_brakes.png",
  "textures/car_illum_night_brakes.png",
  "textures/font_white.png",
  "textures/icons_illumination.png",
  "textures/blank_illumination.png",
  "textures/blank_illumination.png",
  "textures/blank_illumination.png",
  "textures/buildings/illum-9pane_size-3-4-4_illumination-3.png",
  "textures/buildings/illum-9pane_size-3-4-4_illumination-3.png",
  "textures/palette_illumination.png",
  "textures/blank_illumination.png",
  "textures/blank_illumination.png",
  "textures/land.png",
  "textures/null_illumination.png",
  "textures/null_illumination.png",
  "textures/null_illumination.png",

  "textures/stoplights/stoplight_illumination0.png",
  "textures/stoplights/stoplight_illumination1.png",
  "textures/stoplights/stoplight_illumination2.png",
  "textures/stoplights/stoplight_illumination3.png",
  "textures/stoplights/stoplight_illumination4.png",
  "textures/stoplights/stoplight_illumination5.png",
  "textures/stoplights/stoplight_illumination6.png",
  "textures/stoplights/stoplight_illumination7.png",
  "textures/stoplights/stoplight_illumination8.png",
  "textures/stoplights/stoplight_illumination9.png",
  "textures/stoplights/stoplight_illumination10.png",
  "textures/stoplights/stoplight_illumination11.png"
};

const char* flagsFilenames[] = {
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/palette_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",

  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
  "textures/null_flags.png",
};

void getTextureId(GLuint* id) {
  if (textureIDPool.size() < 1) {
    handleError("Texture ID Pool is empty");
    *id = 0;

  } else {
    *id = textureIDPool.back();
    textureIDPool.pop_back();
  }
}

void refillTextureIDPool() {
  const item targetSize = 1000;
  item poolSize = textureIDPool.size();
  textureIDPool.resize(targetSize);
  glGenTextures(targetSize - poolSize, textureIDPool.get(poolSize));
}

Texture* getTexture(int ndx) {
  return textures->get(ndx);
}

item loadImage(const char* filename, bool filter, int wrapX, int wrapY,
    vec2* dimensions) {
  if (filename == 0) return 0;
  int x = 0, y = 0, n = 0;
  GLuint id = 0;
  unsigned char *data = loadImage(filename, &x, &y, &n, 4);
  //SPDLOG_INFO("loadImage {} {}", filename, data == 0);
  if (data == 0) return 0;

  dimensions->x = x;
  dimensions->y = y;

  int64_t dataLn = x*y*4;
  int64_t dataCount = 0;
  for (int i = 0; i < dataLn; i++) {
    dataCount += data[i];
  }

  char* dataHashChar = sprintf_o("%dx%d %0x", x, y, dataCount);
  string dataHash = dataHashChar;
  free(dataHashChar);
  item linkedTex = textureByData[dataHash];
  //SPDLOG_INFO("loadImage hash {} file {} tex {}", dataHash, filename, linkedTex);

  if (linkedTex) {
    id = linkedTex;

  } else {
    getTextureId(&id);
    //SPDLOG_INFO("loadImage {} {} {} {} {}", id, x, y, n, filename);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, x, y,
      0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.f);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapX);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapY);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
      filter ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
      filter ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);

    textureByData[dataHash] = id;
  }

  freeImage(data);

  return id;
}

item loadImage(const char* filename, bool filter, bool wrapX, bool wrapY,
    vec2* dimensions) {
  return loadImage(filename, filter,
      wrapX ? GL_REPEAT : GL_CLAMP_TO_EDGE,
      wrapY ? GL_REPEAT : GL_CLAMP_TO_EDGE,
      dimensions);
}

item loadImage(const char* filename, bool filter, bool wrapX, bool wrapY) {
  vec2 trash;
  return loadImage(filename, filter, wrapX, wrapY, &trash);
}

item loadImage(const char* filename, bool filter, int wrapX, int wrapY) {
  vec2 trash;
  return loadImage(filename, filter, wrapX, wrapY, &trash);
}

item loadFlagsImage(const char* filename) {
  if (filename == 0) return 0;
  int x = 0, y = 0, n = 0;
  GLuint id = 0;
  unsigned char *data = loadImage(filename, &x, &y, &n, 4);
  //SPDLOG_INFO("loadImage {} {}", filename, data == 0);
  if (data == 0) return 0;

  getTextureId(&id);
  //SPDLOG_INFO("loadImage {} {} {} {} {}", id, x, y, n, filename);
  glBindTexture(GL_TEXTURE_2D, id);
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, x, y,
   // 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, x, y,
   // 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y,
    0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  checkOpenGLError();

  //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.f);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
    GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
    GL_NEAREST_MIPMAP_NEAREST);
  glGenerateMipmap(GL_TEXTURE_2D);

  freeImage(data);

  return id;
}

void loadTexture(int ndx) {
  if (ndx == textTexture) return;
  Texture* tex = getTexture(ndx);

  bool wrapAll = ndx == blueNoiseTexture || ndx == rainTexture ||
    ndx == snowTexture || ndx == landGrainTexture || ndx == cloudTexture;
  bool wrapX = wrapAll;
  bool wrapY = wrapAll || ndx >= roadTexture && ndx <= railTexture;
  bool illumFilter = ndx != landTexture || c(CLandTextureFiltering);
  bool illumWrapX = ndx == landTexture;
  bool illumWrapY = ndx == landTexture;
  tex->filename = strdup_s(lookupFile(filenames[ndx-1], 0).c_str());
  tex->illumFilename = strdup_s(lookupFile(illuminationFilenames[ndx-1], 0).c_str());
  tex->textureID = loadImage(filenames[ndx-1], true, wrapX, wrapY,
      &tex->dimensions);
  tex->illuminationTextureID = loadImage(illuminationFilenames[ndx-1],
      illumFilter, illumWrapX, illumWrapY);
  tex->flagsTextureID = loadFlagsImage(flagsFilenames[ndx-1]);
  //SPDLOG_INFO("loadTexture {} {} => {}",
   //   filenames[ndx-1], flagsFilenames[ndx-1], tex->flagsTextureID);

  /*
  int x=0, y=0, n=0;
  unsigned char *data = loadImage(filenames[ndx-1], &x, &y, &n, 4);

  getTextureId(&tex->textureID);
  glBindTexture(GL_TEXTURE_2D, tex->textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, x, y,
    0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.f);

  if (ndx >= vehicleTexture && ndx <= vehicleTextureBrakesNight) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  } else if (ndx == buildingTexture) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
    GL_LINEAR_MIPMAP_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);

  freeImage(data);

  data = loadImage(illuminationFilenames[ndx-1], &x, &y, &n, 4);

  getTextureId(&tex->illuminationTextureID);
  glBindTexture(GL_TEXTURE_2D, tex->illuminationTextureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y,
    0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.f);

  if (ndx >= vehicleTexture && ndx <= vehicleTextureBrakesNight) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }

  if (ndx == landTexture) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
      GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  freeImage(data);
  */
}

void loadTextAtlas(uint8_t* atlas, int x, int y, int n) {
  Texture* tex = getTexture(textTexture);

  getTextureId(&tex->textureID);
  tex->illuminationTextureID = tex->textureID;
  glBindTexture(GL_TEXTURE_2D, tex->textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, x, y,
    0, GL_RED, GL_UNSIGNED_BYTE, atlas);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.f);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
      GL_LINEAR_MIPMAP_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);

  //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void loadTextures() {
  for (int i = 1; i <= textures->size(); i++) {
    Texture* tex = getTexture(i);

    if (tex->filename != 0) {
      free(tex->filename);
      tex->filename = 0;
    }

    if (tex->illumFilename != 0) {
      free(tex->illumFilename);
      tex->illumFilename = 0;
    }

    if (tex->albedoImage.data != 0) {
      free(tex->albedoImage.data);
      tex->albedoImage.data = 0;
    }
  }
  textures->clear();

  textureByFile.clear();
  textureByData.clear();
  texturesToLoad.resetToSize(2048);
  makeTextureCache.clear();
  lastTexture = -1;

  textureIDPool.clear();
  refillTextureIDPool();

  for (int i = 1; i < numTextures; i ++) {
    textures->create();
    loadTexture(i);
  }

  numAnimatedPalettes = 0;
  for (int s = 0; s < 4; s++) {
    char* season = printSeasonString(s);
    for (int i = 1; i <= c(CPaletteFrames); i++) {
      item ndx = textures->create();
      Texture* tex = getTexture(ndx);

      char* fn = sprintf_o("textures/palette/%s-albedo-%d.png", season, i);
      char* illumFn = sprintf_o("textures/palette/%s-illum-%d.png", season, i);
      tex->filename = strdup_s(lookupFile(fn, 0).c_str());
      tex->illumFilename = strdup_s(lookupFile(illumFn, 0).c_str());
      //SPDLOG_INFO("loadTexture {} {}", fn, fileExists(fn));
      tex->textureID = loadImage(fn, true, false, false,
          &tex->dimensions);
      tex->illuminationTextureID = loadImage(illumFn,
          true, false, false);
      tex->flagsTextureID = 0;
      free(fn);
      free(illumFn);
      numAnimatedPalettes ++;
    }
    free(season);
  }

  paletteImage = loadImage("textures/palette.png");
  blueNoiseImage = loadImage("textures/blue-noise.png");

  refillTextureIDPool();
}

void setTexture(int ndx, item s, item mesh) {
  if (s >= UIPaletteShader) mesh = 0;
  int aniSeason = getAnimationSeason();
  int paletteFrames = c(CPaletteFrames);
  if (paletteFrames <= 0) paletteFrames = 1;
  item currentPalette = aniSeason*c(CPaletteFrames) +
    (mesh + getPaletteAnimationFrame()) % paletteFrames;
  currentPalette = animatedPaletteNdx +
    iclamp(currentPalette, 0, numAnimatedPalettes-1);

  /*
  if (s == VehicleShader) {
    SPDLOG_INFO("setTexture({}, {}, {}) {} {}",
        ndx, s, mesh, currentPalette, paletteTexture);
  }
  */

  if (ndx == paletteTexture) {
    ndx = currentPalette;
  }

  if (ndx == lastTexture) {
    return;
  }

  lastTexture = ndx;

  glActiveTexture(GL_TEXTURE5);
  item noiseText = s == LandShader ? landGrainTexture : blueNoiseTexture;
  glBindTexture(GL_TEXTURE_2D, getTexture(noiseText)->textureID);

  glActiveTexture(GL_TEXTURE6);
  glBindTexture(GL_TEXTURE_2D, getTexture(currentPalette)->textureID);
    checkOpenGLError();

  if (ndx <= 0 || ndx > textures->size()) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, 0);

  } else {
    Texture* texture = getTexture(ndx);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->textureID);
    //glBindTexture(GL_TEXTURE_2D, texture->flagsTextureID);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture->illuminationTextureID);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, texture->flagsTextureID);
    checkOpenGLError();
  }
}

item makeTexture(item texID, item illumTexID, const char* filename, const char* illumFilename) {

  item hashNdx = (texID << 16) + illumTexID;
  item ndx = makeTextureCache[hashNdx];

  if (ndx == 0) {
    ndx = textures->create();
    Texture* tex = getTexture(ndx);
    tex->textureID = texID;
    tex->illuminationTextureID = illumTexID;
    if (filename != 0) tex->filename = strdup_s(lookupFile(filename, 0).c_str());
    if (illumFilename != 0) tex->illumFilename = strdup_s(lookupFile(illumFilename, 0).c_str());
    makeTextureCache[hashNdx] = ndx;
  }

  return ndx;
}

item importTexture(const char* filename, const char* illumFilename) {
  item ndx = textures->create();
  Texture* tex = getTexture(ndx);
  tex->textureID = loadImage(filename, true, false, false, &tex->dimensions);
  tex->illuminationTextureID = loadImage(illumFilename, true, false, false);
  if (filename != 0) tex->filename = strdup_s(lookupFile(filename, 0).c_str());
  if (illumFilename != 0) tex->illumFilename = strdup_s(lookupFile(illumFilename, 0).c_str());
  return ndx;
}

const char* getTextureFilename(item ndx) {
  if (ndx <= 0 || ndx > textures->size()) return "";
  return textures->get(ndx)->filename;
}

const char* getTextureIllumFilename(item ndx) {
  if (ndx <= 0 || ndx > textures->size()) return "";
  return textures->get(ndx)->illumFilename;
}

item getTexture(const char* filename) {
  item ndx = textureByFile[filename];

  if (ndx == 0 && spareTextures.size() > 0) { // && texturesToLoad.size() < 2) {
    ndx = spareTextures.back();
    spareTextures.pop_back();
    textureByFile[filename] = ndx;
    Texture* tex = getTexture(ndx);
    tex->filename = strdup_s(lookupFile(filename, 0).c_str());
    texturesToLoad.push(ndx);
  }

  return ndx;
}

Image getPaletteImage() {
  return paletteImage;
}

Image getBlueNoiseImage() {
  return blueNoiseImage;
}

vec2 getTextureDimensions(const char* filename) {
  item ndx = textureByFile[filename];

  if (ndx == 0) {
    return vec2(0,0);
  }

  return getTexture(ndx)->dimensions;
}

void invalidateTexture(const char* filename) {
  SPDLOG_INFO("invalidateTexture {}", filename);
  textureByFile[filename] = 0;
}

void swapTextures() {
  refillTextureIDPool();

  item loadNdx = 0;
  while (texturesToLoad.pop(&loadNdx)) {
    Texture* tex = getTexture(loadNdx);
    if (tex->filename != 0) {
      tex->textureID = loadImage(tex->filename, true, false, false, &tex->dimensions);
      tex->illuminationTextureID = 0;
      tex->filename = strdup_s(lookupFile(tex->filename, 0).c_str());
    }
  }

  refillTextureIDPool();

  while (spareTextures.size() < 2048) {
    spareTextures.push_back(textures->create());
  }
}

item sizeTextures() {
  return textures->size();
}
