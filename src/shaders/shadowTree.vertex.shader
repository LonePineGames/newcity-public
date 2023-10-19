#include "headerTree.vertex.shader"

const float waveSize = 240;
const float waveSpeed = 60*24;

uniform mat4 depthV;

#include "global.shader"
#include "ripple.shader"

void main(){
  entityFlags = flags;
  UV = convertUV(vertexUV);

  vec3 vert = vertexPosition_modelspace;
  /*
  vert.xy = gl_InstanceID%2==0 ? vert.xy : vec2(vert.y, -vert.x);
  vert.xy = gl_InstanceID%4<2 ? vert.xy : -vert.xy;
  */

  if (bool(flags.y & _entityBillboard)) {
    vec3 CameraRight_worldspace = vec3(depthV[0][0], depthV[1][0], depthV[2][0]);
    //vec3 CameraUp_worldspace = vec3(V[0][1], V[1][1], V[2][1]);
    vert.xy = -CameraRight_worldspace.xy*vert.x;
      //+ CameraUp_worldspace * vert.z;
  }

  vec3 pos = vert*location.w + location.xyz;
  vec4 Position_worldspace4 = M * vec4(pos,1);
  z_modelspace = vert.z*location.w;

  if (bool(flags.y & _entityWind) && z_modelspace > 0.01) {
    float ripple = computeRipple(Position_worldspace4.xyz +
        vec3(z_modelspace, z_modelspace, 0));
    float movement = -sin(ripple * treeMovement) * z_modelspace;
    vec3 offset = vec3(movement*sin(time/10), movement*cos(time/10),
        (cos(ripple*treeMovement)-1)*movement);
    Position_worldspace4 += vec4(offset,0);
  }

  gl_Position =  depthVP * Position_worldspace4;
}

