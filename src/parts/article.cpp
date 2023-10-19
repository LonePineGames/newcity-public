#include "article.hpp"

#include "block.hpp"
#include "button.hpp"
#include "citipediaPanel.hpp"
#include "chart.hpp"
#include "economyPanel.hpp"
#include "hr.hpp"
#include "icon.hpp"
#include "image.hpp"
#include "label.hpp"
#include "messageBoard.hpp"
#include "panel.hpp"
#include "span.hpp"

#include "../color.hpp"
#include "../draw/camera.hpp"
#include "../economy.hpp"
#include "../icons.hpp"
#include "../md4c/md4c.h"
#include "../platform/file.hpp"
#include "../platform/lookup.hpp"
#include "../platform/lua.hpp"
#include "../string_proxy.hpp"
#include "../tutorial.hpp"
#include "../time.hpp"

#include "spdlog/spdlog.h"

#ifdef WIN32
#include <Windows.h>
#include <shellapi.h>
#endif

const float captionPadding = 0.25;

const uint32_t _cursorWebLink        = 1 << 0;
const uint32_t _cursorMath           = 1 << 1;
const uint32_t _cursorUnderline      = 1 << 2;
const uint32_t _cursorStrikethrough  = 1 << 3;
const uint32_t _cursorCaption        = 1 << 4;
const uint32_t _cursorCaptionApplied = 1 << 5;

struct PartCursor {
  uint32_t flags = 0;
  Part* part;
  ArticleRenderConfig config;
  float indent;
  float defaultScale;
  vec2 size;
  vec2 loc;
  vec2 captionStart;
  char* linkTarget = 0;
  vector<vec2> blockquotes;
};

void nextLine(PartCursor* cursor) {
  cursor->loc.x = cursor->indent;
  if (cursor->config.flags & _articleTraditionalParagraphs) {
    cursor->loc.x += cursor->defaultScale*1.5f;
  }
  cursor->loc.y += cursor->size.y;
}

char* sanitizeArticleLink(char* in) {
  if (in == 0) return strdup_s("404");
  char* raw = strdup_s(in);
  char* trimmed = trimWhiteSpace(raw);
  toLower(trimmed);
  return trimmed;
}

void webLink(char* url) {
  #ifdef WIN32
    ShellExecuteA(0, "open", url, NULL, NULL, SW_SHOWNORMAL);
  #else
    system(sprintf_o("xdg-open %s", url));
  #endif
}

bool webLink(Part* part, InputEvent event) {
  char* url = (char*) part->ptrData;
  webLink(url);
  return true;
}

void followLink(char* link) {
  link = sanitizeArticleLink(link);

  if (startsWith(link, "tutorial::")) {
    selectTutorial(link);

  } else if (startsWith(link, "http://") ||
      startsWith(link, "https://")) {
    webLink(link);

  } else {
    selectCitipediaArticle(link);
  }
}

bool followLink(Part* part, InputEvent event) {
  char* link = (char*) part->ptrData;
  followLink(link);
  return true;
}

FileBuffer readArticle(char** articleCode, ArticleRenderConfig* config) {
  if (*articleCode == 0) *articleCode = strdup_s("index");
  char* filename = sprintf_o("docs/%s.md", *articleCode);
  string fn = lookupFile(filename, 0);
  free(filename);
  FileBuffer result = readFromFile(fn.c_str());

  if (result.length > 0 || endsWith(*articleCode, "404")) {
    config->filename = strdup_s(fn.c_str());
    return result;
  } else {
    // Somethings wrong, 404
    //*articleCode = strdup_s("404");
    char* tempCode = strdup_s("404");
    freeBuffer(&result);
    result = readArticle(&tempCode, config);
    free(tempCode);
    return result;
  }
}

