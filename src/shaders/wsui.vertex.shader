#include "global.shader"

void main() {
  entityFlags = flags;
  UV = convertUV(vertexUV);
  z_modelspace = vertexPosition_modelspace.z;
  vec4 Position_worldspace4 = (M * vec4(0,0,0,1));

  vec3 CameraRight_worldspace = vec3(V[0][0], V[1][0], V[2][0]);
  vec3 CameraUp_worldspace = vec3(V[0][1], V[1][1], V[2][1]);

  Position_worldspace4.xyz -=
    CameraRight_worldspace * vertexPosition_modelspace.x
    + CameraUp_worldspace * vertexPosition_modelspace.y;

  Position_worldspace = Position_worldspace4.xyz;
  gl_Position =  VP * Position_worldspace4;
  Position_worldspace4.z += vertexPosition_modelspace.z;

  if (bool(entityFlags.y & _entityBringToFront)) {
    //gl_Position.z = (VP * vec4(Position_worldspace4.xy, 300, 1)).z;
    gl_Position.z = (gl_Position.z+1) * 0.01 - 1;
  } else {
    int raiseAmount = int(
        (entityFlags.y & _entityRaiseMask) >> _entityRaiseShift);
    gl_Position.z = (VP * vec4(Position_worldspace4.xyz +
      vec3(0, 0, 0.1*raiseAmount),1)).z;
  }

  //shade = clamp(ambientStrength + 0.5, 0,1);
  shade = 0.5; //clamp(ambientStrength + cosTheta*.5f, 0,1);

  vec3 dir = Position_worldspace - cameraPosition_worldSpace;
  cameraDistance = dir.x*dir.x + dir.y*dir.y + dir.z*dir.z;
}

