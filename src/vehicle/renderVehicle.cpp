#include "renderVehicle.hpp"

#include "model.hpp"
#include "vehicle.hpp"

#include "../draw/entity.hpp"
#include "../draw/texture.hpp"
#include "../import/mesh-import.hpp"
#include "../graph/transit.hpp"
#include "../renderUtils.hpp"
#include "../string.hpp"

#include "spdlog/spdlog.h"

item vehicleLightMesh;

void initVehicleMeshes() {
  vehicleLightMesh = addMesh();
}

void renderVehicleStyles() {
  Mesh* lightMesh = getMesh(vehicleLightMesh);
  makeLightCone(lightMesh, vec3( 1,2.5,1), vec3(0,c(CHeadlightSizeY),0),
      c(CHeadlightSizeZ));
  makeLightCone(lightMesh, vec3(-1,2.5,1), vec3(0,c(CHeadlightSizeY),0),
      c(CHeadlightSizeZ));
  bufferMesh(vehicleLightMesh);
}

void renderVehicle(item ndx) {
  Vehicle* v = getVehicle(ndx);
  if (!(v->flags & _vehicleExists)) return;

  if (v->entity == 0) {
    v->entity = addEntity(VehicleShader);
  }
  if (v->trailing == 0 && v->lightEntity == 0) {
    v->lightEntity = addEntity(LightingShader);
  }

  bool transit = v->flags & _vehicleIsTransit;
  bool wanderer = v->flags & _vehicleWanderer;
  Entity* entity = getEntity(v->entity);
  setEntityIlluminated(v->entity, true);
  entity->texture = paletteTexture;
  float cullDist = transit ? c(CTransitVehicleCull) :
    wanderer ? c(CWandererCull) :
    c(CVehicleCull);
  setCull(v->entity, 20, cullDist);
  entity->simpleDistance = c(CVehicleCullSimplify);
  item modelNdx = v->model;
  if (modelNdx > vehicleModelsSize() || modelNdx <= 0) {
    SPDLOG_WARN("bad vehicle model {} {}", ndx, modelNdx);
    logStacktrace();
    modelNdx = 0;
  }

  uint32_t color = 0;
  if (transit) {
    TransitLine* tl = getTransitLine(v->transitLine);
    color = getTransitLineColor(v->transitLine);
    TransitSystem* sys = getTransitSystem(tl->system);
    color |= sys->color[1] << 6;
  } else {
    color = vehicleColor(v) % numVehicleColors;
    color |= color << 6;
  }
  entity->dataFlags = color;

  if (modelNdx != 0) {
    VehicleModel* model = getVehicleModel(modelNdx);
    assignMeshImport(v->entity, model->meshImport);
    setEntityBlueHighlight(v->entity, model->flags & _modelColorized);
  }

  if (v->lightEntity != 0) {
    Entity* lightEntity = getEntity(v->lightEntity);
    setEntityVisible(v->lightEntity, false);
    lightEntity->mesh = vehicleLightMesh;
    setCull(v->lightEntity, 20, c(CHeadlightCullDistance));
  }

  if (v->transitLine != 0) {
    renderVehicleNumber(ndx);
  }
}

