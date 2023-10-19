#include "amenityInfo.hpp"
#include "designOrganizerPanel.hpp"
#include "mainMenu.hpp"
#include "root.hpp"
#include "span.hpp"

#include "../building/building.hpp"
#include "../building/design.hpp"
#include "../building/statue.hpp"
#include "../amenity.hpp"
#include "../business.hpp"

static ScrollState buildingPanelOverviewScroll;

enum BuildingPanelTabs {
  BuildingOverviewTab, BuildingFinancialTab, BuildingWhatsInsideTab,
  numBuildingPanelTabs
};

item buildingPanelTab = BuildingOverviewTab;

const char* const buildingPanelTabLabels[numBuildingPanelTabs] = {
  "Overview", "Financial", "Who's Inside"
};

bool setBuildingPanelTab(Part* part, InputEvent event) {
  buildingPanelTab = part->itemData;
  return true;
}

bool loadInBuildingDesigner(Part* part, InputEvent event) {
  closeMenus();
  Building* b = getBuilding(getSelection());
  Design* d = getDesign(b->design);
  gameLoad(LoadTargetFilename, ModeBuildingDesigner, strdup_s(d->name));
  return true;
}

void buildingPanel(Part* result, Building* b) {
  result->flags |= _partLowered;

  Design* d = getDesign(b->design);
  float y = 0;
  float scrollBoxTop = 8;
  vec3 ico = iconZoneMono[d->zone];
  char* name = b->name != 0 ? strdup_s(b->name) :
    d->zone == GovernmentZone ? strdup_s(d->displayName) :
    sprintf_o("%s Building", zoneName[d->zone]);
  selectionNameState.text = &b->name;
  bool isHotel = d->flags & _designIsHotel;

  r(result, icon(vec2(0,0), ico));

  if (selectionNameState.flags & _textBoxEditing) {
    r(result, textBoxLabel(vec2(sclPad,0),
          vec2(spWidth-sclPad-1,scl), &selectionNameState));
    y += 1;

  } else {
    r(result, button(vec2(spWidth-sclPad*2, 0), iconPencil,
          vec2(scl, scl), editSelectionName, 0));

    y += 1-scl;
    vec2 end;
    r(result, span(vec2(sclPad+spPadding,y), 0, vec2(spWidth-sclPad*2, scl),
        strdup_s(name), &end));
    y = end.y;
    if (end.x > 0) y += scl;
  }

  // Index Number
  char* ndxStr = sprintf_o("#%d", getSelection());
  float ndxStrWidth = stringWidth(ndxStr)*0.75f;
  r(result, label(vec2(0,y-.25f), 0.75, ndxStr));

  // HR
  r(result, hr(vec2(ndxStrWidth,y), spWidth-ndxStrWidth));
  y += 0.25;

  Part* followButt = button(vec2(spWidth-1,y), iconEye, followSelection);
  if (isFollowingSelection()) {
    followButt->flags |= _partHighlight;
  }
  r(result, followButt);

  if (isFeatureEnabled(FNoteObject)) {
    Part* butt = button(vec2(spWidth-2,y), iconPin, vec2(1,1),
          toggleBuildingMessage, getSelection());
    if (hasMessage(BuildingMessage, getSelection())) {
      butt->flags |= _partHighlight;
    }
    if (blinkFeature(FNoteObject)) {
      butt->flags |= _partBlink;
    }
    r(result, butt);
  }

  y += 0.25;
  float x = 0;
  for (int i = 0; i < numBuildingPanelTabs; i++) {
    float tabButtWidth = stringWidth(buildingPanelTabLabels[i])*.65 +
      spPadding*2;
    Part* tab = r(result, button(vec2(x,y), vec2(tabButtWidth, 0.85),
          strdup_s(buildingPanelTabLabels[i]), setBuildingPanelTab));
    tab->itemData = i;
    tab->flags |= _partAlignCenter;
    if (buildingPanelTab == i) {
      tab->flags |= _partRaised;
    }
    x += tabButtWidth + spPadding;
  }
  y += 0.85;

  float ipWidth = spWidth - spPadding*2;
  float ipHeight = spHeight - y - spPadding*2;

  if (buildingPanelTab == BuildingOverviewTab) {
    Part* inner = scrollbox(vec2(0,0),
        vec2(ipWidth + spPadding*2, ipHeight + spPadding*2));
    //Part* inner = panel(vec2(0,y),
        //vec2(ipWidth + spPadding*2, ipHeight + spPadding*2));
    inner->padding = spPadding;
    inner->flags &= ~_partLowered;
    //r(result, inner);
    float scrollboxY = y;
    y = 0;

    r(inner, graphElementButton(vec2(0,y), b->graphLoc));
    y += sclPad;

    if (!(b->flags & _buildingComplete)) {
      r(inner, label(vec2(0, y), scl, strdup_s("Planned")));
      return;
    }

    r(inner, label(vec2(0, y), scl, sprintf_o("Built %s",
            printDateString(b->builtTime))));
    y += sclPad;

    if (d->zone != GovernmentZone &&
        b->flags & _buildingAbandoned) {
      r(inner, labelRed(vec2(0,y), 1, strdup_s("Abandoned")));
      y += sclPad;
    }
    y++;

    if (d->flags & _designHasStatue) {
      int numStatues = 0;
      for (int i = 0; i < d->decos.size(); i++) {
        Deco deco = d->decos[i];
        if (deco.decoType == statueDecoType()) {
          numStatues ++;
          item statueNdx = getStatueForBuilding(getSelection(), i);
          r(inner, label(vec2(scl, y+sclPad*numStatues), scl,
                strdup_s(getStatueName(statueNdx))));
        }
      }

      if (numStatues > 0) {
        r(inner, label(vec2(0, y), scl, strdup_s(
          numStatues > 1 ? "Features Statues of" : "Features a Statue of")));
        y += sclPad * (numStatues+2);
      }
    }

    if (d->zone == GovernmentZone) {
      float statsSizeY = .85f*4;
      amenityStats(inner, vec2(0,y), vec2(ipWidth-0.5, statsSizeY), b->design);
      y += statsSizeY + 1;

      /*
      for (int i = 0; i < numEffects; i++) {
        float val = d->effect[i];
        int number = abs(val);
        if (number == 0) continue;
        bool negative = val < 0;

        r(inner, icon(vec2(1, y), vec2(0.75, 0.75), getEffectIcon(i)));
        Part* numLabel = labelRight(vec2(0, y+0.125), vec2(1, 0.75),
          sprintf_o("%s%d", negative ? "-" : "+", number));
        r(inner, numLabel);

        Part* nameLabel = label(vec2(1.75, y+0.125), 0.75,
          strdup_s(getEffectString(i)));
        r(inner, nameLabel);

        if (negative) {
          numLabel->foregroundColor = PickerPalette::RedLight;
          nameLabel->foregroundColor = PickerPalette::RedLight;
        }

        char* desc = getEffectDescriptor(i, val, d->flags);
        r(inner, label(vec2(4.75, y+0.125), 0.75, desc));
        y += sclPad;
      }
      y ++;
      */

      /*
      float prevY = y;
      int k = 0;
      float xOff = 0;
      for (int i = 0; i < numEffects; i++) {
        int number = abs(d->effect[i]);
        if (number == 0) continue;
        bool negative = d->effect[i] < 0;
        vec3 ico = negative ? iconRedTile : iconGreenTile;

        r(inner, icon(vec2(xOff, y), getEffectIcon(i)));
        r(inner, label(vec2(xOff + 1, y+0.125), 0.75,
          sprintf_o("%s%d %s", negative ? "-" : "+",
            number, getEffectString(i))));

        for (int j = 0; j < number; j++) {
          vec2 loc = vec2(xOff + int(j/2)*0.5, y + (j%2)*0.5 + 0.0);
          vec2 size = vec2(0.5, 0.5);
          r(inner, icon(loc, size, ico));
        }

        y += 1.1;
        k ++;
        if (k > 2) {
          y = prevY;
          xOff += 7;
          k = 0;
        }
      }
      y = prevY + 3.3;
      */
    }

    float hmBase = 2.6f;
    float hmPad = 0.1f;
    float hmWidth = numHeatMaps*1.1f+.2f;
    Part* heatmapPanel = panel(
        vec2((ipWidth-hmWidth)*.5f, y),
        vec2(numHeatMaps*1.1f+.2f, hmBase + 1.3f));
    heatmapPanel->padding = hmPad;
    heatmapPanel->flags |= _partLowered;

    bool anyHM = false;
    for (int i=0; i < numHeatMaps; i++) {
      if (!isFeatureEnabled(featureForHeatmap(i))) continue;
      anyHM = true;

      float hx = i*1.1f + hmPad;
      float hmVal = heatMapGet((HeatMapIndex)i, b->location);
      int hmValI = round(hmVal*10);//+0.05;
      hmValI = clamp(hmValI, 0, 10);

      Part* hmButt = panel(vec2(hx, 0.0f),
          vec2(1.0f, heatmapPanel->dim.end.y-(hmPad*2)));
      hmButt->flags |= _partHover;
      hmButt->itemData = HeatMapIndex::Pollution+i;
      hmButt->onClick = setHeatMap;
      hmButt->renderMode = RenderTransparent;
      setPartTooltipValues(hmButt, QueSelHMPol+i);

      r(hmButt, icon(vec2(0.0f, hmBase+hmPad), vec2(1,1), iconHeatmap[i]));
      for (int j = 0; j < hmValI && j < 10; j++) {
        //float tileSize = 1.f;//(j+1 >= value) ? 1.f : (value - j);
        //float tileSize = clamp(value - j, 0.f, 1.f);
        float tileSize = 1.f;
        tileSize *= .5f;
        r(hmButt, icon(
            vec2(0.0f+(j%2)*.5, hmBase - (j/2+1)*.5 + .5-tileSize),
            vec2(.5, tileSize), iconHeatmapColor[i]));
      }

      r(heatmapPanel, hmButt);
    }

    if (anyHM) {
      r(inner, heatmapPanel);
      y += heatmapPanel->dim.end.y+1;
    } else {
      freePart(heatmapPanel);
    }

    r(inner, label(vec2(0, y), scl, sprintf_o("Design file: %s",
            d->name)));
    y += sclPad;

    Part* designButt = r(inner, button(vec2(0,y), iconWrench, vec2(ipWidth,scl),
        strdup_s("Edit Design"), setFixBuildingMode, 1));
    setPartTooltipValues(designButt, DesignerFixBuilding);
    y += sclPad;

    /*
    Part* designButt = r(inner, button(vec2(0,y), iconCash, vec2(ipWidth,scl),
        strdup_s("Load in Building Designer"), saveConfirm, 0));
    setPartTooltipValues(designButt, EditInDesigner);
    designButt->ptrData = (void*) loadInBuildingDesigner;
    designButt->flags &= ~_partFreePtr;
    y += sclPad;
    */

    if (b->entity != 0) {
      item texNdx = getEntity(b->entity)->texture;
      vec2 end;
      r(inner, span(vec2(0.125, y+.125), scl, vec2(ipWidth, scl-.125),
            sprintf_o("Texture file:\n%s", getTextureFilename(texNdx)), &end));
      y = end.y + sclPad;
    }

    r(inner, label(vec2(0, y), scl, sprintf_o("Min Density: %d",
            int(d->minDensity*10))));
    y += sclPad;

    r(inner, label(vec2(0, y), scl, sprintf_o("Min Value: %d",
            int(d->minLandValue*10))));
    y += sclPad;

    r(inner, label(vec2(0, y), 1, strdup_s(" "))); // to force scrollbox padding

    Part* sbFrame = r(result, scrollboxFrame(vec2(0,scrollboxY),
        vec2(ipWidth + spPadding*2, ipHeight + spPadding*2),
        &buildingPanelOverviewScroll, inner));
    sbFrame->flags &= ~_partLowered;
  }

  if (buildingPanelTab == BuildingFinancialTab) {
    Part* inner = panel(vec2(0,y),
        vec2(ipWidth + spPadding*2, ipHeight + spPadding*2));
    inner->padding = spPadding;
    r(result, inner);
    y = 0;

    if (d->zone == GovernmentZone) {
      char* value = printMoneyString(getBuildingValue(getSelection()));
      r(inner, label(vec2(0,y), scl,
            sprintf_o("%s value", value)));
      free(value);
      y += sclPad;

      if (isFeatureEnabled(FSellBuilding)) {
        Part* sellButt = button(vec2(0, y), iconCash, vec2(7, scl),
          strdup_s(d->cost < 0 ? "Pay to demolish" : "Sell for half"), sellGovBuilding, 0);
        r(inner, sellButt);
        //if (blinkFeature(FSellBuilding)) {
          //sellButt->flags |= _partBlink;
        //}
      }
      y += sclPad+1;

      bool enabled = b->flags & _buildingEnabled;
      if (enabled) {
        float maintVal = getMaintenance(getSelection());
        char* maint = printMoneyString(abs(maintVal));
        if (maintVal < 0) {
          r(inner, label(vec2(0, y), scl,
            sprintf_o("Pays %s/year", maint)));
        } else {
          r(inner, label(vec2(0, y), scl,
            sprintf_o("%s/year", maint)));
        }
        free(maint);
      } else {
        r(inner, labelRed(vec2(0,y), scl, strdup_s("Shutdown")));
      }
      y += sclPad;

      if (isFeatureEnabled(FShutDownBuilding)) {
        Part* shutButt = button(vec2(0,y),
          enabled ? iconClosed : iconOpen, vec2(5,scl),
          strdup_s(enabled ? "Shutdown" : "Reopen"),
          toggleEnableGovBuilding, 0);
        r(inner, shutButt);
        if (blinkFeature(FShutDownBuilding)) {
          shutButt->flags |= _partBlink;
        }
      }
      y += sclPad+1;

    } else {
      char* value = printMoneyString(getBuildingValue(getSelection()));
      r(inner, label(vec2(0,y), scl,
            sprintf_o("%s value", value)));
      free(value);
      y += sclPad;

      char* taxes = printMoneyString(getBuildingTaxes(getSelection()));
      r(inner, label(vec2(0,y), scl,
            sprintf_o("Pays %s/year in taxes", taxes)));
      free(taxes);
      y += sclPad+1;

      if(b->lots.size() > 0) {
        int maxDens = getLotMaxDensity(b->lots[0]);
        r(inner, label(vec2(0, y), scl,
          sprintf_o("Max Lot Density: %d", maxDens)));
      }
      y += sclPad+1;

      r(inner, label(vec2(0, y), scl,
        sprintf_o("%d buildings share this design.", d->numBuildings)));
      y += sclPad;
      r(inner, label(vec2(0, y), scl,
        sprintf_o("That's %2.2f%% of buildings in this zone",
          designCommonness(b->design)*100)));
      y += sclPad+1;

      bool historical = b->flags & _buildingHistorical;
      Part* historicalButt = button(vec2(0,y),
          historical ? iconBusiness : iconNull,
          vec2(6.5f,scl),
          strdup_s(historical ? "Historical Building" : "Mark Historical"),
          toggleBuildingHistorical, 0);
      r(inner, historicalButt);
      if (historical) historicalButt->flags |= _partHighlight;
      y += sclPad;
    }
  }

  if (buildingPanelTab == BuildingWhatsInsideTab) {
    Part* inner = panel(vec2(0,y),
        vec2(ipWidth + spPadding*2, ipHeight + spPadding*2));
    inner->padding = spPadding;
    r(result, inner);
    y = 0;

    float x = 0;
    r(inner, labelRight(vec2(x, y), vec2(scl,scl),
          sprintf_o("%d", b->peopleInside.size())));
    r(inner, icon(vec2(x+scl, y), vec2(scl,scl), iconPersonMan));
    x+=2;

    item numHomes = getDesignNumHomes(b->design);
    item numFams = b->families.size();
    if (numHomes > 0 || numFams > 0) {
      r(inner, labelRight(vec2(x, y), vec2(scl*3,scl),
            sprintf_o("%d/%d", numFams, numHomes)));
      r(inner, icon(vec2(x+scl*3, y), vec2(scl,scl),
            isHotel ? iconHotelRoom : iconFamily));
      x+=4*scl;
    }

    int numBiz[5] = {0,0,0,0,0};
    for (int i = 0; i < b->businesses.size(); i++) {
      BusinessType type = getBusinessType(b->businesses[i]);
      if (type >= 0 && type < 5) {
        numBiz[type] ++;
      }
    }

    for (int i = 0; i < 5; i++) {
      if (isDesignEducation(b->design) && i != Retail) continue;
      item maxBiz = d->numBusinesses[i];
      if (numBiz[i] == 0 && maxBiz == 0) continue;

      if (x+4*scl >= ipWidth) {
        x = 0;
        y += scl;
      }

      r(inner, labelRight(vec2(x, y), vec2(scl*3,scl),
            sprintf_o("%d/%d", numBiz[i], maxBiz)));
      r(inner, icon(vec2(x+scl*3, y), vec2(scl,scl), iconBusinessType[i]));
      x+=4*scl;
    }

    y++;

    r(inner, objectList(vec2(0,y),
          vec2(ipWidth,ipHeight-y),
      &b->businesses, &b->families, &b->peopleInside));
  }
}

