#ifndef SELECTION_H
#define SELECTION_H

#include "item.hpp"
#include "main.hpp"
#include "route/location.hpp"

enum SelectionTypes {
  NoSelection=0, SelectionVehicle, SelectionBuilding, SelectionPerson,
  SelectionGraphElement, SelectionFamily, SelectionBusiness, SelectionLot,
  SelectionPillar, SelectionStop, SelectionLaneBlock,
  SelectionStructure, SelectionDeco, // Building Designer
  numSelectionTypes
};

void setHighlight(bool highlight, item querySelectionType,
    item querySelection);
void setRedHighlight(bool highlight, item selectionType, item selection);
void setFollowingSelection(bool follow);
bool isFollowingSelection();
void resetSelection();
void renderSelection();
void clearSelection();
void selectLocation(Location loc);
void setSelection(item selectionType, item element);
void deselect(item selectionType, item element);
void updateSelection(double deltaTime);
bool isSelected(item st, item element);
bool isMonitored(item st, item element);
item getSelectionType();
item getSelection();
vec3 getSelectionLocation();

#endif
