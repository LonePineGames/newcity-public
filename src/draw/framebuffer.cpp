#include "framebuffer.hpp"

#include "camera.hpp"
#include "texture.hpp"

#include "spdlog/spdlog.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

typedef void (*FramebufferInit)(Framebuffer f);
typedef void (*FramebufferSetup)(Framebuffer f);

static item nextFbToCapture = -1;
static item fbToCapture = -1;
static std::string nextCaptureFilename;
static std::string filenameToInvalidate = "";
static GLuint captureDepthTex = 0;

struct FramebufferInfo {
  FramebufferInit init;
  FramebufferSetup setup;
  vec2 size;
  GLuint name = 0;
  GLuint texture = 0;
};

void initMapFramebuffer(Framebuffer f);
void initCaptureFramebuffer(Framebuffer f);
void initShadowFramebuffer(Framebuffer f);
void initLightingFramebuffer(Framebuffer f);
void initMainFramebuffer(Framebuffer f);
void setupMapFramebuffer(Framebuffer f);
void setupCaptureFramebuffer(Framebuffer f);
void setupShadowFramebuffer(Framebuffer f);
void setupLightingFramebuffer(Framebuffer f);
void setupMainFramebuffer(Framebuffer f);

FramebufferInfo framebufferInfo[numFramebuffers] = {
  {initMapFramebuffer, setupMapFramebuffer, vec2(8192, 8192)},
  {initCaptureFramebuffer, setupCaptureFramebuffer, vec2(960, 640)},
  {initShadowFramebuffer, setupShadowFramebuffer, vec2(4096, 4096)},
  {initLightingFramebuffer, setupLightingFramebuffer, vec2(2048, 2048)},
  {initMainFramebuffer, setupMainFramebuffer, vec2(0, 0)},
};

void initMapFramebuffer(Framebuffer f) {
  framebufferInfo[f].size.x = c(CSatMapResolution);
  framebufferInfo[f].size.y = c(CSatMapResolution);
  SPDLOG_INFO("c(CSatMapResolution) = {}", c(CSatMapResolution));

  glGenFramebuffers(1, &framebufferInfo[f].name);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferInfo[f].name);
  glGenTextures(1, &framebufferInfo[f].texture);

  FramebufferInfo info = framebufferInfo[f];

  glBindTexture(GL_TEXTURE_2D, info.texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, info.size.x, info.size.y,
    0, GL_RGBA, GL_FLOAT, 0);
  checkOpenGLError();

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.f);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
      info.texture, 0);
}

void initCaptureFramebuffer(Framebuffer f) {
  glGenFramebuffers(1, &framebufferInfo[f].name);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferInfo[f].name);
  glGenTextures(1, &framebufferInfo[f].texture);
  FramebufferInfo info = framebufferInfo[f];

  glBindTexture(GL_TEXTURE_2D, info.texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, info.size.x, info.size.y,
    0, GL_RGBA, GL_FLOAT, 0);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.f);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
      info.texture, 0);

  glGenTextures(1, &captureDepthTex);
  glBindTexture(GL_TEXTURE_2D, captureDepthTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
      info.size.x, info.size.y,
      0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
    GL_COMPARE_R_TO_TEXTURE);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, captureDepthTex, 0);
}

void initShadowFramebuffer(Framebuffer f) {
  glGenFramebuffers(1, &framebufferInfo[f].name);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferInfo[f].name);
  glGenTextures(1, &framebufferInfo[f].texture);
  FramebufferInfo info = framebufferInfo[f];

  glBindTexture(GL_TEXTURE_2D, info.texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
    info.size.x, info.size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
    GL_COMPARE_R_TO_TEXTURE);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, info.texture, 0);
  glDrawBuffer(GL_NONE);
}

void initLightingFramebuffer(Framebuffer f) {
  glGenFramebuffers(1, &framebufferInfo[f].name);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferInfo[f].name);
  glGenTextures(1, &framebufferInfo[f].texture);
  FramebufferInfo info = framebufferInfo[f];

  glBindTexture(GL_TEXTURE_2D, info.texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, info.size.x, info.size.y,
    0, GL_RGBA, GL_FLOAT, 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
      info.texture, 0);
}

