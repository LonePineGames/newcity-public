#include "shader.hpp"

#include "../util.hpp"
#include "../game/game.hpp"
#include "../heatmap.hpp"
#include "../land.hpp"
#include "../option.hpp"
#include "../platform/file.hpp"
#include "../platform/lookup.hpp"
#include "../platform/mod.hpp"
#include "../string_proxy.hpp"
#include "../time.hpp"
#include "../weather.hpp"

#include "camera.hpp"
#include "buffer.hpp"
#include "entity.hpp"
#include "framebuffer.hpp"
#include "texture.hpp"

#include "spdlog/spdlog.h"
#define STB_INCLUDE_IMPLEMENTATION
#define STB_INCLUDE_LINE_GLSL
#include "stb_include.h"

const char* uniformName[numUniforms] = {
  "M", "inClip", "flags", "inTexture", "illuminationTexture", "flagsTexture",
  "mapTexture",
  "blueNoiseTexture", "paletteTexture", "snow", "time", "skyColor", "rainPos",
  "rainAmount", "windSpeed", "tide", "illuminationStrength",
  "mapSize", "globalFlags", "pollutionIntensity", "cameraPosition_worldSpace",
  "invZoom",
};

GLuint uniforms[numShaders][numUniforms];

const int _shaderSkip           = 1 << 0;
const int _shaderInstanced      = 1 << 1;
const int _shaderCastShadow     = 1 << 2;
const int _shaderIsShadow       = 1 << 3;
const int _shaderUseHeatmap     = 1 << 4;
const int _shaderUseLighting    = 1 << 5;
const int _shaderUseGuideShadow = 1 << 6;
const int _shaderClearDepth     = 1 << 7;
const int _shaderIsWater        = 1 << 8;
const int _shaderIsSkybox       = 1 << 9;
const int _shaderText           = 1 << 10;
const int _shaderWSUI           = 1 << 11;
const int _shaderInMap          = 1 << 12;
const int _shaderNotInCapture   = 1 << 13;

const uint32_t _shaderGlobalContourLines = 1 << 1;
const uint32_t _shaderGlobalGridLines    = 1 << 2;
const uint32_t _shaderGlobalIsMap        = 1 << 3;
const uint32_t _shaderGlobalIsCapture    = 1 << 4;

static Framebuffer lastFramebuffer = (Framebuffer)-1;

struct ShaderInfo {
  int flags;
  CameraSetup setupCamera;
  Framebuffer framebuffer;
  const char* filename;
  GLuint programID;
};

const Shader drawOrder[numShaders] = {
  ShadowSingleShader,
  ShadowTreeShader,
  ShadowInstancedShader,
  ShadowTextShader,

  WSUIShadowShader,
  WSUIShadowInstancedShader,
  WSUIShadowTextShader,

  LightingShader,
  SkyboxShader,
  TransitCoverageShader,
  PaletteShader,
  LandShader,
  TreeShader,
  BuildingShader,
  DecoShader,
  LotShader,
  RoadShader,
  WearShader,

  VehicleShader,
  SignShader,
  WaterShader,
  TextShader,

  WSUIShader,
  WSUIInstancedShader,
  WSUITextShader,

  RainShader,

  UIShadowShader,
  UIPaletteShader,
  UIIconShader,
  UITextShader,
};

