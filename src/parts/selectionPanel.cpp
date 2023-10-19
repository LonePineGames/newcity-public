#include "selectionPanel.hpp"

#include "../amenity.hpp"
#include "../building/building.hpp"
#include "../building/design.hpp"
#include "../business.hpp"
#include "../draw/entity.hpp"
#include "../draw/texture.hpp"
#include "../game/constants.hpp"
#include "../game/feature.hpp"
#include "../game/game.hpp"
#include "../graph/stop.hpp"
#include "../graph/transit.hpp"
#include "../heatmap.hpp"
#include "../icons.hpp"
#include "../import/mesh-import.hpp"
#include "../intersection.hpp"
#include "../lot.hpp"
#include "../option.hpp"
#include "../person.hpp"
#include "../platform/file.hpp"
#include "../platform/lookup.hpp"
#include "../tools/query.hpp"
#include "../selection.hpp"
#include "../string.hpp"
#include "../string_proxy.hpp"
#include "../time.hpp"
#include "../vehicle/model.hpp"
#include "../vehicle/travelGroup.hpp"
#include "../vehicle/vehicle.hpp"
#include "../zone.hpp"

#include "blank.hpp"
#include "button.hpp"
#include "label.hpp"
#include "hr.hpp"
#include "icon.hpp"
#include "leftPanel.hpp"
#include "messageBoard.hpp"
#include "panel.hpp"
#include "scrollbox.hpp"
#include "slider.hpp"
#include "textBox.hpp"

#include "spdlog/spdlog.h"
#include <math.h>

const float scl = .8f;
const float sclPad = .85f;
const float startY = 1.5f;
float spPadding = 0.25;
const vec2 selectionPanelLoc = vec2(0.1, 1.5);
const float selectionScrollMaxDif = 20.0f;
const float spWidth = 14;
const float spHeight = 18.5;
const float scrollBoxHeight = 8;
const vec2 selectionPanelSize = vec2(spWidth+spPadding*2,
  spHeight + spPadding*2);
const float scrollBoxTop = spHeight - scrollBoxHeight;
static ScrollState selectionScroll;
static TextBoxState selectionNameState;

void clearSelectionPanel() {
  selectionNameState.flags &= ~_textBoxEditing;
}

bool selectPerson(Part* part, InputEvent event) {
  setSelection(SelectionPerson, part->itemData);
  return true;
}

bool selectGraphElement(Part* part, InputEvent event) {
  setSelection(SelectionGraphElement, part->itemData);
  return true;
}

bool selectLaneBlock(Part* part, InputEvent event) {
  setSelection(SelectionLaneBlock, part->itemData);
  return true;
}

bool selectVehicle(Part* part, InputEvent event) {
  setSelection(SelectionVehicle, part->itemData);
  return true;
}

bool selectBuilding(Part* part, InputEvent event) {
  setSelection(SelectionBuilding, part->itemData);
  return true;
}

bool selectFamily(Part* part, InputEvent event) {
  setSelection(SelectionFamily, part->itemData);
  return true;
}

bool selectBusiness(Part* part, InputEvent event) {
  setSelection(SelectionBusiness, part->itemData);
  return true;
}

bool selectTransitLine(Part* part, InputEvent event) {
  TransitLine* tl = getTransitLine(part->itemData);
  item cursor = -1;
  for (int i = 0; i < tl->legs.size(); i++) {
    if (tl->legs[i].stop == getSelection()) {
      cursor = i+1;
      break;
    }
  }

  //clearSelection();
  setLeftPanel(TransitPanel);
  setCurrentLine(part->itemData);
  if (cursor >= 0) setTransitCursor(cursor);

  return true;
}

bool selectLocation(Part* part, InputEvent event) {
  Location loc = (Location)part->itemData;
  selectLocation(loc);
  return true;
}

bool clearSelection(Part* part, InputEvent event) {
  clearSelection();
  return true;
}

Part* graphElementButton(vec2 loc, item ndx) {
  if (ndx == 0) {
    return button(loc, iconRoad, vec2(spWidth-2,scl),
        strdup_s("(Null)"), 0, 0);
  } else {
    return button(loc, iconRoad, vec2(spWidth-2,scl), graphElementName(ndx),
      selectGraphElement, ndx);
  }
}

Part* graphElementButton(vec2 loc, GraphLocation graphLoc) {
  item ndx = 0;
  if (graphLoc.lane != 0) {
    LaneBlock* block = getLaneBlock(graphLoc);
    ndx = block->graphElements[1];
  }
  return graphElementButton(loc, ndx);
}

Part* laneBlockButton(vec2 loc, item ndx) {
  return button(loc, iconVerticalBar, vec2(spWidth-2,scl),
        sprintf_o("Lane Block #%d", ndx), selectLaneBlock, ndx);
}

Part* buildingButton(vec2 loc, char* str, item buildingNdx) {
  Building* building = getBuilding(buildingNdx);
  vec3 ico = iconNo;
  if (building->flags & _buildingExists) {
    ico = iconZoneMono[building->zone];
  }
  return button(loc, ico, vec2(spWidth-2, scl),
    str, selectBuilding, buildingNdx);
}

Part* personList(vec2 start, vector<item>* list) {
  Part* result = scrollbox(vec2(0,0), vec2(spWidth-1, scrollBoxHeight));
  vec2 buttSize = vec2(spWidth-1.5,0.75);

  for (int i=0; i < list->size(); i++) {
    item personNdx = list->at(i);
    Person* person = getPerson(personNdx);
    Part* butt = button(vec2(0,i), getPersonIcon(person), buttSize,
      getPersonDescriptor(personNdx), selectPerson, personNdx);
    butt->itemData = personNdx;
    r(result, butt);
  }

  return scrollboxFrame(start, vec2(spWidth, scrollBoxHeight),
    &selectionScroll, result);
}

