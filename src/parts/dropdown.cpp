//-----------------------------------------------------------------------------
// dropdown - A dropdown menu UI element, with variable options passed 
// as a vector of elements with a string identifier and an InputCallback
//-----------------------------------------------------------------------------

#include "dropdown.hpp"


Part* dropdown(vec2 pos, vec2 size, bool isOpen, uint32_t activeEleIndex, 
  std::vector<DropdownElement> eleVector, InputCallback dropClickCallback) {
  // Is the activeEleIndex valid
  bool activeIndexValid = eleVector.size() > 0 && activeEleIndex < eleVector.size();

  // Check for active element string
  std::string activeStr = activeIndexValid ? activeStr = eleVector[activeEleIndex].text 
    : activeStr = dropdownPromptTxt;

  // Internal X-axis end, to make room for the dropdown button
  float dropXEnd = size.x - size.y;
  
  // Create the base panel and label
  Part* dropBase = panel(pos, size);
  Part* dropActiveLabel = label(pos, size-vec2(dropXEnd,0.0f), activeStr.c_str());

  // Add the dropdown button
  vec2 buttonSize = vec2(size.x-dropXEnd, size.y);
  Part* dropButton = button(vec2(dropXEnd, 0.0f), iconDown, buttonSize, dropClickCallback, 0);
  r(dropBase, dropActiveLabel);
  r(dropBase, dropButton);

  if(isOpen) {

  }

  return dropBase;
}