void legacyBuildingPanel(Part* result, Building* selection) {
//void buildingPanel(Part* result, Building* selection) {
  Design* d = getDesign(selection->design);
  float y = 0;
  float scrollBoxTop = 8;
  vec3 ico = iconZoneMono[selection->zone];
  char* name = selection->name != 0 ? strdup_s(selection->name) :
    selection->zone == GovernmentZone ? strdup_s(d->displayName) :
    sprintf_o("%s Building", zoneName[selection->zone]);
  selectionNameState.text = &selection->name;

  r(result, icon(vec2(0,0), ico));

  if (selectionNameState.flags & _textBoxEditing) {
    r(result, textBoxLabel(vec2(sclPad,0),
          vec2(spWidth-sclPad-1,scl), &selectionNameState));
    y += 1;

  } else {
    r(result, multiline(vec2(sclPad,0), vec2(spWidth-sclPad, scl),
        strdup_s(name), &y));
    r(result, button(vec2(spWidth-sclPad*2, 0), iconPencil,
          vec2(scl, scl), editSelectionName, 0));
  }

  // Index Number
  char* ndxStr = sprintf_o("#%d", getSelection());
  float ndxStrWidth = stringWidth(ndxStr)*0.75f;
  r(result, label(vec2(0,y-.25f), 0.75, ndxStr));

  // HR
  r(result, hr(vec2(ndxStrWidth,1.0f), spWidth-ndxStrWidth));
  y += 0.2;

  r(result, graphElementButton(vec2(0,y), selection->graphLoc));
  y += sclPad;

  if (!(selection->flags & _buildingComplete)) {
    r(result, label(vec2(0, y), scl, strdup_s("Planned")));
    return;
  }

  r(result, label(vec2(0, y), scl, sprintf_o("Built %s",
          printDateString(selection->builtTime))));
  y += sclPad;

  r(result, label(vec2(0, y), 0.7, sprintf_o("Design file: %s",
          d->name)));
  y += sclPad;

  if (selection->entity != 0) {
    item texNdx = getEntity(selection->entity)->texture;
    r(result, label(vec2(0, y), 0.6, sprintf_o("Texture file:\n%s",
            getTextureFilename(texNdx))));
    y += 0.6*2;
  }

  if (selection->zone == GovernmentZone) {
    bool enabled = selection->flags & _buildingEnabled;
    if (enabled) {
      char* maint = printMoneyString(getMaintenance(getSelection()));
      r(result, label(vec2(0,y), scl,
            sprintf_o("%s/year", maint)));
      free(maint);
    } else {
      r(result, labelRed(vec2(0,y), scl, strdup_s("Shutdown")));
    }

    if (isFeatureEnabled(FShutDownBuilding)) {
      Part* shutButt = button(vec2(5,y),
        enabled ? iconClosed : iconOpen, vec2(5,1),
        strdup_s(enabled ? "Shutdown" : "Reopen"),
        toggleEnableGovBuilding, 0);
      r(result, shutButt);
      if (blinkFeature(FShutDownBuilding)) {
        shutButt->flags |= _partBlink;
      }
    }
    y += sclPad;

    char* value = printMoneyString(getBuildingValue(getSelection()));
    r(result, label(vec2(0,y), scl,
          sprintf_o("%s value", value)));
    free(value);

    if (isFeatureEnabled(FSellBuilding)) {
      Part* sellButt =button(vec2(5,y), iconCash, vec2(7,1),
            strdup_s("Sell for half"), sellGovBuilding, 0);
      r(result, sellButt);
      //if (blinkFeature(FSellBuilding)) {
        //sellButt->flags |= _partBlink;
      //}
    }
    y += sclPad;

  } else {
    char* taxes = printMoneyString(getBuildingTaxes(getSelection()));
    r(result, label(vec2(0,y), scl,
          sprintf_o("%s taxes/year", taxes)));
    free(taxes);

    bool historical = selection->flags & _buildingHistorical;
    Part* historicalButt = button(vec2(5.5f,y),
        historical ? iconBusiness : iconNull,
        vec2(6.5f,scl),
        strdup_s(historical ? "Historical Building" : "Mark Historical"),
        toggleBuildingHistorical, 0);
    r(result, historicalButt);
    if (historical) historicalButt->flags |= _partHighlight;

    y += sclPad;

    char* value = printMoneyString(getBuildingValue(getSelection()));
    r(result, label(vec2(0,y), scl,
          sprintf_o("%s value", value)));
    free(value);

    if(selection->lots.size() > 0) {
      int maxDens = getLotMaxDensity(selection->lots[0]);
      r(result, label(vec2(6, y), scl,
        sprintf_o("Max Lot Density: %d", maxDens)));
    }

    y += sclPad;
  }

  if (selection->zone != GovernmentZone &&
      selection->flags & _buildingAbandoned) {
    r(result, labelRed(vec2(0,y), 1, strdup_s("Abandoned")));
    y += sclPad;
  }

  if (selection->zone == GovernmentZone) {
    for (int i = 0; i < numEffects; i++) {
      float val = d->effect[i];
      int number = abs(val);
      if (number == 0) continue;
      bool negative = val < 0;

      r(result, icon(vec2(1, y), vec2(0.75, 0.75), getEffectIcon(i)));
      Part* numLabel = labelRight(vec2(0, y+0.125), vec2(1, 0.75),
        sprintf_o("%s%d", negative ? "-" : "+", number));
      r(result, numLabel);

      Part* nameLabel = label(vec2(1.75, y+0.125), 0.75,
        strdup_s(getEffectString(i)));
      r(result, nameLabel);

      if (negative) {
        numLabel->foregroundColor = PickerPalette::RedLight;
        nameLabel->foregroundColor = PickerPalette::RedLight;
      }

      char* desc = getEffectDescriptor(i, val, d->flags);
      r(result, label(vec2(4.75, y+0.125), 0.75, desc));
      y += sclPad;
    }

    /*
    float prevY = y;
    int k = 0;
    float xOff = 0;
    for (int i = 0; i < numEffects; i++) {
      int number = abs(d->effect[i]);
      if (number == 0) continue;
      bool negative = d->effect[i] < 0;
      vec3 ico = negative ? iconRedTile : iconGreenTile;

      r(result, icon(vec2(xOff, y), getEffectIcon(i)));
      r(result, label(vec2(xOff + 1, y+0.125), 0.75,
        sprintf_o("%s%d %s", negative ? "-" : "+",
          number, getEffectString(i))));

      for (int j = 0; j < number; j++) {
        vec2 loc = vec2(xOff + int(j/2)*0.5, y + (j%2)*0.5 + 0.0);
        vec2 size = vec2(0.5, 0.5);
        r(result, icon(loc, size, ico));
      }

      y += 1.1;
      k ++;
      if (k > 2) {
        y = prevY;
        xOff += 7;
        k = 0;
      }
    }
    y = prevY + 3.3;
    */
  }

  scrollBoxTop = y + 4;
  y = scrollBoxTop-1.1;
  float ixs = (spWidth-numHeatMaps*1.1f-.3f)/3.5;
  float ix = ixs*.25f;
  float iys = 1.f;
  float tys = .7f;
  float ins = .8f;
  float ipad = .1f;
  vec2 icoSize(iys-ipad*2, iys-ipad*2);
  int numBiz = selection->businesses.size();
  int numFam = selection->families.size();
  int maxFam = getDesignNumHomes(selection->design);
  int numPop = selection->peopleInside.size();
  int maxBiz = getNumBusinesses(selection->design);

  r(result, labelCenter(vec2(ix, y-iys), vec2(ins, tys),
        sprintf_o("%d", numPop)));
  r(result, icon(vec2(ix,y+ipad), icoSize, iconPersonMan));
  ix += ixs;

  if (numBiz > 0 || maxBiz > 0) {
    r(result, labelCenter(vec2(ix, y-iys), vec2(ins, tys),
          sprintf_o("%d/%d", numBiz, maxBiz)));
    r(result, icon(vec2(ix,y+ipad), icoSize, iconBusiness));
  }
  ix += ixs;

  if (numFam > 0 || maxFam > 0) {
    r(result, labelCenter(vec2(ix, y-iys), vec2(ins, tys),
          sprintf_o("%d/%d", numFam, maxFam)));
    r(result, icon(vec2(ix,y+ipad), icoSize, iconFamily));
  }
  ix += ixs;

  float hmBase = 2.6f;
  float hmPad = 0.1f;
  Part* heatmapPanel = panel(
      vec2(spWidth-numHeatMaps*1.1f-.2f, scrollBoxTop-hmBase-1.4f),
      vec2(numHeatMaps*1.1f+.2f, hmBase + 1.3f));
  heatmapPanel->padding = hmPad;
  heatmapPanel->flags |= _partLowered;

  bool anyHM = false;
  for (int i=0; i < numHeatMaps; i++) {
    if (!isFeatureEnabled(featureForHeatmap(i))) continue;
    anyHM = true;

    float hx = i*1.1f + hmPad;

    Part* hmButt = panel(vec2(hx, 0.0f), vec2(1.0f, heatmapPanel->dim.end.y-(hmPad*2)));
    hmButt->flags |= _partHover;
    hmButt->itemData = HeatMapIndex::Pollution+i;
    hmButt->onClick = setHeatMap;
    hmButt->renderMode = RenderTransparent;
    setPartTooltipValues(hmButt, QueSelHMPol+i);

    float value = heatMapGet((HeatMapIndex)i, selection->location);
    value = round(value*10)+.05;
    value = value > 10 ? 10 : value;
    r(hmButt, icon(vec2(0.0f, hmBase+hmPad), vec2(1,1), iconHeatmap[i]));
    for (int j = 0; j <= value && j < 10; j++) {
      //float tileSize = 1.f;//(j+1 >= value) ? 1.f : (value - j);
      //float tileSize = clamp(value - j, 0.f, 1.f);
      float tileSize = 1.f;
      tileSize *= .5f;
      r(hmButt, icon(
          vec2(0.0f+(j%2)*.5, hmBase - (j/2+1)*.5 + .5-tileSize),
          vec2(.5, tileSize), iconHeatmapColor[i]));
    }

    r(heatmapPanel, hmButt);
  }

  if (anyHM) {
    r(result, heatmapPanel);
  } else {
    freePart(heatmapPanel);
  }

  r(result, objectList(vec2(0,scrollBoxTop),
        vec2(spWidth,spHeight-scrollBoxTop),
    &selection->businesses, &selection->families, &selection->peopleInside));
}
