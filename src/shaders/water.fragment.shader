#include "header.fragment.shader"

float f(float n, float h) {
  float k = mod(n + h/30, 12);
  return clamp(min(k-3, 9-k), 0, 1);
}

// input: h as an angle in [0,360] and s,l in [0,1] - output: r,g,b in [0,1]
vec3 hue2rgb(float h) {
   return vec3(f(0,h), f(8,h),f(4,h));
}

void main(){
  vec4 albedo = texture(inTexture, vec2(UV)).rgba;
  albedo.rgb = mix(albedo.rgb, snowColor, snow);
  float shine = float(Position_worldspace.z > -4) * getShine(8, 0.2);
  float visibility = getVisibility();
  vec3 irridecense = vec3(0,0,0);

  int hm = heatmapData % 256;
  if (heatmapData == 0) {
    albedo.a = 0.98;
    //albedo.rgb *= 0.8;
    //
  } else if (hm == 250) { // Ambient Pollution
    int numTiles = heatmapData / 256 / 256;
    float heatMapR = texture(heatMapTexture,
        Position_worldspace.xy / (numTiles*tileSize)).r;
    float heatMapM = 1 - heatMapR*2;
    heatMapM = clamp(heatMapM, 0, 1);
    //float heatMapM = heatMapK*heatMapK;
    float heatMapL = 1 - heatMapM*heatMapM;
    albedo.rgb = mix(albedo.rgb, getHeatmapColor(0,1)*.25,
        heatMapL);
    float irridecenseKey = shade + heatMapR*50;

    irridecense = hue2rgb(mod(shade*3600, 330));
    float irrAmount = clamp(heatMapL*2-heatMapL*heatMapL*2, 0, 0.5);
    irridecense  = irridecense * clamp(shine, 0.01, 0.5) * irrAmount;
    shine *= 1-heatMapL;

  } else if (hm == 0) { // Pollution heatmap
    albedo.rgb *= 0.5;
    // we handle this again later in this file

  } else if (hm > 250 && hm != 253 && hm != 255) {
    albedo.a = 0.75;
    albedo.rgb *= 10.f; // water gets really dark, can't figure out why

  } else {
    albedo.a = 0.95;
    vec3 colorize = hm == 255 ? trafficColor0 :
      vec3(1,1,1);
    albedo.rgb = .5f * getValue(albedo.rgb) * colorize;
  }

  /*
  if (heatmapData != 0 && (hm < 250 || hm == 255)) {
    vec3 colorize = hm == 255 ? trafficColor0 :
      hm == 254 ? albedo.rgb :
      hm == 253 ? zoneColor0 :
      vec3(1,1,1);
    //albedo.rgb = .5f * colorize * getValue(albedo.rgb);
    albedo.a = 0.75;
  } else if (hm == 250 || heatmapData == 0) {
    albedo.a = 0.95;
    albedo.rgb *= 0.8;
  } else {
    albedo.rgb *= 2.f;
    albedo.a = 0.75;
  }
  */

  color = vec4((albedo.rgb + shine) * visibility * LightColor + irridecense, albedo.a);

  if (hm == 0) { // Pollution heatmap
    applyHeatmap();
  }

  /*
  // Fog
  vec2 fogPos = Position_worldspace.xy + vec2(1,1) * time*50000;
  vec4 fogTex = texture(blueNoiseTexture, fogPos*0.00001);
  float fogL = fogTex.r;
  fogL = clamp(fogL*.2f, 0, 1);
  //fogL *= clamp(Position_worldspace.z*.1, 0, 1);
  fogL *= fogL;
  color.rgb = mix(color.rgb, LightColor, fogL);
  */

  fadeInDistance();
}
