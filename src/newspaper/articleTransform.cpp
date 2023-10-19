#include "../md4c/md4c.h"

#include "../platform/lookup.hpp"
#include "../parts/image.hpp"
#include "../util.hpp"

//const uint32_t _cursorWebLink        = 1 << 0;

struct ArticleCursor {
  uint32_t flags = 0;
  const char* filename;
  string result;
  string imageSrc;
};

const uint32_t _cursorWebLink        = 1 << 0;
const uint32_t _cursorMath           = 1 << 1;
const uint32_t _cursorUnderline      = 1 << 2;
const uint32_t _cursorStrikethrough  = 1 << 3;
const uint32_t _cursorCaption        = 1 << 4;
const uint32_t _cursorCaptionApplied = 1 << 5;

FileBuffer readNewspaperArticle(const char* articleFile) {
  string fn = lookupFile(articleFile, 0);
  FileBuffer result = readFromFile(fn.c_str(), true);

  if (result.length > 0) {
    return result;
  } else {
    return readNewspaperArticle("newspaper/404.md");
  }
}

void transformImage(ArticleCursor* cursor, MD_SPAN_IMG_DETAIL* det) {
  char* src = strdup_s(det->src.text, det->src.size);
  if (src == 0) return;
  if (strlength(src) <= 0) {
    free(src);
    return;
  }

  cursor->result += "![";
  cursor->imageSrc = src;
}

void transformImageCaption(ArticleCursor* cursor) {
  if (cursor->imageSrc.length() != 0) {
    char* fn = strdup_s(cursor->imageSrc.c_str());

    selectFrameInFilename(fn, randItem(10000));
    cursor->result += "](";
    cursor->result += fn;
    cursor->result += ")";
    cursor->imageSrc = "";
    free(fn);
  }
}

