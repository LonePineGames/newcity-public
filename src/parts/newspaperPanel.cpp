#include "newspaperPanel.hpp"

#include "article.hpp"
#include "block.hpp"
#include "button.hpp"
#include "citipediaPanel.hpp"
#include "icon.hpp"
#include "label.hpp"
#include "leftPanel.hpp"
#include "messageBoard.hpp"
#include "panel.hpp"
#include "scrollbox.hpp"
#include "span.hpp"

#include "../color.hpp"
#include "../draw/camera.hpp"
#include "../icons.hpp"
#include "../money.hpp"
#include "../newspaper/article.hpp"
#include "../newspaper/newspaper.hpp"
#include "../option.hpp"
#include "../string_proxy.hpp"
#include "../string.hpp"
#include "../util.hpp"
#include "../weather.hpp"

#include "spdlog/spdlog.h"

static ScrollState newspaperScroll;
static item currentIssue = 0;

const float npHeight = 19;
const float npWidth = 36;
const float npPad = 0.25;
const float npScale = 0.75;
const float npTitleScale = 1.5;
static const float subScl = 0.75;
static const Line npGrad = line(colorNewsGrad0, colorNewsGrad1);
static const Line npDarkGrad = line(colorNewsDarkGrad0, colorNewsDarkGrad1);

struct Column {
  vec2 loc;
  float width;
};

bool closeNewspaperPanel(Part* part, InputEvent event) {
  setLeftPanel(NoPanel);
  return true;
}

bool shiftIssue(Part* part, InputEvent event) {
  currentIssue = iclamp(currentIssue + part->itemData, 0,
      getNumNewspaperIssues()-1);
  return true;
}

bool openLatestNewspaper(Part* part, InputEvent event) {
  setLeftPanel(NewspaperPanel);
  currentIssue = getNumNewspaperIssues()-1;
  return true;
}

bool toggleFinNewsMessage(Part* part, InputEvent event) {
  toggleMessage(FinNewsMessage, 0);
  return true;
}

Part* newspaperArticle(Column* column, NewspaperIssueArticle* article, float date) {
  bool ninetyThree = date >= (1993-c(CStartYear))*oneYear;

  Part* result = panel(column->loc, vec2(column->width, 1));
  result->flags &= ~_partTextShadow;
  result->renderMode = RenderTransparent;

  vec2 titleEnd(0,0);
  Part* titlePart = r(result, spanCenter(vec2(0,0.5f), 0,
      vec2(column->width, npTitleScale),
      strdup_s(article->title), &titleEnd));

  ArticleRenderConfig config;
  config.flags = _articleTraditionalParagraphs;
  if (!ninetyThree || !c(CNewspaperTextured)) config.flags |= _articleDesaturateCharts;
  if (!ninetyThree) config.flags |= _articleDesaturateImages;
  config.textColor = PickerPalette::Black;
  config.captionTextColor = PickerPalette::GrayLight;
  config.linkTextColor = PickerPalette::Black;
  config.captionBackground = npDarkGrad;
  config.filename = article->code;
  config.linkHandler = followLink;
  config.chartDate = date;

  Part* art = r(result, articleText(vec2(0,titleEnd.y),
      vec2(column->width, npScale),
      article->text, config));
  art->renderMode = RenderTransparent;

  float y = art->dim.end.y + titleEnd.y - 0.5f;
  result->dim.end = vec3(column->width, y, 0);
  column->loc.y += y;

  return result;
}

