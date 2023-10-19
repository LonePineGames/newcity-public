#include "version.shader"

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in ivec3 vertexUV;
layout(location = 3) in vec4 location;

uniform mat4 M;
uniform float zOffset;
uniform uvec2 flags;

#include "headerShared.vertex.shader"

