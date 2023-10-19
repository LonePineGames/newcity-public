
out vec3 UV;
out float shade;
out float z_modelspace;
out float cameraDistance;
out vec4 ShadowCoord;
out vec2 LightingCoord;
out vec3 normal_cameraspace;
out vec3 Position_worldspace;
out vec3 position_modelspace;
flat out uvec2 entityFlags;

#include "uniforms.shader"

