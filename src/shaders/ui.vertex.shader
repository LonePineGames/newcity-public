#include "uiHeader.vertex.shader"

void main() {
  clip = vec4(M[0][0], M[0][1], M[0][2], M[0][3]);
  UV = convertUV(vertexUV);
  entityFlags = flags;

  position_modelspace = vertexPosition_modelspace;
  position_modelspace.y += M[1][0];
  Position_worldspace = vertexPosition_modelspace;
  gl_Position =  VP * vec4(Position_worldspace,1);

  int fColorIndex = int(entityFlags.x);
  foregroundColor = texture(paletteTexture, getColorInPalette(fColorIndex)).rgb;
}

