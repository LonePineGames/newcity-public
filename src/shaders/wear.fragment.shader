#include "header.fragment.shader"

void main(){

  //Albedo
  vec4 albedo = texture(inTexture, vec2(UV)).rgba;
  albedo.a *= (entityFlags.y >> 23) / 255.f;
  if (albedo.a < 0.1) {
    discard;
  }

  //Shadows and shading
  float visibility = getVisibility();

  //Lighting
  float lightingPower = 0;
  for (int i=0;i<8;i++){
    int index = i;
    vec2 offset = poissonDisk[index]/500.0;
    vec2 lightLoc = vec2(LightingCoord.xy + offset);
    lightingPower += .2*texture(lightingTex, lightLoc).r;
  }
  lightingPower += float(bool(entityFlags.y & _entityHighlight));
  int hm = heatmapData % 256;
  lightingPower += float(hm == 255);
  lightingPower = clamp(lightingPower, 0, 1);

  color = vec4(
    albedo.xyz * visibility * LightColor +
      lightingPower * lightingColor * albedo.xyz,
    albedo.a);

  applyEffects();
}
