#include "../color.hpp"
#include "../economy.hpp"
#include "../route/router.hpp"
#include "../time.hpp"

static item cursorEntityNdx[2];
static vec3 cursorLoc[2];
static GraphLocation cursorGraphLoc[2];
static bool cursorSet[2] = {false,false};
static int atEnd = 0;
item routeEntity;
Route route[2];

void allocRouteCursorEntity() {
  if (cursorEntityNdx[0] != 0) return;

  for (int i = 0; i < 2; i++) {

    cursorEntityNdx[i] = addEntity(PaletteShader);
    Entity* cursorEntity = getEntity(cursorEntityNdx[i]);
    cursorEntity->texture = paletteTexture;
    setEntityVisible(cursorEntityNdx[i], false);
    //setEntityBringToFront(cursorEntityNdx[i], true);
    setEntityTransparent(cursorEntityNdx[i], false);
    setEntityHighlight(cursorEntityNdx[i], true);
    createMeshForEntity(cursorEntityNdx[i]);

    Mesh* mesh = getMeshForEntity(cursorEntityNdx[i]);
    makeCone(mesh, vec3(0,0,1), vec3(0,0,-1),
        1, i ? colorBlue : colorRed, true);
    bufferMesh(cursorEntity->mesh);
  }

  routeEntity = addEntity(PaletteShader);
  Entity* routeEnt = getEntity(routeEntity);
  routeEnt->texture = paletteTexture;
  setEntityVisible(routeEntity, false);
  //setEntityBringToFront(routeEntity, true);
  setEntityTransparent(routeEntity, false);
  setEntityHighlight(routeEntity, true);
  setEntityRaise(routeEntity, 3);
  setCull(routeEntity, 1000*1000*1000, 1000*1000*1000);
  createMeshForEntity(routeEntity);
}

void deallocRouteCursorEntity() {
  if (cursorEntityNdx[0] == 0) return;

  removeEntity(cursorEntityNdx[0]);
  removeEntity(cursorEntityNdx[1]);
  removeEntity(routeEntity);
  cursorEntityNdx[0] = 0;
  cursorEntityNdx[1] = 0;
  routeEntity = 0;
}

void routes_select() {
}

void routes_reset() {
  deallocRouteCursorEntity();
  cursorLoc[0] = vec3(0,0,0);
  cursorLoc[1] = vec3(0,0,0);
  cursorGraphLoc[0] = {0,0};
  cursorGraphLoc[1] = {0,0};
  cursorSet[0] = false;
  cursorSet[1] = false;
  atEnd = 0;
  route[0].steps.clear();
  route[1].steps.clear();
}

void renderCursor() {
  allocRouteCursorEntity();

  float dist = getCameraDistance();
  float size = dist/15;

  for (int i = 0; i < 2; i++) {
    setEntityVisible(cursorEntityNdx[i], cursorSet[i]);
    if (cursorSet[i]) {
      Entity* cursorEntity = getEntity(cursorEntityNdx[i]);
      placeEntity(cursorEntityNdx[i], cursorLoc[i], 0, 0, size);
    }
  }

  setEntityVisible(routeEntity, cursorSet[0] && cursorSet[1]);
  if (cursorSet[0] && cursorSet[1] && (
        route[0].source/10 != cursorGraphLoc[0].lane/10 ||
        route[0].destination/10 != cursorGraphLoc[1].lane/10)) {

    vec3 offset = (cursorLoc[0] + cursorLoc[1]) * .5f;
    vec3 offsetDown = offset + vec3(0,0,-4);
    placeEntity(routeEntity, offset+vec3(0,0,100), 0, 0);

    item meshNdx = getEntity(routeEntity)->mesh;
    Mesh* mesh = getMesh(meshNdx);

    for (int j = 0; j < 2; j++) {
      route[j].source = getLaneBlockIndex(cursorGraphLoc[0].lane);
      route[j].destination = getLaneBlockIndex(cursorGraphLoc[1].lane);
      vector<Location> steps = routeInstant(route[j].source,
          route[j].destination, j);

      if (j == 1 && route[j].steps.size() > 0) {
        steps.insert(steps.begin(), route[j].source);
        steps.push_back(route[j].destination);
      }

      route[j].steps.fromVector(steps);

      int stepS = steps.size();
      item lastStepType = -1;
      Location lastTransitLoc = 0;

      for (int i = 0; i < stepS; i++) {
        Location step = steps[i];
        Location nextStep = i+1 < stepS ? steps[i+1] : 0;
        item stepType = locationType(step);
        item nextStepType = locationType(nextStep);

        if (stepType == LocLaneBlock) {
          if (step < 10) {
            // no-op, possible error

          } else {
            item lane = step;
            float start = 0;
            float end = -1;
            if (nextStepType == LocDap) {
              //lane = getLaneBlockIndex(step) + getBlockLanes(step)-1;
              if (lastStepType == LocLaneBlock) {
                start = 0;
                end = locationNdx(nextStep)-5;

              } else {
                start = locationNdx(nextStep)+5;
                end = -1;
              }
            }

            renderLane(mesh, lane, start, end,
                c(CTransitLineWidth)*.7f, 0, offset, colorRed);
          }

        } else if (stepType == LocDap) {
          // no-op

        } else if (stepType == LocTransitLeg) {
          item stopNdx = getStopForLeg_g(step);
          renderStopDisc(mesh, stopNdx, offsetDown);

          if (lastTransitLoc > 0) {
            item lineNdx = locationLineNdx(lastTransitLoc);
            if (lineNdx == locationLineNdx(step)) {
              item endNdx = locationLegNdx(step);
              for (int l = locationLegNdx(lastTransitLoc); l < endNdx; l++) {
                renderTransitLeg(meshNdx, lineNdx, l, offsetDown);
              }
            }
            lastTransitLoc = step;
          } else {
            lastTransitLoc = step;
          }
        }

        lastStepType = stepType;
      }
    }

    bufferMesh(meshNdx);
  }
}

