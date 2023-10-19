#include "image.hpp"

#include "blank.hpp"

#include "../draw/camera.hpp"
#include "../draw/texture.hpp"
#include "../string_proxy.hpp"

#include "spdlog/spdlog.h"

void selectFrameInFilename(char* filename, int camTime) {
  bool inBrackets = false;
  int offset = 0;
  int value = 0;
  for (int i = 0; ; i++) {
    char c = filename[i];

    if (inBrackets) {
      if (c == ']') {
        inBrackets = false;
        int frame = camTime % value;
        char* num = sprintf_o("%d", frame);
        for (int k = 0; num[k] != '\0'; k++) {
          filename[i-offset] = num[k];
          offset--;
        }

      } else if (c >= '0' && c <= '9') { // ignore other characters
        value = value*10 + (c-'0');
      }

      offset ++;

    } else if (c == '[') {
      inBrackets = true;
      offset ++;

    } else {
      filename[i-offset] = c;
    }

    if (c == '\0') break;
  }
}

Part* image(vec2 loc, float width, char* rawFilename, float* y) {
  // Find what frame we're on
  char* filename = strdup_s(rawFilename);
  int camTime = int(getCameraTime());
  selectFrameInFilename(filename, camTime);

  // Pre-load the next frame
  char* f = strdup_s(rawFilename);
  selectFrameInFilename(f, camTime+1);
  getTexture(f);
  free(f);
  free(rawFilename);

  vec2 imgDim = getTextureDimensions(filename);
  if (imgDim.x == 0) {
    imgDim = vec2(1240,720);

    /*
    // Go through every possible frame (up to 48) and pre-load it.
    for (int i = 0; i < 48; i++) {
      char* f = strdup_s(rawFilename);
      selectFrameInFilename(f, i);
      getTexture(f);
      free(f);
    }
    */
  }

  vec2 size(width, imgDim.y*width/imgDim.x);
  *y = size.y;

  Part* result = part(loc);
  result->dim.end = vec3(size, 0);
  result->renderMode = RenderImage;
  result->text = filename;
  return result;
}

Part* image(vec2 loc, float width, item textureNdx, float* y) {
  char* filename = 0;

  if (textureNdx > 0) {
    Texture* tex = getTexture(textureNdx);
    filename = strdup_s(tex->filename);
  }

  if (filename == 0) filename = strdup_s("");

  return image(loc, width, filename, y);
}
