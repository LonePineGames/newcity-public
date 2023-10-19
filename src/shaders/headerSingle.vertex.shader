#include "version.shader"

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in ivec3 vertexUV;

uniform mat4 M;
uniform uvec2 flags;

#include "headerShared.vertex.shader"

