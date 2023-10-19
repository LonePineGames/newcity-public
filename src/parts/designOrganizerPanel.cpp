#include "selectionPanel.hpp"
#include "mainMenu.hpp"

#include "../selection.hpp"
#include "../building/designOrganizer.hpp"
#include "../building/designPackage.hpp"

bool fixBuildingMode = false;

bool isFixBuildingMode() {
  return fixBuildingMode;
}

void setFixBuildingMode(bool val) {
  fixBuildingMode = val;
}

bool setFixBuildingMode(Part* part, InputEvent event) {
  setFixBuildingMode(part->itemData > 0);
  return true;
}

bool closeFixBuilding(Part* part, InputEvent event) {
  setFixBuildingMode(false);
  clearSelection();
  return true;
}

bool setOrganizerZone(Part* part, InputEvent event) {
  if (getGameMode() == ModeDesignOrganizer) {
    setOrganizerZone(part->itemData);
  } else if (isFixBuildingMode() && getSelectionType() == SelectionBuilding && getSelection() > 0) {
    Building* b = getBuilding(getSelection());
    Design* d = getDesign(b->design);
    d->zone = part->itemData;
  }
  return true;
}

bool deleteDesign(Part* part, InputEvent event) {
  item bNdx = getSelection();
  Building* b = getBuilding(bNdx);
  item dNdx = b->design;
  removeBuilding(bNdx);
  deleteDesign(dNdx);
  return true;
}

