#include "header.fragment.shader"

void main(){
  //Albedo
  vec4 texColor = texture(inTexture, vec2(UV)).rgba;
  //if (texColor.a < 0.01) { discard; }
  vec4 albedo;
  float m = max(texColor.r, min(texColor.g, texColor.b));
  if (bool(entityFlags.y & _entityBlueHighlight) && m < 0.1) {
    int bColorIndex = int(entityFlags.x);
    int gColorIndex = bColorIndex >> 6;
    bColorIndex %= 64;
    //bColorIndex &= (1 << 6)-1;
    vec3 bColor = texture(paletteTexture, getColorInPalette(bColorIndex)).rgb;
    vec3 gColor = texture(paletteTexture, getColorInPalette(gColorIndex)).rgb;
    albedo = vec4(bColor * texColor.b, texColor.a);
    albedo.rgb += gColor * texColor.g;
    //albedo.rgb = pow(albedo.rgb, vec3(2.2)); // Gamma correction
  } else {
    albedo = texColor;
  }

  /*
  // Hide off-map roads
  float halfMapSize = mapSize*.5f;
  float dropOff = max(abs(Position_worldspace.x - halfMapSize),
      abs(Position_worldspace.y - halfMapSize)) - halfMapSize;
  dropOff = 1 - clamp(dropOff/(tileSize*6), 0, 1);
  albedo.a *= dropOff*dropOff;
  */

  //Shadows and shading
  float visibility = getVisibility();
  float shine = getShine(2, 0.5) * visibility;
  shine *= clamp(2*(texColor.r + texColor.g + texColor.b), 0, 1);

  //Lighting
  float lightingPower = 0;
  lightingPower += float(bool(entityFlags.y & _entityHighlight));
  lightingPower = clamp(lightingPower, 0, 1);

  //Illumination
  vec4 illumination = texture(illuminationTexture, vec2(UV)).rgba *
    float(bool(entityFlags.y & _entityIlluminate));
  illumination.rgb *= illumination.a * illuminationStrength;

  /*if (bool(entityFlags.y & _entityIlluminate)) {
    int index = int((gl_FragCoord.x*2 + gl_FragCoord.y)*10) % 16;
    vec2 offset = index%3 * (poissonDisk[index]/400.0);
    vec2 illumUV = UV.xy + offset;
    illumination = texture(illuminationTexture, illumUV).rgba;
  }*/

  color = vec4(
    (albedo.xyz + shine) * visibility * LightColor +
      lightingPower * lightingColor * albedo.xyz +
      illumination.xyz * illumination.a*2.,
    max(albedo.a, illumination.a * illuminationStrength));

  if (color.a < 0.01) discard;

  applyEffects();
}