void renderImage(PartCursor* cursor, MD_SPAN_IMG_DETAIL* det) {
  char* src = strdup_s(det->src.text, det->src.size);
  if (src == 0) return;
  if (strlength(src) <= 0) {
    free(src);
    return;
  }

  bool isImage = endsWith(src, ".png"); // it's an image
  bool isBar = startsWith(src, "Bar::");
  bool isChart = startsWith(src, "Chart::");
  bool isIcon = !isImage && !isBar && !isChart;

  if (!isIcon) {
    bool trad = cursor->config.flags & _articleTraditionalParagraphs;
    if (trad) {
      cursor->loc.x -= cursor->defaultScale*1.5f;
      if (cursor->loc.x < 0) cursor->loc.x = 0;
    }
    if (cursor->loc.x > cursor->indent) nextLine(cursor);
  }

  if (isImage) {
    float y = 0;
    Part* img = r(cursor->part, image(cursor->loc, cursor->size.x, src, &y));
    img->foregroundColor = PickerPalette::White;
    if (cursor->config.flags & _articleDesaturateImages) {
      img->flags |= _partDesaturate;
    }

    if (cursor->linkTarget != 0) {
      img->onClick = cursor->config.linkHandler;
      img->ptrData = strdup_s(cursor->linkTarget);
      img->flags |= _partHover;
    }

    cursor->captionStart = cursor->loc;
    cursor->captionStart.y += y;
    cursor->loc.y += y + captionPadding;
    cursor->indent += captionPadding;
    cursor->loc.x = cursor->indent;
    cursor->size.x -= 2*captionPadding;
    cursor->flags |= _cursorCaption;
    cursor->flags &= ~_cursorCaptionApplied;

  } else if (isChart || isBar) {
    float y = isBar ? cursor->size.y*2 : cursor->size.x*2/3;

    // Figure out what stat it is, and whether it's national
    // yes I know this is awful
    char* statStr = &src[isBar?5:7];
    bool national = false;
    for (int i = 0; statStr[i] != '\0'; i++) {
      if (statStr[i] == ':') {
        statStr[i] = '\0';
        if (statStr[i+1] == 'N') national = true;
        break;
      }
    }

    Statistic statistic = (Statistic)getStatisticForCode(statStr);
    item timePeriod = 2;
    float endDate = cursor->config.chartDate;
    if (endDate == 0) endDate = getCurrentDateTime();
    item econ = national ? nationalEconNdx() : ourCityEconNdx();
    item statItem = statistic*10 + timePeriod;

    vec2 pinLoc = cursor->loc + vec2(cursor->size.x-1, 0);
    Part* pin = r(cursor->part, button(pinLoc,
          iconPin, toggleChartMessage));
    pin->vecData.x = econ;
    pin->dim.start.z += 15;
    pin->dim.end.z += 15;
    pin->itemData = statItem;
    if (hasMessage(ChartMessage, statItem) ||
        hasMessage(ChartMessage, -statItem)) {
      pin->flags |= _partHighlight;
    }

    Part* chrt = chart(cursor->loc, vec2(cursor->size.x, y), econ, statistic, timePeriod, true, isBar, endDate);
    chrt->onClick = selectChart;
    chrt->itemData = statItem;
    chrt->vecData.x = econ;

    if (cursor->config.flags & _articleDesaturateCharts) {
      chrt->renderMode = RenderTransparent;
      chrt->flags |= _partDesaturate;
      chrt->flags &= ~_partLowered;

      for (int c = chrt->numContents-1; c >= 0; c --) {
        Part* cont = chrt->contents[c];
        if (cont->foregroundColor != PickerPalette::Transparent) {
          cont->foregroundColor = PickerPalette::Black;
        }
      }
    }

    r(cursor->part, chrt);

    cursor->loc.y += y + captionPadding;
    cursor->captionStart = cursor->loc;
    cursor->indent += captionPadding;
    cursor->loc.x = cursor->indent;
    cursor->size.x -= 2*captionPadding;
    cursor->flags |= _cursorCaption;
    cursor->flags &= ~_cursorCaptionApplied;

  } else { // it's an icon
    vec3 ico = getIcon(src);
    free(src);
    float size = cursor->size.y*0.85f;
    vec2 loc;

    if (cursor->loc.x + size >= cursor->size.x - 0.25) {
      cursor->loc.x = cursor->indent;
      cursor->loc.y += cursor->size.y;
    }

    if (cursor->config.flags & _articleCenter) {
      loc = vec2((cursor->size.x-size)*.5f, cursor->loc.y);
      cursor->loc.y += cursor->size.y;
    } else {
      loc = cursor->loc;
      cursor->loc.x += size;
    }

    vec2 icoSize = vec2(size, size);
    Part* icoPart = r(cursor->part, icon(loc, icoSize, ico));

    if (cursor->linkTarget != 0) {
      icoPart->onClick = cursor->config.linkHandler;
      icoPart->ptrData = strdup_s(cursor->linkTarget);
      icoPart->flags |= _partHover;
    }
  }
}

