#include "global.shader"

float getVisibility() {
  float visibility = 1.4;
  //for (int i=0;i<10;i++){
    //int index = i;
    float shadowZ = (ShadowCoord.z-shadowBias)/ShadowCoord.w;
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
  visibility = clamp(visibility, 0.05, 1);
  visibility += ambientStrength;
  return visibility;
}

void fadeInDistance() {
  if (!bool(globalFlags & _shaderGlobalIsMap)) {
    float fade = cameraDistance*min(invZoom*invZoom,0.0000005)*.005f;
    color.a *= clamp(1.5-fade, 0, 1);
    color.rgb = mix(color.rgb, LightColor*.1f, clamp(fade*2-.75, 0, 1));
  }
}

