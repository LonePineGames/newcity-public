#include "header.fragment.shader"

void main() {

  vec4 albedo = vec4(1,1,1, texture(inTexture, vec2(UV)).r);
  if (albedo.a < 0.1) {
    discard;
  }

  //Shadows and shading
  float visibility = getVisibility();

  //Lighting
  float lightingPower = float(bool(entityFlags.y & _entityHighlight));

  //Illumination
  //vec4 illumination = texture(illuminationTexture, vec2(UV)).rgba *
    //int((entityFlags.y & _entityIlluminate) > 0);

  if (bool(entityFlags.y & _entityRedHighlight)) {
    albedo.rgb = redHighlight;
  } else if (bool(entityFlags.y & _entityBlueHighlight)) {
    //albedo.rgb = blueHighlight; //vec3(0.25,0.25,0.25);
    albedo.rgb = vec3(2,2,2);
  } else if (heatmapData % 256 == 254) { // Transit View
    albedo.rgb = vec3(0.02,0.02,0.02);
  }

  color = vec4(
    albedo.rgb * visibility * LightColor +
      lightingPower * vec3(0.6,0.8,1.0) * albedo.rgb,
      //illumination.rgb * illumination.a,
    albedo.a);

  // Hide off-map roads
  float halfMapSize = mapSize*.5f;
  float dropOff = max(abs(Position_worldspace.x - halfMapSize),
      abs(Position_worldspace.y - halfMapSize)) - halfMapSize;
  dropOff = 1 - clamp(dropOff/(tileSize*6), 0, 1);
  color.a *= dropOff;
}
