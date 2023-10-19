#include "header.fragment.shader"

void main(){

  //Albedo
  vec4 albedo = texture(inTexture, vec2(UV)).rgba;
  if (albedo.a < 0.1) discard;

  //Shadows and shading
  float visibility = getVisibility();

  color = vec4(albedo.rgb * LightColor * visibility, albedo.a);
  applyHeatmap();
}

