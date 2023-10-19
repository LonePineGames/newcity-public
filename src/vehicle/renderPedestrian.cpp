#include "renderPedestrian.hpp"

#include "../color.hpp"
#include "../draw/entity.hpp"
#include "../draw/mesh.hpp"
#include "../draw/texture.hpp"
#include "../renderUtils.hpp"

#include "travelGroup.hpp"

const int numPedColors = 5;
const vec3 pedColors[numPedColors] = {
  colorWhite, colorGray, colorWaterBlue, colorRed, colorDarkGray
};

Cup<item> groupEntity_g;
item groupMesh_g[numPedColors];

void resetPedestrianRender_g() {
  for (int i = 0; i < numPedColors; i++) {
    groupMesh_g[i] = 0;
  }
  groupEntity_g.clear();
}

void updatePedestrianRender_g(item groupNdx) {
  item color = groupNdx % numPedColors;
  if (groupMesh_g[color] == 0) {
    groupMesh_g[color] = addMesh();
    Mesh* mesh = getMesh(groupMesh_g[color]);
    makeCylinder(mesh, vec3(0,0,0), 2,
      0.25, 6, pedColors[color]);
    bufferMesh(groupMesh_g[color]);
  }

  TravelGroup* group = getTravelGroup_g(groupNdx);
  item locType = locationType(group->location);
  bool show = locType == LocTransitStop || locType == LocPedestrian;
  show = show && (group->flags & _groupExists);
  groupEntity_g.ensureSize(groupNdx+1);
  item entityNdx = groupEntity_g[groupNdx];

  if (!show) {
    if (entityNdx > 0) {
      removeEntity(entityNdx);
      groupEntity_g.set(groupNdx, 0);
    }
    return;

  } else if (entityNdx <= 0) {
    entityNdx = addEntity(PaletteShader);
    groupEntity_g.set(groupNdx, entityNdx);
    Entity* entity = getEntity(entityNdx);
    entity->texture = paletteTexture;
    entity->mesh = groupMesh_g[color];
    entity->flags |= _entityTransit;
    setCull(entityNdx, 0, 2000);
  }

  vec3 loc = locationToWorldspace_g(group->location);
  loc += vec3(groupNdx%10-5, (groupNdx/10)%10-5, 0) * .5f;
  placeEntity(entityNdx, loc, 0, 0);
}

