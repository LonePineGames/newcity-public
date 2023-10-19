#include "header.fragment.shader"

void main(){

  //Albedo
  vec4 albedo = texture(inTexture, vec2(UV)).rgba;
  bool abandoned = bool(entityFlags.y & _entityHeatmapLimited);
  if (abandoned) {
    albedo.rgb = getValue(albedo.rgb) * abandonedBuildingColor;
  } else {
    albedo.rgb = mix(albedo.rgb, snowColor, UV.z*snow);
  }

  if (objectViewer && z_modelspace < 0) {
    albedo.a = clamp(1 + z_modelspace*10, 0, 1);
  }
  if (albedo.a < 0.1) discard;

  vec2 visLightPow = vec2(getVisibility(),
      float(bool(entityFlags.y & _entityHighlight)));

  //Illumination (visibility, lightingPower)
  vec4 illumination = texture(illuminationTexture, vec2(UV)).rgba *
    float(bool(entityFlags.y & _entityIlluminate));
  illumination.rgb *= illumination.a * illuminationStrength;

  color = vec4(
    albedo.rgb * visLightPow.x * LightColor +
      visLightPow.y * vec3(0.6f,0.9f,1.0f) * albedo.rgb +
      illumination.rgb,
    albedo.a);

  applyEffects();
}

