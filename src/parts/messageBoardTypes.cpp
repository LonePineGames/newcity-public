#include "../amenity.hpp"
#include "../business.hpp"
#include "../building/building.hpp"
#include "../building/design.hpp"
#include "../economy.hpp"
#include "../game/achievement.hpp"
#include "../game/feature.hpp"
#include "../graph.hpp"
#include "../graph/transit.hpp"
#include "../money.hpp"
#include "../newspaper/newspaper.hpp"
#include "../person.hpp"
#include "../time.hpp"
#include "../tools/building.hpp"
#include "../zone.hpp"

#include "../parts/block.hpp"
#include "../parts/chart.hpp"
#include "../parts/economyPanel.hpp"
#include "../parts/hr.hpp"
#include "../parts/icon.hpp"
#include "../parts/label.hpp"
#include "../parts/leftPanel.hpp"
#include "../parts/messageBoard.hpp"
#include "../parts/newspaperPanel.hpp"
#include "../parts/selectionPanel.hpp"
#include "../parts/span.hpp"
#include "../parts/transitPanel.hpp"
#include "../parts/toolbar.hpp"

#include <string.h>

const char* const infoMessageText[numInfoMessages] = {
  //LogUploadNotification
  "NewCity sends some data "
  "to a server. This can be "
  "disabled in the game options."
};

void chartMessage(Part* result, Message message) {
  bool minimized = message.object < 0;
  item obj = abs(message.object);
  item stat = obj / 10;
  item timePeriod = obj % 10;
  float ySize = minimized ? 1.3 : 4;
  const float subScl = 0.6f;

  Part* chrt = chart(vec2(0,0), vec2(messageWidth+messagePadding*2, ySize),
      message.data, (Statistic)stat, timePeriod, true, minimized);
  r(result, chrt);

  if (message.data != ourCityEconNdx()) {
    Part* econLbl = r(chrt, labelRight(
          vec2(0, ySize-subScl),
          vec2(messageWidth+messagePadding, subScl),
          strdup_s(getEconName(message.data))));
  }

  result->padding = 0;
  result->onClick = selectChart;
  result->itemData = obj;
  result->dim.end.y = chrt->dim.end.y;
}

void infoMessage(Part* result, Message message) {
  const char* info = infoMessageText[message.object];
  float ySize = 0;
  r(result, multiline(vec2(0,0), vec2(messageWidth-1, msgScl),
        strdup_s(info), &ySize));

  result->dim.end.y = ySize;
}

void newspaperMessage(Part* result, Message message) {
  result->flags &= ~_partTextShadow;
  if (c(CNewspaperTextured)) result->flags |= _partTextured;
  result->flags |= _partHover;
  result->foregroundColor = PickerPalette::Black;
  result->renderMode = RenderPanelGradient;
  result->texture = line(colorNewsGrad0, colorNewsGrad1);
  result->onClick = openLatestNewspaper;
  float ySize = 0;
  float y = 0;

  r(result, spanCenter(vec2(0,y), 0, vec2(messageWidth, msgScl),
        strdup_s(getNewspaperNameShort().c_str()), &y));

  r(result, blackBlock(vec2(0,msgScl), vec2(messageWidth,0.1f)));
  y += 0.2f;

  NewspaperIssue* issue = getLatestNewspaperIssue();
  for (int i = 0; i < issue->articles.size(); i++ ) {
    if (i > 0) {
      r(result, blackBlock(vec2(1,y-0.05f), vec2(messageWidth-2,0.02f)));
    }

    r(result, spanCenter(vec2(0,y), 0, vec2(messageWidth, 0.6f),
          strdup_s(issue->articles[i].title), &y));
  }

  result->dim.end.y = y + result->padding;
}

