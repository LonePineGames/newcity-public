//-----------------------------------------------------------------------------
// dropdown - A dropdown menu UI element, with variable options passed 
// as an array of tuples with a string identifier and a InputCallback
//-----------------------------------------------------------------------------

#pragma once

#include "../iconsEnum.hpp"
#include "button.hpp"
#include "label.hpp"
#include "panel.hpp"

const std::string dropdownPromptTxt = "Choose an option";
const std::string dropdownEmptyTxt = "No elements";
const uint32_t dropdownMaxElements = 32;

struct DropdownElement {
  std::string text = "";
  InputCallback callback = 0;

  DropdownElement(std::string t, InputCallback i) {
    text = t;
    callback = i;
  }
};


Part* dropdown(vec2 pos, vec2 size, bool isOpen, uint32_t activeEleIndex, 
  std::vector<DropdownElement> eleVector, InputCallback dropClickCallback);