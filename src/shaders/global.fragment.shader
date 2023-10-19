#include "global.shader"

float getVisibility() {
  if (bool(globalFlags & _shaderGlobalIsMap)) return 1.0f;
  float visibility = 1.4f;
  //for (int i=0;i<10;i++){
    //int index = i;
    float shadowZ = (ShadowCoord.z-shadowBias)/ShadowCoord.w;
    //vec2 offset = poissonDisk[index]/1000.0;
    vec3 shadowLoc = vec3(ShadowCoord.xy, shadowZ);
    if (shadowLoc.x < 0.0f || shadowLoc.x > 1.0f ||
        shadowLoc.y < 0.0f || shadowLoc.y > 1.0f) {
      //visibility -= 0.15;
    } else {
      float shadowValue = texture(shadowMap, shadowLoc);
      visibility -= 1.4f*(1.0f-shadowValue);
    }
  //}
  visibility = min(visibility, shade - 1.0f);
  if (visibility < 0.0f) visibility *= -0.25f;
  visibility = clamp(visibility, 0.0f, 1.0f);
  visibility += ambientStrength;
  return visibility;
}

void fadeInDistance() {
  if (!bool(globalFlags & _shaderGlobalIsMap)) {
    float fade = cameraDistance*min(invZoom*invZoom,0.0000005f) * 0.005f * (1.f + rainAmount*10.f) + rainAmount;
    color.a *= clamp(2.0f - fade, 0.0f, 1.0f);
    color.rgb = mix(color.rgb, LightColor * 0.1f, clamp(fade* 2.0f - 0.75f, 0.0f, 1.0f));
  }
}

