const float waveSize = 240;
const float waveSpeed = 60*24;

#include "global.shader"
#include "ripple.shader"

void main(){
  UV = convertUV(vertexUV);
  vec4 Position_worldspace4 = M * vec4(vertexPosition_modelspace,1);

  if (UV.z > 0) { //bool(flags & _entityWind) && vertexUV.z > 0.01) {
    float ripple = computeRipple(Position_worldspace4.xyz);
    float movement = -sin(ripple * treeMovement) * UV.z * .25;
    vec3 offset = vec3(movement*sin(time/10), movement*cos(time/10),
        (cos(ripple*treeMovement)-1)*movement);
    Position_worldspace4 += vec4(offset,0);
  }

  gl_Position =  depthVP * Position_worldspace4;
}

