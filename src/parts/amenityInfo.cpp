#include "amenityInfo.hpp"

#include "article.hpp"
#include "block.hpp"
#include "button.hpp"
#include "hr.hpp"
#include "icon.hpp"
#include "image.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "span.hpp"

#include "../amenity.hpp"
#include "../building/design.hpp"
#include "../business.hpp"
#include "../draw/texture.hpp"
#include "../icons.hpp"
#include "../platform/file.hpp"
#include "../platform/lookup.hpp"
#include "../string.hpp"
#include "../string_proxy.hpp"
#include "../zone.hpp"

#include "spdlog/spdlog.h"

static item currentEffect = -1;
static float lastSwitchTime = 0;

bool selectEffect(Part* part, InputEvent event) {
  currentEffect = part->itemData;
  lastSwitchTime = getCameraTime() + 30;
  return true;
}

void effectIcon(Part* pnl, vec2 loc, item effect, float val) {
  float x = loc.x;
  float y = loc.y;
  int number = abs(val);
  bool negative = val < 0;
  float numberSpace = 1.15;

  Part* numLabel = labelRight(vec2(x, y), vec2(numberSpace, 0.75),
    sprintf_o("%s%d", negative ? "-" : "+", number));
  r(pnl, numLabel);

  Part* effIcon = icon(vec2(x+numberSpace, y), vec2(0.75, 0.75),
      getEffectIcon(effect));
  r(pnl, effIcon);

  if (negative) {
    effIcon->foregroundColor = PickerPalette::RedLight;
    numLabel->foregroundColor = PickerPalette::RedLight;
  }
}

bool openCitipediaEffectPage(Part* part, InputEvent event) {
  item effect = part->itemData;
  string article = getEffectString(effect);
  article = "effects/" + article;
  followLink(strdup_s(article.c_str()));
  return true;
}

bool openCitipediaHomelessnessPage(Part* part, InputEvent event) {
  string article = "concepts/homeless";
  followLink(strdup_s(article.c_str()));
  return true;
}

bool preventAmenityInfoDotAdvance(Part* part, InputEvent event) {
  float targetTime = getCameraTime()+2;
  if (lastSwitchTime < targetTime) {
    lastSwitchTime = targetTime;
  }
  return true;
}

Part* amenityInfoPart(vec2 loc, vec2 size,
    item designNdx, item buildingNdx, bool unaffordableRed) {
  //Building* b = getBuilding(buildingNdx);
  Design* d = getDesign(designNdx);
  Part* pnl = panel(loc, size);
  //pnl->flags |= _partTextShadow;

  r(pnl, label(vec2(0,0), 1, strdup_s(d->displayName)));
  //r(pnl, hr(vec2(0,1), pnl->dim.end.x-pnl->padding*2));
  float y = 1.1;
  float padding = 0.1;

  float credit = getCredit();
  if (credit < 0) credit = 0;
  float earnings = getEarnings();
  if (earnings < 0) earnings = 0;

  if (d->zone != GovernmentZone) {
    float cost = getDesignValue(designNdx) * getInflation();
    char* costStr = printMoneyString(cost);
    r(pnl, label(vec2(0,y), 1, sprintf_o("%s", costStr), cost > credit));
    free(costStr);
    return pnl;
  }

  float maintenance = 0;
  if (buildingNdx != 0) {
    maintenance = getMaintenance(buildingNdx);
  } else {
    maintenance = c(CAmenityMaintMult) * d->maintenance
      * getInflation()
      * getBudgetControl(getBudgetLineFromBuildingCategory(d->category));
  }

  float cost = c(CAmenityCostMult) * d->cost * getInflation();
  char* costStr = printMoneyString(abs(cost));
  char* maintStr = printMoneyString(abs(maintenance));
  float costStrWidth = stringWidth(costStr)*0.8+.4;
  Part* costLbl = r(pnl, label(vec2(0,y), 1, sprintf_o("%s,", costStr),
        unaffordableRed && cost > credit));
  Part* maintLbl = r(pnl, label(vec2(costStrWidth,y), 1, sprintf_o("%s/year", maintStr), unaffordableRed && maintenance > earnings));
  free(costStr);
  free(maintStr);

  if (cost < 0) costLbl->foregroundColor = PickerPalette::GreenLight;
  if (maintenance < 0) maintLbl->foregroundColor = PickerPalette::GreenLight;

  if (d->flags & _designSingleton) {
    r(pnl, label(vec2(8.6, y+0.2), 0.75, strdup_s("Limit one per city")));
  }

  y += 1.2;

  string imageFile = "designs/";
  imageFile = imageFile + d->name + "/design.design.png";
  imageFile = lookupFile(imageFile, 0);
  if (!fileExists(imageFile)) {
    imageFile = "designs/";
    imageFile = imageFile + d->name + ".design.png";
    imageFile = lookupFile(imageFile, 0);
  }
  float x = 0;

  getTexture(imageFile.c_str()); // to trigger loading
  vec2 imgDim = getTextureDimensions(imageFile.c_str());
  if (imgDim.y > 0) {
    float aspect = imgDim.x / imgDim.y;
    float sizeY = size.y-y-padding;
    float maxX = std::min(.5f*(pnl->dim.end.x-padding*2), sizeY*aspect);
    float imgSizeX = clamp(size.x, 0.f, maxX);
    Part* img = r(pnl, image(vec2(0,y), imgSizeX,
          strdup_s(imageFile.c_str()), &sizeY));
    img->dim.start.z -= 0.5f;
    x += imgSizeX + padding;
  }

  amenityStats(pnl, vec2(x,y),
      vec2(pnl->dim.end.x-x, pnl->dim.end.y-y), designNdx);

  return pnl;
}