Part* objectList(vec2 start, vec2 size, vector<item>* businesses,
  vector<item>* families, vector<item>* people
) {
  Part* result = scrollbox(vec2(0,0), size);
  float k = 0;
  vec2 buttSize = vec2(spWidth-.7,0.75);

  float scrollShowMin = selectionScroll.amount - selectionScrollMaxDif;
  float scrollShowMax = selectionScroll.amount + selectionScrollMaxDif;

  for (int i=0; i < businesses->size(); i++) {
    if(k >= scrollShowMin && k <= scrollShowMax) {
      item businessNdx = businesses->at(i);
      Business* business = getBusiness(businessNdx);
      BusinessType type = getBusinessType(businessNdx);
      vec3 icon = iconBusinessType[type];
      if (!(business->flags & _businessExists)) {
        icon = iconNo;
      }
      Part* butt = button(vec2(0, k), icon, buttSize,
        getBusinessDescriptor(businessNdx), selectBusiness, businessNdx);
      r(result, butt);
    }
    k++;
  }

  if (businesses->size() > 0 && (families->size() > 0 || people->size() > 0)) {
    r(result, hr(vec2(0.f, k), spWidth-1.5));
    k+=0.5f;
  }

  for (int i=0; i < families->size(); i++) {
    if(k >= scrollShowMin && k <= scrollShowMax) {
      item familyNdx = families->at(i);
      Family* family = getFamily(familyNdx);
      vec3 icon = iconFamily;
      if (!(family->flags & _familyExists)) {
        icon = iconNo;
      } else if (family->flags & _familyIsTourists) {
        icon = iconTourists;
      }
      Part* butt = button(vec2(0, k), icon, buttSize,
        getFamilyDescriptor(familyNdx),
        selectFamily, familyNdx);
      r(result, butt);
    }
    k++;
  }

  if (families->size() > 0 && people->size() > 0) {
    r(result, hr(vec2(0.f, k), spWidth-1.5));
    k+=0.5f;
  }

  for (int i=0; i < people->size(); i++) {
    if(k >= scrollShowMin && k <= scrollShowMax) {
      item personNdx = people->at(i);
      Person* person = getPerson(personNdx);
      Part* butt = button(vec2(0, k), getPersonIcon(person), buttSize,
        getPersonDescriptor(personNdx),
        selectPerson, personNdx);
      butt->itemData = personNdx;
      r(result, butt);
    }
    k++;
  }

  return scrollboxFrame(start, size, k, &selectionScroll, result);
}

bool deleteVehicle(Part* part, InputEvent event) {
  removeVehicle(part->itemData);
  return true;
}

void vehiclePanel(Part* result, Vehicle* selection) {
  float y = startY;

  // Header
  vec3 ico = !(selection->flags & _vehicleExists) ? iconNo :
    !(selection->flags & _vehiclePlaced) ? iconNull :
    !isVehicleActive_g(getSelection()) ? iconEye :
    selection->entity == 0 ? iconZone[NoZone] :
    iconCar;
  r(result, icon(vec2(0,0), ico));
  r(result, label(vec2(1,0), 1, strdup_s("Vehicle")));

  r(result, graphElementButton(vec2(0, y), selection->laneLoc));
  y += sclPad;

  if (selection->pilot.lane/10 != selection->laneLoc.lane/10) {
    r(result, graphElementButton(vec2(0,y), selection->pilot));
    y += sclPad;
  }

  if (selection->model > 0) {
    VehicleModel* model = getVehicleModel(selection->model);
    MeshImport* import = getMeshImport(model->meshImport);
    r(result, label(vec2(0,y), scl, sprintf_o("Model: %s", model->code)));
    y += sclPad;
    string objFileInMod = lookupFile(import->objFile, 0);
    r(result, label(vec2(0,y), 0.65, strdup_s(objFileInMod.c_str())));
    y += 0.7;
  }

  r(result, label(vec2(0,y), scl, printSpeedString("Traveling at ",
      3.5f*length(selection->velocity)/c(CVehicleSpeed), ".")));
  y += sclPad;

  if (selection->vehicleAhead[0] != 0) {
    r(result, button(vec2(0,y), iconCar, vec2(spWidth-2,scl),
      strdup_s("Vehicle Ahead"), selectVehicle, selection->vehicleAhead[0]));
    y += sclPad;
  }

  if (selection->vehicleAhead[1] != 0) {
    r(result, button(vec2(0,y), iconCar, vec2(spWidth-2,scl),
      strdup_s("Vehicle Ahead"), selectVehicle, selection->vehicleAhead[1]));
    y += sclPad;
  }

  if (selection->yieldTo != 0) {
    r(result, button(vec2(0,y), iconCar, vec2(spWidth-2,scl),
      strdup_s("Yielding To"), selectVehicle, selection->yieldTo));
    y += sclPad;
  }

  if (selection->yieldFrom != 0) {
    r(result, button(vec2(0,y), iconCar, vec2(spWidth-2,scl),
      strdup_s("Yielded To By"), selectVehicle, selection->yieldFrom));
    y += sclPad;
  }

  if (selection->trailing != 0) {
    r(result, button(vec2(0,y), iconCar, vec2(spWidth-2,scl),
      strdup_s("Trailing"), selectVehicle, selection->trailing));
    y += sclPad;
  }

  if (selection->trailer != 0) {
    r(result, button(vec2(0,y), iconCar, vec2(spWidth-2,scl),
      strdup_s("Trailer"), selectVehicle, selection->trailer));
    y += sclPad;
  }

  r(result, button(vec2(0,y), iconNo, vec2(spWidth-2,scl),
    strdup_s("Delete"), deleteVehicle, getSelection()));
  y += sclPad;

  vector<item> passengers;
  item vNdx = getSelection();
  //while (vNdx != 0) {
    Vehicle* v = getVehicle(vNdx);
    for (int i = 0; i < v->travelGroups.size(); i++) {
      TravelGroup* group = getTravelGroup_g(v->travelGroups[i]);
      for (int j = 0; j < group->members.size(); j++) {
        passengers.push_back(group->members[j]);
      }
    }
    //vNdx = v->trailing;
  //}
  r(result, personList(vec2(0,scrollBoxTop), &passengers));

  if (!(selection->flags & _vehicleHasRoute)) {
    r(result, label(vec2(0,y), scl, strdup_s("Waiting for Route")));
    y += sclPad;
  }

}

