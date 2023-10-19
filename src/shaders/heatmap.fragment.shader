
/*
const int numHeatmaps = 8;
const vec3 noHM = vec3(1,1,1)*.25f;

const vec3 pollutionColor1 = vec3(1.0f, 0.8f, 0.4f); // * .8f;
const vec3 pollutionColor2 = vec3(1.0f, 0.6f, 0.1f);

const vec3 badLandColor = vec3(0.6f, 0.0f, 0.2f);
//const vec3 medLandColor = vec3(0.3f, 0.5f, 0.4f);
const vec3 goodLandColor = vec3(0.0f, 1.0f, 0.6f);

const vec3 densityColor2 = vec3(0.6f, 0.1f, 1.0f);
const vec3 densityColor1 = vec3(0.1f, 0.1f, 1.0f);

const vec3 crimeColor1 = vec3(0.6f, 0.1f, 0.4f);
const vec3 crimeColor2 = vec3(0.9f, 0.1f, 0.2f);

const vec3 educationColor1 = vec3(0.4f, 0.4f, 1.0f);
const vec3 educationColor2 = vec3(0.4f, 1.0f, 0.4f);

const vec3 abandonedColor = vec3(0.38f, 0.21f, 0.17f)*.25f;
const vec3 prosperousColor = vec3(0.02f, 0.61f, 0.25f);

const vec3 communityColorLow = vec3(0.05f, 0.1f, 0.05f);
const vec3 communityColorMed = vec3(0.1f, 0.8f, 0.6f);
const vec3 communityColorHigh = vec3(0.8f, 0.4f, 1.0f);

const vec3 healthColorLow = vec3(0.2f, 0.1f, 0.6f);
const vec3 healthColorMed = vec3(0.7f, 1.0f, 0.3f);
const vec3 healthColorHigh = vec3(1.0f, 0.7f, 0.6f);

const vec3 trafficColor1 = vec3(1.0f, 0.8f, 0.4f)*.5f;
const vec3 trafficColor2 = vec3(1.0f, 1.0f, 1.0f);

const vec3 heatMapColor[numHeatmaps*3] = vec3[numHeatmaps*3](
  noHM, pollutionColor1, pollutionColor2,
  badLandColor, noHM, goodLandColor,
  noHM, densityColor1, densityColor2,
  noHM, crimeColor1, crimeColor2,
  noHM, educationColor1, educationColor2,
  abandonedColor, noHM, prosperousColor,
  communityColorLow, communityColorMed, communityColorHigh,
  healthColorLow, healthColorMed, healthColorHigh
);
*/

vec3 getHeatmapColor(int hm, float val) {
  return texture(paletteTexture, vec2(
    (35+2*hm)/paletteSize,
    (9-8*val)/paletteSize)).rgb;
}

void applyHeatmap() {
  int hm = heatmapData % 256;

  if (heatmapData == 0) {
    // no-op

  } else if (bool(entityFlags.y & _entityNoHeatmap)) {
    if (hm != 250) { // Pollution
      color.rgb = getValue(color.rgb)*vec3(1,1,1);
    }

  } else if (hm == 255) { // Traffic
    vec4 albedo = color;
    if (bool(entityFlags.y & _entityTraffic)) {
      color.rgb *= 2.f;
    } else {
      color.rgb = getValue(color.rgb)*trafficColor0;
    }

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

    color.rgb += lightingPower * lightingColor * albedo.rgb;

  } else if (hm == 254) { // Transit
    if (bool(entityFlags.y & _entityTransit)) {
      color.rgb = color.rgb*2;
      color = color*color;
    } else {
      float value = getValue(color.rgb);
      value = value*.5f+.4f;
      color.rgb = value*lightingColor;
    }

  } else if (hm == 253) { // Zones
    color.rgb = getValue(color.rgb)*zoneColor0;

  } else if (hm == 252) { // Roads
    color.rgb = getValue(color.rgb)*roadColor0;

  } else if (hm == 250) { // Pollution
    int numTiles = heatmapData / 256 / 256;
    float heatMapR = texture(heatMapTexture,
        Position_worldspace.xy / (numTiles*tileSize)).r;
    float heatMapM = 1 - heatMapR;
    //float heatMapM = heatMapK*heatMapK;
    float heatMapL = 1 - heatMapM*heatMapM;
    heatMapL *= clamp(1 - z_modelspace*0.01, 0, 1);
    color.rgb = mix(color.rgb, getHeatmapColor(0,.5),
        heatMapL*pollutionIntensity);

  } else if (!(bool(entityFlags.y & _entityHeatmapLimited))) {
    int numTiles = heatmapData / 256 / 256;
    float heatMapR = texture(heatMapTexture,
        Position_worldspace.xy / (numTiles*tileSize)).r;
    float heatMapA = round(pow(heatMapR,0.5f)*10.f)*.1f;
    /*
    float heatMapV = pow(heatMapR,0.5)*10;
    float heatMapI = round(heatMapV);
    float heatMapD = clamp((heatMapV - heatMapI)*100-.5, 0, 1);
    float heatMapA = (heatMapD + heatMapI)/10;
    */
    vec3 heatColor = getHeatmapColor(hm, heatMapA);

    color.rgb = 2.f*getValue(color.rgb)*heatColor;
  }

  /*
  // Fog
  vec2 fogPos = Position_worldspace.xy + vec2(1,1) * time*50000;
  vec4 fogTex = texture(blueNoiseTexture, fogPos*0.00001);
  float fogL = fogTex.r;
  fogL = clamp(fogL*.2f, 0, 1);
  //fogL *= clamp(Position_worldspace.z*.1, 0, 1);
  fogL *= clamp(1 - Position_worldspace.z*0.005, 0, 1);
  fogL *= fogL;
  color.rgb = mix(color.rgb, LightColor, fogL);
  */

  fadeInDistance();

  //Guide shadow
  vec2 distVec = Position_worldspace.xy - guide.xy;
  float dist = dot(distVec, distVec);
  //float effect = 1 - clamp(abs(dist - guide.z) / (tileSize*3), 0, 1);
  float effect = 1 - clamp((dist - guide.z) / (tileSize*3), 0, 1);
  effect = 0.8 + 0.2 * effect * effect;
  effect = clamp(effect, 0.0, 1);
  color.rgb *= effect;
}

void applyEffects() {
  bool doHM = !bool(entityFlags.y & _entityHighlight);

  if (bool(entityFlags.y & _entityDesaturate)) {
    color.rgb = getValue(color.rgb)*vec3(1,1,1);
    doHM = false;
  } else if (bool(entityFlags.y & _entityRedHighlight)) {
    color.rgb = getValue(color.rgb)*redHighlight;
    doHM = false;
  //} else if (bool(entityFlags.y & _entityBlueHighlight)) {
    //color.rgb = getValue(color.rgb)*blueHighlight;
    //doHM = false;
  }

  if (bool(entityFlags.y & _entityTransparent)) {
    color.a = 0.75;
    doHM = false;
  }

  if (doHM) {
    applyHeatmap();
  }
}

