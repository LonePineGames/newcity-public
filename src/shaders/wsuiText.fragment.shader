#include "header.fragment.shader"

void main() {
  vec4 albedo = vec4(1,1,1, texture(inTexture, vec2(UV)).r);

  //Lighting
  float lightingPower = float(bool(entityFlags.y & _entityHighlight));

  if (bool(entityFlags.y & _entityRedHighlight)) {
    albedo.rgb = redHighlight;
  } else if (bool(entityFlags.y & _entityBlueHighlight)) {
    //albedo.rgb = blueHighlight; //vec3(0.25,0.25,0.25);
    albedo.rgb = vec3(1.5,1.5,1.5);
  } else if (heatmapData % 256 == 254) { // Transit View
    albedo.rgb = vec3(0.1,0.1,0.1);
    lightingPower *= -1;
  }

  // Text Shadow
  float shadowHits = 0;
  for (int i=0;i<8;i++) {
    vec2 sampleLoc = vec2(UV)+ring[i]*0.005;
    shadowHits += texture(inTexture, sampleLoc).r;
  }
  albedo.rgb = mix(vec3(0,0,0), albedo.rgb, albedo.a);
  albedo.a = max(shadowHits*.25, albedo.a);

  if (albedo.a < 0.1) {
    discard;
  }

  gl_FragDepth = gl_FragCoord.z - albedo.a*.05;
  color = vec4(
    albedo.rgb * shade * LightColor * (1-.5f*lightingPower),
    albedo.a);

  if (bool(entityFlags.y & _entityTransparent)) {
    color.a *= 0.5;
  }
}

