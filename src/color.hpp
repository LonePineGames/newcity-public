#pragma once

#include "line.hpp"

const int numColors = 32;
const float paletteSize = 64;

enum PickerPalette {
  RedDark, RedMedD, RedMedL, RedLight,
  OrangeDark, OrangeMedD, OrangeMedL, OrangeLight,
  YellowDark, YellowMedD, YellowMedL, YellowLight,
  GreenDark, GreenMedD, GreenMedL, GreenLight,
  CyanDark, CyanMedD, CyanMedL, CyanLight,
  BlueDark, BlueMedD, BlueMedL, BlueLight,
  PurpleDark, PurpleMedD, PurpleMedL, PurpleLight,
  Black, GrayDark, GrayLight, White,
  Transparent, // special
};

vec3 getColorInPalette(int color);
vec3 getTerrainColor(float u, bool desert);
Line getHeatmapGradient(int hm);

const vec3 colorDarkGray = vec3(11/paletteSize, 5/paletteSize,0);
const vec3 colorTransparentWhite = vec3(9/paletteSize, 3/paletteSize, 0);
const vec3 colorWhite = vec3(7/paletteSize, 1/paletteSize, 0);
const vec3 colorTree = vec3(1/paletteSize,1/paletteSize,0);
const vec3 colorBrown = vec3(3/paletteSize,1/paletteSize,0);
const vec3 colorBeige = vec3(5/paletteSize,1/paletteSize,0);
const vec3 colorPavement = vec3(9/paletteSize,5/paletteSize,0);
const vec3 colorLightGreen = vec3(5/paletteSize,5/paletteSize,0);
const vec3 colorBrightGreen = vec3(11/paletteSize,1/paletteSize,0);
const vec3 colorYellow = vec3(7/paletteSize,5/paletteSize,0);
const vec3 colorYellowFarm = vec3(13/paletteSize,5/paletteSize,0);
const vec3 colorDarkSand = vec3(7/paletteSize,5/paletteSize,0);
const vec3 colorGray = vec3(13/paletteSize,1/paletteSize,0);
const vec3 colorDensityPavement = vec3(11/paletteSize,3/paletteSize,0);
const vec3 colorWaterBlue = vec3(1/paletteSize,5/paletteSize,0);
const vec3 colorRed = vec3(9/paletteSize, 1/paletteSize, 0);
const vec3 colorBlue = vec3(15/paletteSize, 29/paletteSize, 0);

const vec3 colorPanelGrad0 = vec3(3/paletteSize,31/paletteSize, 0);
const vec3 colorPanelGrad1 = vec3(3/paletteSize,29/paletteSize, 0);
const vec3 colorTransparentBlack = vec3(1/paletteSize,31/paletteSize, 0);
const vec3 colorTransparent0 = vec3(17/paletteSize,29/paletteSize, 0);
const vec3 colorTransparent1 = vec3(17/paletteSize,31/paletteSize, 0);
const vec3 colorDarkGrad0 = vec3(9/paletteSize,31/paletteSize, 0);
const vec3 colorDarkGrad1 = vec3(9/paletteSize,29/paletteSize, 0);
const vec3 colorRaisedGrad0 = vec3(7/paletteSize,31/paletteSize, 0);
const vec3 colorRaisedGrad1 = vec3(7/paletteSize,29/paletteSize, 0);
const vec3 colorLoweredGrad0 = vec3(5/paletteSize,31/paletteSize, 0);
const vec3 colorLoweredGrad1 = vec3(5/paletteSize,29/paletteSize, 0);
const vec3 colorGoldGrad0 = vec3(13/paletteSize,31/paletteSize, 0);
const vec3 colorGoldGrad1 = vec3(13/paletteSize,29/paletteSize, 0);
const vec3 colorRedGrad0 = vec3(11/paletteSize,31/paletteSize, 0);
const vec3 colorRedGrad1 = vec3(11/paletteSize,29/paletteSize, 0);
const vec3 colorBlueGrad0 = vec3(15/paletteSize,29/paletteSize, 0);
const vec3 colorBlueGrad1 = vec3(15/paletteSize,31/paletteSize, 0);
const vec3 colorGreenGrad0 = vec3(5/paletteSize,31/paletteSize, 0);
const vec3 colorGreenGrad1 = vec3(5/paletteSize,29/paletteSize, 0);
const vec3 colorNewsGrad0 = vec3(19/paletteSize,29/paletteSize, 0);
const vec3 colorNewsGrad1 = vec3(19/paletteSize,31/paletteSize, 0);
const vec3 colorNewsDarkGrad0 = vec3(21/paletteSize,29/paletteSize, 0);
const vec3 colorNewsDarkGrad1 = vec3(21/paletteSize,31/paletteSize, 0);

