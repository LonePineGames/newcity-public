#include "headerSingle.vertex.shader"
#include "global.shader"

void main() {
  entityFlags = flags;
  UV = convertUV(vertexUV);

  mat4 MVP = VP * M;
  vec4 Position_worldspace4 = (M * vec4(vertexPosition_modelspace,1));
  Position_worldspace = Position_worldspace4.xyz;
  normal_cameraspace = normalize((V * M * vec4(vertexNormal_modelspace,0)).xyz);
  Position_worldspace = Position_worldspace4.xyz;
  gl_Position =  VP * Position_worldspace4;
  ShadowCoord = DepthBiasVP * Position_worldspace4;

  float cosTheta = clamp(dot(normal_cameraspace,
        lightDirection_cameraSpace), 0, 1);
  shade = clamp(ambientStrength + cosTheta, 0, 1);
}


