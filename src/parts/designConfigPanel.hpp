#pragma once

#include "part.hpp"

Part* designConfigPanel();
Part* designOrganizerPanel();
bool deleteSelected(Part* part, InputEvent event);
void updateDesignDimensionStrings();
void designConfigPanelSelection();
bool openTextureSelect(Part* part, InputEvent event);
bool closeTextureSelect(Part* part, InputEvent event);

