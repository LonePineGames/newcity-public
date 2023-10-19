#include "header.fragment.shader"

void main(){

  //Albedo
  vec4 albedo = texture(inTexture, vec2(UV)).rgba;

  //Shadows and shading
  float visibility = getVisibility();

  //Lighting
  float lightingPower = 0;
  lightingPower += float(bool(entityFlags.y & _entityHighlight));
  lightingPower = clamp(lightingPower, 0, 1);

  color = vec4(
    albedo.xyz * visibility * LightColor +
      lightingPower * lightingColor * albedo.xyz,
    albedo.a);

  int hm = heatmapData % 256;
  if (hm != 253) { // Zones
    applyEffects();
  }
}

