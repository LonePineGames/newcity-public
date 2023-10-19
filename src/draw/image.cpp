#include "image.hpp"

#include "../platform/lookup.hpp"
#include "spdlog/spdlog.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned char* loadImage(const char* filename,
    int* x, int* y, int* n, int d) {
  string modFilename = lookupFile(filename, 0);
  //SPDLOG_INFO("loadImage {} {}", filename, modFilename);
  unsigned char* result = stbi_load(modFilename.c_str(), x, y, n, d);
  return result;
}

void freeImage(unsigned char* image) {
  stbi_image_free(image);
}

Image loadImage(const char* filename) {
  Image result;
  string modFilename = lookupFile(filename, 0);
  //SPDLOG_INFO("loadImage {} {}", filename, modFilename);
  result.data = stbi_load(modFilename.c_str(),
      &result.x, &result.y, &result.channels, 4);
  return result;
}

Image createImage(int x, int y) {
  Image result;
  result.x = x;
  result.y = y;
  result.channels = 4;
  result.data = (unsigned char*) calloc(sizeof(char)*result.channels, x*y);
  return result;
}

void freeImage(Image img) {
  stbi_image_free(img.data);
}

void blitImage(Image source, Image dest, int x, int y);

vec4 samplePixelExact(Image source, int x, int y) {
  x = (x+source.x)%source.x;
  y = (y+source.y)%source.y;
  int ndx = (x + y*source.x)*source.channels;
  if (ndx < 0 || ndx >= source.x*source.y*source.channels) {
    SPDLOG_INFO("samplePixelExact {}x{} at {},{}",
        source.x, source.y, x, y);
  }
  //vec4 result(0,0,0,0);
  vec4 result(float(source.data[ndx]), float(source.data[ndx+1]),
      float(source.data[ndx+2]), float(source.data[ndx+3]));
  return result / 256.f;
}

vec4 samplePixel(Image source, float x, float y) {
  vec4 result(0,0,0,0);
  x -= int(x);
  y -= int(y);
  x *= source.x;
  y *= source.y;

  for (int i = 0; i < 4; i++) {
    int ix = int(x) + i%2;
    int iy = int(y) + i/2;
    float fade = (1 - abs(x-ix)) * (1 - abs(y-iy));
    vec4 sample = samplePixelExact(source, ix, iy);
    result += sample*fade;
  }

  //result = samplePixelExact(source, x, y);
  //SPDLOG_INFO("x:{} y:{} ({},{},{},{})",
      //x, y, result.x, result.y, result.z, result.w);

  return result;
}

