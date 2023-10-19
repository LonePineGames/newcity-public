#include "budgetPanel.hpp"

#include "../amenity.hpp"
#include "../draw/camera.hpp"
#include "../game/feature.hpp"
#include "../icons.hpp"
#include "../money.hpp"
#include "../string_proxy.hpp"
#include "../tutorial.hpp"
#include "../zone.hpp"

#include "article.hpp"
#include "blank.hpp"
#include "block.hpp"
#include "button.hpp"
#include "icon.hpp"
#include "label.hpp"
#include "leftPanel.hpp"
#include "messageBoard.hpp"
#include "panel.hpp"
#include "slider.hpp"
#include "scrollbox.hpp"
#include "tooltip.hpp"

#include "spdlog/spdlog.h"

static bool bpExpanded = false;
static bool groupCollapsed[3];
static ScrollState budgetScroll;
static int budgetPage = 0;
const float maxLoanTime = 10;
const float maxTaxRate[5] = {0.1, 0.05, 0.2, 0.2, 1.0};
static const Line bpDarkGrad = line(colorNewsDarkGrad0, colorNewsDarkGrad1);

void resetBudgetPanel() {
  bpExpanded = false;
  resetScrollState(&budgetScroll);
  for (int i = 0; i < 3; i++) groupCollapsed[i] = false;
  budgetPage = 0;
}

bool moveBudgetPage(Part* part, InputEvent event) {
  budgetPage += part->itemData;
  return true;
}

bool toggleBPExpanded(Part* part, InputEvent event) {
  bpExpanded = !bpExpanded;
  return true;
}

bool toggleGroupCollapsed(Part* part, InputEvent event) {
  int i = part->itemData;
  groupCollapsed[i] = !groupCollapsed[i];
  return true;
}

bool toggleBudgetPanel(Part* part, InputEvent event) {
  if (getLeftPanel() == BudgetPanel) {
    setLeftPanel(NoPanel);
  } else {
    setLeftPanel(BudgetPanel);
    stopBlinkingFeature(FBudget);
    reportTutorialUpdate(TutorialUpdateCode::SelectedBudgetPanel);
  }

  return true;
}

bool setBudgetControl(Part* part, InputEvent event) {
  float value = int(part->vecData.x*10*c(CMaxBudgetControl))/10.f;
  setBudgetControl((BudgetLine)part->itemData, value);
  recalculateEffectsAllGovernmentBuildings();
  reportTutorialUpdate(TutorialUpdateCode::UpdatedBudgetControl);
  return true;
}

bool setLoanTime(Part* part, InputEvent event) {
  float value = int(part->vecData.x*maxLoanTime*2)/2.0;
  setLoanRepaymentTime(value);
  stopBlinkingFeature(FLoanTerm);
  reportTutorialUpdate(TutorialUpdateCode::UpdatedBudgetLoan);
  return true;
}

bool setTaxRate(Part* part, InputEvent event) {
  BudgetLine tax = (BudgetLine)part->itemData;
  float value = int(part->vecData.x*20)/20.f*maxTaxRate[tax];
  setTaxRate(tax, value);
  stopBlinkingFeature(FPropertyTax+tax-1);
  reportTutorialUpdate(TutorialUpdateCode::UpdatedBudgetTax);
  return true;
}

bool toggleBudgetMessage(Part* part, InputEvent event) {
  toggleMessage(BudgetMessage, part->itemData);
  stopBlinkingFeature(FNoteBudget);
  return true;
}

bool openCitipediaBudgetPage(Part* part, InputEvent event) {
  BudgetLine line = (BudgetLine)part->itemData;
  string article = getBudgetLineCode(line);
  article = "budget/" + article;
  followLink(strdup_s(article.c_str()));
  return true;
}

