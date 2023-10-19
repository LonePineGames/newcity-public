#include "item.hpp"

item renderVirtualLot(vec3 loc, vec3 normal, item zone, item density,
    bool highlight);
void renderLotAs(item ndx, item zone, item density, bool highlight);
void renderLot(item ndx);
item getLotMesh(item zone, item density);
void resetLotRender();
void setLotHighlight(item ndx, bool highlight);

