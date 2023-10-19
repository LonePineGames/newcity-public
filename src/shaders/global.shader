
const uint _entityExists          = 1u << 0u;
const uint _entityVisible         = 1u << 1u;
const uint _entityIlluminate      = 1u << 2u;
const uint _entityHighlight       = 1u << 3u;
const uint _entityRedHighlight    = 1u << 4u;
const uint _entityBlueHighlight   = 1u << 5u;
const uint _entityTransparent     = 1u << 6u;
const uint _entityDesaturate      = 1u << 7u;
const uint _entityWind            = 1u << 8u;
const uint _entityBringToFront    = 1u << 9u;
const uint _entityNoHeatmap       = 1u << 10u;
const uint _entityHeatmapLimited  = 1u << 11u;
const uint _entityHardTransitions = 1u << 12u;
const uint _entityTransit         = 1u << 13u;
const uint _entityTraffic         = 1u << 14u;
const uint _entityBillboard       = 1u << 15u;
const uint _entityRaiseShift      = 16u;
const uint _entityRaiseMask       = 7u << _entityRaiseShift;

const uint _shaderGlobalContourLines = 1u << 1u;
const uint _shaderGlobalGridLines    = 1u << 2u;
const uint _shaderGlobalIsMap        = 1u << 3u;
const uint _shaderGlobalIsCapture    = 1u << 4u;

const uint _tfWindShift       = 8u;
const uint _tfWindMask        = 7u << _tfWindShift;
const uint _tfSnow            = 1u << 12;
const uint _tfShineShift      = 16u;
const uint _tfShineMask       = 7u << _tfShineShift;

const uint _shaderIsMapPass = 1u << 0u;

const float ambientStrength = 0.2;
const float treeMovement = 0.15;
const float shadowBias = 0.001;
const int tileSize = 25;
const float snowLine = 900;
const float beachLine = 5;
const float beachLineAdj = 4;
const float grassLine = 300;
const float halfPi = 1.57079632679489661923;
const bool objectViewer = false;
const bool renderGridlines = true;
const float gridSize = 3125;
const float landShadowBias = 0.0015;
const int numColors = 32;
const float paletteSize = 64;

const vec3 lightingColor = vec3(1.0f, 0.95f, 0.8f);
const vec3 snowColor = vec3(211/255.f, 204/255.f, 202/255.f);
const vec3 redHighlight = vec3(1,.1,.2);
const vec3 blueHighlight = vec3(0,.35,.56);
const vec3 trafficColor0 = vec3(0.01f, 0.01f, 0.2f);
const vec3 transitColor0 = vec3(1,1,1);
const vec3 zoneColor0 = vec3(0.1,0.1,0.1);
const vec3 roadColor0 = vec3(0.1,0.1,0.1);
const vec3 abandonedBuildingColor = vec3(0.38f, 0.21f, 0.17f);

vec2 poissonDisk[16] = vec2[](
   vec2( -0.94201624, -0.39906216 ),
   vec2( 0.94558609, -0.76890725 ),
   vec2( -0.094184101, -0.92938870 ),
   vec2( 0.34495938, 0.29387760 ),
   vec2( -0.91588581, 0.45771432 ),
   vec2( -0.81544232, -0.87912464 ),
   vec2( -0.38277543, 0.27676845 ),
   vec2( 0.97484398, 0.75648379 ),
   vec2( 0.44323325, -0.97511554 ),
   vec2( 0.53742981, -0.47373420 ),
   vec2( -0.26496911, -0.41893023 ),
   vec2( 0.79197514, 0.19090188 ),
   vec2( -0.24188840, 0.99706507 ),
   vec2( -0.81409955, 0.91437590 ),
   vec2( 0.19984126, 0.78641367 ),
   vec2( 0.14383161, -0.14100790 )
);

vec2 ring[8] = vec2[](
    vec2( 0.92388, 0.38268),
    vec2( 0.38268, 0.92388),
    vec2(-0.38268, 0.92388),
    vec2(-0.92388, 0.38268),
    vec2(-0.92388,-0.38268),
    vec2(-0.38268,-0.92388),
    vec2( 0.38268,-0.92388),
    vec2( 0.92388,-0.38268)
);

float getValue(vec3 color) {
  color = pow(color, vec3(2.2)); // Gamma correction
  float result = 0.2125*color.r + 0.7154*color.g + 0.0721*color.b;
  return pow(result, 1/2.2); // Gamma correction
}

float convertUV(int val) {
  return val / 32767.f; //* 64.f / SHRT_MAX;
}

vec3 convertUV(ivec3 v) {
  return vec3(
    convertUV(v.x),
    convertUV(v.y),
    float(v.z)
  );
}

vec2 getColorInPalette(int color) {
  if (color < 0 || color >= numColors) {
    color = 0;
  }

  int col = color / 4;
  int row = color % 4;
  return vec2(col*2+1, row*2+1) / paletteSize + vec2(0.5,0.5);
}