void renderVehicleNumber(item vNdx) {
  Vehicle* v = getVehicle(vNdx);
  if (!(v->flags & _vehicleIsTransit)) return;
  if (!(v->flags & _vehicleExists)) return;
  if (v->transitLine == 0) return;
  if (v->trailing != 0) return;

  if (!isTransitVisible()) {
    if (v->numberEntity != 0) {
      setEntityVisible(v->numberEntity, false);
      setEntityVisible(v->numberShadowEntity, false);
    }
    return;
  }

  //if (v->numPassengers == v->lastNumberRendered) return;
  VehicleModel* model = getVehicleModel(v->model);
  item cap = model->passengers;
  item num = v->numPassengers;

  float cars = 1;
  Vehicle* t = v;
  while (t->trailer) {
    t = getVehicle(t->trailer);
    VehicleModel* m = getVehicleModel(t->model);
    cars ++;
    cap += m->passengers;
    num += t->numPassengers;
  }

  bool halfCap = num > cap*.5f;
  v->lastNumberRendered = num;

  if (v->numberEntity == 0) {
    v->numberEntity = addEntity(WSUITextShader);
    v->numberShadowEntity = addEntity(WSUIShader);
  }

  TransitLine* line = getTransitLine(v->transitLine);
  /*
  bool isInLine = false;
  for (int i = 0; i < line->vehicles.size(); i++) {
    if (line->vehicles[i] == vNdx) {
      isInLine = true;
      break;
    }
  }

  if (!isInLine) {
    line->vehicles.push_back(vNdx);
  }
  */

  Entity* numberEntity = getEntity(v->numberEntity);
  setEntityVisible(v->numberEntity, true);
  setCull(v->numberEntity, 50, c(CHeadlightCullDistance));
  numberEntity->texture = textTexture;
  numberEntity->flags |= _entityTransit;
  setEntityVisible(v->numberEntity, true);
  setEntityRedHighlight(v->numberEntity, halfCap);
  setEntityBlueHighlight(v->numberEntity, !halfCap);
  setEntityRaise(v->numberEntity, 1);

  Entity* numberShadowEntity = getEntity(v->numberShadowEntity);
  setCull(v->numberShadowEntity, 50, c(CHeadlightCullDistance));
  numberShadowEntity->texture = paletteTexture;
  numberShadowEntity->flags |= _entityTransit;
  setEntityVisible(v->numberShadowEntity, true);

  createMeshForEntity(v->numberEntity);
  createMeshForEntity(v->numberShadowEntity);
  Mesh* m = getMeshForEntity(v->numberEntity);
  Mesh* ms = getMeshForEntity(v->numberShadowEntity);

  float fontSize = 40;
  char* nStr = sprintf_o("%d", num);
  char* cStr = sprintf_o("/%d", cap);
  char* lStr = line->name;
  float nStrLng = fontSize*(stringWidth(nStr)+.125f);
  float cStrLng = .5f*fontSize*(stringWidth(cStr)+.25f);
  float lStrLng = .25f*fontSize*(stringWidth(lStr)+.5f);
  float rStrLng = std::max(cStrLng, lStrLng);
  float strLng = nStrLng + rStrLng;
  float right = strLng*.5f;
  float left = -right;
  vec3 start = vec3(left,-fontSize,0);
  vec3 tr = start;
  vec3 tl = tr + vec3(strLng,0,0);
  vec3 br = tr + vec3(0,fontSize,0);
  vec3 bl = tl + vec3(0,fontSize,0);
  vec3 nStart = vec3(left+fontSize*.125f, -fontSize, 0);
  vec3 cStart = vec3(left+nStrLng, -fontSize*.625f, 0);
  vec3 lStart = vec3(right-lStrLng, -fontSize*.875f, 0);

  renderString(m, nStr, nStart, fontSize);
  renderString(m, cStr, cStart, fontSize*.5f);
  renderString(m, lStr, lStart, fontSize*.25f);
  vec3 color = getTransitLineColorInPalette(v->transitLine);
  makeQuad(ms, tr, tl, br, bl, color, color);
  bufferMeshForEntity(v->numberEntity);
  bufferMeshForEntity(v->numberShadowEntity);
  free(nStr);
  free(cStr);
}

void setVehicleHighlight(item ndx, bool highlight) {
  Vehicle* vehicle = getVehicle(ndx);
  //while (vehicle->trailing != 0) {
    //vehicle = getVehicle(vehicle->trailing);
  //}

  if (vehicle->entity != 0) {
    setEntityHighlight(vehicle->entity, highlight);
  }

  //while (vehicle->trailer != 0) {
    //vehicle = getVehicle(vehicle->trailer);
    //if (vehicle->entity != 0) {
      //setEntityHighlight(vehicle->entity, highlight);
    //}
  //}
}

void resetVehicleRender() {
  vehicleLightMesh = 0;
}

