#pragma once

#include "part.hpp"
#include "../serialize.hpp"

struct ArticleRenderConfig {
  uint32_t flags;
  item textColor;
  item captionTextColor;
  item linkTextColor;
  float chartDate;
  Line captionBackground;
  const char* filename;
  InputCallback linkHandler = 0;
};

const uint32_t _articleDesaturateImages = 1 << 0;
const uint32_t _articleDesaturateCharts = 1 << 1;
const uint32_t _articleTraditionalParagraphs = 1 << 2;
const uint32_t _articleCenter = 1 << 3;

void followLink(char* link);
bool followLink(Part* part, InputEvent event);
char* sanitizeArticleLink(char* in);
Part* article(vec2 loc, vec2 size, char** articleCode, char** title,
    ArticleRenderConfig config);
Part* articleText(vec2 loc, vec2 size, char* text, ArticleRenderConfig config);

