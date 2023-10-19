#include "headerSingle.vertex.shader"

//const float waveSize = 500;
//const float waveSpeed = 100*24;

#include "global.shader"
//#include "ripple.shader"

void main() {
  entityFlags = flags;
  UV = convertUV(vertexUV);

  mat4 MVP = VP * M;
  gl_Position =  MVP * vec4(vertexPosition_modelspace,1);

  /*
  if (bool(entityFlags.y & _entityBringToFront)) {
    gl_Position.z = (gl_Position.z+1) * 0.01 - 1;
  } else {
    int raiseAmount = (entityFlags.y & _entityRaiseMask) >> _entityRaiseShift;
    gl_Position.z = (MVP * vec4(vertexPosition_modelspace +
      vec3(0, 0, 0.1*raiseAmount),1)).z;
  }
  */

  vec4 Position_worldspace4 = (M * vec4(vertexPosition_modelspace,1));
  Position_worldspace = Position_worldspace4.xyz;
  //position_cameraspace = (V * Position_worldspace4).xyz;
  ShadowCoord = DepthBiasVP * Position_worldspace4;
  vec3 Normal_cameraspace = ( V * M * vec4(vertexNormal_modelspace,0)).xyz;
  vec3 n = normalize( Normal_cameraspace );
  float cosTheta = clamp( dot(n, lightDirection_cameraSpace )+1, 0,2 );
  shade = cosTheta; //clamp(ambientStrength + cosTheta*.5f, 0,1);

  vec3 dir = Position_worldspace - cameraPosition_worldSpace;
  cameraDistance = dir.x*dir.x + dir.y*dir.y + dir.z*dir.z;

  vec4 LightingCoord4 = (lightingBiasVP * Position_worldspace4);
  LightingCoord = LightingCoord4.xy / LightingCoord4.w;

  // Ripples
  /*
  const float waveIntensity = 0.05;
  float ripple = computeRipple(Position_worldspace, 1);
  shade += ripple * waveIntensity;
  shade = clamp(shade, 0.2, 1);
  */
}

