float getShine(float power, float shine) {
  vec2 normFrag = gl_FragCoord.xy / screenSize - vec2(0.5, 0.5);
  float xrot = normFrag.x * 4.f;
  //float incidence = -lightDirection_cameraSpace.z * cos(xrot)
    //-lightDirection_cameraSpace.x * sin(xrot);
  //vec3 reflection = 2 * dot(normal_cameraspace, lightDirection_cameraSpace) *
    //normal_cameraspace - lightDirection_cameraSpace;
  vec3 reflection = reflect(-lightDirection_cameraSpace, normal_cameraspace);
  //float incidence = reflection.z * cos(xrot)
    //+ reflection.x * sin(xrot);
  float incidence = dot(reflection, vec3(sin(xrot), -sin(normFrag.y*.2f),
        cos(xrot)));
  //incidence *= clamp(dot(normal_cameraspace,
        //lightDirection_cameraSpace + vec3(normFrag.x, 0, 0)), 0, 1);
  incidence = pow(max(incidence, 0), power);
  return shine * incidence;
}