void renderImageCaption(PartCursor* cursor) {
  if (!(cursor->flags & _cursorCaption)) return;
  cursor->flags &= ~_cursorCaption;

  cursor->size.x += 2*captionPadding;
  cursor->indent -= captionPadding;

  float yDiff = cursor->loc.y - cursor->captionStart.y;
  if (cursor->flags & _cursorCaptionApplied) {
    cursor->loc.y += captionPadding;
    nextLine(cursor);
    yDiff = cursor->loc.y - cursor->captionStart.y;
    vec2 size = vec2(cursor->size.x, yDiff);
    r(cursor->part, gradientBlock(cursor->captionStart, size,
          cursor->config.captionBackground));

  } else if (!(cursor->config.flags & _articleCenter)) {
    cursor->loc.y -= captionPadding + cursor->size.y*.5f;
  }
}

int renderEnterBlock(MD_BLOCKTYPE type, void* detail, void* userdata) {
  PartCursor* cursor = (PartCursor*) userdata;
  cursor->size.y = cursor->defaultScale;
  bool doNextLine = cursor->loc.y > 0 && cursor->loc.x == 0 &&
    !(cursor->config.flags & _articleTraditionalParagraphs) &&
    !(cursor->config.flags & _articleCenter);

  switch(type) {
      case MD_BLOCK_DOC:      break;
      case MD_BLOCK_QUOTE:    {
        //float size = cursor->size.y;
        //cursor->blockquotes.push_back(cursor->loc + vec2(0,size));
        cursor->blockquotes.push_back(cursor->loc);
        cursor->indent ++;
        cursor->loc.x ++;
        cursor->loc.y += captionPadding;
        doNextLine = false;
      } break;

      case MD_BLOCK_UL:       {doNextLine = false;} break;
      case MD_BLOCK_OL:       {doNextLine = false;} break;

      case MD_BLOCK_LI:       {
        nextLine(cursor);
        vec2 icoSize = vec2(cursor->size.y, cursor->size.y);
        r(cursor->part, icon(cursor->loc, icoSize, iconDotSmall));
        cursor->indent ++;
        cursor->loc.x ++;
        doNextLine = false;
      } break;

      case MD_BLOCK_HR:       {
        float y = cursor->size.y*0.1;
        r(cursor->part, gradientBlock(cursor->loc, vec2(cursor->size.x-cursor->loc.x, y), cursor->config.captionBackground));
        cursor->loc.y += y*2;
        doNextLine = false;
      } break;

      case MD_BLOCK_H:        {
        float level = ((MD_BLOCK_H_DETAIL*)detail)->level;
        float invLevel = 7-level;
        float factor = pow(1.25, invLevel);
        //cursor->size.y = ((7-level) * 0.25 + 1) * cursor->defaultScale;
        cursor->size.y = factor * cursor->defaultScale;
      } break;

      case MD_BLOCK_CODE:     break;
      case MD_BLOCK_HTML:     break;
      case MD_BLOCK_P:        break;
      case MD_BLOCK_TABLE:    break;
      case MD_BLOCK_THEAD:    break;
      case MD_BLOCK_TBODY:    break;
      case MD_BLOCK_TR:       break;
      case MD_BLOCK_TH:       break;
      case MD_BLOCK_TD:       break;
  }

  if (doNextLine) {
    nextLine(cursor);
  }

  return 0;
}

int renderLeaveBlock(MD_BLOCKTYPE type, void* detail, void* userdata) {
  PartCursor* cursor = (PartCursor*) userdata;
  bool doNextLine = !(cursor->config.flags & _articleCenter);

  switch(type) {
      case MD_BLOCK_DOC:      break;
      case MD_BLOCK_QUOTE:    {
        float size = cursor->size.y;
        if (cursor->blockquotes.size() > 0) {
          vec2 lastQuote = cursor->blockquotes.back();
          lastQuote.x += 0.75;
          cursor->loc.y += captionPadding;
          vec2 blockSize = vec2(cursor->size.x - lastQuote.x, cursor->loc.y - lastQuote.y);
          blockSize.x = cursor->size.x-lastQuote.x;
          //blockSize.x = lastQuote.x+0.75;
          //lastQuote.x = 0;
          Part* blk = r(cursor->part, darkBlock(lastQuote, blockSize));
          blk->foregroundColor = PickerPalette::GrayDark;
          cursor->blockquotes.pop_back();
        }
        cursor->indent --;
        doNextLine = cursor->loc.x <= cursor->indent;
        cursor->loc.x = cursor->indent;
      } break;
      case MD_BLOCK_UL:       {doNextLine = (cursor->indent <= 0);} break;
      case MD_BLOCK_OL:       {doNextLine = (cursor->indent <= 0);} break;
      case MD_BLOCK_LI:       {
        cursor->indent --;
        cursor->loc.x = cursor->indent;
        doNextLine = false;
      } break;
      case MD_BLOCK_HR:       {doNextLine = false;} break;
      case MD_BLOCK_H:        break;
      case MD_BLOCK_CODE:     break;
      case MD_BLOCK_HTML:     break;
      case MD_BLOCK_P:        break;
      case MD_BLOCK_TABLE:    break;
      case MD_BLOCK_THEAD:    break;
      case MD_BLOCK_TBODY:    break;
      case MD_BLOCK_TR:       break;
      case MD_BLOCK_TH:       break;
      case MD_BLOCK_TD:       break;
  }

  if (doNextLine) {
    nextLine(cursor);
  }

  return 0;
}