bool toggleEnableGovBuilding(Part* part, InputEvent event) {
  Building* b = getBuilding(getSelection());
  bool enabled = b->flags & _buildingEnabled;
  setGovBuildingEnabled(getSelection(), !enabled);
  stopBlinkingFeature(FShutDownBuilding);
  return true;
}

bool sellGovBuilding(Part* part, InputEvent event) {
  sellGovBuilding(getSelection());
  clearSelection();
  return true;
}

bool editSelectionName(Part* part, InputEvent event) {
  focusTextBox(&selectionNameState);
  return true;
}

bool toggleBuildingHistorical(Part* part, InputEvent event) {
  Building* b = getBuilding(getSelection());
  if (b->flags & _buildingHistorical) {
    b->flags &= ~_buildingHistorical;
  } else {
    b->flags |= _buildingHistorical;
  }
  return true;
}

void familyPanel(Part* result, Family* selection) { 
  float y = startY;
  bool isTourists = selection->flags & _familyIsTourists;

  // Header
  r(result, icon(vec2(0, 0), isTourists ? iconTourists : iconFamily));
  r(result, label(vec2(1, 0), 1,
    sprintf_o("%s Household", selection->name)));

  if (isTourists) {
    if (selection->home != 0) {
      r(result, button(vec2(0, y), iconHouse, vec2(spWidth - 2, scl),
        strdup_s("Accommodations"), selectBuilding, selection->home));
    }
  } else if (selection->home == 0) {
    r(result, label(vec2(0, y), scl, strdup_s("Homeless")));
  } else {
    r(result, button(vec2(0, y), iconHouse, vec2(spWidth - 2, scl),
      strdup_s("Home"), selectBuilding, selection->home));
  }
  y += sclPad;

  float timeSinceStore = getCurrentDateTime() - selection->lastStoreTime;
  if (timeSinceStore >= c(CStoreDays)) {
    r(result, labelRed(vec2(0, y), scl, sprintf_o("No fresh food for %d days",
            (int)timeSinceStore)));
    y += sclPad;
  }

  if (isTourists) {
    char* dateTimeStr = printDateTimeString(selection->lastWorkTime);
    r(result, label(vec2(0, y), scl, sprintf_o("Tourists Arrived %s",
            dateTimeStr)));
    free(dateTimeStr);
    y += sclPad;

  } else {
    float timeSinceWork = getCurrentDateTime() - selection->lastWorkTime;
    if (timeSinceWork >= c(CWorkDays)) {
      r(result, labelRed(vec2(0, 3.5), scl, sprintf_o("No income for %d days",
              (int)timeSinceWork)));
      y += sclPad;
    }
  }

  r(result, personList(vec2(0,scrollBoxTop), &selection->members));
}

void businessPanel(Part* result, Business* selection) {
  float y = startY;

  // Header  
  BusinessType type = getBusinessType(getSelection());
  r(result, icon(vec2(0, 0), iconBusinessType[type]));
  r(result, label(vec2(1, 0), 1, strdup_s(selection->name)));

  // Basic info
  r(result, label(vec2(0,y), scl, strdup_s(getBusinessTypeName(type))));
  y += sclPad;
  r(result, label(vec2(0,y), scl, sprintf_o("Established %s", printDateString(selection->foundedTime))));
  y += sclPad;

  Building* building = getBuilding(selection->building);
  Design* design = getDesign(building->design);
  vec3 buildingIco = iconZoneMono[building->zone];

  r(result, button(vec2(0,y), buildingIco, vec2(spWidth - 2, scl), 
    (building->zone == GovernmentZone ? strdup_s(design->displayName) :
     sprintf_o("%s Building", zoneName[building->zone])),
    selectBuilding, selection->building));
  y += sclPad;

  if (selection->flags & _businessOpen) {
    r(result, label(vec2(0, y), scl, strdup_s("Currently Open")));
  } else {
    r(result, labelRed(vec2(0, y), scl, strdup_s("Currently Closed")));
  }
  y += sclPad;

  float time = getCurrentDateTime();
  if (longTimeSinceCustomer(getSelection())) {
    float timeSinceCustomer = time - selection->lastCustomerTime;
    if (type == Retail) {
      r(result, labelRed(vec2(0, y), scl,
        sprintf_o("No customers for %d days", (int)timeSinceCustomer)));
    } else {
      r(result, labelRed(vec2(0, y), scl,
        sprintf_o("No freight out for %d days", (int)timeSinceCustomer)));
    }
    y += sclPad;
  }

  if (longTimeSinceFreight(getSelection())) {
    float timeSinceFreight = time - selection->lastFreightTime;
    r(result, labelRed(vec2(0, y), scl,
      sprintf_o("No freight in for %d days", (int)timeSinceFreight)));
    y += sclPad;
  }

  int positions[numEducationLevels] = {0};
  int unfilled[numEducationLevels] = {0};
  vector<item> employees;
  for (int i = 0; i < selection->positions.size(); i++) {
    Position p = selection->positions[i];
    positions[p.minEducation]++;
    if (p.employee == 0) {
      unfilled[p.minEducation]++;
    } else {
      employees.push_back(p.employee);
    }
  }

  for (int i = numEducationLevels-1; i >= 0; i--) {
    if (positions[i] == 0) continue;
    const char* edu = i == 0 ? "Requiring no Education." :
      i == 1 ? "Requiring a High School Diploma." :
      i == 2 ? "Requiring a Bachelor's Degree." :
      i == 3 ? "Requiring a Doctorate." : "Error";

    r(result, label(vec2(0, y), 0.75,
          sprintf_o("%2d Postions %s", positions[i], edu)));

    if (unfilled[i] != 0) {
      y += sclPad;
      r(result, labelRed(vec2(0, y), 0.75,
          sprintf_o("%2d Openings", unfilled[i], edu)));
    }
    y += sclPad;
  }

  r(result, label(vec2(0,scrollBoxTop-1), 1, strdup_s("Employees")));
  r(result, personList(vec2(0,scrollBoxTop), &employees));
}

