#include "uiHeader.fragment.shader"

void main(){
  if (clip.x > Position_worldspace.x ||
      clip.y > Position_worldspace.y ||
      clip.z < Position_worldspace.x ||
      clip.w < Position_worldspace.y) {
    discard;
  }

  //vec2 diff = max(abs(UV.xy -.5f) - .5f, 0);
  vec2 diff = UV.xy;
  //max(abs(UV.x-.5f) - .5f, 0);
  //float dx = max(abs(px - x) - width / 2, 0);
  //dy = max(abs(py - y) - height / 2, 0);
  float dist = diff.x*diff.x + diff.y*diff.y; //dot(diff, diff);
  //float dist = max(UV.x, UV.y);

  //float alpha = .5f - max(abs(UV.x-.5f), abs(UV.y-.5f));
  //float alpha = clamp(.5f - sqrt(dist)*200.f, 0, 1);
  float alpha = 1 - dist;
  alpha = clamp(alpha, 0, 0.8);
  //float alpha = max(UV.x, UV.y);
  //alpha = clamp(alpha*2.f, 0, .25f)*2.f;
  color = vec4(0,0,0,alpha);
  //gl_FragDepth += color.a*0.00001;

  /*
  color = texture(inTexture, vec2(UV)).rgba;
  color = vec4(color.rgb*.75f, color.a);
  if (color.a < 0.2) {
    discard;
  }

  if (bool(entityFlags.y & _entityRedHighlight)) {
    color.rgb = getValue(color.rgb)*redHighlight;
  } else if (bool(entityFlags.y & _entityBlueHighlight)) {
    color.rgb = getValue(color.rgb)*blueHighlight;
  }
  if (bool(entityFlags.y & _entityTransparent)) {
    color.a *= .25;
  }
  */
}

