const float waveSize = 250;
const float waveSpeed = 200*24;

#include "global.shader"
#include "ripple.shader"

void main() {
  entityFlags = flags;
  UV = convertUV(vertexUV);
  vec3 normModel = vertexNormal_modelspace;
  z_modelspace = vertexPosition_modelspace.z;

  // Ripples
  vec4 Position_worldspace4 = (M * vec4(vertexPosition_modelspace,1));
  uint textureFlags = texture(flagsTexture, vec2(UV)).r;
  //int windAm = int((textureFlags & _tfWindMask) >> _tfWindShift);
  if (UV.z > 0) {
    float ripple = computeRipple(Position_worldspace4.xyz);
    float movement = -sin(ripple * treeMovement) * UV.z * .25;
    vec3 offset = vec3(movement*sin(time/10), movement*cos(time/10),
        (cos(ripple*treeMovement)-1)*UV.z*.25);
    Position_worldspace4 += vec4(offset,0);
    //Position_worldspace4.x += movement;
    //Position_worldspace4.y -= movement;
    //Position_worldspace4.z -= (1-cos(ripple*treeMovement)) * UV.z;
    normModel += offset*.1f;
  }

  Position_worldspace = Position_worldspace4.xyz;
  gl_Position =  VP * Position_worldspace4;
  if (bool(entityFlags.y & _entityBringToFront)) {
    //gl_Position.z = (VP * vec4(Position_worldspace4.xy, 300, 1)).z;
    //gl_Position.z = 0.01;
    gl_Position.z = (gl_Position.z+1) * 0.01 - 1;
  } else {
    int raiseAmount = int(
        (entityFlags.y & _entityRaiseMask) >> _entityRaiseShift);
    gl_Position.z = (VP * vec4(Position_worldspace +
      vec3(0, 0, 0.1*raiseAmount),1)).z;
  }

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

