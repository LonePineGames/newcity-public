#include "header.fragment.shader"

void main() {

  vec4 albedo = texture(inTexture, vec2(UV)).rgba;
  if (albedo.a < 0.1) {
    discard;
  }

  //Shadows and shading
  float visibility = getVisibility();

  //Lighting
  float lightingPower = float(bool(entityFlags.y & _entityHighlight));

  //Illumination
  vec4 illumination = texture(illuminationTexture, vec2(UV)).rgba *
    float(bool(entityFlags.y & _entityIlluminate));

  color = vec4(
    albedo.rgb * visibility * LightColor +
      lightingPower * vec3(0.6,0.8,1.0) * albedo.rgb +
      illumination.rgb * illumination.a,
    albedo.a);

  applyEffects();
}