ShaderInfo shaderInfo[numShaders] = {
  // ShadowSingleShader
  {_shaderIsShadow | _shaderInMap,
    setupShadowCamera, ShadowFramebuffer, "shadowSingle"},
  // ShadowTreeShader
  {_shaderIsShadow | _shaderInstanced | _shaderInMap,
    setupShadowCamera, ShadowFramebuffer, "shadowTree"},
  // ShadowInstancedShader
  {_shaderIsShadow | _shaderInstanced | _shaderInMap,
    setupShadowCamera, ShadowFramebuffer, "shadowInstanced"},
  // ShadowTextShader
  {_shaderIsShadow | _shaderText | _shaderNotInCapture,
    setupShadowCamera, ShadowFramebuffer, "shadowText"},

  // WSUIShadowShader
  {_shaderIsShadow | _shaderWSUI,
    setupShadowCamera, ShadowFramebuffer, "wsuiShadowSingle"},
  // WSUIShadowInstancedShader
  {_shaderIsShadow | _shaderInstanced | _shaderWSUI,
    setupShadowCamera, ShadowFramebuffer, "wsuiShadowInstanced"},
  // WSUIShadowTextShader
  {_shaderIsShadow | _shaderText | _shaderWSUI,
    setupShadowCamera, ShadowFramebuffer, "wsuiShadowText"},
  // LightingShader
  {_shaderInstanced, setupLightingCamera, LightingFramebuffer, "lighting"},
  // SkyboxShader
  {_shaderIsSkybox | _shaderUseHeatmap,
    setupSkyboxCamera, MainFramebuffer, "skybox"},

  // TransitCoverageShader
  {_shaderInstanced, setupMainCamera, MainFramebuffer,
    "transitCoverage"},
  // BuildingShader
  {_shaderInstanced | _shaderCastShadow | _shaderUseHeatmap | _shaderInMap,
    setupMainCamera, MainFramebuffer, "building"},
  // DecoShader
  {_shaderInstanced | _shaderCastShadow | _shaderUseHeatmap | _shaderInMap,
    setupMainCamera, MainFramebuffer, "deco"},
  // LotShader
  {_shaderInstanced | _shaderCastShadow | _shaderUseHeatmap |
    _shaderNotInCapture,
    setupMainCamera, MainFramebuffer, "lot"},

  // PaletteShader
  {_shaderCastShadow | _shaderUseHeatmap,
    setupMainCamera, MainFramebuffer, "single"},
  // RoadShader
  {_shaderCastShadow | _shaderUseHeatmap | _shaderInMap,
    setupMainCamera, MainFramebuffer, "road"},
  // WaterShader
  {_shaderIsWater | _shaderUseHeatmap,
    setupMainCamera, MainFramebuffer, "water"},
  // TreeShader
  {_shaderCastShadow | _shaderUseHeatmap | _shaderInstanced | _shaderInMap,
    setupMainCamera, MainFramebuffer, "tree"},
  // LandShader
  {_shaderCastShadow | _shaderUseHeatmap | _shaderUseGuideShadow,
    setupMainCamera, MainFramebuffer, "land"},

  // Transparent Shaders
  // WearShader
  {_shaderCastShadow | _shaderUseHeatmap | _shaderNotInCapture,
    setupMainCamera, MainFramebuffer, "wear"},
  // WSUIShader
  {_shaderCastShadow | _shaderWSUI | _shaderUseHeatmap | _shaderNotInCapture,
    setupMainCamera, MainFramebuffer, "wsuiSingle"},
  // WSUIInstancedShader
  {_shaderCastShadow | _shaderInstanced | _shaderWSUI | _shaderNotInCapture,
    setupMainCamera, MainFramebuffer, "wsuiInstanced"},
  // WSUITextShader
  {_shaderCastShadow | _shaderText | _shaderWSUI | _shaderUseHeatmap |
    _shaderNotInCapture,
    setupMainCamera, MainFramebuffer, "wsuiText"},

  // VehicleShader
  {_shaderInstanced | _shaderCastShadow | _shaderUseHeatmap,
    setupMainCamera, MainFramebuffer, "vehicle"},
  // SignShader
  {_shaderCastShadow | _shaderUseHeatmap,
    setupMainCamera, MainFramebuffer, "sign"},
  // TextShader
  {_shaderCastShadow | _shaderUseHeatmap | _shaderText | _shaderNotInCapture,
    setupMainCamera, MainFramebuffer, "text"},
  // RainShader
  {_shaderUseHeatmap | _shaderNotInCapture, setupWeatherCamera,
    MainFramebuffer, "rain"},

  // UI Shaders
  // UIPaletteShader
  {_shaderClearDepth | _shaderNotInCapture,
    setupUICamera, MainFramebuffer, "ui"},
  // UIIconShader
  {_shaderNotInCapture, setupUICamera, MainFramebuffer, "ui"},
  // UITextShader
  {_shaderText | _shaderNotInCapture, setupUICamera, MainFramebuffer, "uiText"},
  // UIShadowShader
  {_shaderNotInCapture, setupUICamera, MainFramebuffer, "uiShadow"},
};

