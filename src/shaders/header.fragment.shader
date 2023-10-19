#include "version.shader"

in vec3 UV;
in vec4 ShadowCoord;
in vec2 LightingCoord;
in vec3 Position_worldspace;
in vec3 normal_cameraspace;
in float shade;
in float z_modelspace;
in float cameraDistance;
in vec3 position_modelspace;
flat in uvec2 entityFlags;

layout(location = 0) out vec4 color;

#include "uniforms.shader"
#include "global.fragment.shader"
#include "heatmap.fragment.shader"
#include "shine.fragment.shader"