void finNewsMessage(Part* result, Message message) {
  result->flags &= ~_partTextShadow;
  if (c(CNewspaperTextured)) result->flags |= _partTextured;
  result->foregroundColor = PickerPalette::Black;
  result->renderMode = RenderPanelGradient;
  result->texture = line(colorNewsGrad0, colorNewsGrad1);
  result->onClick = openLatestNewspaper;
  result->flags |= _partHover;
  float ySize = 0;
  float y = 0;
  float dividerWidth = 0.05;
  vec2 scoreSize = vec2(result->dim.end.x, 0.7) -
    vec2(result->padding*2, 0);
  vec2 scoreMiddle(scoreSize.x*.6f,msgScl);

  r(result, labelCenter(vec2(0,y), scoreSize, strdup_s("Financial")));
  r(result, blackBlock(vec2(0,msgScl), vec2(messageWidth,0.1f)));
  y += 0.2f;

  NewspaperIssue* issue = getLatestNewspaperIssue();
  item currentIssue = getNumNewspaperIssues()-1;

  // Financial
  item numFin = issue->financialScores.size();
  NewspaperIssue* prevIssue = 0;
  item numPrevFin = 0;
  if (currentIssue > 0) {
    prevIssue = getNewspaperIssue(currentIssue-1);
    numPrevFin = prevIssue->financialScores.size();
  }
  y += scoreSize.y;
  for (int i = 0; i < numFin; i++) {
    FinancialScore score = issue->financialScores[i];
    r(result, label(vec2(0,y), scoreSize,
          strdup_s(score.statName)));

    if (abs(score.score) < 100) {
      r(result, labelRight(vec2(0,y), scoreSize,
          sprintf_o("%2.3f", score.score)));
    } else {
      r(result, labelRight(vec2(0,y), scoreSize,
          sprintf_o("%d", int(score.score))));
    }

    for (int j = 0; j < numPrevFin; j++) {
      FinancialScore prevScore = prevIssue->financialScores[j];
      if (!streql(prevScore.statName, score.statName)) continue;
      float diff = score.score - prevScore.score;
      r(result, labelRight(vec2(0,y), scoreMiddle,
            sprintf_o("%.2f", diff)));

      Part* ico = r(result, icon(vec2(scoreSize.x*.65f,y),
            vec2(scoreSize.y, scoreSize.y), diff < 0 ? iconDown : iconUp));
      if (getCurrentYear() >= 1993) {
        ico->foregroundColor = diff < 0 ? RedLight : GreenMedL;
      }

      break;
    }

    y += scoreSize.y;
  }

  // Articles
  result->dim.end.y = y + result->padding;
}

void budgetMessage(Part* result, Message message) {
  BudgetLine l = (BudgetLine) message.object;
  if (l > BudgetBalance) {
    Budget b0 = getBudget(0);
    float amount0 = b0.line[BudgetBalance] + b0.line[LineOfCredit];
    Budget b1 = getBudget(1);
    float amount1 = b1.line[BudgetBalance] + b1.line[LineOfCredit];
    r(result, icon(vec2(0,0), vec2(msgScl, msgScl), iconCash));
    r(result, label(vec2(msgScl,0), msgScl,
          sprintf_o("%d Cash to Spend", getCurrentYear())));
    r(result, label(vec2(-0.25f,msgScl), msgScl,
          printPaddedMoneyString(amount0), amount0 < 0));
    r(result, label(vec2(2.5f,msgScl), msgScl, strdup_s("YTD"), amount0 < 0));
    r(result, label(vec2(3.75f,msgScl), msgScl,
          printPaddedMoneyString(amount1), amount1 < 0));
    r(result, label(vec2(6.5f,msgScl), msgScl, strdup_s("Est."), amount1 < 0));

  } else {
    r(result, icon(vec2(0,0), vec2(msgScl, msgScl), iconCash));
    r(result, label(vec2(msgScl,0), msgScl,
          sprintf_o("%d %s", getCurrentYear(), getBudgetLineName(l))));
    float amount0 = getBudget(0).line[l];
    float amount1 = getBudget(1).line[l];
    r(result, label(vec2(-0.25f,msgScl), msgScl,
          printPaddedMoneyString(amount0), amount0 < 0));
    r(result, label(vec2(2.5f,msgScl), msgScl, strdup_s("YTD"), amount0 < 0));
    r(result, label(vec2(3.75f,msgScl), msgScl,
          printPaddedMoneyString(amount1), amount1 < 0));
    r(result, label(vec2(6.5f,msgScl), msgScl, strdup_s("Est."), amount1 < 0));
  }

  if (c(CNewspaperTextured)) result->flags |= _partTextured;
  result->flags &= ~_partTextShadow;
  result->renderMode = RenderPanelGradient;

  vec3 white = getColorInPalette(PickerPalette::GrayLight);
  Line bpGrad = line(white, white);
  result->texture = bpGrad;
  result->foregroundColor = PickerPalette::Black;

  result->dim.end.y = 2*msgScl;
}

