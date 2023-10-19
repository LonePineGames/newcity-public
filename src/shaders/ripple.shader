const vec3 one = vec3(1,1,1);

mat4 rotationMatrix(vec3 axis, float angle)
{
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;

  return mat4(oc * axis.x * axis.x + c,
    oc * axis.x * axis.y - axis.z * s,
    oc * axis.z * axis.x + axis.y * s,  0.0,
    oc * axis.x * axis.y + axis.z * s,
    oc * axis.y * axis.y + c,
    oc * axis.y * axis.z - axis.x * s,  0.0,
    oc * axis.z * axis.x - axis.y * s,
    oc * axis.y * axis.z + axis.x * s,
    oc * axis.z * axis.z + c,           0.0,
    0.0, 0.0, 0.0,                      1.0);
}

float computeRipple(vec3 Position_worldspace, int iters) {
  float ripple = 0;
  float adjTime = time*waveSpeed;
  vec4 wv = vec4(Position_worldspace / (waveSize*40), 1);
  mat4 rot = rotationMatrix(vec3(0, 0, 1), 1.0);
  float shift = wv.x - wv.y + sin((wv.x + wv.y)/200);
  adjTime += shift + vertexUV.z*0.05;

  for (int i = 0; i < iters; i ++) {
    adjTime += adjTime * .01 + shift;
    wv = wv * 0.5;
    wv *= rot;
    vec3 s = texture(blueNoiseTexture, wv.xy + adjTime*0.001).rgb;
    float r = dot(s, one);
    ripple += r;
    adjTime += r * .1;
  }

  return sin(ripple) * windSpeed * 3;
}

/*
float computeRipple(vec3 Position_worldspace, int iters) {
  float ripple = 0;
  float adjTime = time*waveSpeed;
  vec4 wv = vec4(Position_worldspace / waveSize, 1);
  mat4 rot = rotationMatrix(vec3(0, 0, 1), 1.0);
  float shift = wv.x - wv.y + sin((wv.x + wv.y)/200);
  adjTime += shift + vertexUV.z*0.05;

  for (int i = 0; i < iters; i ++) {
    float sx = sin(wv.x + adjTime);
    float sy = cos(wv.y + adjTime);
    ripple += sx + sy;
    adjTime += (sx + sy) *.1;
    adjTime += adjTime*.01;
    wv *= 0.8;
    wv *= rot;
  }

  return ripple * windSpeed;
}
*/

float computeRipple(vec3 Position_worldspace) {
  return computeRipple(Position_worldspace, 4);
}