void amenityStats(Part* pnl, vec2 loc, vec2 size, item designNdx) {
  float x = loc.x;
  float y = loc.y;
  float padding = 0.1;
  Design* d = getDesign(designNdx);

  if (getCameraTime() - lastSwitchTime > 2) {
    lastSwitchTime = getCameraTime();
    currentEffect ++;
  }

  vector<item> dots;
  for (int i = 0; i < numEffects; i++) {
    if (d->effect[i] != 0) dots.push_back(i);
  }

  bool hsDiploma = d->flags & _designProvidesHSDiploma;
  bool bclDegree = d->flags & _designProvidesBclDegree;
  bool phd = d->flags & _designProvidesPhd;
  bool healthCare = d->flags & _designProvidesHealthCare;
  bool noEmDom = d->flags & _designNoEminentDomain;
  bool aquatic = d->flags & _designAquatic;
  bool anyFam = d->numFamilies > 0;
  bool noHealth = d->effect[Health] == 0;
  bool noEdu = d->effect[EducationEffect] == 0;

  bool anyBiz = false;
  for (int i = 0; i < 4; i ++) {
    if (d->numBusinesses[i] > 0) {
      if (isDesignEducation(designNdx) && i != Retail) continue;
      anyBiz = true;
      break;
    }
  }

  bool miscEffect = (noEdu && (hsDiploma || bclDegree || phd)) ||
    (noHealth && healthCare) ||
    noEmDom || aquatic || anyFam || anyBiz ||
    d->minLandValue > 0 || d->minDensity > 0;

  if (miscEffect) {
    dots.push_back(-1);
  }

  float effectSpace = pnl->dim.end.x - x;
  float dotSizeX = 1.9f;
  float dotSizeY = .75f;

  for (int i = 0; i < numEffects+1; i++) {
    if (currentEffect < -1) currentEffect = -1;
    if (currentEffect >= numEffects) currentEffect = -1;
    if (currentEffect >= 0 && d->effect[currentEffect] != 0) break;
    if (currentEffect == -1 && miscEffect) break;
    currentEffect ++;
  }

  float dotY = y;
  for (int i = 0; i < dots.size(); i++) {
    if (dotY + dotSizeY > y + size.y) {
      dotY = y;
      x += dotSizeX;
    }

    item effect = dots[i];
    Part* dot = r(pnl, greenBlock(vec2(x, dotY), vec2(dotSizeX, dotSizeY)));
    dot->renderMode = RenderTransparent;
    if (effect == currentEffect) {
      dot->flags |= _partLowered;
    }
    dot->onClick = selectEffect;
    dot->itemData = effect;
    dot->flags |= _partIsPanel | _partHover;

    if (effect < 0) {
      float littleDotSize = 0.25;
      r(dot, block(vec2(dotSizeX-littleDotSize, 0.75-littleDotSize)*.5f,
            vec2(littleDotSize, littleDotSize)));
    } else {
      effectIcon(dot, vec2(0,0), effect, d->effect[effect]);
    }

    dotY += dotSizeY + padding;
  }
  x += dotSizeX + padding*2;

  r(pnl, sunkenBlock(vec2(x-0.05,y), vec2(0.1, size.y-padding*2)));
  x += 0.1;

  float spaceX = size.x+loc.x-x;
  float iconX = x + spaceX*.5f - 1.25f;
  float spanSpace = size.x + loc.x;
  float smallTxt = 0.55;

  if (currentEffect >= 0) {
    float val = d->effect[currentEffect];
    if (val != 0) {
      float titleScale = 0.85;
      //effectIcon(pnl, vec2(0,y), currentEffect, val);
      Part* nameLabel = labelCenter(vec2(x, y), vec2(spaceX-titleScale, titleScale), strdup_s(getEffectString(currentEffect)));
      r(pnl, nameLabel);

      Part* citipediaButt = r(pnl, button(vec2(x+spaceX-titleScale,y), iconCitipedia, vec2(titleScale,titleScale), openCitipediaEffectPage, currentEffect));
      citipediaButt->onHover = preventAmenityInfoDotAdvance;
      y += titleScale;
      //r(pnl, hr(vec2(x,y), pnl->dim.end.x-pnl->padding*2-x));
      //y += 0.2;

      char* desc = getEffectDescriptor(currentEffect, val, d->flags);
      Part* descLabel = spanCenter(vec2(x, y), x,
          vec2(spanSpace, smallTxt), desc, &y);
      r(pnl, descLabel);

      if (val < 0) {
        nameLabel->foregroundColor = PickerPalette::RedLight;
        descLabel->foregroundColor = PickerPalette::RedLight;
      }
    }
    y += padding;

    if (currentEffect == Health && healthCare) {
      r(pnl, spanCenter(vec2(x, y), x,
          vec2(spanSpace, smallTxt), strdup_s("Provides Health Care"), &y));
    }

    if (currentEffect == EducationEffect) {
      if (hsDiploma) {
        r(pnl, spanCenter(vec2(x, y), x, vec2(spanSpace, smallTxt),
              strdup_s("Grants High School Diplomas"), &y));
      }

      if (bclDegree) {
        r(pnl, spanCenter(vec2(x, y), x,
            vec2(spanSpace, smallTxt), strdup_s("Grants Bachelor's Degrees"), &y));
      }

      if (phd) {
        r(pnl, spanCenter(vec2(x, y), x,
            vec2(spanSpace, smallTxt), strdup_s("Grants Doctorates"), &y));
      }
    }

  } else {
    float startX = x;
    float x = startX+padding;

    bool isHotel = d->flags & _designIsHotel;
    if (anyFam) {
      char* famStr = sprintf_o("%d", d->numFamilies);
      float strX = stringWidth(famStr);
      if (x+strX+1 > spanSpace) {
        y ++;
        x = startX+padding;
      }

      r(pnl, labelRight(vec2(x, y), vec2(strX,1), famStr));
      r(pnl, icon(vec2(x+2, y), vec2(1,1),
            isHotel ? iconHotelRoom : iconFamily));
      x+=strX+1;
    }

    for (int i = 0; i < 4; i++) {
      if (d->numBusinesses[i] == 0) continue;
      if (isDesignEducation(designNdx) && i != Retail) continue;
      char* bizStr = sprintf_o("%d", d->numBusinesses[i]);
      float strX = stringWidth(bizStr);
      if (x+strX+1 > spanSpace) {
        y ++;
        x = startX+padding;
      }

      r(pnl, labelRight(vec2(x, y), vec2(strX,1), bizStr));
      r(pnl, icon(vec2(x+strX, y), vec2(1,1), iconBusinessType[i]));
      x+=strX+1+padding;
    }
    if (x > startX+padding) y += 1 + padding;
    x = startX;

    if (noHealth && healthCare) {
      r(pnl, spanCenter(vec2(x, y), x,
          vec2(spanSpace, smallTxt), strdup_s("Provides Health Care"), &y));
    }

    item numHomes = getDesignNumHomes(designNdx);
    if (!isHotel && getDesignNumHomes(designNdx)) {
      Part* citipediaButt = r(pnl, button(vec2(spanSpace-smallTxt,y), iconCitipedia, vec2(smallTxt,smallTxt), openCitipediaHomelessnessPage, 0));
      r(pnl, spanCenter(vec2(x, y), x, vec2(spanSpace-smallTxt, smallTxt), sprintf_o("Provides %d Homes for homeless families.", numHomes), &y));
    }


    if (noEdu) {
      if (hsDiploma) {
        r(pnl, spanCenter(vec2(x, y), x, vec2(spanSpace, smallTxt),
            strdup_s("Grants High School Diplomas"), &y));
      }
      if (bclDegree) {
        r(pnl, spanCenter(vec2(x, y), x,
            vec2(spanSpace, smallTxt), strdup_s("Grants Bachelor's Degrees"), &y));
      }
      if (phd) {
        r(pnl, spanCenter(vec2(x, y), x,
            vec2(spanSpace, smallTxt), strdup_s("Grants Doctorates"), &y));
      }
    }

    if (aquatic) {
      r(pnl, spanCenter(vec2(x, y), x,
          vec2(spanSpace, smallTxt), strdup_s("Must be Placed on a Coast"), &y));
    }

    if (noEmDom) {
      r(pnl, spanCenter(vec2(x, y), x,
          vec2(spanSpace, smallTxt),
          strdup_s("Sponsor will Pay for Eminent Domain"), &y));
    }

    if (d->minLandValue > 0) {
      r(pnl, spanCenter(vec2(x, y), x,
          vec2(spanSpace, smallTxt),
          sprintf_o("Min Value %d", int(d->minLandValue*10)), &y));
    }

    if (d->minDensity > 0) {
      r(pnl, spanCenter(vec2(x, y), x,
          vec2(spanSpace, smallTxt),
          sprintf_o("Min Density %d", int(d->minDensity*10)), &y));
    }
  }
}

