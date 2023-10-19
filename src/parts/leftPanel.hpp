#pragma once

#include "part.hpp"

enum LeftPanelSelection {
  NoPanel, EconomyPanel, BudgetPanel, TransitPanel,
  CitipediaPanel, NewspaperPanel,
  numLeftPanels
};

Part* leftPanel();
void setLeftPanel(LeftPanelSelection next);
LeftPanelSelection getLeftPanel();