void lotPanel(Part* result, Lot* selection) {
  r(result, icon(vec2(0,0), iconZone[selection->zone]));
  char* name = sprintf_o("%s Lot", zoneName[selection->zone]);
  r(result, label(vec2(1.1,0), scl, name));
  r(result, graphElementButton(vec2(0, 1.5), selection->elem));
  r(result, label(vec2(1.0f, 3.0f), scl,
        sprintf_o("Max Density: %d", getLotMaxDensity(getSelection()))));
}

void stopPanel(Part* result, Stop* selection) {
  r(result, icon(vec2(0,0), iconBus));
  selectionNameState.text = &selection->name;
  r(result, textBoxLabel(vec2(1.1,0), vec2(spWidth-2,1), &selectionNameState));

  LaneBlock* b = getLaneBlock(selection->graphLoc.lane);
  r(result, graphElementButton(vec2(0, 1.5), b->graphElements[1]));

  vector<item> s = selection->lines;
  float linesHeight = 5;
  r(result, label(vec2(0,spHeight-scrollBoxHeight-1), scl,
        sprintf_o("Serves %d Line%s", s.size(), s.size() == 1 ? "" : "s")));

  Part* linesPanel = scrollbox(vec2(0,0), vec2(spWidth-1, scrollBoxHeight));
  vec2 buttSize = vec2(spWidth-1.5,0.75);
  vector<item> tg = selection->travelGroups;
  vector<item> passengersPerLine;
  float y = 0;

  for (int i = 0; i < tg.size(); i++) {
    TravelGroup* group = getTravelGroup_g(tg[i]);
    Route route = group->route;
    for (int j = route.steps.size()-1; j >= 0; j--) {
      Location loc = route.steps[j];
      if (locationType(loc) == LocTransitLeg &&
          getStopForLeg_g(loc) == getSelection()) {
        item lineNdx = locationLineNdx(loc);
        if (passengersPerLine.size() <= lineNdx) {
          passengersPerLine.resize(lineNdx+1);
        }
        passengersPerLine[lineNdx] += group->members.size();
        break;
      }
    }
  }

  for (int i = 0; i < s.size(); i++) {
    item lineNdx = s[i];
    item numPass = passengersPerLine.size() <= lineNdx ? 0 :
      passengersPerLine[lineNdx];
    r(linesPanel, button(vec2(0, y), iconBus, buttSize,
        sprintf_o("%s - %d waiting",
          getTransitLine(lineNdx)->name, numPass),
        selectTransitLine, lineNdx));
    y ++;
  }

  if (s.size() > 0 && tg.size() > 0) {
    r(linesPanel, hr(vec2(0.25,y+0.45), spWidth-1.25));
    y ++;
  }

  for (int i = 0; i < tg.size(); i++) {
    TravelGroup* group = getTravelGroup_g(tg[i]);
    for (int j = 0; j < group->members.size(); j++) {
      item personNdx = group->members[j];
      Person* person = getPerson(personNdx);
      Part* butt = button(vec2(0,y), getPersonIcon(person), buttSize,
        getPersonDescriptor(personNdx), selectPerson, personNdx);
      butt->itemData = personNdx;
      r(linesPanel, butt);
      y++;
    }
  }

  r(result, scrollboxFrame(vec2(0,spHeight-scrollBoxHeight),
          vec2(spWidth, scrollBoxHeight), &selectionScroll, linesPanel));
}

