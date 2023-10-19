#version 330 core

in vec4 lightingOrigin_worldspace;
in float lightingDistance;

out vec4 color;

void main(){
  color = lightingOrigin_worldspace;
  /*
  gl_FragDepth = length(lightingOrigin_worldspace -
    lightingPosition_worldspace) / 10000;
  */

  float ldsqrd = pow(1-lightingDistance, 3);
  //gl_FragDepth = lightingDistance;
  color.a = ldsqrd;
  //color = vec4(ldsqrd, 0, 0, ldsqrd);
}