int renderEnterSpan(MD_SPANTYPE type, void* detail, void* userdata) {
  PartCursor* cursor = (PartCursor*) userdata;

  switch(type) {
    case MD_SPAN_EM:                break;
    case MD_SPAN_STRONG:            break;
    case MD_SPAN_U:                 {cursor->flags |= _cursorUnderline;} break;

    case MD_SPAN_A:                 {
      cursor->flags |= _cursorWebLink;
      MD_SPAN_A_DETAIL* det = (MD_SPAN_A_DETAIL*) detail;
      cursor->linkTarget = strdup_s(det->href.text, det->href.size);
    } break;

    case MD_SPAN_IMG:               {
      renderImage(cursor, (MD_SPAN_IMG_DETAIL*)detail);
    } break;
    case MD_SPAN_CODE:              break;
    case MD_SPAN_DEL:        {cursor->flags |= _cursorStrikethrough;} break;
    case MD_SPAN_LATEXMATH:         { cursor->flags |= _cursorMath; } break;
    case MD_SPAN_LATEXMATH_DISPLAY: { cursor->flags |= _cursorMath; } break;

    case MD_SPAN_WIKILINK:          {
      MD_SPAN_WIKILINK_DETAIL* det = (MD_SPAN_WIKILINK_DETAIL*) detail;
      cursor->linkTarget = strdup_s(det->target.text, det->target.size);
    } break;
  }

  return 0;
}

int renderLeaveSpan(MD_SPANTYPE type, void* detail, void* userdata) {
  PartCursor* cursor = (PartCursor*) userdata;

  switch(type) {
    case MD_SPAN_EM:                break;
    case MD_SPAN_STRONG:            break;
    case MD_SPAN_U:                 {cursor->flags &= ~_cursorUnderline;} break;

    case MD_SPAN_A:                 {
      free(cursor->linkTarget);
      cursor->linkTarget = 0;
      cursor->flags &= ~_cursorWebLink;
    } break;

    case MD_SPAN_IMG:               {
      renderImageCaption(cursor);
    } break;

    case MD_SPAN_CODE:              break;
    case MD_SPAN_DEL:        {cursor->flags &= ~_cursorStrikethrough;} break;
    case MD_SPAN_LATEXMATH:         { cursor->flags &= ~_cursorMath; } break;
    case MD_SPAN_LATEXMATH_DISPLAY: { cursor->flags &= ~_cursorMath; } break;

    case MD_SPAN_WIKILINK:          {
      free(cursor->linkTarget);
      cursor->linkTarget = 0;
    } break;
  }

  return 0;
}

