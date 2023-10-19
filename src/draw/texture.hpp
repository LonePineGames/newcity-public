#pragma once

#include "../main.hpp"
#include "../item.hpp"
#include "image.hpp"

enum {
  noTexture = 0,
  grayTexture,
  vehicleTexture,
  vehicleTextureNight,
  vehicleTextureBrakes,
  vehicleTextureBrakesNight,
  textTexture,
  iconTexture,
  roadTexture,
  expresswayTexture,
  railTexture,
  buildingTexture,
  buildingAbandonedTexture,
  paletteTexture,
  rainTexture,
  snowTexture,
  landTexture,
  landGrainTexture,
  blueNoiseTexture,
  cloudTexture,

  stopLight0,
  stopLight1,
  stopLight2,
  stopLight3,
  stopLight4,
  stopLight5,
  stopLight6,
  stopLight7,
  stopLight8,
  stopLight9,
  stopLight10,
  stopLight11,
  numTextures
};

struct Texture {
  unsigned int textureID;
  unsigned int illuminationTextureID;
  unsigned int flagsTextureID;
  char* filename;
  char* illumFilename;
  vec2 dimensions;
  Image albedoImage;
};

Texture* getTexture(int ndx);
void loadTextures();
void setTexture(int ndx, item s, item mesh);
void loadTextAtlas(uint8_t* atlas, int x, int y, int n);
item loadImage(const char* filename, bool filter, bool wrapX, bool wrapY);
item loadImage(const char* filename, bool filter, int wrapX, int wrapY);
item importTexture(const char* filename, const char* illumFilename);
item makeTexture(item texID, item illumTexID, const char* filename, const char* illumFilename);
const char* getTextureFilename(item texNdx);
const char* getTextureIllumFilename(item texNdx);
item getTexture(const char* filename);
vec2 getTextureDimensions(const char* filename);
void invalidateTexture(const char* filename);
Image getPaletteImage();
Image getBlueNoiseImage();
void swapTextures();
item sizeTextures();

