#include "header.fragment.shader"

void main(){
  color = texture(inTexture, vec2(UV)).rgba;
}