Part* newspaperAdvertisement(Column* column, item issue, item adNum, float targetSize) {
  const float pd = 0.05;
  float date = getNewspaperIssue(issue)->date;
  bool ninetyThree = date >= (1993-c(CStartYear))*oneYear;
  string text = getNewspaperAdvertisement(issue, adNum);
  Part* result = panel(column->loc+vec2(0,0.85f), vec2(column->width, 1));
  result->flags &= ~_partTextShadow;
  result->renderMode = RenderTransparent;

  ArticleRenderConfig config;
  config.flags = _articleCenter;
  if (!ninetyThree || !c(CNewspaperTextured)) config.flags |= _articleDesaturateCharts;

  if (!ninetyThree) {
    config.flags |= _articleDesaturateImages;
    config.textColor = PickerPalette::Black;
    config.captionTextColor = PickerPalette::GrayLight;
  } else {
    config.textColor = PickerPalette::RedMedL;
    config.captionTextColor = PickerPalette::RedMedL;
  }

  config.linkTextColor = PickerPalette::Black;
  config.captionBackground = npDarkGrad;
  config.filename = "Ad";
  config.linkHandler = followLink;
  config.chartDate = date;

  Part* art = r(result, articleText(vec2(pd*2, pd*2),
      vec2(column->width-pd*4, npScale),
      strdup_s(text.c_str()), config));
  // SPDLOG_INFO("DEBUG: Ad text {}", text.c_str());
  art->renderMode = RenderTransparent;

  float y = art->dim.end.y + pd*4 + 0.2;
  if (y < targetSize - 0.5) {
    if (targetSize > y + 4) targetSize = y + 4;
    art->dim.start.y += (targetSize - y)*.5f;
    y = targetSize;
  }

  result->dim.end = vec3(column->width, y, 0);
  column->loc.y += y + 1;

  // box
  float bsx = column->width;
  float bsy = y-pd;
  r(result, colorBlock(vec2(0,0), vec2(bsx, pd), config.textColor));
  r(result, colorBlock(vec2(0,0), vec2(pd, bsy), config.textColor));
  r(result, colorBlock(vec2(0,bsy), vec2(bsx, pd), config.textColor));
  r(result, colorBlock(vec2(bsx-pd,0), vec2(pd, bsy), config.textColor));

  return result;
}

Part* newspaperWidget(vec2 bottomRight) {
  vec2 size(3, 1);
//  vec2 loc = bottomRight - size - vec2(0, 0.5);
  vec2 loc = vec2(npWidth-size.x,-size.y-0.25f);
  Part* widget = panel(loc, size);
  widget->flags &= ~_partTextShadow;
  widget->renderMode = RenderPanelGradient;
  widget->texture = npGrad;
  widget->foregroundColor = PickerPalette::Black;

  r(widget, button(vec2(2,0), iconX, vec2(1,1),
        closeNewspaperPanel, 0));

  Part* backButt = r(widget, button(vec2(0,0), iconLeft, vec2(1,1),
        shiftIssue, -1));
  if (currentIssue <= 0) backButt->foregroundColor = PickerPalette::GrayLight;

  item numIssues = getNumNewspaperIssues();
  Part* foreButt = r(widget, button(vec2(1,0), iconRight, vec2(1,1),
        shiftIssue, 1));
  if (currentIssue >= numIssues-1) {
    foreButt->foregroundColor = PickerPalette::GrayLight;
  }

  return widget;
}

bool articleComparator (item a, item b) {
  NewspaperIssue* issue = getNewspaperIssue(currentIssue);
  NewspaperIssueArticle* articleA = issue->articles.get(a);
  NewspaperIssueArticle* articleB = issue->articles.get(b);
  return strlen(articleA->text) > strlen(articleB->text);
}

vector<item> getArticleOrder() {
  vector<item> articleOrder;
  NewspaperIssue* issue = getNewspaperIssue(currentIssue);
  item numArticles = issue->articles.size();
  for (int i = 0; i < numArticles; i++) {
    articleOrder.push_back(i);
  }
  std::sort (articleOrder.begin(), articleOrder.end(), articleComparator);
  return articleOrder;
}


