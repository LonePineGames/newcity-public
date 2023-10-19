#include "header.fragment.shader"

void main(){
  if (heatmapData != 0 && heatmapData%256 != 250) { // Pollution
    color.rgb = vec3(.5f,.5f,.5f);
  } else {
    vec4 albedo = texture(inTexture, vec2(UV)).rgba;
    float visibility = getVisibility();
    color = vec4(albedo.xyz * visibility * LightColor, albedo.a);
    color.rgb *= 0.8;
  }
}
