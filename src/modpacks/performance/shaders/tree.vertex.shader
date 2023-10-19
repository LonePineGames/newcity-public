#include "headerTree.vertex.shader"
#include "global.shader"

void main() {
  entityFlags = flags;
  UV = convertUV(vertexUV);
  vec3 normModel = vertexNormal_modelspace;
  vec3 pos = vertexPosition_modelspace*location.w + location.xyz;
  z_modelspace = pos.z;
  vec4 Position_worldspace4 = (M * vec4(pos,1));

  Position_worldspace = Position_worldspace4.xyz;
  gl_Position =  VP * Position_worldspace4;

  ShadowCoord = DepthBiasVP * Position_worldspace4;
  vec3 Normal_cameraspace = ( V * M * vec4(normModel,0)).xyz;
  vec3 n = normalize( Normal_cameraspace );
  float cosTheta = clamp( dot( n,lightDirection_cameraSpace ), 0,1 );
  shade = clamp(ambientStrength + cosTheta, 0,1);
}

