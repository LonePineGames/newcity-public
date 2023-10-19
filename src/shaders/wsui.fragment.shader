#include "header.fragment.shader"

void main() {
  vec4 albedo = texture(inTexture, vec2(UV)).rgba;
  if (albedo.a < 0.1) {
    discard;
  }

  //Lighting
  float lightingPower = float(bool(entityFlags.y & _entityHighlight));

  color = vec4(
    albedo.rgb * shade * LightColor +
      lightingPower * vec3(0.6,0.8,1.0) * albedo.rgb,
    albedo.a);

  //if (heatmapData % 256 == 254 && color.a > .99) { // transit
    //color.rgb = vec3(0,0,0);
  //}

  if (bool(entityFlags.y & _entityRedHighlight)) {
    color.rgb = getValue(color.rgb)*redHighlight;
  } else if (bool(entityFlags.y & _entityBlueHighlight)) {
    color.rgb = getValue(color.rgb)*blueHighlight;
  }

  if (bool(entityFlags.y & _entityTransparent)) {
    color.a = 0.5;
  }
}