void initMainFramebuffer(Framebuffer f) {
  // No-op
}

void setupMapFramebuffer(Framebuffer f) {
  FramebufferInfo info = framebufferInfo[f];
  glBindFramebuffer(GL_FRAMEBUFFER, info.name);
  glViewport(0, 0, info.size.x, info.size.y);
  clearColorAndDepth();
}

void setupCaptureFramebuffer(Framebuffer f) {
  FramebufferInfo info = framebufferInfo[f];
  glBindFramebuffer(GL_FRAMEBUFFER, info.name);
  glViewport(0, 0, info.size.x, info.size.y);
  clearColorAndDepth();
}

void setupShadowFramebuffer(Framebuffer f) {
  FramebufferInfo info = framebufferInfo[f];
  glBindFramebuffer(GL_FRAMEBUFFER, info.name);
  glViewport(0, 0, info.size.x, info.size.y);
  clearDepth();
}

void setupLightingFramebuffer(Framebuffer f) {
  FramebufferInfo info = framebufferInfo[f];
  glBindFramebuffer(GL_FRAMEBUFFER, info.name);
  glViewport(0, 0, info.size.x, info.size.y);
  clearColorAndDepth();
}

void setupMainFramebuffer(Framebuffer f) {
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, framebufferInfo[ShadowFramebuffer].texture);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, framebufferInfo[LightingFramebuffer].texture);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  setMainViewport();
  clearSky();
}

unsigned int getMapTexture() {
  return framebufferInfo[MapFramebuffer].texture;
}

void generateMapMipMap() {
  glBindTexture(GL_TEXTURE_2D, framebufferInfo[MapFramebuffer].texture);
  glGenerateMipmap(GL_TEXTURE_2D);
}

void initFramebuffers() {
  SPDLOG_INFO("Initializing Framebuffers");
  for (int f = 0; f < numFramebuffers; f++) {
    framebufferInfo[f].init((Framebuffer)f);
  }
}

void setupFramebuffer(Framebuffer f) {
  framebufferInfo[f].setup(f);
}

void captureFramebuffer() {
  if (fbToCapture < 0) return;
  if (fbToCapture >= numFramebuffers) {
    SPDLOG_INFO("Did not capture framebuffer {}: out of range",
        fbToCapture);
    fbToCapture = -1;
  }

  FramebufferInfo info = framebufferInfo[fbToCapture];
  item size = info.size.x * info.size.y * 4;
  if (size <= 0) {
    fbToCapture = -1;
    return;
  }

  void* pixels = calloc(size, sizeof(char));
  glBindTexture(GL_TEXTURE_2D, info.texture);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  stbi_flip_vertically_on_write(1); // flag is non-zero to flip data vertically
  int err = stbi_write_png(nextCaptureFilename.c_str(),
      info.size.x, info.size.y, 4, pixels, info.size.x*4*sizeof(char));
  free(pixels);
  SPDLOG_INFO("Captured framebuffer {} to {}; {}x{}",
      fbToCapture, nextCaptureFilename, info.size.x, info.size.y);
  fbToCapture = -1;
  filenameToInvalidate = nextCaptureFilename;
}

void captureFramebuffer(Framebuffer f, std::string filename) {
  nextFbToCapture = f;
  nextCaptureFilename = filename;
};

vec2 getCaptureDimensions() {
  return framebufferInfo[CaptureFramebuffer].size;
}

void swapFramebuffers() {
  if (fbToCapture != -1 || nextFbToCapture != -1) {
    SPDLOG_INFO("swapFramebuffers {} {}", fbToCapture, nextFbToCapture);
  }

  if (fbToCapture == -1) {
    fbToCapture = nextFbToCapture;
    nextFbToCapture = -1;
  }

  if (filenameToInvalidate.length() > 0) {
    invalidateTexture(filenameToInvalidate.c_str());
    filenameToInvalidate = "";
  }
}