void laneBlockPanel(Part* result, LaneBlock* selection) {
  //r(result, icon(vec2(0,0), iconVerticalBar));
  r(result, label(vec2(0,0), scl, strdup_s("Lane Block")));

  float y = 1.5;
  r(result, graphElementButton(vec2(0, y), selection->graphElements[1]));
  y ++;

  bool exists = selection->flags & _laneExists;
  bool open = selection->flags & _laneOpen;
  bool active = selection->flags & _laneActive;
  r(result, icon(vec2(0,y), vec2(scl, scl), exists ? iconYes : iconNo));
  r(result, label(vec2(scl,y), scl, strdup_s("Exists")));
  r(result, icon(vec2(3,y), vec2(scl, scl), open ? iconYes : iconNo));
  r(result, label(vec2(3+scl,y), scl, strdup_s("Open")));
  r(result, icon(vec2(6,y), vec2(scl, scl), active ? iconYes : iconNo));
  r(result, label(vec2(6+scl,y), scl, strdup_s("Active")));
  y += scl;

  r(result, label(vec2(0,y), scl,
        sprintf_o("%d Lane(s)", selection->numLanes)));
  y += scl;

  r(result, label(vec2(0,y), scl, strdup_s("Traversal Time")));
  y += scl;
  char* dur1 = printDurationString(selection->staticTimeEstimate);
  r(result, label(vec2(1,y), scl, sprintf_o("%s Estimate", dur1)));
  free(dur1);
  y += scl;
  dur1 = printDurationString(selection->timeEstimate);
  r(result, label(vec2(1,y), scl, sprintf_o("%s Average", dur1)));
  free(dur1);
  y += scl;

  Part* drainsPanel = scrollbox(vec2(0,0), vec2(spWidth-1, scrollBoxHeight));
  float dy = 0;
  r(drainsPanel, label(vec2(0,dy), scl, strdup_s("Sources")));
  dy ++;

  r(drainsPanel, graphElementButton(vec2(0, dy), selection->graphElements[0]));
  dy ++;

  for (int i = 0; i < selection->sources.size(); i++) {
    r(drainsPanel, laneBlockButton(vec2(0,dy), selection->sources[i]));
    dy ++;
  }

  dy += 0.25;
  r(drainsPanel, hr(vec2(0,dy), spWidth-1.5));
  dy += 0.25;
  r(drainsPanel, label(vec2(0,dy), scl, strdup_s("Drains")));
  dy ++;

  r(drainsPanel, graphElementButton(vec2(0, dy), selection->graphElements[2]));
  dy ++;

  for (int i = 0; i < selection->drains.size(); i++) {
    r(drainsPanel, laneBlockButton(vec2(0,dy), selection->drains[i]));
    dy ++;
  }

  r(result, scrollboxFrame(vec2(0,spHeight-scrollBoxHeight),
          vec2(spWidth, scrollBoxHeight), &selectionScroll, drainsPanel));
}