void routes_mouse_button_callback(InputEvent event) {
  int button = event.button;
  int action = event.action;

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    if (cursorSet[0]) {
      playSound(_soundClickCancel);
      atEnd = atEnd == 0 ? 1 : 2;
      reportTutorialUpdate(InspectedRoute);
    }

  } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    playSound(_soundClickPrimary);
    atEnd = atEnd == 2 ? 1 : 0;
    cursorSet[1] = false;
  }
}

void routes_mouse_move_callback(InputEvent event) {
  if (atEnd > 1) {
    renderCursor();
    return;
  }

  GraphLocation loc = graphLocation(event.mouseLine);
  if (loc.lane < 10) {
    cursorSet[atEnd] = false;

  } else {
    cursorSet[atEnd] = true;
    Lane* lane = getLane(loc.lane);
    LaneBlock* blk = getLaneBlock(loc.lane);
    loc.dap = clamp(loc.dap, c(CTileSize), lane->length - c(CTileSize));
    loc.dap = round(loc.dap/c(CTileSize)) * c(CTileSize);
    cursorGraphLoc[atEnd] = loc;
    cursorLoc[atEnd] = getLocation(loc);
  }

  renderCursor();
}

bool routes_visible() {
  return true;
}

Part* routes_render(Line dim) {
  Part* result = panel(dim);
  float y = 0;
  r(result, label(vec2(0,0), 1, strdup_s("Route Inspector")));
  //r(result, hr(vec2(0,1.0), dim.end.x-toolPad*2));
  r(result, multiline(vec2(0, 1.25), vec2(10, .8), strdup_s(
    "Left click on a road to see routes"
    " starting there."
    " Left click on a different"
    " road to inspect that route."), &y));

  if (c(CValueOfTime) != 0) {
    float dollarPerHour = c(CValueOfTime)*getInflation();
    float daysPerDollar = 1.f/(dollarPerHour*24);
    char* moneyVal = printMoneyString(dollarPerHour);
    char* timeVal = printDurationString(daysPerDollar);
    Part* p = r(result, multiline(vec2(0,4), vec2(10, 0.75),
          sprintf_o("Note: citizens value their time"
            " at %s per hour, so they'll wait %s to save one dollar.",
            moneyVal, timeVal), &y));
    free(moneyVal);
    free(timeVal);
  }

  return result;
}

