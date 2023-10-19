#pragma once

#include "part.hpp"

#include "../serialize.hpp"

void setEPChart(item stat, item timePeriod);
bool toggleEconomyPanel(Part* part, InputEvent event);
Part* economyPanel();
void writeEconomyPanel(FileBuffer* file);
void readEconomyPanel(FileBuffer* file, int version);
bool toggleChartMessage(Part* part, InputEvent event);
bool selectChart(Part* part, InputEvent event);

