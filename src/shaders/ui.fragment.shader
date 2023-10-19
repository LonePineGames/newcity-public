#include "uiHeader.fragment.shader"

void main(){
  if (clip.x > Position_worldspace.x ||
      clip.y > Position_worldspace.y ||
      clip.z < Position_worldspace.x ||
      clip.w < Position_worldspace.y) {
    discard;
  }

  color = texture(inTexture, vec2(UV)).rgba;
  color = vec4(color.rgb*foregroundColor, color.a);

  if (bool(entityFlags.y & _entityBlueHighlight)) {
    float shadowHits = 0;
    for (int i=0;i<8;i++) {
      vec2 sampleLoc = vec2(UV)+ring[i]*0.0005;
      shadowHits += texture(inTexture, sampleLoc).a;
    }
    shadowHits += clamp(shadowHits, 0, 2);
    color.rgb = mix(vec3(0,0,0), color.rgb, color.a);
    color.a = max(shadowHits*.2, color.a);
    //gl_FragDepth = gl_FragCoord.z; // - color.a*.001;
  }
  //gl_FragDepth += color.a*0.00001;

  if (color.a < 0.1) {
    discard;
  }

  if (bool(entityFlags.y & _entityDesaturate)) {
    /* DITHERING ALGORITHM for newspaper */
    float dotScale = 12;
    vec2 pixel = position_modelspace.xy*dotScale;
    pixel.x += mod(pixel.y+.5f, 2.f) >= 1.f ? .5f : 0.f;
    vec2 dot = floor(pixel+.5f);
    vec2 locInDot = pixel - dot;
    float dist = locInDot.x*locInDot.x + locInDot.y*locInDot.y;
    dist = clamp(dist, 0, 1);

    /*
    float k1 = dot.x*.5f+dot.y;
    float k2 = dot.x*.5f-dot.y;
    float k = (mod(k1, 8.f)+mod(k2, 8.f))/16.f + (mod(k1, 4.f)+mod(k2, 4.f))/8.f + (mod(k1, 2.f)+mod(k2, 2.f))/4.f;
    k = 0.f ;//k / 3.f - 0.5f;
    */

    float val = getValue(color.rgb);
    val = 1 - val; val = 1 - val*val; // wash out
    val = mix(0.1f, 1.25f, clamp(val, 0.f, 1.f)); // wash out
    val = 1 - val; // invert
    if (val < dist) discard;

    color = vec4(0.01, 0.01, 0.01, clamp((val-dist)*2, 0, 1));
  }

  if (bool(entityFlags.y & _entityRedHighlight)) { // textured for newspaper
    vec2 loc = position_modelspace.xy;
    vec4 noise = texture(blueNoiseTexture, loc/20);
    noise += texture(blueNoiseTexture, loc/7);
    noise += texture(blueNoiseTexture, loc/100);

    float noiseAmount = getValue(noise.rgb);
    noiseAmount = clamp(1.1 - noiseAmount*.2f, 0.f, 1.f);
    color.rgb *= noiseAmount;
  }

  //if (bool(entityFlags.y & _entityRedHighlight)) {
    //color.rgb = getValue(color.rgb)*redHighlight;
  //} else if (bool(entityFlags.y & _entityBlueHighlight)) {
    //color.rgb = getValue(color.rgb)*blueHighlight;
  //}
  if (bool(entityFlags.y & _entityTransparent)) {
    color.a *= .25;
  }

}

