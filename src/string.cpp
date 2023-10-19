#include "string.hpp"

#include "draw/texture.hpp"
#include "error.hpp"
#include "line.hpp"
#include "main.hpp"
#include "platform/lookup.hpp"
#include "renderUtils.hpp"
#include "string_proxy.hpp"
#include "util.hpp"

#include "spdlog/spdlog.h"
#include <stdio.h>
#include <ft2build.h>
#include FT_FREETYPE_H

const uint32_t maxStrLen = 40320; // 8!
const int numCharacters = 128;

struct Font {
  FT_Face face;
  item texture;
  float xHeight;
  float atlasHeight;
  float zeroWidth;
  vec2 start[numCharacters];
  vec2 offset[numCharacters];
  vec2 size[numCharacters];
  vec2 outerSize[numCharacters];
  float advance[numCharacters];
  uint8_t* data[numCharacters];
};

Font mainFont;

char pixelAt(unsigned const char* image, int x, int y,
  int numComponents, int width) {
  //get alpha channel, last component
  return image[numComponents * (y*width + x) + numComponents-1];
}

void parseFont() {
  string filenameStr = lookupFile("fonts/font.ttf", 0);
  SPDLOG_INFO("Parsing Font {}", filenameStr);

  FT_Library library;
  int error = FT_Init_FreeType(&library);
  if (error) handleError("Problem initing freetype");

  FT_Face face;
  error = FT_New_Face(library, filenameStr.c_str(), 0, &face);
  if (error == FT_Err_Unknown_File_Format) {
    handleError("Unknown font file format %s", filenameStr.c_str());
  } else if (error) {
    handleError("Error %d loading font %s", error, filenameStr.c_str());
  }

  error = FT_Set_Pixel_Sizes(face, 0, c(CTextResolution));
  if (error) handleError("Error setting font size");

  mainFont.face = face;
  FT_GlyphSlot g = face->glyph;
  float tallest = 0;
  vec2 cursor(c(CTextAtlasPadding), c(CTextAtlasPadding));

  for (int i = 0; i < numCharacters; i++) {
    error = FT_Load_Char(face, i, FT_LOAD_DEFAULT);
    if (error) handleError("Error loading font glyph");

    error = FT_Render_Glyph(g, FT_RENDER_MODE_NORMAL);
    if (error) handleError("Error rendering font glyph");

    int width = g->bitmap.width;
    int height = g->bitmap.rows;
    uint8_t* data = (uint8_t*) malloc(width*height);
    memcpy_s(data, g->bitmap.buffer, width*height);

    mainFont.size[i] = vec2(width, height);
    mainFont.data[i] = data;
    mainFont.offset[i] = vec2(g->bitmap_left, -(g->bitmap_top));

    if (cursor.x + width > c(CTextAtlasWidth) - c(CTextAtlasPadding)) {
      cursor = vec2(c(CTextAtlasPadding),
          cursor.y + tallest + c(CTextAtlasPadding));
      tallest = 0;
    }

    mainFont.start[i] = cursor;
    mainFont.advance[i] = g->advance.x / c(CTextResolution) / 64.f;
    mainFont.advance[i] *= c(CTextWidthAdjust);
    cursor.x += width + c(CTextAtlasPadding);
    if (height > tallest) tallest = height;
    if (i == 'X') {
      mainFont.xHeight = -mainFont.offset[i].y / c(CTextResolution);
      mainFont.xHeight *= c(CXHeightAdjust);
    }
    if (i == '0') {
      mainFont.zeroWidth = mainFont.advance[i] * c(CZeroWidthAdjust);
    }
  }

  cursor.y += tallest+c(CTextAtlasPadding);
  mainFont.atlasHeight = cursor.y;
  uint8_t* atlas = (uint8_t*) calloc(mainFont.atlasHeight, c(CTextAtlasWidth));
  for (int i = 0; i < numCharacters; i++) {
    vec2 start = mainFont.start[i];
    vec2 size = mainFont.size[i];
    uint8_t* data = mainFont.data[i];

    for (int j = 0; j < size.y; j++) {
      int atlasNdx = (start.x + (start.y + j) * c(CTextAtlasWidth));
      int dataNdx = j * size.x;

      //for (int k = 0; k < size.x; k++) {
        //uint8_t val = data[dataNdx+k];
        //atlas[atlasNdx + k] = val;
        //Upscale to a 4-element image
        //for (int z = 0; z < 4; z++) {
          //atlas[atlasNdx + k*4 + z] = val;
        //}
      //}
      memcpy_s(atlas + atlasNdx, data + dataNdx, (int)size.x);
    }

    free(data);
    mainFont.data[i] = 0;
    mainFont.start[i].x /= c(CTextAtlasWidth);
    mainFont.start[i].y /= mainFont.atlasHeight;
    mainFont.size[i]+=8;
    mainFont.outerSize[i] = vec2(mainFont.size[i])/c(CTextResolution);
    mainFont.outerSize[i].x *= c(CTextWidthAdjust);
    mainFont.size[i].x /= c(CTextAtlasWidth);
    mainFont.size[i].y /= mainFont.atlasHeight;
    mainFont.offset[i] /= c(CTextResolution);
  }

  loadTextAtlas(atlas, c(CTextAtlasWidth), mainFont.atlasHeight, 1);
  free(atlas);
}

