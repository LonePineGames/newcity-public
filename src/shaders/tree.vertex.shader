#include "headerTree.vertex.shader"

const float waveSize = 250;
const float waveSpeed = 200*24;

#include "global.shader"
#include "ripple.shader"

void main() {
  entityFlags = flags;
  UV = convertUV(vertexUV);
  vec3 normModel = vertexNormal_modelspace;

  vec3 vert = vertexPosition_modelspace;
  /*
  vert.xy = gl_InstanceID%2==0 ? vert.xy : vec2(vert.y, -vert.x);
  normModel.xy = gl_InstanceID%2==0 ? normModel.xy :
    vec2(normModel.y, -normModel.x);
  vert.xy = gl_InstanceID%4<2 ? vert.xy : -vert.xy;
  normModel.xy = gl_InstanceID%4<2 ? normModel.xy : -normModel.xy;
  */

  if (bool(flags.y & _entityBillboard)) {
    vec3 CameraRight_worldspace = vec3(V[0][0], V[1][0], V[2][0]);
    //vec3 CameraUp_worldspace = vec3(V[0][1], V[1][1], V[2][1]);
    vert.xy = -CameraRight_worldspace.xy*vert.x;
      //+ CameraUp_worldspace * vert.z;
  }

  vec3 pos = vert*location.w + location.xyz;
  z_modelspace = vert.z*location.w;
  vec4 Position_worldspace4 = (M * vec4(pos,1));

  // Ripples
  if (bool(flags.y & _entityWind) && z_modelspace > 0.01) {
    float ripple = computeRipple(Position_worldspace4.xyz +
        vec3(z_modelspace, z_modelspace, 0))*.5;
    float movement = -sin(ripple * treeMovement) * z_modelspace;
    vec3 offset = vec3(movement*sin(time/10), movement*cos(time/10),
        (cos(ripple*treeMovement)-1)*z_modelspace);
    Position_worldspace4 += vec4(offset,0);
    normModel += offset*.1f;
  }

  Position_worldspace = Position_worldspace4.xyz;
  gl_Position =  VP * Position_worldspace4;
  ShadowCoord = DepthBiasVP * Position_worldspace4;
  normal_cameraspace = normalize((V * M * vec4(normModel,0)).xyz);
  float cosTheta = clamp( dot(normal_cameraspace,
        lightDirection_cameraSpace )+1, 0,2 );
  shade = cosTheta; //clamp(ambientStrength + cosTheta*.5f, 0,1);
}

