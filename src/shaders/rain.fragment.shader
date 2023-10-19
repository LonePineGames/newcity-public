#include "header.fragment.shader"

void main(){
  vec4 albedo = vec4(LightColor,1) * texture(inTexture, vec2(UV)).rgba;
  albedo.a *= (entityFlags.y >> 23) / 255.f;
  if (albedo.a < 0.1) {
    discard;
  }
  color = albedo;

  if (heatmapData != 0) {
    vec3 colorize = heatmapData%256==255 ? trafficColor0 :
      heatmapData%256==254 ? transitColor0 : vec3(1,1,1);
    color.rgb = .5f * colorize * getValue(color.rgb);
  }
}