void routeInspectorInstructionPanel(Part* result) {
  Line dim = result->dim;

  int routesValid = 0;
  int betterRoute = -1;
  float betterRouteCost = -1;
  float scl = 0.7;
  float routeCost[2] = {0,0};
  float columnSpacing = 9;

  //r(result, hr(vec2(0, 1), dim.end.x-result->padding*2));

  if (cursorSet[0] && cursorSet[1]) {
    for (int i = 0; i < 2; i++) {
      Cup<Location>* steps = &route[i].steps;
      int stepS = steps->size();
      RouteInfo info = computeRouteInfo_g(steps, i, false);

      if (stepS == 0) {
        float x = columnSpacing*i;
        float y = 7-scl-result->padding*1;
        r(result, icon(vec2(x,y), vec2(scl, scl),  iconNo));
        r(result, label(vec2(x+1,y), scl, strdup_s("No Route")));
      }

      float y = 0;
      float x = columnSpacing*i;
      float xs = 2;

      r(result, labelRight(vec2(x,y), vec2(4,1.f),
            strdup_s(i ? "By Transit" : "By Car")));
      y += 1;

      if (stepS == 0) continue;

      r(result, labelRight(vec2(x+2,y), vec2(xs,scl),
            printDurationString(info.travelTime)));
      y += scl;

      if (info.walkingTime > 0) {
        r(result, labelRight(vec2(x,y), vec2(2,scl), strdup_s("+ ")));
        r(result, labelRight(vec2(x+2,y), vec2(xs,scl),
              printDurationString(info.walkingTime)));
      }
      y += scl;

      if (info.waitTime > 0) {
        r(result, labelRight(vec2(x,y), vec2(2,scl), strdup_s("+ ")));
        r(result, labelRight(vec2(x+2,y), vec2(xs,scl),
              printDurationString(info.waitTime)));
      }
      y += scl;

      r(result, hr(vec2(x, y), 4));
      y += 0.1;
      if (info.multiplier != 1) {
        r(result, labelRight(vec2(x,y), vec2(2,scl), sprintf_o("%1.1f x ",
                info.multiplier)));
        Part* bonusCont = r(result, panel(vec2(x-.5f,y), vec2(2,2)));
        bonusCont->renderMode = RenderTransparent;
        r(bonusCont, icon(vec2(.95,.6),
              vec2(.5f, .5f),  iconUp));
        r(bonusCont, labelCenter(vec2(0,0.8), vec2(2.3,0.6),
              strdup_s(info.multiplier < 1 ? "Bonus!" : "Penalty!")));
      }
      r(result, labelRight(vec2(x+2,y), vec2(xs,scl),
            printDurationString(info.time)));
      y += scl;

      if (!i) {
        r(result, labelRight(vec2(x,y), vec2(2,scl), strdup_s("+ ")));
        r(result, labelRight(vec2(x+2,y), vec2(xs,scl),
              printMoneyString(info.fuelMaintCost)));
      }
      y += scl;

      if (i && info.ticketCost > 0) {
        r(result, labelRight(vec2(x,y), vec2(2,scl), strdup_s("+ ")));
        r(result, labelRight(vec2(x+2,y), vec2(xs,scl),
              printMoneyString(info.ticketCost)));
      }
      y += scl;

      r(result, hr(vec2(x, y), 4));
      y += 0.1;
      r(result, labelRight(vec2(x,y), vec2(2,scl), strdup_s("= ")));
      r(result, labelRight(vec2(x+2,y), vec2(xs,scl),
            printDurationString(info.costAdjustedTime)));
      y += scl;

      routesValid ++;
      if (betterRoute == -1 || info.costAdjustedTime < betterRouteCost) {
        betterRouteCost = info.costAdjustedTime;
        betterRoute = i;
      }
    }
  }

  if (betterRoute >= 0 && routesValid == 2) {
    float x = columnSpacing*betterRoute;
    float y = 7-scl-result->padding*1;
    r(result, icon(vec2(x,y), vec2(scl, scl),  iconYes));
    r(result, label(vec2(x+1,y), scl, strdup_s("Preferred")));
  }

  float y = 1;
  float x = 4.75;
  r(result, label(vec2(x,y), scl, strdup_s("Travel Time")));
  y += scl;
  r(result, label(vec2(x,y), scl, strdup_s("Walking Time")));
  y += scl;
  r(result, label(vec2(x,y), scl, strdup_s("Wait Time")));
  y += scl;
  r(result, hr(vec2(x, y), 3.5));
  y += 0.1;
  r(result, label(vec2(x,y), scl, strdup_s("Total Time")));
  y += scl;
  r(result, label(vec2(x,y), scl, strdup_s("Fuel & Maint.")));
  y += scl;
  r(result, label(vec2(x,y), scl, strdup_s("Ticket Price")));
  y += scl;
  r(result, hr(vec2(x, y), 3.5));
  y += 0.1;
  r(result, label(vec2(x,y), scl, strdup_s("Cost-Adjusted\nTime")));

  //r(result, block(vec2(4.75, 0), vec2(0.05, dim.end.y-result->padding*2)));
  //r(result, block(vec2(9.75, 0), vec2(0.05, dim.end.y-result->padding*2)));
}

Tool toolRoute = {
  routes_mouse_button_callback,
  routes_mouse_move_callback,
  routes_select,
  routes_reset,
  routes_visible,
  iconRoute,
  routes_render,
  queryInstructionPanel,
};