int transformEnterBlock(MD_BLOCKTYPE type, void* detail, void* userdata) {
  ArticleCursor* cursor = (ArticleCursor*) userdata;
  if (cursor->result.length() > 0) cursor->result += "\n\n";

  switch(type) {
      case MD_BLOCK_DOC:      break;
      case MD_BLOCK_QUOTE:    {
        cursor->result += "> ";
      } break;

      case MD_BLOCK_UL:       break;
      case MD_BLOCK_OL:       break;

      case MD_BLOCK_LI:       {
        cursor->result += "* ";
      } break;

      case MD_BLOCK_HR:       {
        cursor->result += "---";
      } break;

      case MD_BLOCK_H:        {
        float level = ((MD_BLOCK_H_DETAIL*)detail)->level;
        for (int i = 0; i < level; i++) {
          cursor->result += "#";
        }
        cursor->result += " ";
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

  return 0;
}

int transformLeaveBlock(MD_BLOCKTYPE type, void* detail, void* userdata) {
  ArticleCursor* cursor = (ArticleCursor*) userdata;

  switch(type) {
      case MD_BLOCK_DOC:      break;
      case MD_BLOCK_QUOTE:    break;
      case MD_BLOCK_UL:       break;
      case MD_BLOCK_OL:       break;
      case MD_BLOCK_LI:       break;
      case MD_BLOCK_HR:       break;
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

  return 0;
}

int transformEnterSpan(MD_SPANTYPE type, void* detail, void* userdata) {
  ArticleCursor* cursor = (ArticleCursor*) userdata;

  switch(type) {
    case MD_SPAN_EM:                { cursor->result += "*"; } break;
    case MD_SPAN_STRONG:            { cursor->result += "**"; } break;
    case MD_SPAN_U:                 { cursor->result += "_"; } break;
    case MD_SPAN_A:                 break;

    case MD_SPAN_IMG:               {
      transformImage(cursor, (MD_SPAN_IMG_DETAIL*)detail);
    } break;

    case MD_SPAN_CODE:              break;
    case MD_SPAN_DEL:               { cursor->result += "~"; } break;
    case MD_SPAN_LATEXMATH:         { cursor->flags |= _cursorMath; } break;
    case MD_SPAN_LATEXMATH_DISPLAY: { cursor->flags |= _cursorMath; } break;
    case MD_SPAN_WIKILINK:          break;
  }

  return 0;
}

int transformLeaveSpan(MD_SPANTYPE type, void* detail, void* userdata) {
  ArticleCursor* cursor = (ArticleCursor*) userdata;

  switch(type) {
    case MD_SPAN_EM:                { cursor->result += "*"; } break;
    case MD_SPAN_STRONG:            { cursor->result += "**"; } break;
    case MD_SPAN_U:                 { cursor->result += "_"; } break;
    case MD_SPAN_A:                 break;

    case MD_SPAN_IMG:               {
      transformImageCaption(cursor);
    } break;

    case MD_SPAN_CODE:              break;
    case MD_SPAN_DEL:               { cursor->result += "~"; } break;
    case MD_SPAN_LATEXMATH:         { cursor->flags &= ~_cursorMath; } break;
    case MD_SPAN_LATEXMATH_DISPLAY: { cursor->flags &= ~_cursorMath; } break;
    case MD_SPAN_WIKILINK:          break;
  }

  return 0;
}

void transformTextInner(MD_TEXTTYPE type, char* text, ArticleCursor* cursor) {
  if (type == MD_TEXT_LATEXMATH) {
    char* code = text;
    text = interpretLua(code, cursor->filename);
    free(code);
  }

  if (text == 0) return;
  if (strlength(text) <= 0) {
    free(text);
    return;
  }

  cursor->result += text;

  //if (type == MD_TEXT_LATEXMATH) { // always loses a space
    //cursor->result += " ";
  //}

  if (cursor->flags & _cursorCaption) {
    cursor->flags |= _cursorCaptionApplied;
  }
}

int transformText(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size,
    void* userdata) {
  ArticleCursor* cursor = (ArticleCursor*) userdata;

  switch(type) {
    case MD_TEXT_NULLCHAR:  break;
    case MD_TEXT_BR:        // fallthrough
    case MD_TEXT_SOFTBR:    {
      cursor->result += "\n\n";
    } break;
    case MD_TEXT_HTML:      break;
    case MD_TEXT_ENTITY:    break;
    default:                {
      transformTextInner(type, strdup_s(text, size), cursor);
    } break;
  }

  return 0;
}

void transformDebug(const char* msg, void* userdata) {
  SPDLOG_WARN("Markdown Parsing Error: {}", msg);
}

string transformArticleToIssueArticleInner(string data, char* file) {
  ArticleCursor cursor;
  cursor.filename = file;

  MD_PARSER parser;
  parser.abi_version = 0;
  parser.enter_block = transformEnterBlock;
  parser.leave_block = transformLeaveBlock;
  parser.enter_span = transformEnterSpan;
  parser.leave_span = transformLeaveSpan;
  parser.text = transformText;
  parser.debug_log = transformDebug;
  parser.syntax = 0;

  parser.flags =
    MD_FLAG_STRIKETHROUGH |
    MD_FLAG_UNDERLINE |
    //MD_FLAG_WIKILINKS |
    MD_FLAG_NOHTML |
    MD_FLAG_COLLAPSEWHITESPACE |
    MD_FLAG_LATEXMATHSPANS |
    //MD_FLAG_PERMISSIVEURLAUTOLINKS |
    //MD_FLAG_PERMISSIVEWWWAUTOLINKS |
    0;

  md_parse(data.c_str(), data.length(), &parser, (void*) &cursor);
  return cursor.result;
}

NewspaperIssueArticle transformArticleToIssueArticle
    (NewspaperArticle* article) {

  FileBuffer file = readNewspaperArticle(article->file);

  string data = file.data;
  string resStr = transformArticleToIssueArticleInner(data, article->file);
  freeBuffer(&file);

  const char* text = resStr.c_str();
  int curs = 0;
  for (;text[curs] != '\0' && text[curs] != '\n'; curs++);

  char* title = strdup_s(text, curs);

  char* resText = strdup_s(text + curs + 1);

  NewspaperIssueArticle result;
  result.code = strdup_s(article->code);
  result.text = resText;
  result.title = title;
  return result;
}

