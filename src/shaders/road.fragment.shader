#include "header.fragment.shader"

void main() {
  int hm = heatmapData % 256;

  //Albedo
  vec4 albedo = texture(inTexture, vec2(UV)).rgba;

  // Hide off-map roads
  if (hm != 252 && hm !=254 && hm != 255) {
    float halfMapSize = mapSize*.5f;
    float dropOff = max(abs(Position_worldspace.x - halfMapSize),
        abs(Position_worldspace.y - halfMapSize)) - halfMapSize;
    dropOff = 1 - clamp(dropOff/(tileSize*100), 0, 1);
    albedo.a *= dropOff*dropOff;
  }

  if (albedo.a < 0.01) discard;

  if (hm == 252) { // Road Map
    int colorIndex = int(entityFlags.x);
    colorIndex %= 32;
    color = texture(paletteTexture, getColorInPalette(colorIndex));
    color = mix(color, vec4(1,1,1,1), colorIndex > 28 ? 0.35 : 0);
    //color *= 1.5;
    return;
  }

  //Shadows and shading
  float visibility = getVisibility();
  //uint textureFlags = texture(flagsTexture, vec2(UV)).r;
  //int shineAm = int((textureFlags & _tfShineMask) >> _tfShineShift);
  float shine = 0; //getShine(100.f, 0.5f + rainAmount*rainAmount) * visibility;
  //shine *= clamp(10000000.f/cameraDistance-0.1f, 0.f, 1.f);

  //Lighting
  float lightingPower = 0;
  if (UV.z > 0.1) {
    for (int i=0;i<8;i++){
      int index = i;
      vec2 offset = poissonDisk[index]/500.0*(1+rainAmount);
      vec2 lightLoc = vec2(LightingCoord.xy + offset);
      if (lightLoc.x < -1 || lightLoc.x > 1 ||
          lightLoc.y < -1 || lightLoc.y > 1) {
      } else {
        lightingPower += .25*texture(lightingTex, lightLoc).r;
      }
    }
  }

  lightingPower = pow(lightingPower,2);
  lightingPower *= illuminationStrength;
  lightingPower += float(bool(entityFlags.y & _entityHighlight));
  lightingPower = clamp(lightingPower, 0, 1);

  color = vec4(
    (albedo.rgb + shine) * visibility * LightColor,
    albedo.a);

  if (hm == 255) { // Traffic
    float heatMapA = clamp((entityFlags.x >> 16)/255.f, 0, 1);
    //vec3 heatColor = getHeatmapColor(255, heatMapA);
    vec3 heatColor = texture(paletteTexture, vec2(
      33/paletteSize,
      (9-8*heatMapA)/paletteSize)).rgb;
    //heatColor = heatMapA < 0.5 ?
      //mix(trafficColor0, trafficColor1, heatMapA*2) :
      //mix(trafficColor1, trafficColor2, heatMapA*2-1);
    color.rgb = 2.f*getValue(color.rgb)*heatColor;
    color.rgb += lightingPower * lightingColor * albedo.rgb;

  } else {
    color.rgb += lightingPower * lightingColor * albedo.rgb;
    applyEffects();
  }
}
