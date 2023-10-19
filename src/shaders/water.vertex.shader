#include "headerSingle.vertex.shader"

const float waveSize = 25;
const float waveSpeed = 200*24;

#include "global.shader"
#include "ripple.shader"

void main() {
  entityFlags = flags;
  UV = convertUV(vertexUV);

  mat4 MVP = VP * M;
  vec4 Position_worldspace4 = (M * vec4(vertexPosition_modelspace,1));
  Position_worldspace = Position_worldspace4.xyz;
  normal_cameraspace = normalize((V * M * vec4(vertexNormal_modelspace,0)).xyz);

  int hm = heatmapData % 256;
  float hmFactor = float(hm <= 250 && hm != 255);
  float rippleFactor = float(Position_worldspace.z > -0.1) * hmFactor;
  float ripple = computeRipple(Position_worldspace, 4);
  float ripple1 = computeRipple(Position_worldspace+vec3(0,2,0), 4);
  float ripple2 = computeRipple(Position_worldspace+vec3(2,0,0), 4);

  const float waveHeight = 0; //.1;
  Position_worldspace4.z += ripple * waveHeight * rippleFactor;
    //+ tide * rippleFactor;
  normal_cameraspace = normalize(normal_cameraspace +
      vec3(ripple1-ripple,ripple2-ripple,0)*(5.f*hmFactor));

  float cosTheta = clamp(dot(normal_cameraspace,
        lightDirection_cameraSpace)*.15+.25, 0.2, 1);
  shade = cosTheta * rippleFactor + 1;
  //shade = clamp((ambientStrength + cosTheta)*rippleFactor, 0,1);
  //shade += Position_worldspace4.z * 0.1 + 0.4;
    //mix(Position_worldspace4.z * 0.005,
      //ripple * waveIntensity, rippleFactor);

  Position_worldspace = Position_worldspace4.xyz;
  gl_Position =  VP * Position_worldspace4;
  ShadowCoord = DepthBiasVP * Position_worldspace4;
  //LightingCoord = (lightingBiasVP * Position_worldspace4).xy;

  vec3 dir = Position_worldspace - cameraPosition_worldSpace;
  cameraDistance = dir.x*dir.x + dir.y*dir.y + dir.z*dir.z;
}

