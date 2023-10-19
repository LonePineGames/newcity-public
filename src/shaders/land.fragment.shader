#include "header.fragment.shader"

float getVisibilityLand() {
  float visibility = 1.4;

  //for (int i=0;i<10;i++){
    //int index = i;
    float shadowZ = (ShadowCoord.z-landShadowBias)/ShadowCoord.w;
    //vec2 offset = poissonDisk[index]/1000.0;
    vec3 shadowLoc = vec3(ShadowCoord.xy, shadowZ);
    if (shadowLoc.x < 0 || shadowLoc.x > 1 ||
        shadowLoc.y < 0 || shadowLoc.y > 1) {
      //visibility -= 0.15;
    } else {
      float shadowValue = texture(shadowMap, shadowLoc);
      visibility -= 1.4*(1.0-shadowValue);
    }
  //}
  visibility = min(visibility, shade-1);
  if (visibility < 0) visibility *= -0.25;
  visibility = clamp(visibility, 0.0, 1);
  visibility += ambientStrength;
  return visibility;
}

void main(){

  //Albedo
  vec4 albedo = texture(paletteTexture, vec2(UV)).rgba;
  vec4 overlay = texture(illuminationTexture, Position_worldspace.xy/gridSize);
  //vec4 grass = texture(blueNoiseTexture, Position_worldspace.xy/625);
  //grass.rg *= .5f;
  //overlay -= grass;
  float beach = clamp(.25*(Position_worldspace.z - beachLineAdj), 0, 1);
  albedo.rgb = mix(albedo.rgb, snowColor, snow * beach);
  if (bool(globalFlags & _shaderGlobalGridLines)) {
    albedo *= mix(vec4(1,1,1,1), overlay, beach);
  }
  albedo.rgb *= mix(1, .75f,
      bool(globalFlags & _shaderGlobalContourLines) &&
      mod(Position_worldspace.z, 50) < 5);
  //if (mod(Position_worldspace.z, 50) < 5) albedo.rgb *= .75f;

  vec4 grass = texture(blueNoiseTexture, Position_worldspace.xy/1000);
  albedo.rgb *= mix(vec3(1,1,1), grass.rgb, grass.a);

  //Shadows and shading
  float visibility = getVisibilityLand();
  visibility = clamp(visibility, 0.05, 1);

  vec2 mapLoc = Position_worldspace.xy/mapSize;
  mapLoc = vec2(1 - mapLoc.y, 1 - mapLoc.x);
  vec4 map = texture(mapTexture, mapLoc);
  float cameraDistance = gl_FragCoord.z * distanceFactor;
  map.a *= clamp(0.00015f*cameraDistance-1, 0, 1);
  //float dropoff = clamp(1 - position_cameraspace.z*0.005, 0, 1);
  //map.a *= dropoff;
  color.a = 1;
  color.rgb = LightColor * mix(albedo.rgb, map.rgb, map.a);
  color.rgb *= visibility;

  applyHeatmap();
}