void draw(Shader s) {
  Perspective perspective = getPerspective_r();
  bool isMap = perspective == SatMapPerspective;
  bool isCapture = perspective == CapturePerspective;
  ShaderInfo info = shaderInfo[s];
  if (info.flags & _shaderSkip) { return; }
  if (isMap && !(info.flags & _shaderInMap)) { return; }
  if (isCapture && (info.flags & _shaderNotInCapture)) { return; }
  if (s == TransitCoverageShader && getHeatMap_d() != TransitHeatMap) return;

  Framebuffer targetFramebuffer = info.framebuffer;
  if (targetFramebuffer == MainFramebuffer) {
    if (isMap) {
      targetFramebuffer = MapFramebuffer;
    } else if (isCapture) {
      targetFramebuffer = CaptureFramebuffer;
    }
  }

  if (lastFramebuffer != targetFramebuffer) {
    lastFramebuffer = targetFramebuffer;
    setupFramebuffer(lastFramebuffer);
  }
  if (info.flags & _shaderClearDepth) { clearDepth(); }

  Camera cam = isCapture ? getCaptureCameraBack() : getMainCameraBack();
  LightInformation light = cam.light;
  glUseProgram(info.programID);
  info.setupCamera(info.programID);
  if (isMap || isCapture) {
    setupNoHeatmap(info.programID);
  } else {
    setupHeatmap(info.programID, s != SkyboxShader);
  }
  //if (info.flags & _shaderUseGuideShadow) { setGuideUniform(info.programID); }
  glUniform3fv(uniforms[s][SkyColorUniform], 1,
      (const GLfloat*) &light.skyColor);
  vec3 rainPos = getRainPos();
  glUniform3fv(uniforms[s][RainPosUniform], 1, (const GLfloat*) &rainPos);

  if (s == UIShadowShader) {
    glDisable(GL_DEPTH_TEST);
  }

  Weather w = getWeather();
  float time = getAnimationTime();
  float windSpeed = length(w.wind)/15.f;
  float tide = cos(pi_o*.5f*(time-.5f))*5 - 5;
  float illuminationStrength = clamp(.8 - light.level, 0., 1.);
  illuminationStrength *= 2;
  if (!isMap && getHeatMap_d() == TrafficHeatMap) illuminationStrength = 2;
  if (s == VehicleShader) illuminationStrength = clamp(illuminationStrength, .5f, 1.f);

  if (getGameMode() != ModeGame && getLightSeason() > 3) {
    w.snow = (getLightSeason()-3)*2.f;
  }
  float snow = c(CRenderSnow) && isShowWeather() ? w.snow : 0;

  float rain = isShowWeather() ? clamp(w.percipitation - 0.1f, 0.f, 1.f) : 0.f;
  if (s == WaterShader) {
    if (getGameMode() == ModeGame) {
      time = getWaterTime();
      snow = getWaterSnow();
      windSpeed = mix(windSpeed, .15f, getIceFade());
    } else if (snow > 0) {
      time = 0;
      windSpeed = 0;
    }
  }
  snow = clamp(snow, 0.f, 1.f);

  float pollution = 0.1f * clamp(float(light.level)*2-.5f, 0.2f, 1.f);

  uint32_t globalFlags = 0;
  if (isMap) globalFlags |= _shaderGlobalIsMap;
  if (isCapture) globalFlags |= _shaderGlobalIsCapture;
  if (getContourLinesVisible()) globalFlags |= _shaderGlobalContourLines;
  if (!(getOptionsFlags() & _optionHideGridlines)) {
    globalFlags |= _shaderGlobalGridLines;
  }

  checkOpenGLError();
  setGuideUniform(info.programID);
  glUniform1f(uniforms[s][TideUniform], tide);
  glUniform1i(uniforms[s][TextureUniform], 0);
  glUniform1i(uniforms[s][IllumTextureUniform], 3);
  glUniform1i(uniforms[s][BlueNoiseUniform], 5);
  glUniform1i(uniforms[s][PaletteUniform], 6);
  glUniform1i(uniforms[s][FlagsTextureUniform], 8);
  glUniform1i(uniforms[s][MapTextureUniform], 9);
  glUniform1ui(uniforms[s][GlobalFlagsUniform], globalFlags);
  glUniform1f(uniforms[s][SnowUniform], snow);
  glUniform1f(uniforms[s][WindUniform], windSpeed);
  glUniform1f(uniforms[s][TimeUniform], time);
  glUniform1f(uniforms[s][MapSizeUniform], getMapSize());
  glUniform1f(uniforms[s][IlluminationStrengthUniform], illuminationStrength);
  glUniform1f(uniforms[s][RainAmountUniform], rain);
  glUniform1f(uniforms[s][PollutionUniform], pollution);
  glUniform1f(uniforms[s][InvZoom], 1.f/cam.distance);
  checkOpenGLError();

  vec3 camPos = vec3(cam.position);
  glUniform3fv(uniforms[s][CameraPosition], 1,
      (const GLfloat*) &camPos);
  checkOpenGLError();

  if (info.flags & _shaderIsShadow) {
    int instanced = info.flags & _shaderInstanced;
    int text = info.flags & _shaderText;
    int isWSUI = info.flags & _shaderWSUI;
    bool isTree = (s == ShadowTreeShader);
    for (int k = 0; k < numShaders; k++) {
      ShaderInfo other = shaderInfo[k];
      if (other.flags & _shaderCastShadow &&
          (isTree == (k == TreeShader)) &&
          ((other.flags & _shaderWSUI) == isWSUI) &&
          ((other.flags & _shaderInstanced) == instanced) &&
          ((other.flags & _shaderText) == text)) {
        runDrawCommands(s, (Shader)k);
      }
    }

  } else {
    runDrawCommands(s, s);
  }

  if (s == TransitCoverageShader) { clearDepth(); }
  glBindVertexArray(0);
  glUseProgram(0);
}

