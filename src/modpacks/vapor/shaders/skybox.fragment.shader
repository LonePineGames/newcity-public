#include "header.fragment.shader"

const float skyboxSize = 500;

void main(){
  //color = texture(blueNoiseTexture, UV.xy);
  float z = position_modelspace.z*.25f/skyboxSize + .75;
  z = clamp(z, 0, 1);
  z = 1 - (1-z)*skyColor.x;
  float x = 5 - 4*skyColor.y;
  float y = 57-9*z;

  vec2 cloudLoc = UV.xy;
  cloudLoc.x *= 0.1;
  cloudLoc.x += time;
  vec4 cloudTex = skyColor.z *
    texture(blueNoiseTexture, cloudLoc);
  float cloud = cloudTex.r + cloudTex.g + cloudTex.b + skyColor.z;
  //cloud = clamp(cloud*2-1, 0, 1);
  cloud *= clamp(UV.y*10-1, 0, 1);
  vec4 cloudColor = texture(paletteTexture, vec2(x+10, y)/paletteSize);
  vec4 skyColor = texture(paletteTexture, vec2(x, y)/paletteSize);
  color = skyColor;
  color = mix(skyColor, cloudColor, clamp(cloud*2, 0, 1));
  color.rgb *= clamp(1-.5*(cloud-1), 0, 1);
  //color.r = cloud;

  if (heatmapData != 0) {
    vec3 colorize = heatmapData%256==255 ? trafficColor0 :
      heatmapData%256==254 ? transitColor0 : vec3(1,1,1);
    color.rgb = .5f * colorize * getValue(color.rgb);
  }
}
