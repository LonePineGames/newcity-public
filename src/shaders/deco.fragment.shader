#include "header.fragment.shader"

void main(){
  //Albedo
  vec4 albedo = texture(inTexture, vec2(UV)).rgba;
  if (objectViewer && z_modelspace < 0) {
    albedo.a = clamp(1 + z_modelspace*10, 0, 1);
  }

  uint textureFlags = texture(flagsTexture, vec2(UV)).r;
  bool abandoned = bool(entityFlags.y & _entityHeatmapLimited);
  if (abandoned) {
    albedo.rgb = getValue(albedo.rgb) * abandonedBuildingColor;
  //} else if (textureFlags == 0u) { //bool(textureFlags & _tfSnow)) {
    //albedo.rgb = mix(albedo.rgb, snowColor, snow);
  }

  //Shadows and shading
  float visibility = getVisibility();

  int shineAm = int((textureFlags & _tfShineMask) >> _tfShineShift);
  float shine = shineAm == 0 ? 0 :
    getShine(shineAm, 0.1f + rainAmount * rainAmount) * visibility;

  //Lighting
  float lightingPower = float(bool(entityFlags.y & _entityHighlight));

  //Illumination (visibility, lightingPower)
  vec4 illumination = texture(illuminationTexture, vec2(UV)).rgba *
    float(bool(entityFlags.y & _entityIlluminate));
  illumination.rgb *= illumination.a * (illuminationStrength+.2f);

  color = vec4(
    ((albedo.rgb + shine) * visibility * LightColor +
      lightingPower * vec3(0.6f,0.9f,1.0f) * albedo.rgb) * albedo.a +
    illumination.rgb,
    max(albedo.a, illumination.a * illuminationStrength));

  if (color.a < 0.1) discard;

  applyEffects();
}

