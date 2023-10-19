#include "renderSelection.hpp"

#include "color.hpp"
#include "draw/entity.hpp"
#include "draw/texture.hpp"
#include "graph.hpp"
#include "graph/stop.hpp"
#include "graph/transit.hpp"
#include "heatmap.hpp"
#include "intersection.hpp"
#include "person.hpp"
#include "renderGraph.hpp"
#include "renderUtils.hpp"
#include "selection.hpp"
#include "util.hpp"
#include "vehicle/travelGroup.hpp"
#include "vehicle/vehicle.hpp"

#include "parts/root.hpp"

#include "spdlog/spdlog.h"

void renderSelection(item entityNdx, int selectionType, item selected,
    vector<item> lanes) {
  if (!isRenderEnabled()) { return; }

  Entity* entity = getEntity(entityNdx);
  entity->texture = paletteTexture;
  //setEntityRaise(entityNdx, 3);
  //setEntityBringToFront(entityNdx, 3);
  setEntityHighlight(entityNdx, true);
  float zOff = 0;
  vec3 location = getSelectionLocation();
  createMeshForEntity(entityNdx);
  Mesh* mesh = getMeshForEntity(entityNdx);
  Route route;
  item destBuilding = 0;

  if (selectionType == SelectionVehicle) {
    zOff = 50;
    Vehicle* v = getVehicle(selected);
    route = v->route;
    if (!(v->flags & _vehicleIsTransit) && v->travelGroups.size() > 0) {
      TravelGroup* group = getTravelGroup_g(v->travelGroups[0]);
      if (group->members.size() > 0) {
        Person* p = getPerson(group->members[0]);
        destBuilding = p->activityBuilding;
      }
    }

  } else if (selectionType == SelectionPerson) {
    zOff = 50;
    Person* person = getPerson(selected);
    if (person->flags & _personTraveling) {
      TravelGroup* group = getTravelGroup_g(locationNdx(person->location));
      route = group->route;
      destBuilding = person->activityBuilding;
    }

  } else if (selectionType == SelectionStop) {
    if (getHeatMap() != TransitHeatMap) {
      renderStopDisc(mesh, selected, vec3(0,0,-4));
    }
  }

  int stepS = route.steps.size();
  item lastStepType = -1;
  Location lastTransitLoc = 0;
  for (int i = 0; i < stepS; i++) {
    Location step = route.steps[i];
    Location nextStep = i+1 < stepS ? route.steps[i+1] : 0;
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
            c(CTransitLineWidth)*.7f, 0, location, colorRed);
      }

    } else if (stepType == LocDap) {
      // no-op

    } else if (stepType == LocTransitLeg) {
      item stopNdx = getStopForLeg_g(step);
      renderStopDisc(mesh, stopNdx, location+vec3(0,0,-4));

      if (lastTransitLoc > 0) {
        item lineNdx = locationLineNdx(lastTransitLoc);
        if (lineNdx == locationLineNdx(step)) {
          item endNdx = locationLegNdx(step);
          for (int l = locationLegNdx(lastTransitLoc); l < endNdx; l++) {
            renderTransitLeg(entity->mesh, lineNdx, l, location+vec3(0,0,-4));
          }
        }
        lastTransitLoc = step;
      } else {
        lastTransitLoc = step;
      }
    }

    lastStepType = stepType;
  }

  /*
  if (selectedVehicle > 0) {
    Vehicle* vehicle = getVehicle(selectedVehicle);

    if (vehicle->flags & _vehicleHasRoute) {
      item currentLane = vehicle->laneLoc.lane;
      item lastLane = vehicle->destination.lane;
      if (currentLane != 0 && currentLane == lastLane) {
        renderLane(mesh, currentLane, vehicle->laneLoc.dap,
            vehicle->destination.dap);
      } else {
        if (currentLane != 0) {
          renderLane(mesh, currentLane, vehicle->laneLoc.dap, -1);
        }
        renderLane(mesh, lastLane, 0, vehicle->destination.dap);
      }
    }
  }
  */

  for (int i=0; i < lanes.size(); i++) {
    renderLaneBlock(mesh, lanes[i]);
  }

  float coneSize = getCameraDistance()/10;
  vec3 coneUp = vec3(0,0,coneSize*1.5-zOff);
  vec3 coneDown = vec3(0,0,-coneSize);
  vec3 color = getHeatMap() == TransitHeatMap ?
    colorRed : colorTransparentWhite;
  //makeCone(mesh, location + vec3(0,0,60-zOff),
      //vec3(0,0,-48), 30, color, true);
  makeCone(mesh, coneUp, coneDown,
      coneSize, color, true);

  if (destBuilding != 0) {
    makeCone(mesh, getBuildingTop(destBuilding) + coneUp - location,
        coneDown, coneSize, colorRed, true);
  }

  placeEntity(entityNdx, location + vec3(0,0,zOff), 0, 0);
  bufferMesh(entity->mesh);
}