Part* newspaperPanel() {
  vec2 npSize = vec2(npWidth, npHeight);
  float uiX = uiGridSizeX * getAspectRatio();
  float npX = uiX - npWidth - messageWidthOuter - .25f;
  vec2 npLoc = vec2(npX, 1.5f);
  Part* result = panel(npLoc, npSize);
  if (c(CNewspaperTextured)) result->flags |= _partTextured;
  result->flags &= ~_partTextShadow;
  result->padding = 0;
  result->renderMode = RenderPanelGradient;
  //result->renderMode = RenderTransparent;
  result->texture = npGrad;
  result->foregroundColor = PickerPalette::Black;
  float scrollbarSpacing = .6f;
  float innerWidth = npWidth-npPad*2;
  float dividerWidth = 0.05;

  r(result, newspaperWidget(vec2(0, npWidth)));

  NewspaperIssue* issue = getNewspaperIssue(currentIssue);
  bool ninetyThree = issue->date >= (1993-c(CStartYear))*oneYear;
  item numIssues = getNumNewspaperIssues();
  currentIssue = iclamp(currentIssue, 0, numIssues-1);
  if (numIssues <= 0) return result;

  Part* scroll = scrollbox(vec2(0,0), vec2(npWidth, npHeight));
  float y = 0;

  string name = getNewspaperName();
  float nameWidth = stringWidth(name.c_str());
  float nameSpacing = (innerWidth)/nameWidth;
  Part* namePart = r(scroll, labelCenter(vec2(scrollbarSpacing,0),
        vec2(innerWidth, nameSpacing),
        strdup_s(name.c_str())));
  y += nameSpacing + 0.25f;

  r(scroll, blackBlock(vec2(scrollbarSpacing, y),
        vec2(innerWidth, dividerWidth)));
  y += 0.1 + dividerWidth;

  r(scroll, label(vec2(scrollbarSpacing, y), subScl,
        printMoneyString(issue->price)));
  r(scroll, labelCenter(vec2(scrollbarSpacing, y), vec2(innerWidth, subScl),
        printDateString(issue->date)));
  float tempX = scrollbarSpacing+innerWidth-1 - (issue->weather != WeatherNull);
  r(scroll, labelRight(vec2(tempX, y), vec2(1, subScl),
        useMetric() ? sprintf_o("%2dC", int(issue->temp)) :
        sprintf_o("%2dF", int(issue->temp*1.8 + 32))));
  r(scroll, icon(vec2(scrollbarSpacing+innerWidth-subScl, y),
        vec2(subScl, subScl), getWeatherIconVec3(issue->weather)));
  y += subScl;

  y += 0.1;
  r(scroll, blackBlock(vec2(scrollbarSpacing, y),
        vec2(innerWidth, dividerWidth)));
  y += 0.1 + dividerWidth;

  // Setup Columns
  float colStartY = y;
  vector<Column> columns;
  float colPad = 0.25;
  float columnSize = (innerWidth-colPad*3)*.25;
  columns.push_back(Column { vec2(scrollbarSpacing, y), columnSize });
  columns.push_back(Column { vec2(scrollbarSpacing+columnSize+colPad, y),
      columnSize*2+colPad });
  columns.push_back(Column { vec2(scrollbarSpacing+(columnSize+colPad)*3, y),
      columnSize });

  // Sports
  item numSports = issue->sportsScores.size();
  vec2 scoreSize(columns[0].width,npScale);
  vec2 scoreMiddle(columns[0].width*.6f,npScale);
  r(scroll, labelCenter(columns[0].loc, scoreSize, strdup_s("Sports")));
  columns[0].loc.y += npScale;
  for (int i = 0; i < numSports; i++) {
    SportsScore score = issue->sportsScores[i];
    for (int t = 0; t < 2; t ++) {
      r(scroll, label(columns[0].loc, scoreSize,
            strdup_s(score.teamNames[t])));
      r(scroll, labelRight(columns[0].loc, scoreSize,
            sprintf_o("%d", score.scores[t])));
      columns[0].loc.y += npScale;
    }
    r(scroll, blackBlock(columns[0].loc+vec2(1,0),
          vec2(columns[0].width-2, dividerWidth)));
    columns[0].loc.y += 0.1f + dividerWidth;
  }
  r(scroll, labelCenter(columns[0].loc, scoreSize,
        strdup_s("More Sports News on Page 4")));
  columns[0].loc.y += npScale;
  r(scroll, blackBlock(columns[0].loc, vec2(columns[0].width, dividerWidth)));
  columns[0].loc.y += 0.1f + dividerWidth;

  // Financial
  item numFin = issue->financialScores.size();
  NewspaperIssue* prevIssue = 0;
  item numPrevFin = 0;
  if (currentIssue > 0) {
    prevIssue = getNewspaperIssue(currentIssue-1);
    numPrevFin = prevIssue->financialScores.size();
  }
  r(scroll, labelCenter(columns[0].loc, scoreSize, strdup_s("Financial")));
  r(scroll, button(columns[0].loc+vec2(scoreSize.x-1,0), iconPin, vec2(1,1),
        toggleFinNewsMessage, 0));
  columns[0].loc.y += npScale;
  for (int i = 0; i < numFin; i++) {
    FinancialScore score = issue->financialScores[i];
    r(scroll, label(columns[0].loc, scoreSize,
          strdup_s(score.statName)));
    if (abs(score.score) < 100) {
      r(scroll, labelRight(columns[0].loc, scoreSize,
          sprintf_o("%2.3f", score.score)));
    } else {
      r(scroll, labelRight(columns[0].loc, scoreSize,
          sprintf_o("%d", int(score.score))));
    }

    for (int j = 0; j < numPrevFin; j++) {
      FinancialScore prevScore = prevIssue->financialScores[j];
      if (!streql(prevScore.statName, score.statName)) continue;
      float diff = score.score - prevScore.score;
      r(scroll, labelRight(columns[0].loc, scoreMiddle,
            sprintf_o("%.2f", diff)));

      Part* ico = r(scroll, icon(columns[0].loc+vec2(columns[0].width*.65f,0),
            vec2(npScale, npScale), diff < 0 ? iconDown : iconUp));
      if (ninetyThree) {
        ico->foregroundColor = diff < 0 ? RedLight : GreenMedL;
      }

      break;
    }

    columns[0].loc.y += npScale;
  }
  r(scroll, labelCenter(columns[0].loc, scoreSize,
        strdup_s("More Financial News on Page 7")));
  columns[0].loc.y += npScale;
  r(scroll, blackBlock(columns[0].loc, vec2(columns[0].width, dividerWidth)));
  columns[0].loc.y += 0.1f + dividerWidth;

  // Articles
  item numArticles = issue->articles.size();
  vector<item> articleOrder = getArticleOrder();
  int numArticlesInCol[3] = {0,0,0};
  for (int i = 0; i < numArticles; i++) {

    item bestCol = 0;
    float bestColSpace = 0;
    for (int colNum = 0; colNum < columns.size(); colNum ++) {
      Column col = columns[colNum];
      float yDiff = y - col.loc.y;
      float space = (yDiff+1) * col.width;
      if (space > bestColSpace) {
        bestCol = colNum;
        bestColSpace = space;
      }
    }

    if (numArticlesInCol[bestCol] > 0) {
      columns[bestCol].loc.y += 0.2;
      r(scroll, blackBlock(columns[bestCol].loc, vec2(columns[bestCol].width, dividerWidth)));
    }

    item articleNdx = articleOrder[i];
    NewspaperIssueArticle* article = issue->articles.get(articleNdx);
    Part* art = r(scroll, newspaperArticle(&columns.data()[bestCol], article, issue->date));
    y = std::max(y, columns[bestCol].loc.y);
    numArticlesInCol[bestCol] ++;
  }

  float dividerXOff = (colPad+dividerWidth)*.5f;
  for (int i = 1; i < columns.size(); i++) {
    Column col1 = columns[i];
    Column col0 = columns[i-1];
    float colEndy = std::min(col1.loc.y, col0.loc.y);
    r(scroll, blackBlock(vec2(col1.loc.x-dividerXOff, colStartY),
          vec2(dividerWidth, colEndy - colStartY - 0.5)));
  }

  if (numAdArticles() > 0) {
    if (y < c(CNewspaperMinLength)) y = c(CNewspaperMinLength);
    int adNum = 0;
    Part* lastAd[3] = {0,0,0};
    for (int i = 0; i < columns.size(); i++) {
      Column* col = &columns.data()[i];
      int colAdNum = 0;

      while (col->loc.y < y - 0.1) {
        adNum ++;
        colAdNum ++;
        Part* ad = newspaperAdvertisement(col, currentIssue, adNum, y - col->loc.y);
        if (col->loc.y > y+1) {
          freePart(ad);
          if (colAdNum > 20) break;
        } else {
          r(scroll, ad);
          lastAd[i] = ad;
        }
      }
    }

    /*
    y = 0;
    for (int i = 0; i < columns.size(); i++) {
      y = std::max(y, columns[i].loc.y);
    }
    */

    for (int i = 0; i < columns.size(); i++) {
      Part* ad = lastAd[i];
      if (ad != 0) {
        ad->dim.start.y = y - ad->dim.end.y;
      }
    }
  }

  Part* scrollFrame = scrollboxFrame(vec2(-scrollbarSpacing, 0),
      vec2(npWidth+scrollbarSpacing, npHeight),
      &newspaperScroll, scroll, true);
  r(result, scrollFrame);

  Part* slider = newspaperScroll.slider;
  if (slider != 0) {
    slider->texture = npDarkGrad;
    slider->renderMode = RenderGradient;
  }

  result->flags &= ~_partClip;
  result->dim.end.z = newspaperScroll.amount;
  scrollFrame->flags &= ~_partLowered;
  scrollFrame->renderMode = RenderTransparent;
  scroll->renderMode = RenderTransparent;

  return result;
}

