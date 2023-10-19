#include "leftPanel.hpp"

#include "../amenity.hpp"
#include "../game/game.hpp"
#include "../graph/transit.hpp"
#include "../heatmap.hpp"
#include "../selection.hpp"

#include "blank.hpp"
#include "budgetPanel.hpp"
#include "citipediaPanel.hpp"
#include "designOrganizerPanel.hpp"
#include "economyPanel.hpp"
#include "newspaperPanel.hpp"
#include "selectionPanel.hpp"
#include "transitPanel.hpp"
#include "transitDesignerPanel.hpp"
#include "tutorialPanel.hpp"

static LeftPanelSelection currentPanel = NoPanel;

Part* leftPanel() {
  if (getGameMode() == ModeBuildingDesigner ||
      getGameMode() == ModeDesignOrganizer ||
      isFixBuildingMode()) {
    return blank();
  } else if (currentPanel == BudgetPanel) {
    return budgetPanel();
  } else if (currentPanel == EconomyPanel) {
    return economyPanel();
  } else if (currentPanel == TransitPanel) {
    return transitPanel();
  } else if (currentPanel == CitipediaPanel) {
    return citipediaPanel();
  } else if (currentPanel == NewspaperPanel) {
    return newspaperPanel();
  } else if(getSelectionType() != NoSelection) {
    return selectionPanel();
  } else {
    return blank();
  }
}

void setLeftPanel(LeftPanelSelection next) {
  if (currentPanel == next) return;

  LeftPanelSelection lastPanel = currentPanel;
  currentPanel = next;
  bool isTransit = next == TransitPanel; //next == TransitDesignerPanel;
  bool wasTransit = lastPanel == TransitPanel;
    //lastPanel == TransitDesignerPanel;

  if (wasTransit && getHeatMap() == TransitHeatMap) {
    paintTransit();
    //setHeatMap(Pollution, false);
  } else if (isTransit && getHeatMap() != TransitHeatMap) {
    setHeatMap(TransitHeatMap, true);
  }
}

LeftPanelSelection getLeftPanel() {
  return currentPanel;
}