void personPanel(Part* result, Person* selection) {
  Family* family = getFamily(selection->family);
  float y = startY;
  float time = getCurrentDateTime();
  int age = (time - selection->birthday)/oneYear;
  bool isTourist = selection->flags & _personIsTourist;

  // Header
  r(result, icon(vec2(0,-0.1), getPersonIcon(selection)));
  r(result, label(vec2(scl,0), 1,
    sprintf_o("%s %s, %d", selection->name, family->name, age)));

  // Tourist Info
  if (isTourist) {
    char* dateTimeStr = printDateTimeString(family->lastWorkTime);
    r(result, label(vec2(0, y), scl, sprintf_o("Tourist Arrived %s",
            dateTimeStr)));
    free(dateTimeStr);
    y += sclPad;

    float memorableExperiences = memorableExperiencesForPerson(
        getSelection(), oneHour);
    r(result, label(vec2(0, y), scl,
          sprintf_o("Has %.2f memorable experiences per hour",
            memorableExperiences)));
    y += sclPad;
  }

  // Personal info
  r(result, label(vec2(0, y), scl, sprintf_o("Born in %d",
          getCurrentYear() - age)));
  y += sclPad;

  EducationLevel edu = getEducationForPerson(getSelection());
  const char* eduLabel =
    edu == NoEducation ? "No Education" :
    (selection->flags & _personIsChild) ? "Receiving Education" :
    edu == HSDiploma ? "High School Diploma" :
    edu == BachelorsDegree ? "Bachelor's Degree" :
    "Doctorate";

  r(result, label(vec2(0, y), scl, strdup_s(eduLabel)));
  y += sclPad;

  if (selection->flags & _personSick) {
    Part* s = r(result, label(vec2(0, y), scl, strdup_s("Sick")));
    s->foregroundColor = PickerPalette::RedLight;
    y += sclPad;
  }

  //Family and household
  if (family->members.size() == 1) {
    r(result, label(vec2(0,y), scl, strdup_s("Single")));
    y += sclPad;

  } else {
    r(result, button(vec2(0,y),
      (family->flags & _familyIsTourists) ? iconTourists : iconFamily,
      vec2(spWidth-2,scl), getFamilyDescriptor(selection->family),
      selectFamily, selection->family));
    y += sclPad;
  }

  if(family->home == 0) {
    r(result, label(vec2(0, y), scl, strdup_s("Homeless")));
  }
  else {
    r(result, button(vec2(0, y), iconHouse, vec2(spWidth - 2, scl),
      strdup_s("Home"), selectBuilding, family->home));
  }
  y += sclPad;

  // What are they doing?
  Activity act = (Activity)selection->activity;
  char* actStr = getActivityName(getSelection());
  vec3 actIco = iconNo;

  if(selection->flags & _personTraveling) {
    actIco = iconCar;
  } else if(act == SleepActivity) {
    actIco = iconSleep;
  } else if(act == HomeActivity) {
    actIco = iconTV;
  } else if(act == DoctorActivity) {
    actIco = iconSick;
  } else if(selection->activityBuilding != 0) {
    Building* building = getBuilding(selection->activityBuilding);
    actIco = iconZoneMono[building->zone];
  }

  if (selection->flags & _personTraveling) {
    TravelGroup* tg = getTravelGroup_g(locationNdx(selection->location));
    item locType = locationType(tg->location);
    vec3 locIco = iconNo;
    if (locType == LocTransitStop) {
      locIco = iconBusSign;
    } else if (locType == LocVehicle) {
      Vehicle* v = getVehicle(locationNdx(tg->location));
      if (v->flags & _vehicleIsTransit) {
        locIco = iconBus;
      } else {
        locIco = iconCar;
      }
    }

    r(result, button(vec2(0, y), locIco, vec2(spWidth - 2, scl),
      strdup_s(actStr), selectLocation, tg->location));
    y += sclPad;
    r(result, button(vec2(0, y), actIco, vec2(spWidth - 2, scl), 
      strdup_s("Destination"), selectBuilding, selection->activityBuilding));

  } else if (selection->activityBuilding != 0) {
    r(result, button(vec2(0, y), actIco, vec2(spWidth - 2, scl), 
      strdup_s(actStr), selectBuilding, selection->activityBuilding));
  } else {
    r(result, label(vec2(0, y), scl, strdup_s(actStr)));
  }
  y += sclPad;

  r(result, label(vec2(0, y), scl, sprintf_o("Since %s",
    time - selection->enterTime < 1 ?
    printTimeString(selection->enterTime) :
    printDateTimeString(selection->enterTime))));
  y += sclPad;

  if (selection->flags & _personWaitingForRoute) {
    r(result, label(vec2(0,y), scl, strdup_s("Waiting for Route")));
    y += sclPad;
  }

  #ifdef LP_DEBUG
  r(result, label(vec2(0,y), scl, sprintf_o("Next update %s",
        printDateTimeString(getPersonWakeTime(getSelection())))));
  y += sclPad;
  #endif

  // Employer
  if (selection->employer > 0) {
    Part* employerButt = panel(vec2(0,y), vec2(spWidth, sclPad*2));
    employerButt->renderMode = RenderTransparent;
    employerButt->onClick = selectBusiness;
    employerButt->flags |= _partHover;
    employerButt->itemData = selection->employer;
    r(result, employerButt);

    BusinessType type = getBusinessType(selection->employer);
    r(employerButt, icon(vec2(0,0), vec2(scl, scl), iconBusinessType[type]));
    char* empDesc = getBusinessDescriptor(selection->employer);
    float ySize = 0;
    r(employerButt, multiline(vec2(sclPad,0), vec2(spWidth-sclPad, scl),
      sprintf_o("Employer: %s", empDesc), &ySize));
    free(empDesc);
    employerButt->dim.end.y = ySize;
    y += ySize;

  } else if (selection->flags & _personIsWorker) {
    r(result, labelRed(vec2(scl,y), scl, strdup_s("Unemployed")));
    y += sclPad;
  }

  float actHeight = (numActivities-2)*sclPad+spPadding*1;
  r(result, label(vec2(0,spHeight-actHeight-scl-spPadding), scl,
        strdup_s("Activities:")));
  Part* actvitiesPanel = panel(vec2(0,spHeight-actHeight-spPadding),
      vec2(spWidth, actHeight));
  actvitiesPanel->flags |= _partLowered;
  r(result, actvitiesPanel);

  float acty = 0;
  float scores[numActivities];
  for (int i = 0; i < numActivities; i++) {
    scores[i] = getActivityScore(getSelection(), i);
  }

  // Insertion sort
  item indices[numActivities];
  for (int i = 0; i < numActivities; i++) {
    indices[i] = i;
  }
  for (int i = 0; i < numActivities; i++) {
    item x = indices[i];
    float xScore = scores[x];
    int j = i-1;
    for (; j >= 0 && scores[indices[j]] < xScore; j--) {
      indices[j+1] = indices[j];
    }
    indices[j+1] = x;
  }

  for (int i = 0; i < (numActivities-2); i++) {
    item activity = indices[i];
    float score = scores[activity];
    if (score < -1) continue;
    score = clamp(score, -1.f, 9.99f);

    char* actName = 0;
    if (activity == selection->activity) {
      actName = getActivityName(getSelection());
    } else if (selection->location == family->home) {
      if (activity == SleepActivity) {
        actName = strdup_s("Going to Sleep");
      } else if (activity == HomeActivity) {
        actName = strdup_s("Staying Home");
      }
    }
    if (actName == 0) {
      actName = strdup_s(getRawActivityName(activity));
    }

    Part* act = 0;

    if (debugMode()) {

      // Debug activity presentation
      act = label(vec2(scl, acty), scl, sprintf_o("%+6.1f%% %s",
        score*100, actName));
      if (activity == selection->activity) {
        r(actvitiesPanel, icon(vec2(0, acty), vec2(scl, scl), iconRightSmall));
      }

    } else { 

      // Player-facing activity presentation
      act = label(vec2(scl, acty), scl, sprintf_o("%d. %s",
        i+1, actName));
      if (activity == selection->activity) {
        r(actvitiesPanel, icon(vec2(0, acty), vec2(scl, scl), iconRightSmall));
        act->foregroundColor = (char)PickerPalette::White;
      } else
      {
        act->foregroundColor = (char)PickerPalette::GrayLight;
      }

    }

    // Only add activity leaf and increment acty var
    // if pointer is not null
    if (act != 0) {
      r(actvitiesPanel, act);
      acty += sclPad;
    }

    // Don't forget to free actName char* when we're done
    free(actName);
  }
}

bool setSpawnProbability(Part* part, InputEvent event) {
  Node* node = getNode(getSelection());
  node->spawnProbability = part->vecData.x*c(CMaxSpawn);
  return true;
}

bool spawnVehicle(Part* part, InputEvent event) {
  spawnVehicle(part->itemData, true);
  return true;
}