Part* budgetAlternation(item* lineNum, float* lineStart, float bpWidth, float sfy) {

  Part* blk = 0;

  if ((*lineNum) % 2 == 1) {



    /*
const vec3 colorPanelGrad0 = vec3(3/paletteSize,31/paletteSize, 0);
const vec3 colorPanelGrad1 = vec3(3/paletteSize,29/paletteSize, 0);
const vec3 colorTransparentBlack = vec3(1/paletteSize,31/paletteSize, 0);
const vec3 colorTransparent0 = vec3(17/paletteSize,29/paletteSize, 0);
const vec3 colorTransparent1 = vec3(17/paletteSize,31/paletteSize, 0);
const vec3 colorDarkGrad0 = vec3(9/paletteSize,31/paletteSize, 0);
const vec3 colorDarkGrad1 = vec3(9/paletteSize,29/paletteSize, 0);
const vec3 colorRaisedGrad0 = vec3(7/paletteSize,31/paletteSize, 0);
const vec3 colorRaisedGrad1 = vec3(7/paletteSize,29/paletteSize, 0);
*/

    blk = gradientBlock(vec2(0.f, *lineStart), vec2(bpWidth-1.f, sfy-*lineStart), colorTransparent0, colorTransparent0);
    blk->dim.start.z -= 0.3;
  }

  ++ *lineNum;
  (*lineStart) = sfy;

  return blk;
}

