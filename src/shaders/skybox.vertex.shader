#include "headerSingle.vertex.shader"
#include "global.shader"

void main(){
  UV = convertUV(vertexUV);
  position_modelspace = vertexPosition_modelspace;
  gl_Position =  VP * vec4(vertexPosition_modelspace, 1);
  gl_Position.z = 0.5;
}