bool selectAchievement(Part* part, InputEvent event) {
  item amenity = getAmenityForAchievement(part->vecData.x);
  if (amenity > 0) {
    Design* d = getDesign(amenity);
    if (!(d->flags & _designSingleton) || getGovBuildingsPlaced(amenity) <= 0) {
      setTool(4);
      setAmenityInTool(amenity);
    }
  }

  if (part->itemData > 0) {
    selectBuilding(part, event);
  }
  return true;
}

void achievementMessage(Part* result, Message message) {
  result->renderMode = RenderPanelGradient;
  result->texture = line(colorGoldGrad1, colorGoldGrad0);
  result->flags |= _partTextShadow;

  if (message.object <= 0 || message.object > getNumAchievements()) return;

  Achievement ach = getAchievement(message.object);
  const float subScl = 0.6f;
  const float innerWidth = messageWidth - messagePadding*2.f;
  float y = 0;

  if (ach.name != 0 && strlen(ach.name) > 0) {
    r(result, label(vec2(0,0), msgScl, strdup_s(ach.name)));
    r(result, hr(vec2(0,msgScl), messageWidth));
    y += msgScl + messagePadding + 0.2f;
  }

  if (message.data != 0 || getAmenityForAchievement(message.object)) {
    result->flags |= _partHover;
    result->onClick = selectAchievement;
    result->itemData = message.data;
    result->vecData.x = message.object;
  }

  if (ach.text != 0 && strlen(ach.text) > 0) {
    float ySize = 0;
    r(result, span(vec2(messagePadding,y), messagePadding,
          vec2(innerWidth, subScl), strdup_s(ach.text), &y));
    y += subScl;
  }

  if (ach.effectText != 0 && strlen(ach.effectText) > 0) {
    y += subScl;
    r(result, span(vec2(messagePadding,y), messagePadding,
          vec2(innerWidth, subScl), strdup_s(ach.effectText), &y));
    y += subScl;
  }

  result->dim.end.y = y;// + messagePadding;
}

void personMessage(Part* result, Message message) {
  result->onClick = selectPerson;
  Person* p = getPerson(message.object);
  Family* family = getFamily(p->family);

  r(result, icon(vec2(0,0), vec2(msgScl, msgScl), getPersonIcon(p)));
  r(result, label(vec2(msgScl,0), msgScl,
    sprintf_o("%s %s", p->name, family->name)));

  // Where are they?
  if (p->flags & _personTraveling) {
    r(result, icon(vec2(0,msgScl), vec2(msgScl, msgScl), iconCar));

  } else if (p->location == 0) {
    r(result, icon(vec2(0,msgScl), vec2(msgScl, msgScl), iconWait));

  } else {
    Building* building = getBuilding(p->location);
    vec3 ico = iconZoneMono[building->zone];
    r(result, icon(vec2(0,msgScl), vec2(msgScl, msgScl), ico));
  }

  // What are they doing?
  r(result, label(vec2(msgScl,msgScl), msgScl,
        getActivityName(message.object)));

  result->dim.end.y = 2*msgScl;
}

void businessMessage(Part* result, Message message) {
  result->onClick = selectBusiness;
  Business* b = getBusiness(message.object);
  r(result, icon(vec2(0,0), vec2(msgScl, msgScl), iconBusiness));
  r(result, label(vec2(msgScl,0), msgScl, strdup_s(b->name)));
}

