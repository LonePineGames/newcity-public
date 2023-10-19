#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 3) in mat4 M;

out vec4 lightingOrigin_worldspace;
out float lightingDistance;

uniform mat4 lightingVP;

void main(){
  vec4 lightingPosition_worldspace = M * vec4(vertexPosition_modelspace,1);
  vec4 lightPos4 = lightingVP * lightingPosition_worldspace;
  gl_Position = lightPos4;
  //gl_Position = vec4(lightPos4.xyz / lightPos4.w, 1);
  lightingOrigin_worldspace = M * vec4(vertexNormal_modelspace,1);
  lightingDistance = clamp(length(lightingPosition_worldspace.xyz -
    lightingOrigin_worldspace.xyz)/10, 0, 1);
}

