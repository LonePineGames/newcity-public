#include "shadowHeader.fragment.shader"

void main(){
  if (texture(inTexture, UV.xy).r < 0.1) {
    discard;
  }
  fragmentdepth = gl_FragCoord.z;
}

