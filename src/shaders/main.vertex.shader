#include "global.shader"

void main() {
  entityFlags = flags;
  UV = convertUV(vertexUV);
  z_modelspace = vertexPosition_modelspace.z;
  position_modelspace = vertexPosition_modelspace;

  mat4 MVP = VP * M;
  gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
    //gl_Position.z = (MVP * vec4(vertexPosition_modelspace.xy, 300, 1)).z;
  int raiseAmount = int(
    (entityFlags.y & _entityRaiseMask) >> _entityRaiseShift);
  if (raiseAmount > 0) {
    gl_Position.z = (MVP * vec4(vertexPosition_modelspace +
      vec3(0, 0, 0.25*raiseAmount),1)).z;
  }

  if (bool(entityFlags.y & _entityBringToFront)) {
    gl_Position.z = (gl_Position.z+1) * 0.01 - 1;
  }

  vec4 Position_worldspace4 = (M * vec4(vertexPosition_modelspace,1));
  Position_worldspace = Position_worldspace4.xyz;
  ShadowCoord = DepthBiasVP * Position_worldspace4;
  vec4 LightingCoord4 = (lightingBiasVP * Position_worldspace4);
  LightingCoord = LightingCoord4.xy / LightingCoord4.w;
  normal_cameraspace = normalize((V * M * vec4(vertexNormal_modelspace,0)).xyz);
  float cosTheta = clamp( dot(normal_cameraspace,
        lightDirection_cameraSpace )+1, 0,2 );
  shade = cosTheta; //clamp(ambientStrength + cosTheta*.5f, 0,1);

  vec3 dir = Position_worldspace - cameraPosition_worldSpace;
  cameraDistance = dir.x*dir.x + dir.y*dir.y + dir.z*dir.z;
}

