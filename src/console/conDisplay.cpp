#include "conDisplay.hpp"

static std::string text = "";
static uint32_t numLines = 0;
static vec2 conPos = vec2(1,1);
static vec2 conSize = vec2(conMinSizeX,conMinSizeY);
static bool conOpen = false;

void addText(std::string s) {
  text += s;
  int len = text.length();
  uint32_t maxStrLen = getMaxStrLen();
  if(len > maxStrLen) {
    int diff = len-maxStrLen;
    text.erase(0,diff);
  }
}

std::string consoleGetText() {
  return text;
}

uint32_t consoleGetNumLines() {
  return numLines;
}

// Prints a string without an appended newline character
bool consolePrint(std::string data) {
  if(data.length() == 0 || data[0] == '\0') {
    return false;
  }

  addText(data);
  return true;
}

// Prints a string and appends a newline character
bool consolePrintLine(std::string data) {
  if(data.length() == 0 || data[0] == '\0') {
    return false;
  }

  addText(data + "\n");
  numLines++;
  return true;
}

// Prints a debug line in console, with optional bool arg
// to also pass data to spdlog
bool consolePrintDebug(std::string data, bool log) {
  if(log) {
    SPDLOG_INFO(data);
  }
  // TODO: Modify color or presentation in console
  // to indicate this is a debug line
  return consolePrintLine(data);
}

// Prints an error line in console, appending the info to
// the text for the error fetched with getConsoleError(ConStatus val)
bool consolePrintError(const char* err, std::string info) {
  if(err == 0) {
    return false;
  }

  // TODO: Modify color or presentation in console
  // to indicate this is an error line
  std::string txt(err);
  std::string app = info.length() > 0 ? info : "No info";
  txt += " (" + app + ")";
  consolePrintLine(txt);
  return true;
}

bool consoleEchoInput(std::string input) {
  return consolePrintLine("> " + input);
}

void consoleClear() {
  text.clear();
  numLines = 0;
}

void consoleSetSize(float x,float y) {
  conSize = vec2(x,y);
}

vec2 consoleSize() {
  return conSize;
}

void consoleSetSize(vec2 newSize) {
  conSize = newSize;
}

vec2 consolePos() {
  return conPos;
}

void consoleSetPos(vec2 newPos) {
  conPos = newPos;
}

void consoleToggle() {
  conOpen = !conOpen;
  conPos = vec2(1,1);
}

void consoleSetOpen(bool open) {
  conOpen = open;
}

bool consoleIsOpen() {
  return conOpen;
}

void consoleSizeEnforce(float xMax,float yMax) {
  float conX = conSize.x;
  float conY = conSize.y;

  if(conX == 0 || conY == 0) {
    consoleSetSize(xMax,yMax);
    return;
  }

  if(conX < 1.0f
    || conY < 1.0f) {
    consoleSetSize(
      conX < 1.0f ? 1.0f : conX,
      conY < 1.0f ? 1.0f : conY);
  } else if(conX > xMax
    || conY > yMax) {
    consoleSetSize(
      conX > xMax ? xMax : conX,
      conY > yMax ? yMax : conY);
  }
}