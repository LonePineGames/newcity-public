#include "global.shader"

void main(){
  //UV = convertUV(vertexUV).xy;

  /*
  vec4 Position_worldspace4 = (M * vec4(0,0,0,1));
  vec3 CameraRight_worldspace = vec3(V[0][0], V[1][0], V[2][0]);
  vec3 CameraUp_worldspace = vec3(V[0][1], V[1][1], V[2][1]);
  Position_worldspace4.xyz -=
    CameraRight_worldspace * vertexPosition_modelspace.x
    + CameraUp_worldspace * vertexPosition_modelspace.y;

  gl_Position =  depthVP * Position_worldspace4;
  */
}

