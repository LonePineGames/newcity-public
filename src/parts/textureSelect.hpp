#pragma once

#include "part.hpp"

enum TextureSelectionState {
  TSSClosed, TSSAlbedo, TSSIllumination,
  numTextureSelectionStates
};

Part* textureSelectPanel(vec2 loc, vec2 size, item textureSelectionState);
static bool selectTexture(Part* part, InputEvent event);