void draw() {
  checkOpenGLError();
  bool map = getPerspective_r() == SatMapPerspective;
  if (map) setupFramebuffer(ShadowFramebuffer); // clears shadow framebuffer
  checkOpenGLError();
  updateWindowMode();
  checkOpenGLError();
  bufferABOs();
  checkOpenGLError();
  drawHeatMaps();
  checkOpenGLError();

  glActiveTexture(GL_TEXTURE9);
  glBindTexture(GL_TEXTURE_2D, map ? 0 : getMapTexture());
  checkOpenGLError();

  for (int s = 0; s < numShaders; s++) {
    draw(drawOrder[s]);
  }
  checkOpenGLError();
  swapGLBuffers();
  captureFramebuffer();
  if (map) generateMapMipMap();
  checkOpenGLError();
}

bool isInMap(Shader shader) {
  return shaderInfo[shader].flags & _shaderInMap;
}

bool isInCapture(Shader shader) {
  return !(shaderInfo[shader].flags & _shaderNotInCapture);
}

GLuint loadShaderFile(char* filename, GLuint type) {
  char error[256];
  char* modFilename = strdup_s(lookupFile(filename, 0).c_str());
  char* modShaders = sprintf_o("%sshaders", modDirectory());
  // load from the mods directory
  char* shaderCode0 = stb_include_file(modFilename, "", modShaders, error);
  // load from the base directory
  char* shaderCode = stb_include_string(shaderCode0, "", "shaders",
      modFilename, error);

  if (!startsWith(shaderCode, "#version")) {
    SPDLOG_WARN("Shader file {} does not include #version directive",
        modFilename);
  }

  if (shaderCode == 0) {
    SPDLOG_ERROR("Could not open {}. Wrong directory? Shader Error: {}",
        modFilename, error);
    handleError("Bad shader (file)");
  }

  #ifdef LP_DEBUG
    SPDLOG_INFO("Compiled code for {}:\n{}", modFilename, shaderCode);
  #endif

  // Compile Shader
  GLuint shaderID = glCreateShader(type);
  glShaderSource(shaderID, 1, &shaderCode , NULL);
  glCompileShader(shaderID);

  // Check Shader
  GLint result = GL_FALSE;
  GLsizei infoLogLength = 0;
  glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
  glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
  if (infoLogLength > 0) {
    GLchar *programErrorMessage = new GLchar[infoLogLength+1];
    GLsizei actualLength = 0;
    glGetShaderInfoLog(shaderID, infoLogLength, &actualLength,
        programErrorMessage);
    std::string errorMsg = "";
    if (actualLength > 0)
      errorMsg = programErrorMessage;
    SPDLOG_ERROR("Shader Error ({}, {}): {}", filename, actualLength, errorMsg.length() > 0 ? errorMsg : "No info available");
    delete[] programErrorMessage;
  }
  if (result != GL_TRUE) {
    #ifndef LP_DEBUG
      // Output compiled code on error if not built in debug mode
      SPDLOG_INFO("Compiled code for {}:\n{}", modFilename, shaderCode);
    #endif
    handleError("Bad shader (compile)");
  }

  free(modFilename);
  free(shaderCode);
  free(modShaders);
  free(shaderCode0);

  return shaderID;
}

