#include "version.shader"

in mat4 M;
in vec3 UV;
in vec3 Position_worldspace;
in vec3 position_modelspace;
in vec3 foregroundColor;
flat in vec4 clip;
flat in uvec2 entityFlags;

layout(location = 0) out vec4 color;

#include "global.shader"
#include "uniforms.shader"

