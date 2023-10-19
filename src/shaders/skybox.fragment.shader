#include "header.fragment.shader"

const float skyboxSize = 500;

void main(){
  //color = texture(inTexture, UV.xy);
  float z = position_modelspace.z*.25f/skyboxSize + .5;
  z = clamp(z, 0, 1);
  z = 1 - (1-z)*skyColor.x;
  float x = 11 - 4*skyColor.y;
  float y = 57-8*z;

  vec2 cloudLoc = UV.xy;
  cloudLoc.x -= time*2;
  vec4 cloudTex1 = texture(inTexture, cloudLoc);
  float sat = cloudTex1.b-cloudTex1.r;
  float val = cloudTex1.b;
  float cloud = (1-sat)*skyColor.z + clamp(skyColor.z*2-1, 0, 1);
  cloud = clamp(cloud, 0, 1);
  cloud *= clamp((1-UV.y)*10, 0, 1); // hide clouds at bottom, they don't look good
  vec4 cloudColor = texture(paletteTexture, vec2(x+6, y)/paletteSize);
  vec4 skyColor = texture(paletteTexture, vec2(x, y)/paletteSize);
  color = mix(skyColor, cloudColor, clamp(cloud*1.5, 0, 1));
  float valMix = clamp(skyColor.z*(cloud*3-2), 0, 1);
  color *= mix(1, val, valMix*valMix);

  /*
  //vec4 cloudTex2 = skyColor.z *
    //texture(inTexture, cloudLoc*4);
  float cloud = cloudTex1.r + cloudTex1.g + cloudTex1.b + skyColor.z +
    cloudTex2.r + cloudTex2.g + cloudTex2.b;
  cloud = clamp(cloud, 0, 2);
  cloud *= clamp(UV.y*10-1, 0, 1);
  vec4 cloudColor = texture(paletteTexture, vec2(x+6, y)/paletteSize);
  vec4 skyColor = texture(paletteTexture, vec2(x, y)/paletteSize);
  color = skyColor;
  color = mix(skyColor, cloudColor, clamp(cloud*2, 0, 1));
  color.rgb *= clamp(1-.5*(cloud-.75), 0, 1);
  //color.r = cloud;
  */

  if (heatmapData != 0) {
    vec3 colorize = heatmapData%256==255 ? trafficColor0 :
      heatmapData%256==254 ? transitColor0 : vec3(1,1,1);
    color.rgb = .5f * colorize * getValue(color.rgb);
  }
}
