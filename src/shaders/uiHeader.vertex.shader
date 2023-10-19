#include "version.shader"
#include "global.shader"
#include "uniforms.shader"

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 2) in ivec3 vertexUV;

uniform mat4 M;
uniform uvec2 flags;

out vec3 UV;
out vec3 Position_worldspace;
out vec3 position_modelspace;
out vec3 foregroundColor;
flat out uvec2 entityFlags;
flat out vec4 clip;