Part* designOrganizerPanel() {
  item bNdx = getSelection();
  if (getSelectionType() != SelectionBuilding) bNdx = 0;

  Part* result = panel(vec2(0,0),
      vec2(dcpWidth+dcpPadding*2, (dcpHeight+dcpPadding*2)));
  result->padding = dcpPadding;
  r(result, labelCenter(vec2(0,0), vec2(dcpWidth,1),
        strdup_s(isFixBuildingMode() ? "Edit Design" : "Design Organizer")));

  if (isFixBuildingMode()) {
    r(result, button(vec2(0.f, 0.f), iconLeft, setFixBuildingMode, 0));
    r(result, button(vec2(dcpWidth-1, 0.f), iconX, closeFixBuilding, 0));
  }

  #ifdef INCLUDE_STEAM
    // Even if Steam's included, only show buttons if it has an active connection
    if (steam_isActive() && getSelectedDesignNdx() != 0) {
      Part* steamButt = button(vec2(dcpWidth-1-isFixBuildingMode(), 0.f), iconSteam, openDesignInWorkshop, getSelectedDesignNdx());
      setPartTooltipValues(steamButt, TooltipType::DesignerOpenInWorkshop);
      r(result, steamButt);
    }
  #endif

  float y = 1;
  vec2 dpSize = vec2(dcpWidth*.5f-2,dcpScale + 0.7);
  vec2 zbSize = vec2(dcpWidth*.5f,dcpScale);

  Part* tabPanel = panel(vec2(-dcpPadding,y), vec2(dcpWidth+dcpPadding*2,1));
  tabPanel->flags |= _partLowered;
  r(result, tabPanel);
  y += 1.25f;

  if (bNdx != 0) {
    InputCallback cb = getGameMode() == ModeGame ? saveConfirm : loadInBuildingDesigner;
    Part* designButt = r(result, button(vec2(0,y), iconWrench, vec2(dcpWidth,dcpScale), strdup_s("Load in Building Designer"), cb, 0));
    if (cb == saveConfirm) {
      setPartTooltipValues(designButt, EditInDesigner);
      designButt->ptrData = (void*) loadInBuildingDesigner;
      designButt->flags &= ~_partFreePtr;
    }
  }
  y += dcpScale*2;

  if (getGameMode() == ModeDesignOrganizer || (isFixBuildingMode() && bNdx != 0)) {

    item zone = getOrganizerZone();
    if (bNdx != 0) {
      Building* b = getBuilding(bNdx);
      Design* d = getDesign(b->design);
      zone = d->zone;
    }

    r(result, label(vec2(0,y), dcpScale, strdup_s("Zone")));
    y += dcpScale;
    Part* zones = panel(vec2(0,y),
        vec2(dcpWidth, dcpScale*(numZoneTypes-1)*.5f));
    y += zbSize.y*(numZoneTypes/2) + 0.7;
    for (int i=1; i < numZoneTypes; i++) {
      Part* butt = button(vec2(((i-1)%2)*dcpWidth*.5f,((i-1)/2)*dcpScale),
          zbSize, strdup_s(zoneName[i]), setOrganizerZone);
      butt->itemData = i;
      if (i == zone) butt->flags |= _partHighlight;
      r(zones, butt);
    }
    r(result, zones);
  }

  if (bNdx != 0) {
    Building* b = getBuilding(bNdx);
    Design* d = getDesign(b->design);

    r(tabPanel, label(vec2(0,0), 1, strdup_s(d->name)));

    float deletePadding = (1-dcpScale)/2;
    Part* deleteButt = r(tabPanel, button(vec2(tabPanel->dim.end.x-deletePadding-dcpScale, deletePadding), iconTrash, vec2(dcpScale,dcpScale), deleteDesign, 0));

    bool isGov = d->zone == GovernmentZone;
    bool isEdu = isDesignEducation(b->design);
    bool isHotel = d->flags & _designIsHotel;
    vec2 dpSize = vec2(dcpWidth*.5f-2,dcpScale + 0.7);
    vec2 zbSize = vec2(dcpWidth*.5f,dcpScale);

    for (int param = DPMinYear; param <= DPBiz3; param++) {
      bool odd = ((param-DPMinYear) % 2);
      float x = odd * (dpSize.x+2);

      vec3 ico = designParameterIcon[param];
      if (isEdu&& param >= DPHomes) ico = universityIcons[param-DPHomes];
      if (isHotel && param == DPHomes) ico = iconHotelRoom;
      r(result, icon(vec2(x,y), vec2(dcpScale, dcpScale), ico));
      for (int u = 0; u < 2; u ++) {
        float df = 1;
        if (u%2 == 1) df*=-1;
        float xOff = u % 2 == 0 ? 0 : 0.6f;
        Part* dfButt = r(result, button(vec2(x+xOff,y+dcpScale+0.1),
              vec2(0.6f, 0.6f), strdup_s(u%2==0 ? "+" : "-"),
              designParameterStep));
        dfButt->itemData = param;
        dfButt->vecData.x = df;
      }

      designParameterTextBox(result, vec2(x+1,y), dpSize, param);
      if (odd || param == DPBiz3) y += dpSize.y + dcpPadding;
    }

    r(result, icon(vec2(0,y), vec2(dcpScale, dcpScale), iconHeatmap[1]));
    Line valueLine = getHeatmapGradient(Value);
    Part* valSlide = r(result, slider(vec2(dcpScale,y),
          vec2(dcpWidth-dcpScale,dcpScale),
          d->minLandValue, setMinLandValue, valueLine));
    valSlide->renderMode = RenderGradientRotated;
    y += dcpScale;
    r(result, label(vec2(0,y), dcpScale, sprintf_o("Minimum Land Value - %d",
            (int)round(d->minLandValue*10))));
    y += dcpScale * 1.5f;

    r(result, icon(vec2(0,y), vec2(dcpScale, dcpScale), iconHeatmap[2]));
    Line densLine = getHeatmapGradient(Density);
    Part* densSlide = r(result, slider(vec2(dcpScale,y),
          vec2(dcpWidth-dcpScale,dcpScale),
          d->minDensity, setMinDensity, densLine));
    densSlide->renderMode = RenderGradientRotated;
    y += dcpScale;
    r(result, label(vec2(0,y), dcpScale,
          sprintf_o("Minimum Density - %d", (int)round(d->minDensity*10))));
    y += dcpScale * 1.5f;

    /*
    if (!isGov) {
      float value = getDesignValue(1,
          ourCityEconNdx(), d->minLandValue, d->minDensity);
      char* valueStr = printMoneyString(value);
      r(result, label(vec2(0,y), dcpScale,
            sprintf_o("%s Value (1950)", valueStr)));
      free(valueStr);
      y += dcpScale * 1.5f;
    }

    r(result, button(vec2(0,y),
          d->flags & _designIsHotel ? iconCheck : iconNull,
          vec2(dcpWidth, dcpScale), strdup_s("Is a Hotel"),
          toggleDesignFlag, _designIsHotel));
    y += dcpScale;
    */

    r(result, button(vec2(0,y),
          !(d->flags & _designDisableSpawning) ? iconCheck : iconNull,
          vec2(dcpWidth, dcpScale), strdup_s("Spawnable"),
          toggleDesignFlag, _designDisableSpawning));
    y += dcpScale;

  }
  return result;
}
