#pragma once

#include "main.hpp"
#include "item.hpp"
#include "serialize.hpp"

const uint32_t _labelExists = 1 << 0;
const uint32_t _labelRed = 1 << 1;
const uint32_t _labelBlue = 1 << 2;
const uint32_t _labelSizeShift = 24;
const uint32_t _labelSizeMask = 15 << _labelSizeShift;

struct Label {
  uint32_t flags;
  char* text;
  vec3 location;
  item entity;
};

item addLabel(vec3 loc, uint32_t flags);
void removeLabel(item ndx);
Label* getLabel(item ndx);
void setLabelHighlight(item ndx, bool highlight);
float getLabelFontSize(item ndx);
void setLabelsVisible(bool viz);
bool areLabelsVisible();
void resetLabels();
void initLabelsEntities();
void renderLabels();
void renderLabel(item label);
item intersectLabel(Line mouseLine);
void readLabels(FileBuffer* file);
void writeLabels(FileBuffer* file);