Part* budgetPanel() {
  //How many budgets to show? And which ones?
  int numBudgets = bpExpanded ? 10 : 3;
  if (getAspectRatio() < 1.4 && numBudgets > 7) numBudgets = 7;
  Budget b[10];
  numBudgets = clamp(numBudgets, 0, 3+getNumHistoricalBudgets());
  budgetPage = clamp(budgetPage, -getNumHistoricalBudgets(), 1);
  int firstBudget = clamp(budgetPage-numBudgets+3,
      -getNumHistoricalBudgets(), 3-numBudgets);

  float tableX = 8;
  float columnX = 3;
  float bpWidth = tableX + (columnX) * numBudgets + 1.25;
  float bpHeight = 18.5;
  float bpPadding = 0.25;
  float scale = 0.75;
  float subScale = 0.75;
  float sliderSize = 0.5;
  float sliderOffset = subScale/2 - sliderSize/2;
  float scrollStart = 1.25;
  vec2 iconS = vec2(scale, scale);

  vec2 bpLoc = vec2(0.0, 1.5);
  vec2 bpSize = vec2(bpWidth+bpPadding*2, bpHeight+bpPadding*2);
  Part* result = panel(bpLoc, bpSize);
  result->padding = bpPadding;
  result->renderMode = RenderPanelGradient;
  result->texture = line(colorNewsGrad0,colorNewsGrad1);
  result->foregroundColor = PickerPalette::Black;
  result->flags |= _partTextured;

  r(result, label(vec2(0.125,0.125), 1, strdup_s("Budget")));
  r(result, button(vec2(bpWidth-1,0), iconX, toggleBudgetPanel));

  Part* expandButt = button(vec2(tableX-scale+bpPadding-2,0.125),
      iconPlus, iconS, toggleBPExpanded, 0);
  if (bpExpanded) {
    expandButt->flags |= _partHighlight;
  }
  r(result, expandButt);

  if (firstBudget > -getNumHistoricalBudgets()) {
    Part* pastButt = button(vec2(tableX-scale+bpPadding-1,0.125),
        iconLeft, iconS, moveBudgetPage, -clamp(numBudgets, 2, 5));
    r(result, pastButt);
  }

  if (budgetPage < 0) {
    Part* futureButt = button(vec2(tableX-scale+bpPadding,0.125),
        iconRight, iconS, moveBudgetPage, clamp(numBudgets, 2, 5));
    r(result, futureButt);
  }

  for (int j = 0; j < numBudgets; j++) {
    b[j] = getBudget(firstBudget + j);
    int flags = b[j].flags;
    int year = b[j].year + c(CStartYear);
    const char* affix = flags & _budgetIsYTD ? "YTD" :
      flags & _budgetIsEstimate ? "Est." : "Actual";
    r(result, label(vec2(tableX+j*columnX+1.,-0.125), 0.75,
          sprintf_o("%d\n%s", year, affix)));
  }

  Part* scroll = scrollbox(vec2(0,0), vec2(bpWidth, bpHeight-scrollStart));
  float sfy = 0;
  item lineNum = 0;
  float lineStart = sfy;

  const int groups[] = {
    NullBudget, AssetSalesIncome,
    RoadBuildExpenses, MiscDiscExpenses,
    EducationExpenses, TransitExpenses,
    TotalEarnings, BudgetBalance
  };

  const char* groupName[] = {
    "Income", "Discretionary Expenses", "Mandatory Expenses", "Financials"
  };

  const int groupSum[] = {
    TotalIncome, TotalDiscretionary, TotalMandatory, NullBudget
  };

  for (int g = 0; g < 4; g++) {
    if (g < 3) {

      /*
      bool groupShow = g == 0;
      for (int y = 0; y < numBudgets; y++) {
        if (b[y].line[groupSum[g]] != 0) {
          groupShow = true;
          break;
        }
      }
      if (!groupShow) continue;
      */

      for (int y = 0; y < numBudgets; y++) {
        money amount = b[y].line[groupSum[g]];
        r(scroll, label(vec2(tableX + y*columnX,sfy), scale,
              printPaddedMoneyString(amount), amount < 0));
      }

      if (isFeatureEnabled(FNoteBudget)) {
        Part* gNoteButt = button(vec2(tableX - scale, sfy), iconPin, iconS,
            toggleBudgetMessage, groupSum[g]);
        setPartTooltipValues(gNoteButt,
          TooltipType::GenMsg);
        r(scroll, gNoteButt);

        if (hasMessage(BudgetMessage, g)) {
          gNoteButt->flags |= _partHighlight;
        }
      }
    }

    TooltipType groupTooltip = (TooltipType)(
        TooltipType::BudNullBudget+groupSum[g]);
    if (g <= 2) {
      Part* collapseButt = r(scroll, button(vec2(0, sfy),
          groupCollapsed[g] ? iconPlus : iconMinus,
          vec2(tableX-scale, scale),
          strdup_s(groupName[g]), toggleGroupCollapsed, g));
      setPartTooltipValues(collapseButt, groupTooltip);

    } else {
      Part* groupLbl = r(scroll, label(vec2(0,sfy),
            vec2(tableX-scale, scale), strdup_s(groupName[g])));
      setPartTooltipValues(groupLbl, groupTooltip);
    }

    sfy += scale;
    r(scroll, blackBlock(vec2(0,sfy), vec2(bpWidth-1, 0.1)));
    lineStart = sfy;
    r(scroll, budgetAlternation(&lineNum, &lineStart, bpWidth, sfy));
    sfy += 0.5;
    lineStart = sfy;

    if (g <= 2 && groupCollapsed[g]) continue;

    // Make RepairExpenses a mandatory expense
    vector<int> linesInGroup;
    for (int l = groups[g*2]; l <= groups[g*2+1]; l += 1) {
      if (l == RepairExpenses) continue;
      linesInGroup.push_back(l);
    }
    if(g == 2) linesInGroup.push_back(RepairExpenses);

    for (int i = 0; i < linesInGroup.size(); i++) {
      int l = linesInGroup[i];

      bool show = l == LineOfCredit;
      for (int y = 0; y < numBudgets; y++) {
        money amount = b[y].line[l];
        if (abs(amount) > 0 ||
            (l >= PropertyTax && l <= FinesAndFeesIncome && isTaxEnabled(l))) {
          show = true;
          r(scroll, label(vec2(tableX + y*columnX,sfy), scale,
                printPaddedMoneyString(amount), amount < 0));
        }
      }

      if (g == 2 && getBudgetControl((BudgetLine)l) == 0) {
        show = true;
      }

      if (show) {
        Part* lineIco = r(scroll, icon(vec2(0,sfy), vec2(scale, scale),
              getBudgetLineIcon((BudgetLine) l)));
        lineIco->foregroundColor = PickerPalette::GrayDark;
        Part* lineLbl = r(scroll, label(vec2(scale,sfy),
              vec2(tableX-scale*3,scale),
              strdup_s(getBudgetLineName((BudgetLine) l))));
        Part* lineInfo = r(scroll, button(vec2(tableX - scale*2, sfy),
              iconCitipedia, iconS, openCitipediaBudgetPage, l));
        lineInfo->foregroundColor = PickerPalette::White;
        TooltipType tooltip = (TooltipType)(TooltipType::BudNullBudget+l);
        setPartTooltipValues(lineIco, tooltip);
        setPartTooltipValues(lineLbl, tooltip);
        setPartTooltipValues(lineInfo, TooltipType::DocsCitipedia);

        if (isFeatureEnabled(FNoteBudget)) {
          Part* noteButt = button(vec2(tableX - scale, sfy), iconPin, iconS,
              toggleBudgetMessage, l);
          setPartTooltipValues(noteButt,
            TooltipType::GenMsg);
          r(scroll, noteButt);

          if (hasMessage(BudgetMessage, l)) {
            noteButt->flags |= _partHighlight;
          }
          if (l == TotalEarnings && blinkFeature(FNoteBudget)) {
            noteButt->flags |= _partBlink;
          }
        }

        sfy += scale;
      }

      if (l >= PropertyTax && l <= FuelTaxIncome
          && isTaxEnabled(l) && !isTaxLocked(l)) {
        float tax = getTaxRate((BudgetLine)l);
        Part* sliderContainer = panel(vec2(0,sfy), vec2(3+bpPadding*2,subScale));
        sliderContainer->renderMode = RenderTransparent;
        sliderContainer->padding = 0.05;
        r(scroll, sliderContainer);
        Part* taxSlider = slider(vec2(0,sliderOffset-0.05),
            vec2(3,sliderSize), tax/maxTaxRate[l], setTaxRate);
        if (lineNum % 2 == 1) taxSlider->flags = RenderPanelGradient;
        taxSlider->itemData = l;
        if (blinkFeature(FPropertyTax+l-PropertyTax)) {
          sliderContainer->flags |= _partBlink;
        }
        r(sliderContainer, taxSlider);
        char* str;
        if (l == PropertyTax) {
          str = sprintf_o("%1.1f%%", tax*100);
        } else if (l == SalesTax) {
          str = sprintf_o("%1.1fc/$", tax*100);
        } else if (l == FinesAndFeesIncome) {
          str = strdup_s(tax < .01f ? "None" : tax < .05f ? "Minimal" :
              tax < .1f ? "Modest" : tax < .15f ? "High" : "Aggresive");
        } else if (l == FuelTaxIncome) {
          if (tax == 0) {
            str = strdup_s("No Gas Tax");
          } else {
            str = sprintf_o("%1.1fc/$", tax*100);
          }
        } else {
          str = sprintf_o("%1.1f%%", tax*100);
        }
        r(sliderContainer, label(vec2(3.25,0), subScale, str));
        sfy += subScale;

      } else if (l == LineOfCredit) {
        if (isFeatureEnabled(FLoanTerm)) {
          float time = getLoanRepaymentTime();
          Part* sliderContainer = panel(vec2(0,sfy), vec2(3+bpPadding*2,subScale));
          sliderContainer->renderMode = RenderTransparent;
          sliderContainer->padding = 0.05;
          r(scroll, sliderContainer);
          Part* loanSlider = slider(vec2(0,sliderOffset-0.05),
              vec2(3,sliderSize), time/maxLoanTime, setLoanTime);
          if (blinkFeature(FLoanTerm)) {
            sliderContainer->flags |= _partBlink;
          }
          r(sliderContainer, loanSlider);
          if (time > 0) {
            r(sliderContainer, label(vec2(3.25,0), subScale,
              sprintf_o("%2.1f yr at %2.f%%", time, getInterestRate()*100)));
          }
          sfy += subScale;
        }

      } else if (show && g == 2 && l != TransitExpenses) {
        // Budget Controls
        Part* sliderContainer = panel(vec2(0,sfy), vec2(3+bpPadding*2,subScale));
        sliderContainer->renderMode = RenderTransparent;
        sliderContainer->padding = 0.05;
        r(scroll, sliderContainer);
        float val = getBudgetControl((BudgetLine)l);
        Part* controlSlider = slider(vec2(0,sliderOffset-0.05),
            vec2(3,sliderSize), val/c(CMaxBudgetControl), setBudgetControl);
        controlSlider->itemData = l;
        r(sliderContainer, controlSlider);
        if (val > 0) {
          r(sliderContainer, label(vec2(3.25,0), subScale,
            sprintf_o("%2d%% funding", int(val*100))));
        } else {
          r(sliderContainer, label(vec2(3.25,0), subScale,
            sprintf_o("Shut Down", int(val*100))));
        }
        sfy += subScale;
      }

      if (sfy > lineStart) {
        r(scroll, budgetAlternation(&lineNum, &lineStart, bpWidth, sfy));
      } else {
        lineStart = sfy;
      }
    }

    //r(scroll, budgetAlternation(&lineNum, &lineStart, bpWidth, sfy));
    sfy += scale;
  }

  // Cash to Spend
  //sfy -= scale;
  r(scroll, blackBlock(vec2(0,sfy), vec2(bpWidth-1, 0.1)));
  sfy += 0.1;

  Part* ctsIco = r(scroll, icon(vec2(0,sfy), vec2(scale, scale), iconCash));
  Part* ctsLbl = r(scroll, label(vec2(scale,sfy), vec2(tableX-scale*3,scale),
        strdup_s("Cash to Spend")));
  Part* ctsInfo = r(scroll, button(vec2(tableX - scale*2, sfy),
        iconCitipedia, iconS, openCitipediaBudgetPage, BudgetBalance+1));
  ctsIco->foregroundColor = PickerPalette::GrayDark;
  ctsInfo->foregroundColor = PickerPalette::White;
  TooltipType ctsTooltip = TooltipType::BudCashToSpend;
  setPartTooltipValues(ctsIco, ctsTooltip);
  setPartTooltipValues(ctsLbl, ctsTooltip);
  setPartTooltipValues(ctsInfo, TooltipType::DocsCitipedia);

  Part* ctsNoteButt = button(vec2(tableX - scale, sfy), iconPin, iconS,
      toggleBudgetMessage, BudgetBalance+1);
  setPartTooltipValues(ctsNoteButt,
    TooltipType::GenMsg);
  r(scroll, ctsNoteButt);

  if (hasMessage(BudgetMessage, BudgetBalance+1)) {
    ctsNoteButt->flags |= _partHighlight;
  }

  for (int y = 0; y < numBudgets; y++) {
    money amount = b[y].line[BudgetBalance] + b[y].line[LineOfCredit];
    r(scroll, label(vec2(tableX + y*columnX,sfy), scale,
          printPaddedMoneyString(amount), amount < 0));
  }

  lineStart = sfy;
  sfy += scale;
  r(scroll, budgetAlternation(&lineNum, &lineStart, bpWidth, sfy));
  sfy += scale;

  if (c(CNewspaperTextured)) scroll->flags |= _partTextured;
  scroll->flags &= ~_partTextShadow;
  scroll->renderMode = RenderPanelGradient;

  vec3 white = getColorInPalette(PickerPalette::GrayLight);
  Line bpGrad = line(white, white);
  scroll->texture = bpGrad;
  scroll->foregroundColor = PickerPalette::Black;
  scroll->dim.end.y = sfy;

  Part* scrollFrame = scrollboxFrame(vec2(0,scrollStart),
      vec2(bpWidth, bpHeight-scrollStart),
      &budgetScroll, scroll);
  scrollFrame->renderMode = RenderTransparent;
  scrollFrame->flags &= ~_partRaised;

  Part* slider = budgetScroll.slider;
  if (slider != 0) {
    slider->texture = bpDarkGrad;
    slider->renderMode = RenderGradient;
  }

  r(result, scrollFrame);

  scroll->dim.end.z = budgetScroll.amount*2;

  return result;
}