GLuint loadShaders(const char* filename) {
  SPDLOG_INFO("Loading shader {}", filename);
  char* vertexFilename = sprintf_o("shaders/%s.vertex.shader", filename);
  char* fragmentFilename = sprintf_o("shaders/%s.fragment.shader",
      filename);

  // Create the shaders
  GLuint vertexShaderID = loadShaderFile(vertexFilename, GL_VERTEX_SHADER);
  GLuint fragmentShaderID = loadShaderFile(fragmentFilename,
      GL_FRAGMENT_SHADER);
  free(vertexFilename);
  free(fragmentFilename);

  // Link the program
  GLuint programID = glCreateProgram();
  glAttachShader(programID, vertexShaderID);
  glAttachShader(programID, fragmentShaderID);
  glLinkProgram(programID);

  // Check the program
  GLint result = GL_FALSE;
  GLsizei infoLogLength = 0;
  glGetProgramiv(programID, GL_LINK_STATUS, &result);
  glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
  if (infoLogLength > 0) {
    GLchar *programErrorMessage = new GLchar[infoLogLength+1];
    GLsizei actualLength = 0;
    glGetProgramInfoLog(programID, infoLogLength, &actualLength,
        programErrorMessage);
    std::string errorMsg = "";
    if(actualLength > 0)
      errorMsg = programErrorMessage;
    SPDLOG_ERROR("Shader Link Error ({}, {}): {}", filename, actualLength, errorMsg.length() > 0 ? errorMsg : "No info available" );
    delete[] programErrorMessage;
  }
  if (result != GL_TRUE) {
    handleError("Bad shader (link)");
  }

  glDetachShader(programID, vertexShaderID);
  glDetachShader(programID, fragmentShaderID);

  glDeleteShader(vertexShaderID);
  glDeleteShader(fragmentShaderID);

  return programID;
}

void initShaders() {
  for (int s = 0; s < numShaders; s++) {
    GLuint programID = loadShaders(shaderInfo[s].filename);
    shaderInfo[s].programID = programID;

    for (int u = 0; u < numUniforms; u++) {
      uniforms[s][u] = glGetUniformLocation(programID, uniformName[u]);
    }
  }
}

GLuint getProgramID(Shader s) {
  return shaderInfo[s].programID;
}

const char* getShaderName(Shader s) {
  return shaderInfo[s].filename;
}

bool isShaderInstanced(Shader s) {
  return shaderInfo[s].flags & _shaderInstanced;
}

GLuint getUniformID(Shader s, Uniform u) {
  return uniforms[s][u];
}