bool chainRenameEdge(Part* part, InputEvent event) {
  chainRenameEdge(getSelection(), part->itemData);
  return true;
}

void nodePanel(Part* result, item ndx, Node* selection) {
  float y = startY;

  vec3 ico = iconRoad;
  if (selection->config.type == ConfigTypeExpressway) {
    ico = iconExpressway;
  }

  // Header
  r(result, icon(vec2(0,0), ico));
  r(result, label(vec2(1,0), 1, strdup_s(graphElementName(ndx))));

  // Traffic volume
  r(result, icon(vec2(0,y), vec2(1,scl), iconCar));
  r(result, label(vec2(1,y), scl,
        sprintf_o("%d Cars", numVehiclesInElement(getSelection()))));
  y += sclPad;

  if (getGameMode() == ModeTest && selection->edges.size() == 1) {
    r(result, label(vec2(0,y), scl, sprintf_o("%4d Cars/Hour",
      (int)(selection->spawnProbability*60))));
    y += sclPad;
    r(result, slider(vec2(0,y), vec2(spWidth-2,scl),
      selection->spawnProbability/c(CMaxSpawn), setSpawnProbability));
    y += sclPad;
    r(result, button(vec2(0,y), iconCar, vec2(spWidth-2,scl),
      strdup_s("Spawn Vehicle"), spawnVehicle, getSelection()));
    y += sclPad;
  }

  float ly = 0;
  Part* legsPanel = scrollbox(vec2(0,0), vec2(spWidth-1, scrollBoxHeight));
  r(legsPanel, label(vec2(0,ly), scl, strdup_s("Legs")));
  ly ++;

  for (int i = 0; i < selection->edges.size(); i++) {
    r(legsPanel, graphElementButton(vec2(0, ly), selection->edges[i]));
    ly ++;
  }

  ly += 0.25;
  r(legsPanel, hr(vec2(0,ly), spWidth-1.5));
  ly += 0.25;
  r(legsPanel, label(vec2(0,ly), scl, strdup_s("Lane Blocks")));
  ly ++;

  for (int i = 0; i < selection->laneBlocks.size(); i++) {
    r(legsPanel, laneBlockButton(vec2(0, ly), selection->laneBlocks[i]));
    ly ++;
  }

  r(result, scrollboxFrame(vec2(0,spHeight-scrollBoxHeight),
          vec2(spWidth, scrollBoxHeight), &selectionScroll, legsPanel));
}

bool setRoadWear(Part* part, InputEvent event) {
  setWear(getSelection(), part->vecData.x);
  return true;
}

void edgePanel(Part* result, item ndx, Edge* selection) {
  float y = startY;
  float speedLimit = speedLimits[selection->config.speedLimit];
  float safeSpeed = selection->laneBlocks[0] == 0 ? speedLimit :
    getLaneBlock(selection->laneBlocks[0])->speedLimit;

  vec3 ico = iconRoad;
  if (selection->config.type == ConfigTypeExpressway) {
    ico = iconExpressway;
  }

  // Header
  r(result, icon(vec2(0,0), ico));

  if (selectionNameState.flags & _textBoxEditing) {
    selectionNameState.text = &selection->name;
    renderGraphElement(ndx);
    r(result, textBoxLabel(vec2(sclPad,0),
          vec2(spWidth-sclPad-1,scl), &selectionNameState));
    y += 1;

  } else {
    r(result, multiline(vec2(sclPad,0), vec2(spWidth-sclPad, scl),
        strdup_s(graphElementName(ndx)), &y));
    r(result, button(vec2(spWidth-sclPad*2, 0), iconPencil,
          vec2(scl, scl), editSelectionName, 0));
  }

  Part* cr = r(result, button(vec2(0,y), vec2(spWidth-2,scl),
    strdup_s("Chain Rename"), chainRenameEdge));
  cr->itemData = 0;
  y += sclPad;

  Part* crfc = r(result, button(vec2(0,y), vec2(spWidth-2,scl),
    strdup_s("Chain Rename (Follow Corners)"), chainRenameEdge));
  crfc->itemData = 1;
  y += sclPad;

  // Traffic volume
  r(result, icon(vec2(0,y), iconCar));
  r(result, label(vec2(1,y), scl,
        sprintf_o("%3d Cars", numVehiclesInElement(getSelection()))));
  y += sclPad;

  bool hasEither = selection->laneBlocks[0];
  if (hasEither) {
    bool hasBoth = selection->laneBlocks[1];
    float time0 = getLaneBlock(selection->laneBlocks[0])->timeEstimate;
    char* timeStr;
    char* dur0 = printDurationString(time0);
    if (hasBoth) {
      float time1 = getLaneBlock(selection->laneBlocks[1])->timeEstimate;
      char* dur1 = printDurationString(time1);
      timeStr = sprintf_o("Traversal Time: %s/%s", dur0, dur1);
      free(dur1);
    } else {
      timeStr = sprintf_o("Traversal Time: %s", dur0);
    }
    free(dur0);
    r(result, label(vec2(1,y), scl, timeStr));
    y += sclPad;
  }

  // Edge specifics
  r(result, label(vec2(1,y), scl,
        printSpeedString("", speedLimit, " Speed Limit")));
  y += sclPad;
  r(result, label(vec2(1,y), scl,
        printSpeedString("", safeSpeed, " Safe Speed")));
  y += sclPad;

  r(result, icon(vec2(0,y), iconWrench));
  r(result, label(vec2(1,y), scl,
    sprintf_o("%3d%% Wear", (int)(selection->wear*100))));
  y += sclPad;

  /*
  if (speedLimit > safeSpeed) {
    r(result, label(vec2(1,y), scl,
          strdup_s("Use Repair Tool (5)\nto fix wear.")));
    y += sclPad;
  }
  */

  if (getGameMode() == ModeTest) {
    r(result, slider(vec2(0,y), vec2(spWidth-2,scl),
      selection->wear, setRoadWear));
    y += sclPad;
  }

  float ly = 0;
  Part* legsPanel = scrollbox(vec2(0,0), vec2(spWidth-1, scrollBoxHeight));
  r(legsPanel, label(vec2(0,ly), scl, strdup_s("Ends")));
  ly ++;

  for (int i = 0; i < 2; i++) {
    r(legsPanel, graphElementButton(vec2(0, ly), selection->ends[i]));
    ly ++;
  }

  ly += 0.25;
  r(legsPanel, hr(vec2(0,ly), spWidth-1.5));
  ly += 0.25;
  r(legsPanel, label(vec2(0,ly), scl, strdup_s("Lane Blocks")));
  ly ++;

  for (int i = 0; i < 2; i++) {
    item blkNdx = selection->laneBlocks[i];
    if (blkNdx < 10) continue;
    r(legsPanel, laneBlockButton(vec2(0, ly), blkNdx));
    ly ++;
  }

  r(result, scrollboxFrame(vec2(0,spHeight-scrollBoxHeight),
          vec2(spWidth, scrollBoxHeight), &selectionScroll, legsPanel));
}

