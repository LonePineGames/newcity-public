#include "label.hpp"

#include "../string.hpp"
#include "../string_proxy.hpp"

#include "spdlog/spdlog.h"
#include <string.h>

Part* labelRight(vec2 start, vec2 size, char* text) {
  Part* result = label(start, size.y, text);
  result->dim.end = vec3(size, 0);
  result->flags |= _partAlignRight;
  return result;
}

Part* labelCenter(vec2 start, vec2 size, char* text) {
  Part* result = label(start, size.y, text);
  result->dim.end = vec3(size, 0);
  result->flags |= _partAlignCenter;
  return result;
}

Part* labelRed(vec2 loc, float size, char* text) {
  Part* result = label(loc, size, text);
  result->foregroundColor = PickerPalette::RedDark;
  return result;
}

Part* label(vec2 start, vec2 size, char* text) {
  Part* result = part(start);
  result->padding = 0.125;
  result->dim.end = vec3(size, 0);
  result->renderMode = RenderText;
  result->text = text;
  return result;
}

Part* label(vec2 start, float size, char* text) {
  return label(start, vec2(1, size), text);
}

Part* label(vec2 start, float size, char* text, bool red) {
  Part* result = label(start, size, text);
  if (red) {
    result->foregroundColor = PickerPalette::RedDark;
  }
  return result;
}

Part* multiline(vec2 start, vec2 size, char* text, float* ySize) {
  Part* result = part(start);
  float padding = 0.125;
  result->padding = padding;
  result->renderMode = RenderText;

  char* linePtr = 0;
  char* dup = strdup_s(text);
  char* oline = strtok_r(dup, "\n", &linePtr);
  char* resultText = 0;
  float ySizeLine = (size.y - padding*2);
  float xSizeLine = (size.x - padding*2);
  float lineLength = xSizeLine;
  int nl = 1;

  while (oline != NULL) {
    char* dline = strdup(oline);
    char* wordPtr = 0;
    char* word = strtok_r(dline, " ", &wordPtr);
    char* fline = strdup_s(word == 0 ? "" : word);

    while ((word = strtok_r(NULL, " ", &wordPtr)) != NULL) {
      char* linePlus = sprintf_o("%s %s", fline, word);
      float lngth = stringWidth(linePlus)*ySizeLine;
      if (lngth > lineLength) {
        nl++;
        if (resultText) {
          char* resultPlus = sprintf_o("%s\n%s", resultText, fline);
          free(resultText);
          resultText = resultPlus;
          free(fline);
          fline = strdup(word);
        } else {
          resultText = fline;
          fline = strdup(word);
        }
        free(linePlus);

      } else {
        free(fline);
        fline = linePlus;
      }
    }

    nl++;
    if (resultText) {
      char* resultNewline = sprintf_o("%s\n%s", resultText, fline);
      free(fline);
      free(resultText);
      resultText = resultNewline;
    } else {
      resultText = fline;
    }

    oline = strtok_r(NULL, "\n", &linePtr);
    free(dline);
  }

  *ySize = ySizeLine*nl*1.1f + padding*2;
  free(dup);
  free(text);
  result->lineHeight = ySizeLine;
  result->dim.end.x = stringWidth(resultText)*ySizeLine + padding*2;
  result->dim.end.y = *ySize;
  result->text = resultText;
  return result;
}