void govBuildingMessage(Part* result, Building* b, Message message) {
  Design* d = getDesign(b->design);

  vec3 ico = iconZoneMono[b->zone];
  r(result, icon(vec2(0,0), vec2(msgScl, msgScl), ico));
  r(result, label(vec2(msgScl,0), msgScl, strdup_s(d->displayName)));
  if (b->flags & _buildingComplete) {
    r(result, icon(vec2(0,msgScl), vec2(msgScl, msgScl), iconPersonMan));
    r(result, label(vec2(msgScl,msgScl), msgScl,
          sprintf_o("%d inside", b->peopleInside.size())));

    bool enabled = b->flags & _buildingEnabled;
    if (enabled) {
      float maintenance = getMaintenance(message.object);
      char* maint = printMoneyString(maintenance);
      r(result, label(vec2(messageWidth-msgScl*5, msgScl), msgScl,
            sprintf_o("%s/year", maint)));
      free(maint);
    } else {
      r(result, label(vec2(messageWidth-msgScl*5, msgScl), msgScl,
            strdup_s("Shutdown")));
    }

    if (isFeatureEnabled(FShutDownBuilding)) {
      r(result, button(vec2(messageWidth-msgScl*6,msgScl),
          enabled ? iconOpen : iconClosed,
          vec2(msgScl,msgScl), toggleEnableGovBuilding, 0));
    }

  } else {
    r(result, label(vec2(0, msgScl), msgScl, strdup_s("Planned")));
  }

  result->dim.end.y = 2*msgScl;
}

void buildingMessage(Part* result, Message message) {
  result->onClick = selectBuilding;
  Building* b = getBuilding(message.object);
  if (b->zone == GovernmentZone) {
    govBuildingMessage(result, b, message);

  } else {
    vec3 ico = iconZoneMono[b->zone];
    r(result, icon(vec2(0,0), vec2(msgScl, msgScl), ico));
    r(result, label(vec2(msgScl,0), msgScl,
          sprintf_o("%d people inside", b->peopleInside.size())));

    char* taxes = printMoneyString(getBuildingTaxes(message.object));
    r(result, label(vec2(0,msgScl), msgScl, sprintf_o("%s/year", taxes)));
    free(taxes);
    result->dim.end.y = 2*msgScl;
  }
}

void graphMessage(Part* result, Message message) {
  result->onClick = selectGraphElement;
  Configuration config = getElementConfiguration(message.object);
  vec3 ico = config.type == ConfigTypeExpressway ? iconExpressway : iconRoad;

  if (message.object > 0) {
    Edge* e = getEdge(message.object);
    float speedLimit = speedLimits[e->config.speedLimit];
    float safeSpeed = getLaneBlock(e->laneBlocks[0])->speedLimit;
    bool slowed = speedLimit > safeSpeed;

    r(result, icon(vec2(messageWidth-msgScl*6,msgScl),
          vec2(msgScl,msgScl), iconWrench));
    r(result, label(vec2(messageWidth-msgScl*5,msgScl), msgScl,
      sprintf_o(slowed ? "%2d%%!" : "%2d%%", (int)(e->wear*100))));
  }

  r(result, icon(vec2(0,0), vec2(msgScl, msgScl), ico));
  r(result, label(vec2(msgScl,0), msgScl,
        strdup_s(graphElementName(message.object))));

  int cars = numVehiclesInElement(message.object);
  r(result, icon(vec2(0,msgScl), vec2(msgScl, msgScl), iconCar));
  r(result, label(vec2(msgScl,msgScl), msgScl,
        sprintf_o("%d", cars)));

  result->dim.end.y = msgScl*2;
}

