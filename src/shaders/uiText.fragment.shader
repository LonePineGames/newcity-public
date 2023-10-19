#include "uiHeader.fragment.shader"

void main(){
  if (clip.x > Position_worldspace.x ||
      clip.y > Position_worldspace.y ||
      clip.z < Position_worldspace.x ||
      clip.w < Position_worldspace.y) {
    discard;
  }

  vec4 albedo = vec4(foregroundColor, texture(inTexture, vec2(UV)).r);

  if (bool(entityFlags.y & _entityBlueHighlight)) {
    float shadowHits = 0;
    for (int i=0;i<8;i++) {
      vec2 sampleLoc = vec2(UV)+ring[i]*0.005;
      shadowHits += texture(inTexture, sampleLoc).r;
    }
    shadowHits += clamp(shadowHits, 0, 2);
    albedo.rgb = mix(vec3(0,0,0), albedo.rgb, albedo.a);
    albedo.a = max(shadowHits*.25f, albedo.a);

    //gl_FragDepth = gl_FragCoord.z - albedo.a*0.001;
  }

  //gl_FragDepth += albedo.a*0.00001;

  if (albedo.a < 0.01) {
    discard;
  }

  //if (bool(entityFlags.y & _entityRedHighlight)) {
    //albedo.rgb = getValue(albedo.rgb)*redHighlight;
  //} else if (bool(entityFlags.y & _entityBlueHighlight)) {
    //albedo.rgb = vec3(0,0,0);
  //}
  if (bool(entityFlags.y & _entityTransparent)) {
    albedo.a *= .25;
  }

  color = albedo;
}

