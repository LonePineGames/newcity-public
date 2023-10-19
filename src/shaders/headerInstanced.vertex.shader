#include "version.shader"

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in ivec3 vertexUV;
layout(location = 3) in mat4 M;
layout(location = 7) in uvec2 flags;

#include "headerShared.vertex.shader"