bool isMonospace(char c) {
  return (c >= '0' && c <= '9') ||
    c == '(' || c == ')' || c == '$' || c == '+' || c == '-' ||
    c == '/' || c == '.' || c == ',' || c == ' ';
}

float characterWidth(char c) {
  if (c >= numCharacters) return 0;
  return isMonospace(c) ? mainFont.zeroWidth : mainFont.advance[c];
}

float characterWidth(char c, char next) {
  if (next == '\0' || isMonospace(c)) return characterWidth(c);
  FT_Vector delta;
  FT_Get_Kerning(mainFont.face, c, next, FT_KERNING_DEFAULT, &delta);
  float adj = delta.x / 64.f;
  return adj + characterWidth(c);
}

void renderString(Mesh* mesh, const char* string, vec3 start, vec3 along,
  vec3 down, float firstLineIndent) {

  if (string == 0) return;

  vec3 cursor = start + firstLineIndent*along;
  vec2 xHeight = vec2(0, mainFont.xHeight);
  float fontSize = length(down);
  float padding = 0.1;
  vec3 paddingX = fontSize*padding*normalize(along);
  vec3 paddingY = fontSize*padding*normalize(down);
  vec3 paddingXY = paddingX + paddingY;
  vec3 texPadding = vec3(c(CTextResolution)*padding/c(CTextAtlasWidth),
      c(CTextResolution)*padding/mainFont.atlasHeight, 0);
  int lines = 0;
  for (int i = 0; string[i] != '\0'; i++) {
    char c = string[i];

    if (c == '\n') {
      lines ++;
      cursor = start + down*float(lines)*1.1f;
      continue;
    }

    vec2 o = mainFont.offset[c] + xHeight;
    vec3 os = vec3(mainFont.outerSize[c],0);
    vec3 l = vec3(mainFont.start[c],0)-texPadding;
    vec3 s = vec3(mainFont.size[c],0)+texPadding*2.f;
    float a = characterWidth(c, string[i+1]);
    o.x += .5f * (a - os.x);

    vec3 p = cursor + o.y * down + o.x * along - paddingXY;
    vec3 iAlong = along * os.x + paddingX*2.f;
    vec3 iDown  =  down * os.y + paddingY*2.f;

    makeQuad(mesh, p, p + iAlong, p + iDown, p + iAlong + iDown, l, l+s);

    cursor += a * along;
  }
}

void renderString(Mesh* mesh, const char* string, vec3 start, float size) {
  renderString(mesh, string, start, vec3(size,0,0));
}

void renderString(Mesh* mesh, const char* string, vec3 start, vec3 along,
    float firstLineIndent) {
  vec3 down = vec3(-along.y, along.x, 0);
  renderString(mesh, string, start, along, down, firstLineIndent);
}

void renderString(Mesh* mesh, const char* string, vec3 start, vec3 along) {
  renderString(mesh, string, start, along, 0);
}

void renderString(Mesh* mesh, const char* string, vec3 start, float size,
    float firstLineIndent) {
  renderString(mesh, string, start, vec3(size,0,0), firstLineIndent);
}

void renderString(Mesh* mesh, const char* string, vec3 start, vec3 along,
  vec3 down) {
  renderString(mesh, string, start, along, down, 0);
}

vec2 stringDimensions(const char* string) {
  if(string == 0) return vec2(0,0);
  if(strlen(string) <= 0) return vec2(0,0);

  float current = 0;
  float best = 0;
  int numLines = 1;
  for (int i = 0; string[i] != '\0'; i++) {
    char c = string[i];

    if (c == '\n') {
      if (current > best) {
        best = current;
      }
      current = 0;
      numLines ++;
      continue;
    }

    float a = characterWidth(c, string[i+1]);
    current += a;
  }

  if (current == 0) numLines --;
  if (current > best) best = current;
  return vec2(best, numLines);
}

float stringWidth(const char* string) {
  return stringDimensions(string).x;
}

void renderStringCentered(Mesh* mesh, const char* string,
  vec3 center, vec3 along, vec3 down) {

  vec3 start = center - along * stringWidth(string) * 0.5f;
  renderString(mesh, string, start, along, down);
}

uint32_t getMaxStrLen() {
  return maxStrLen;
}

