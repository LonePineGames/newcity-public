#include "headerSingle.vertex.shader"
#include "global.shader"

uniform vec3 rainPos;
const float skyboxSize = 500;

void main(){
  entityFlags = flags;
  vec3 adjRainPos = rainPos;
  adjRainPos.x *= 4;
  adjRainPos.y *= 0.1;
  vec3 vertexUVf = convertUV(vertexUV);
  UV = vec3(
      vertexUVf.x+adjRainPos.x,
      vertexUVf.y+adjRainPos.y*vertexUV.z,
      vertexUVf.z);

  gl_Position =  VP * vec4(vertexPosition_modelspace, 1);
}