void renderTextInner(MD_TEXTTYPE type, char* text, PartCursor* cursor) {
  if (type == MD_TEXT_LATEXMATH) {
    char* code = text;
    text = interpretLua(code, cursor->config.filename);
    free(code);
  }

  if (text == 0) return;
  if (strlength(text) <= 0) {
    free(text);
    return;
  }

  vec2 endCursor(0,0);
  Part* line;
  if (cursor->config.flags & _articleCenter) {
    line = spanCenter(cursor->loc, cursor->indent, cursor->size,
        text, &endCursor);
  } else {
    line = span(cursor->loc, cursor->indent, cursor->size,
        text, &endCursor);
  }
  r(cursor->part, line);
  cursor->loc = endCursor;

  bool underline = cursor->flags & _cursorUnderline;

  if (cursor->linkTarget != 0) {
    line->flags |= _partHover | _partFreePtr;
    line->foregroundColor = cursor->config.linkTextColor;
    //line->onClick = (cursor->flags & _cursorWebLink) ?
      //webLink : cursor->config.linkHandler;
    line->onClick = cursor->config.linkHandler;
    line->ptrData = strdup_s(cursor->linkTarget);
    underline = true;
  }

  for (int i = 0; i < 2; i++) {
    if (i == 0 && !underline) continue;
    if (i == 1 && !(cursor->flags & _cursorStrikethrough)) continue;
    vec2 offset = vec2(0, (i ? .4f : .7f) * cursor->size.y);

    for (int c = line->numContents-1; c >= 0; c --) {
      Part* cont = line->contents[c];
      vec2 loc = vec2(cont->dim.start) + offset;
      vec2 size = vec2(cont->dim.end.x, cursor->size.y*0.1);

      if (cursor->config.flags & _articleCenter) {
        float sx = stringWidth(cont->text)*(cursor->size.y-0.2);
        loc.x = (size.x - sx)*.5f;
        size.x = sx;
      }

      if (cursor->linkTarget == 0) {
        r(line, colorBlock(loc, size, cursor->config.textColor));
      } else {
        r(line, colorBlock(loc, size, cursor->config.linkTextColor));
      }
    }
  }

  if (cursor->flags & _cursorCaption) {
    cursor->flags |= _cursorCaptionApplied;
    line->foregroundColor = cursor->config.captionTextColor;
  }
}

int renderText(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size,
    void* userdata) {
  PartCursor* cursor = (PartCursor*) userdata;

  switch(type) {
    case MD_TEXT_NULLCHAR:  {
      renderTextInner(type, strdup_s("$0$"), cursor);
    } break;
    case MD_TEXT_BR:        // fallthrough
    case MD_TEXT_SOFTBR:    {
      nextLine(cursor);
    } break;
    case MD_TEXT_HTML:      {
      renderTextInner(type, strdup_s(text, size), cursor);
    } break;
    case MD_TEXT_ENTITY:    {
      renderTextInner(type, strdup_s(text, size), cursor);
    } break;
    default:                {
      renderTextInner(type, strdup_s(text, size), cursor);
    } break;
  }

  return 0;
}

void renderDebug(const char* msg, void* userdata) {
  SPDLOG_WARN("Markdown Parsing Error: {}", msg);
}

Part* articleText(vec2 loc, vec2 size, char* text,
    int textLength, ArticleRenderConfig config) {
  Part* result = panel(loc, size);
  result->renderMode = RenderTransparent;
  float pad = 0.25;
  result->padding = pad;
  result->foregroundColor = config.textColor;

  PartCursor cursor;
  cursor.config = config;
  cursor.part = result;
  cursor.size = size - vec2(pad,pad)*2.f;
  cursor.loc = vec2(0,0);
  cursor.indent = 0;
  cursor.defaultScale = size.y;
  cursor.linkTarget = 0;

  MD_PARSER parser;
  parser.abi_version = 0;
  parser.enter_block = renderEnterBlock;
  parser.leave_block = renderLeaveBlock;
  parser.enter_span = renderEnterSpan;
  parser.leave_span = renderLeaveSpan;
  parser.text = renderText;
  parser.debug_log = renderDebug;
  parser.syntax = 0;

  parser.flags =
    MD_FLAG_STRIKETHROUGH |
    MD_FLAG_UNDERLINE |
    MD_FLAG_WIKILINKS |
    MD_FLAG_NOHTML |
    MD_FLAG_COLLAPSEWHITESPACE |
    MD_FLAG_LATEXMATHSPANS |
    MD_FLAG_PERMISSIVEURLAUTOLINKS |
    MD_FLAG_PERMISSIVEWWWAUTOLINKS |
    0;

  md_parse(text, textLength, &parser, (void*) &cursor);

  result->dim.end.y = cursor.loc.y;
  return result;
}

Part* articleText(vec2 loc, vec2 size, char* text, ArticleRenderConfig config) {
  return articleText(loc, size, text, strlength(text), config);
}

Part* article(vec2 loc, vec2 size, char** articleCode, char** title,
    ArticleRenderConfig config) {

  FileBuffer file = readArticle(articleCode, &config);

  int curs = 0;
  for (;curs < file.length && file.data[curs] != '\n'; curs++);
  *title = strdup_s(file.data, curs);

  Part* result = articleText(loc, size,
      file.data+curs, file.length-curs, config);
  freeBuffer(&file);
  free((void*)config.filename);

  return result;
}

