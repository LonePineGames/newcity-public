#pragma once

#include "../main.hpp"
#include "../cup.hpp"

enum Shader {
  // Pre Shaders
  ShadowSingleShader, ShadowTreeShader, ShadowInstancedShader,
  ShadowTextShader,
  WSUIShadowShader, WSUIShadowInstancedShader, WSUIShadowTextShader,
  LightingShader, SkyboxShader,
  // Instanced Shaders
  TransitCoverageShader, BuildingShader, DecoShader, LotShader,
  // Other Shaders
  PaletteShader, RoadShader, WaterShader, TreeShader, LandShader,
  // Transparent Shaders
  WearShader, WSUIShader, WSUIInstancedShader, WSUITextShader,
  VehicleShader, SignShader, TextShader,
  RainShader,
  // UI Shaders
  UIPaletteShader, UIIconShader, UITextShader, UIShadowShader,

  numShaders
};

enum Uniform {
  MUniform, ClipUniform, FlagsUniform, TextureUniform, IllumTextureUniform,
  FlagsTextureUniform, MapTextureUniform,
  BlueNoiseUniform, PaletteUniform,
  SnowUniform, TimeUniform,
  SkyColorUniform, RainPosUniform, RainAmountUniform, WindUniform,
  TideUniform, IlluminationStrengthUniform, MapSizeUniform,
  GlobalFlagsUniform, PollutionUniform, CameraPosition,
  InvZoom,
  numUniforms
};

struct DrawData {
  float matrix[16];
  uint32_t dataFlags;
  uint32_t flags;
};

struct DrawCommand {
  item mesh;
  item entity;
  item texture;
  item aboID;
  item numInstances;
  Cup<DrawData> data;
};

typedef void (*DrawCommandHandler)(Shader s, DrawCommand& command);
typedef void (*CameraSetup)(GLuint programID);

GLuint getProgramID(Shader s);
GLuint getUniformID(Shader s, Uniform u);
const char* getShaderName(Shader s);
bool isShaderInstanced(Shader s);
void initShaders();
void setupShader();
void draw();
bool isInMap(Shader shader);
bool isInCapture(Shader shader);