bool followSelection(Part* part, InputEvent event) {
  setFollowingSelection(!isFollowingSelection());
  stopBlinkingFeature(FNoteObject);
  return true;
}

bool togglePersonMessage(Part* part, InputEvent event) {
  toggleMessage(PersonMessage, part->itemData);
  stopBlinkingFeature(FNoteObject);
  return true;
}

bool toggleBusinessMessage(Part* part, InputEvent event) {
  toggleMessage(BusinessMessage, part->itemData);
  stopBlinkingFeature(FNoteObject);
  return true;
}

bool toggleBuildingMessage(Part* part, InputEvent event) {
  toggleMessage(BuildingMessage, part->itemData);
  stopBlinkingFeature(FNoteObject);
  return true;
}

bool toggleGraphMessage(Part* part, InputEvent event) {
  toggleMessage(GraphMessage, part->itemData);
  stopBlinkingFeature(FNoteObject);
  return true;
}

bool toggleVehicleMessage(Part* part, InputEvent event) {
  toggleMessage(VehicleMessage, part->itemData);
  stopBlinkingFeature(FNoteObject);
  return true;
}

#include "buildingPanel.cpp"

Part* selectionPanel() {
  item selection = getSelection();
  item selectionType = getSelectionType();
  if (selectionType == NoSelection) {
    return blank();
  }

  Part* result = panel(selectionPanelLoc, selectionPanelSize);
  result->padding = spPadding;

  // Close button
  r(result, button(vec2(spWidth-1,0), iconX, clearSelection));

  // Eyeball button
  if (selectionType != SelectionBuilding) {
    Part* followButt = button(vec2(spWidth-1,1.25), iconEye, followSelection);
    if (isFollowingSelection()) {
      followButt->flags |= _partHighlight;
    }
    r(result, followButt);
  }

  if (selectionType != SelectionBuilding) {
    // Index Number
    char* ndxStr = sprintf_o("#%d", getSelection());
    float ndxStrWidth = stringWidth(ndxStr)*0.75f;
    r(result, label(vec2(0,.75f), 0.75, ndxStr));

    // HR
    r(result, hr(vec2(ndxStrWidth,1), spWidth-ndxStrWidth));
  }

  InputCallback addMessageCallback = 0;
  MessageType msgType = (MessageType)-1;

  if (selectionType == SelectionVehicle) {
    vehiclePanel(result, getVehicle(selection));
    addMessageCallback = toggleVehicleMessage;
    msgType = VehicleMessage;

  } else if (selectionType == SelectionBuilding) {
    buildingPanel(result, getBuilding(selection));
    addMessageCallback = toggleBuildingMessage;
    msgType = BuildingMessage;

  } else if (selectionType == SelectionPerson) {
    personPanel(result, getPerson(selection));
    addMessageCallback = togglePersonMessage;
    msgType = PersonMessage;

  } else if (selectionType == SelectionFamily) {
    familyPanel(result, getFamily(selection));

  } else if (selectionType == SelectionBusiness) {
    businessPanel(result, getBusiness(selection));
    addMessageCallback = toggleBusinessMessage;
    msgType = BusinessMessage;

  } else if (selectionType == SelectionLot) {
    lotPanel(result, getLot(selection));

  } else if (selectionType == SelectionStop) {
    stopPanel(result, getStop(selection));

  } else if (selectionType == SelectionLaneBlock) {
    laneBlockPanel(result, getLaneBlock(selection));

  } else if (selectionType == SelectionGraphElement) {
    addMessageCallback = toggleGraphMessage;
    msgType = GraphMessage;
    if (selection > 0) {
      edgePanel(result, selection, getEdge(selection));
    } else {
      nodePanel(result, selection, getNode(selection));
    }
  }

  if (isFeatureEnabled(FNoteObject) && addMessageCallback != 0 &&
      selectionType != SelectionBuilding) {
    Part* butt = button(vec2(spWidth-1,2.4), iconPin, vec2(1,1),
          addMessageCallback, selection);
    if (hasMessage(msgType, selection)) {
      butt->flags |= _partHighlight;
    }
    if (blinkFeature(FNoteObject)) {
      butt->flags |= _partBlink;
    }
    r(result, butt);
  }

  return result;
}