void vehicleMessage(Part* result, Message message) {
  result->onClick = selectVehicle;
  Vehicle* v = getVehicle(message.object);
  r(result, icon(vec2(0,0), vec2(msgScl, msgScl), iconCar));

  r(result, label(vec2(msgScl,0), msgScl,
    printSpeedString("", length(v->velocity), "")));

  if (v->laneLoc.lane != 0) {
    LaneBlock* block = getLaneBlock(v->laneLoc);
    item graphNdx = block->graphElements[1];
    Configuration config = getElementConfiguration(graphNdx);
    r(result, icon(vec2(0,msgScl), vec2(msgScl,msgScl),
         config.type == ConfigTypeExpressway ? iconExpressway : iconRoad));
    r(result, label(vec2(msgScl,msgScl), msgScl, graphElementName(graphNdx)));
  }

  result->dim.end.y = 2*msgScl;
}

void amenityEffectMessage(Part* result, Message message) {
  int val = getEffectValue(message.object);
  float y = 0;

  r(result, icon(vec2(0, y), vec2(msgScl, msgScl), getEffectIcon(message.object)));
  r(result, labelRight(vec2(msgScl, y), vec2(msgScl*2, msgScl),
      sprintf_o("%s%d", val > 0 ? "+":"", val)));
  r(result, label(vec2(msgScl*3, y), msgScl, strdup_s(getEffectString(message.object))));
  y += msgScl;

  r(result, multiline(vec2(0, y),
      vec2(result->dim.end.x - result->padding*2.f, msgScl),
      getMacroEffectDescriptor(message.object), &y));
  result->dim.end.y = y + result->padding; //+msgScl;
}

void transitDesignMessage(Part* result, Message message) {
  r(result, icon(vec2(0, 0), vec2(msgScl, msgScl), iconBus));
  r(result, label(vec2(msgScl, 0), msgScl, strdup_s("Design Complete!")));
  r(result, hr(vec2(0,msgScl), messageWidth));
  float y = 0;
  TransitSystem* system = getTransitSystem(message.object);
  char* messageText = sprintf_o("%s has completed it's design for %s."
      " Click here to see the results and accept the design.",
      system->bids[0].firmName, system->name);
  r(result, multiline(vec2(0, msgScl+messagePadding),
      vec2(result->dim.end.x - result->padding*2.f, msgScl), messageText, &y));
  result->dim.end.y = y+msgScl;

  result->onClick = setCurrentTransitSystem;
  result->itemData = message.object;
  result->flags |= _partHover;
}

void zoneDemandMessage(Part* result, Message message) {
  r(result, icon(vec2(0, 0), vec2(msgScl, msgScl), iconPin));
  r(result, label(vec2(msgScl, 0), msgScl,
    strdup_s("Zone Demand")));

  float xPos = 0.5f;
  float padding = 0.1f;
  float ySize = 3.25f - padding;

  Part* subPanel = r(result, panel(vec2(0.f, 0.75f),
        vec2(result->dim.end.x-result->padding*2.f, ySize+padding)));
  subPanel->flags |= _partLowered;

  for(int i = 0; i < numZoneTypes; i++)
  {
    int zone = zoneOrder[i];

    if(zone == GovernmentZone || zone == NoZone || zone == ParkZone)
      continue;

    // Only show unlocked zone types
    if(i != 0 && !isFeatureEnabled(FZoneResidential+zone-1)) {
      // If a zone type is not enabled, leave a blank spot for it
      xPos += 1.0f;
      continue;
    }

    float demand = zoneDemandSoft(zone);
    if (demand > 0) demand = pow(demand, 0.5);

    r(subPanel, icon(vec2(xPos, ySize-1),
      vec2(1, 1), iconZone[zone]));
    r(subPanel, icon(vec2(xPos, ySize-1),
      vec2(1, -(ySize-1.4f)*demand), iconZoneColor[zone]));

    xPos += 1.0f;
  }

  r(subPanel, label(vec2(xPos, 0.f), 0.75f, strdup_s("-Hi")));
  r(subPanel, label(vec2(xPos, ySize - 1.4f), 0.75f, strdup_s("-Lo")));

  result->dim.end.y = 4;
}

