#include "camera.hpp"

#include "../console/conDisplay.hpp"
#include "../error.hpp"
#include "../game/game.hpp"
#include "../building/building.hpp"
#include "../building/design.hpp"
#include "../heatmap.hpp"
#include "../input.hpp"
#include "../land.hpp"
#include "../option.hpp"
#include "../parts/part.hpp"
#include "../parts/toolbar.hpp"
#include "../person.hpp"
#include "../string.hpp"
#include "../selection.hpp"
#include "../serialize.hpp"
#include "../string_proxy.hpp"
#include "../time.hpp"
#include "../tutorial.hpp"
#include "../util.hpp"
#include "../weather.hpp"

#include "buffer.hpp"
#include "entity.hpp"
#include "framebuffer.hpp"
#include "shader.hpp"
#include "image.hpp"

#include "spdlog/spdlog.h"
#include <stdio.h>
#include <algorithm>
#include <unordered_map>
#include <glm/gtx/rotate_vector.hpp>

#ifdef _WIN32
  #define GLFW_EXPOSE_NATIVE_WIN32
  #define GLFW_EXPOSE_NATIVE_WGL
  #include "glfw3native.h"
#endif

const float shadowMapSize = 2048;
const float maxFOVDegrees = 120.0f;
const int maxGLErrorDepth = 16;
const float edgeScrollZoneY = 0.98f;
const float edgeScrollZoneX = 0.99f;

static float resolutionDistanceMult = 1;
static float resDistMultTarget = 1;
static bool freezeCamera = true;
static float fov = 0;
static float fovBias = 0;
static vec2 trackPos;
static bool trackingMouse = false;
static vec2 lastCursor;
static bool cameraClassicRMB = false;
static bool cameraEdgeScrolling = false;
static int windowFocus = GLFW_FALSE;
static item msaaSamples = 4;

const GLFWvidmode* nativeVideoMode;
WindowMode nextWindowMode = Fullscreen;
WindowMode nextWindowModeBack = Fullscreen;
WindowMode currentWindowMode = Fullscreen;

Perspective perspective_g = MainPerspective;
Perspective perspective_r = MainPerspective;

Camera camera;
Camera cameraTarget;
Camera cameraBack;
Camera uiCamera;
Camera uiCameraBack;
Camera shadowCamera;
Camera shadowCameraBack;
Camera skyboxCamera;
Camera skyboxCameraBack;
Camera weatherCamera;
Camera weatherCameraBack;
Camera mapCamera;
Camera mapCameraBack;
Camera captureCamera;
Camera captureCameraBack;

GLFWwindow* window;

std::unordered_map<GLuint, unsigned char> debugCallbackCount;

void setWindowState();

#ifndef _WIN32
__attribute__((stdcall)) void debugCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar *msg, const void *data ) {

  char count = debugCallbackCount[id];
  if (count >= 4) {
    return;
  }
  debugCallbackCount[id] = count+1;

  SPDLOG_INFO("GL Debug call: s:{} t:{} i:{} {} {} {}",
      source, type, id, severity, length, msg);
  logStacktrace();

  switch (id) {
    //case 131218: return; //NVIDIA warns about shader recompilation
    case 131185: return; //NVIDIA says it'll use video memory
    default: break;
  }

  //if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
}

#endif

void checkOpenGLError() {
  GLenum errorCode = glGetError();
  const GLubyte* errorString = 0;
  int i = 0;
  // GL error code should be checked in a loop until it returns GL_NO_ERROR
  while (errorCode != GL_NO_ERROR) {
    errorString = gluErrorString(errorCode);
    SPDLOG_ERROR("OpenGL error {} reported ({}/{}): {}", errorCode, i, maxGLErrorDepth, errorString != NULL ? errorString : (const GLubyte*)"Unknown error string" );

    // At max depth const for error depth, break out of loop and mark in log appropriately
    if (i >= maxGLErrorDepth) {
      SPDLOG_ERROR("FATAL ERROR: Max depth for OpenGL errors reached ({}/{}), breaking out of GL error checking!", i, maxGLErrorDepth);
      logStacktrace();
      return;
    }

    // Iterate and check again
    i++;
    errorCode = glGetError();
  }
}

bool getCameraClassicRMB() {
  return cameraClassicRMB;
}

void setCameraClassicRMB(bool val) {
  cameraClassicRMB = val;
}

bool getCameraEdgeScrolling() {
  return cameraEdgeScrolling;
}

void setCameraEdgeScrolling(bool val) {
  cameraEdgeScrolling = val;
}

item getMsaaSamples() {
  return msaaSamples;
}

void setMsaaSamples(item samples) {
  if (samples <= 0 || samples > 16) return;
  msaaSamples = samples;
}

double getCameraTargetZ(Camera cam) {
  if (isFollowingSelection()) {
    return cam.target.z;
  } else {
    return std::max(0., double(pointOnLand(cam.target).z)) +
      std::min(cam.distance * 0.1, 1000.);
  }
}

void resetCamera() {
  float mapSize = getMapSize();
  int gameMode = getGameMode();

  if (gameMode == ModeBuildingDesigner) {
    camera.distance = 150.0f;
    float c = tileSize*getMapTiles()/2;
    camera.target = vec3(tileSize*6, c+tileSize*.5f, 0);
    camera.yaw = pi_o*3/4;

  } else if (gameMode == ModeDesignOrganizer) {
    camera.distance = 4000.0f;
    camera.target = vec3(mapSize/2, mapSize/2, 0);
    camera.yaw = pi_o*5/4;

  } else {
    camera.distance = 20000.0f;
    camera.target = vec3(mapSize/2, mapSize/2, 0);
    camera.yaw = pi_o*3/4;
    if (isGameLoaded()) {
      item n = getTallestBuilding();
      if (n > 0) {
        camera.target = getBuildingTop(n);
        camera.distance = 4000.0f;
      }
    }
  }

  camera.pitch = c(CPitchClassic);
  if (isGameLoaded()) {
    camera.target.z = getCameraTargetZ(camera);
  }
  cameraTarget = camera;

  updateCamera(0);
  updateCamera(0);
}


void zoomCamera(float amount) {
  if(amount < 0.0001f)
    reportTutorialUpdate(TutorialUpdateCode::CameraZoomOut);
  else if(amount > 0.0001f)
    reportTutorialUpdate(TutorialUpdateCode::CameraZoomIn);

  amount *= cameraTarget.distance;
  cameraTarget.distance -= amount;
  float maxDist = 20.f*getMapTiles();
  if (cameraTarget.distance < c(CMinCameraDistance)) {
    cameraTarget.distance = c(CMinCameraDistance);
  } else if (cameraTarget.distance > maxDist) {
    cameraTarget.distance = maxDist;
  }
}

void scrollCamera(InputEvent event) {
  if (c(CEnableCameraMovement)) {
    zoomCamera(event.scroll.y * c(CCameraZoomSpeed));
  }
}

void setWindowSize(GLFWwindow* window, int width, int height) {
  SPDLOG_INFO("Window resized to {}x{} (Aspect Ratio: {})",
      width, height, width*1.0/height);
  //glfwSetWindowSize(window, width, height);
}

WindowMode getWindowMode() {
  return nextWindowMode;
}

void setWindowMode(WindowMode mode) {
  nextWindowMode = mode;
}

Plane::Plane() {
    pt = vec3(0.0f, 0.0f, 0.0f);
    norm = vec3(0.0f, 0.0f, 0.0f);
    D = 0.0f;
  }

Plane::Plane(vec3 p, vec3 n) {
  pt = p;
  norm = n;
  D = -dot(norm, pt);
}

void computeCameraMatricies(Camera* camera, vec3 mirror, float angle,
    bool doOrtho) {

  dvec3 camDir = -normalize(camera->direction);
  dvec3 camX = normalize(cross(dvec3(0.0f, 0.0f, 1.0f), camDir));
  dvec3 camUp = -normalize(cross(camDir, camX));
  camUp = rotate(camUp, camera->roll, camera->direction);

  //dvec3 up = camera->direction + dvec3(0,0,1);
  double effectiveDistance = camera->distance;
  if (fov <= 0) {
    doOrtho = true;
  } else if (!doOrtho) {
    effectiveDistance = camera->distance / fov * (2/3.f);
  }

  camera->position = camera->target -
    dvec3(camera->direction)*effectiveDistance;
  camera->ray = line(camera->position, camera->target);
  float aspectFactor = camera->aspectRatio/std::min(0.5, cos(camera->pitch));

  camera->view = lookAt(
    camera->position,
    camera->target,
    -camUp
  );

  float camNearDist = 0.0f;
  float camFarDist = 0.0f;

  if (doOrtho) {
    float planeDist = (effectiveDistance + camera->target.z)*aspectFactor;
    camNearDist = float(-planeDist * 2);
    camFarDist = float(planeDist*aspectFactor);
    camera->projection = scale(
      ortho(
        -camera->projSize.x, camera->projSize.x,
        -camera->projSize.y, camera->projSize.y,
        camNearDist,
        camFarDist
      ), mirror);

  } else {
    camNearDist = std::max(float(effectiveDistance*.1), 5.f);
    camFarDist = std::max(float(effectiveDistance*20.f), 20000.f);
    camera->projection = scale( perspective(pi_o*fov, // FoV in radians
        camera->aspectRatio,
        camNearDist,
        camFarDist
    ), mirror);
  }

  // Calculate view frustum
  float aspectRatio = getAspectRatio();
  Frustum* frustum = &camera->viewFrustum;
  vec3 camPos = camera->position;
  //vec3 camDir = -normalize(camera->direction);
  //vec3 camX = normalize(cross(vec3(0.0f, 0.0f, 1.0f), camDir));
  //vec3 camUp = -normalize(cross(camDir, camX));
  float hNear = 0.0f;
  float wNear = 0.0f;
  float hFar = 0.0f;
  float wFar = 0.0f;

  // Temp vars for calculating frustum plane normals
  vec3 normal, normDir;
  vec3 cNear, cFar, cTop, cRight, cBottom, cLeft;

  if (doOrtho) {
    hNear = effectiveDistance;
    hFar = effectiveDistance;

  } else {
    float fovRad = fov * pi_o;
    float fovTan = 2 * tan(fovRad*.5f);
    hNear = fovTan * camNearDist;
    hFar = fovTan * camFarDist;
    //SPDLOG_INFO("fov {} fovRad {} fovTan {} hNear {} hFar {}",
     //   fov, fovRad, fovTan, hNear, hFar);
  }

  wNear = hNear * aspectRatio;
  wFar = hFar * aspectRatio;

  // Near plane
  cNear = camPos + (vec3(camDir) * camNearDist);
  camera->viewFrustum.planes[FrustumPlanes::NEARP] =
    Plane(cNear, doOrtho ? vec3(camDir) : -vec3(camDir));

  // Far plane
  cFar = camPos + (vec3(camDir) * camFarDist);
  camera->viewFrustum.planes[FrustumPlanes::FARP] =
    Plane(cFar, doOrtho ? -vec3(camDir) : -vec3(camDir));

  // Top Plane
  cTop = cFar + (vec3(camUp) * hFar);
  normDir = normalize(cTop - camPos);
  normal = doOrtho ? -vec3(camUp) : -cross(normDir, vec3(camX));
  camera->viewFrustum.planes[FrustumPlanes::TOP] = Plane(cTop, normal);

  // Right Plane
  cRight = cFar + (vec3(camX) * wFar);
  normDir = normalize(cRight - camPos);
  normal = doOrtho ? -vec3(camX) : -cross(vec3(camUp), normDir);
  camera->viewFrustum.planes[FrustumPlanes::RIGHT] = Plane(cRight, normal);

  // Bottom Plane
  cBottom = cFar - (vec3(camUp) * hFar);
  normDir = normalize(cBottom - camPos);
  normal = doOrtho ? vec3(camUp) : -cross(vec3(camX), normDir);
  camera->viewFrustum.planes[FrustumPlanes::BOTTOM] = Plane(cBottom, normal);

  // Left Plane
  cLeft = cFar - (vec3(camX) * wFar);
  normDir = normalize(cLeft - camPos);
  normal = doOrtho ? vec3(camX) : -cross(normDir, vec3(camUp));
  camera->viewFrustum.planes[FrustumPlanes::LEFT] = Plane(cLeft, normal);

  frustum->nearDist = camNearDist;
  frustum->farDist = camFarDist;
  frustum->nearH = hNear;
  frustum->nearW = wNear;
  frustum->farH = hFar;
  frustum->farW = wFar;

  //if (angle != 0) {
    //camera->projection = rotate(camera->projection,
        //angle, vec3(0,0,1));
  //}

  camera->viewProjection = camera->projection * camera->view;
  //camera->viewProjection = camera->view * camera->projection;
  if (c(CMeshQuality) <= 0) {
    camera->resolutionDistance = 50000;
  } else {
    camera->resolutionDistance = clamp(camera->distance *
        pow(aspectFactor,.5) * resolutionDistanceMult / c(CMeshQuality),
      0.1, 50000.);
  }

  vec3 nearPoint = camera->target;
  vec3 farPoint = camera->target + normalize(camera->direction);
  vec4 nearPointVP = camera->viewProjection * vec4(nearPoint, 1);
  vec4 farPointVP = camera->viewProjection * vec4(farPoint, 1);
  nearPointVP /= nearPointVP.w;
  farPointVP /= farPointVP.w;
  float dist = farPointVP.z-nearPointVP.z;
  camera->distanceFactor = 1 / (getMeshQuality() * dist);
}

void computeCameraMatricies(Camera* camera, vec3 mirror, bool ortho) {
  computeCameraMatricies(camera, mirror, 0, ortho);
}

void setShadowCam() {
  Camera reference = getCurrentCamera_g();
  float aspectFactor = reference.aspectRatio /
    std::min(0.5, cos(reference.pitch));

  shadowCamera = reference;
  shadowCamera.direction = -reference.light.direction;
  //shadowCamera.target.z += shadowCamera.distance*.02f;
  if (fov > 0) shadowCamera.distance *= 2 + getFOV();

  dvec3 a = normalize(reference.direction);
  dvec3 b = normalize(shadowCamera.direction);
  float shadowAngle = atan2(b.y, b.x) - atan2(a.y, a.x);
  float xSkew = sin(-shadowAngle*2.f) *.5f;
  float ySkew = xSkew * .5f;

  // This math is super ad-hoc
  shadowCamera.projSize.y *= 1 + abs(sin(shadowAngle));
  shadowCamera.projSize.y *= aspectFactor * .25f;
  shadowCamera.projSize *= 3 - abs(sin(shadowAngle));

  //SPDLOG_INFO("shadowCamera");
  computeCameraMatricies(&shadowCamera, vec3(1, 1, 1), shadowAngle, true);

  // De-skew the rotated shadow map
  mat4 skew(
    1.0       , xSkew,      0.0, 0.0,
    ySkew     , 1.0,        0.0, 0.0,
    0.0       , 0.0,        1.0, 0.0,
    0.0       , 0.0,        0.0, 1.0
  );
  shadowCamera.viewProjection = skew * shadowCamera.viewProjection;
}

void positionCamera() {
  int iscreenWidth, iscreenHeight;
  glfwGetFramebufferSize(window,
      &iscreenWidth, &iscreenHeight);
  camera.window = vec2(iscreenWidth, iscreenHeight);
  camera.aspectRatio = camera.window.x / camera.window.y;
  camera.projSize = vec2(
      camera.aspectRatio * camera.distance,
      camera.distance);

  float aspectFactor = camera.aspectRatio/std::min(0.5, cos(camera.pitch));
  float dist = camera.distance;
  if (dist < 100) {
    dist = 100;
  }
  float cameraSize = dist; //*aspectFactor;
  camera.size = cameraSize;
  camera.invProjSize = vec2(1.f/camera.projSize.x, 1.f/camera.projSize.y);

  if (fov > 0) {
    fovBias = 0.3 * pow(fov, -0.778);
  } else {
    fovBias = std::max(camera.invProjSize.x, camera.invProjSize.y);
  }

  float vertCos = cos(camera.pitch);
  float vertSin = sin(camera.pitch);
  camera.direction = -normalize(vec3(
    cos(camera.yaw) * vertSin,
    sin(camera.yaw) * vertSin,
    vertCos
  ));

  // Calculate up and right vectors for camera
  dvec3 rawUp = vec3(0.0f, 1.0f, 0.0f);
  camera.right = cross(rawUp, camera.direction);
  camera.up = cross(camera.direction, camera.right);

  uiCamera.target = vec3(0,0,0);
  uiCamera.window = camera.window;
  uiCamera.size = 1000000;
  uiCamera.distance = 100000;
  uiCamera.aspectRatio = camera.aspectRatio;

  float xSize = uiGridSizeX * uiCamera.aspectRatio;
  float ySize = uiGridSizeY;
  uiCamera.projection = ortho(
    0.f, xSize,
    ySize, 0.f,
    -100.f, 100.f
  );

  uiCamera.view = mat4(1.0);
  uiCamera.viewProjection = uiCamera.projection * uiCamera.view;

  weatherCamera = camera;
  weatherCamera.target = vec3(0,0,0);
  weatherCamera.distance = 400;
  weatherCamera.size = 1000000;
  weatherCamera.projSize = vec2(
      weatherCamera.aspectRatio * weatherCamera.distance,
      weatherCamera.distance);
  vertCos = cos(weatherCamera.pitch);
  vertSin = sin(weatherCamera.pitch);
  weatherCamera.direction = -normalize(vec3(
    cos(weatherCamera.yaw) * vertSin,
    sin(weatherCamera.yaw) * vertSin,
    vertCos
  ));

  skyboxCamera = camera;
  //skyboxCamera.pitch = mix(camera.pitch, (double)c(CMaxPitch), 0.9);
  skyboxCamera.target = vec3(0,0,0);
  skyboxCamera.distance = 500;
  skyboxCamera.size = 1000000;
  skyboxCamera.projSize = vec2(
      skyboxCamera.aspectRatio * skyboxCamera.distance,
      skyboxCamera.distance);
  vertCos = cos(skyboxCamera.pitch);
  vertSin = sin(skyboxCamera.pitch);
  skyboxCamera.direction = -normalize(vec3(
    cos(skyboxCamera.yaw) * vertSin,
    sin(skyboxCamera.yaw) * vertSin,
    vertCos
  ));

  float mapSize = getMapSize();
  mapCamera = camera;
  mapCamera.distance = mapSize/2;
  mapCamera.size = mapCamera.distance;
  mapCamera.target = vec3(mapSize/2, mapSize/2, 0);
  mapCamera.pitch = 0.0001;
  mapCamera.yaw = 0;
  mapCamera.projSize = vec2(mapCamera.distance, mapCamera.distance);
  mapCamera.window = vec2(c(CSatMapResolution), c(CSatMapResolution));
  mapCamera.light.color = vec3(1,1,1);
  mapCamera.light.direction = vec3(0,0,1);
  vertCos = cos(mapCamera.pitch);
  vertSin = sin(mapCamera.pitch);
  mapCamera.direction = -normalize(vec3(
    cos(mapCamera.yaw) * vertSin,
    sin(mapCamera.yaw) * vertSin,
    vertCos
  ));

  captureCamera = camera;
  captureCamera.distance = 400;
  captureCamera.size = captureCamera.distance;

  captureCamera.target = vec3(mapSize/2, mapSize/2, 0);
  captureCamera.yaw = pi_o*3/4;
  if (isGameLoaded()) {
    item n = getTallestBuilding();
    if (n > 0) {
      captureCamera.target = getBuildingTop(n);
      vec3 designSize = getDesign(getBuilding(n)->design)->size;
      captureCamera.target.z -= designSize.z*.5f;
      if (getGameMode() == ModeBuildingDesigner) {
        float dim = std::max(designSize.x,
            std::max(designSize.y, designSize.z));
        dim *= 0.6f;
        dim += 20;
        captureCamera.distance = dim;
      } else {
        captureCamera.distance = c(CPreviewImageDistance);
      }
    }
  }

  captureCamera.pitch = c(CPitchClassic);
  captureCamera.window = getCaptureDimensions();
  float aspectRatio = captureCamera.window.x/captureCamera.window.y;
  captureCamera.projSize = vec2(captureCamera.distance,
      captureCamera.distance/aspectRatio);
  vertCos = cos(captureCamera.pitch);
  vertSin = sin(captureCamera.pitch);
  captureCamera.direction = -normalize(vec3(
    cos(captureCamera.yaw) * vertSin,
    sin(captureCamera.yaw) * vertSin,
    vertCos
  ));

  //SPDLOG_INFO("main camera");
  computeCameraMatricies(&camera, vec3(-1, 1, 1), false);
  //SPDLOG_INFO("weatherCamera");
  computeCameraMatricies(&weatherCamera, vec3(-1, 1, 1), false);
  //SPDLOG_INFO("mapCamera");
  computeCameraMatricies(&mapCamera, vec3(-1, 1, 1), true);
  //SPDLOG_INFO("captureCamera");
  computeCameraMatricies(&captureCamera, vec3(-1, 1, 1), true);
  //SPDLOG_INFO("skyboxCamera");
  computeCameraMatricies(&skyboxCamera, vec3(-1, 1, 1), false);
  camera.light.cameraSpace =
    normalize(vec3(camera.view * vec4(camera.light.direction,0)));

  mapCamera.light.cameraSpace =
    normalize(vec3(mapCamera.view * vec4(mapCamera.light.direction,0)));
  captureCamera.light.cameraSpace = normalize(vec3(captureCamera.view *
          vec4(captureCamera.light.direction,0)));

  setShadowCam();
}

void clearColorAndDepth() {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void clearSky() {
  glClearColor(cameraBack.clearColor.r,
    cameraBack.clearColor.g, cameraBack.clearColor.b, 0.0f);

  /*
  if (isUndergroundView()) {
    if (isHeatMapIntense()) {
      if (getHeatMap() == TrafficHeatMap) {
        vec3 trafficColor0 = vec3(0.01f, 0.01f, 0.2f);
        glClearColor(0.01f, 0.01f, 0.2f, 0.f);
      } else {
        float lightness = getHeatMap() == TransitHeatMap ? 1. : 0.5;
        glClearColor(lightness, lightness, lightness, 0.f);
      }
    } else {
      glClearColor(0.376, 0.208, 0.168, 0.0);
    }

  } else {
    glClearColor(
        cameraBack.light.skyColor[2].r,
        cameraBack.light.skyColor[2].g,
        cameraBack.light.skyColor[2].b,
        0.0f);
  }
  */

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void clearDepth() {
  glClear(GL_DEPTH_BUFFER_BIT);
}

void setMainViewport() {
  glViewport(0, 0, cameraBack.window.x, cameraBack.window.y);
}

void setupShadowCamera(GLuint programID) {
  GLuint shadowMatrixID = glGetUniformLocation(programID, "depthVP");
  glUniformMatrix4fv(shadowMatrixID, 1, GL_FALSE,
      &shadowCameraBack.viewProjection[0][0]);
  GLuint viewMatrixID = glGetUniformLocation(programID, "V");
  glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &cameraBack.view[0][0]);
  GLuint depthVID = glGetUniformLocation(programID, "depthV");
  glUniformMatrix4fv(depthVID, 1, GL_FALSE,
      &shadowCameraBack.view[0][0]);
  //glDisable(GL_FRAMEBUFFER_SRGB);
  //glDisable(GL_CULL_FACE);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);
}

void setupLightingCamera(GLuint programID) {
  GLuint lightingMatrixID = glGetUniformLocation(programID, "lightingVP");
  glUniformMatrix4fv(lightingMatrixID, 1, GL_FALSE,
      &cameraBack.viewProjection[0][0]);
  //glDisable(GL_FRAMEBUFFER_SRGB);
  glDisable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
}

void setupWeatherCamera(GLuint programID) {
  GLuint viewMatrixID = glGetUniformLocation(programID, "V");
  GLuint vpMatrixID = glGetUniformLocation(programID, "VP");
  GLuint lightColorID = glGetUniformLocation(programID, "LightColor");

  glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &weatherCameraBack.view[0][0]);
  glUniformMatrix4fv(vpMatrixID, 1, GL_FALSE,
      &weatherCameraBack.viewProjection[0][0]);
  glUniform3fv(lightColorID, 1, (const GLfloat*) &cameraBack.light.color);

  //glEnable(GL_FRAMEBUFFER_SRGB);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_DEPTH_CLAMP);
}

void setupSkyboxCamera(GLuint programID) {
  GLuint viewMatrixID = glGetUniformLocation(programID, "V");
  GLuint vpMatrixID = glGetUniformLocation(programID, "VP");
  GLuint lightColorID = glGetUniformLocation(programID, "LightColor");

  glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &skyboxCameraBack.view[0][0]);
  glUniformMatrix4fv(vpMatrixID, 1, GL_FALSE,
      &skyboxCameraBack.viewProjection[0][0]);
  glUniform3fv(lightColorID, 1, (const GLfloat*) &cameraBack.light.color);

  //glEnable(GL_FRAMEBUFFER_SRGB);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glDisable(GL_DEPTH_TEST);
}

void setupMainCamera(GLuint programID) {
  Camera myCam = getCurrentCamera_r();

  mat4 biasMatrix(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
  );
  mat4 depthBiasVP = getPerspective_r() == SatMapPerspective ? mat4(1) :
    biasMatrix*shadowCameraBack.viewProjection;
  mat4 lightingBiasVP = biasMatrix*myCam.viewProjection;

  GLuint viewMatrixID = glGetUniformLocation(programID, "V");
  GLuint vpMatrixID = glGetUniformLocation(programID, "VP");
  GLuint shadowBiasID = glGetUniformLocation(programID, "DepthBiasVP");
  GLuint lightingBiasID = glGetUniformLocation(programID, "lightingBiasVP");
  GLuint lightCameraSpaceID = glGetUniformLocation(programID,
    "lightDirection_cameraSpace");
  GLuint lightColorID = glGetUniformLocation(programID, "LightColor");
  GLuint screenSizeID = glGetUniformLocation(programID,
    "screenSize");

  glUniform1i(glGetUniformLocation(programID, "shadowMap"), 1);
  glUniform1i(glGetUniformLocation(programID, "lightingTex"), 2);

  glUniformMatrix4fv(shadowBiasID, 1, GL_FALSE, &depthBiasVP[0][0]);
  glUniformMatrix4fv(lightingBiasID, 1, GL_FALSE, &lightingBiasVP[0][0]);
  glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &myCam.view[0][0]);
  glUniformMatrix4fv(vpMatrixID, 1, GL_FALSE,
      &myCam.viewProjection[0][0]);
  glUniform3fv(lightCameraSpaceID, 1,
      (const GLfloat*) &myCam.light.cameraSpace);
  glUniform3fv(lightColorID, 1, (const GLfloat*) &myCam.light.color);
  glUniform2fv(screenSizeID, 1, (const GLfloat*) &myCam.window);
  glUniform1f(glGetUniformLocation(programID, "distanceFactor"),
      myCam.distanceFactor);

  //glEnable(GL_FRAMEBUFFER_SRGB);
  glEnable(GL_CULL_FACE);
  //glDisable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
  glEnable(GL_DEPTH_TEST);
}

void setupUICamera(GLuint programID) {
  GLuint viewMatrixID = glGetUniformLocation(programID, "V");
  GLuint vpMatrixID = glGetUniformLocation(programID, "VP");
  glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &uiCameraBack.view[0][0]);
  glUniformMatrix4fv(vpMatrixID, 1, GL_FALSE,
      &uiCameraBack.viewProjection[0][0]);
  //glEnable(GL_FRAMEBUFFER_SRGB);
  glDisable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
}

void glfwErrorCallback(int code, const char* description)
{
  SPDLOG_ERROR("ERROR: {:X} {}", code, description);
  logStacktrace();
}

void glfwFocusCallback(GLFWwindow* window, int focus)
{
  windowFocus = focus;
  SPDLOG_INFO("NewCity focus: {}", focus);
}

void setupCursor(float windowWidth, float windowHeight) {
  GLFWimage cursorImage;
  int x=0, y=0, n=0;
  cursorImage.pixels = loadImage("textures/cursor.png", &x, &y, &n, 4);
  cursorImage.width = x;
  cursorImage.height = y;
  GLFWcursor* cursor = glfwCreateCursor(&cursorImage, x*.5f, y*.5f);
  glfwSetCursor(window, cursor);
  glfwSetCursorPos(window, windowWidth*.5, windowHeight*.75);
  free(cursorImage.pixels);
}

int initGraphics() {
  SPDLOG_INFO("Initializing GLFW");
  if( !glfwInit() )
  {
    handleError("Failed to initialize GLFW");
  }

  glfwSetErrorCallback(glfwErrorCallback);
  glfwWindowHint(GLFW_SAMPLES, msaaSamples);
  SPDLOG_INFO("Setting MSAA Samples to {}", msaaSamples);
  //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  // To make MacOS happy; should not be needed
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  #ifdef LP_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
  #endif

  SPDLOG_INFO("Initializing Window");
  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  nativeVideoMode = mode;
  glfwWindowHint(GLFW_RED_BITS, mode->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

  SPDLOG_INFO("Monitor: {}, Mode: {}x{}@{}Hz, nextWindowMode: {}",
    monitor != 0, mode->width, mode->height,
    mode->refreshRate, nextWindowMode);

  // Open a window and create its OpenGL context
  currentWindowMode = nextWindowModeBack = nextWindowMode;
  if (currentWindowMode == WindowedFullscreen) {
    SPDLOG_INFO("glfwCreateWindow as WindowedFullscreen");
    window = glfwCreateWindow(
      mode->width,
      mode->height,
      "NewCity",
      NULL,
      NULL
    );

  } else if (currentWindowMode == Windowed) {
    SPDLOG_INFO("glfwCreateWindow as Windowed");
    window = glfwCreateWindow(
      mode->width*.5f, mode->height*.5f,
      "NewCity",
      NULL,
      NULL
    );

  } else if (currentWindowMode == Fullscreen) {
    SPDLOG_INFO("glfwCreateWindow as Fullscreen");
    window = glfwCreateWindow(
      mode->width,
      mode->height,
      "NewCity",
      monitor,
      NULL
    );

  } else {
    handleError("Unrecognized window mode");
  }

  if (window == NULL) {
    handleError("Could not open window");
  } else {
    windowFocus = GLFW_TRUE;
    glfwSetWindowFocusCallback(window, glfwFocusCallback);
  }

  #ifdef WIN32
    int x=0, y=0, n=0;
    GLFWimage image;
    image.pixels = loadImage("textures/icon.png", &x, &y, &n, 4);
    image.width = x;
    image.height = y;
    glfwSetWindowIcon(window, 1, &image);
    free(image.pixels);
  #endif

  glfwSetWindowSizeCallback(window, setWindowSize);

  if( window == NULL ){
    handleError("Failed to open GLFW window.");
  }
  SPDLOG_INFO("glfwMakeContextCurrent");
  glfwMakeContextCurrent(window);
  if(glfwGetCurrentContext() == NULL) {
    handleError("OpenGL context not created");
  }

  int err = glfwGetError(NULL);
  if (err != GLFW_NO_ERROR) {
    handleError("GLFW error: %s", std::to_string(err));
  }

  setWindowState();

  // Initialize GLEW
  SPDLOG_INFO("glewInit");
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    handleError("Failed to initialize GLEW");
  }

  SPDLOG_INFO("GL version: {}", glGetString(GL_VERSION));
  SPDLOG_INFO("GL vender: {}", glGetString(GL_VENDOR));
  SPDLOG_INFO("GL renderer: {}", glGetString(GL_RENDERER));
  SPDLOG_INFO("GLSL version: {}", glGetString(GL_SHADING_LANGUAGE_VERSION));
  #ifdef __linux__
    SPDLOG_INFO("registering debug callback\n");
    glDebugMessageCallback((GLDEBUGPROC)debugCallback, NULL);
  #endif

  GLint texture_units = 0;
  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
  SPDLOG_INFO("Texture units: {}", texture_units);

  initShaders();
  initFramebuffers();

  SPDLOG_INFO("Configuring OpenGL state, setting up framebuffers");
  glfwSwapInterval(1);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  if (msaaSamples > 1) {
    glEnable(GL_MULTISAMPLE);
  } else {
    glDisable(GL_MULTISAMPLE);
  }
  if (c(CGammaCorrect)) {
    glEnable(GL_FRAMEBUFFER_SRGB);
  } else {
    glDisable(GL_FRAMEBUFFER_SRGB);
  }

  setupCursor(mode->width, mode->height);
  SPDLOG_INFO("Graphics initialized successfully");
  return 0;
}

void resetGraphics() {
  SPDLOG_INFO("Resetting graphics");
  glfwTerminate();
}

void setWindowState() {
  bool fullscreen = currentWindowMode == Fullscreen;
  bool decorated = currentWindowMode == Windowed;
  bool maximized = currentWindowMode != Fullscreen;

  glfwSetWindowAttrib(window, GLFW_DECORATED, decorated);
  glfwSetWindowMonitor(window, fullscreen ? glfwGetPrimaryMonitor() : NULL,
      0, 0, nativeVideoMode->width, nativeVideoMode->height,
      nativeVideoMode->refreshRate);
  if (maximized) {
    glfwMaximizeWindow(window);
  }

  #ifdef _WIN32
    HWND hwnd = glfwGetWin32Window(window);
    if (maximized) {
      ShowWindowAsync(hwnd, SW_MAXIMIZE);
    }

    //Remove decorations
    LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
    if (decorated) {
      lStyle |= WS_OVERLAPPEDWINDOW;
    } else {
      lStyle &= ~(WS_CAPTION | WS_THICKFRAME |
          WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
    }
    SetWindowLongPtr(hwnd, GWL_STYLE, lStyle);
    SetWindowPos(hwnd, NULL, 0,0,0,0,
      SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOOWNERZORDER);
  #endif
}

void updateWindowMode() {
  if (currentWindowMode != nextWindowModeBack) {
    currentWindowMode = nextWindowModeBack;
    setWindowState();

    /*
    if (currentWindowMode == WindowedFullscreen) {
      SPDLOG_INFO("glfwSetWindowMonitor as WindowedFullscreen");
      glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
      glfwSetWindowMonitor(window, NULL, 0, 0,
          nativeVideoMode->width, nativeVideoMode->height,
          nativeVideoMode->refreshRate);

    } else if (currentWindowMode == Windowed) {
      SPDLOG_INFO("glfwSetWindowMonitor as Windowed");
      glfwSetWindowMonitor(window, NULL, 0, 0,
          nativeVideoMode->width*.5f, nativeVideoMode->height*.5f,
          nativeVideoMode->refreshRate);
      glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
      #ifdef _WIN32
        ShowWindowAsync(hwnd, SW_MAXIMIZE);
      #elif
        glfwMaximizeWindow(window);
      #endif

    } else if (currentWindowMode == Fullscreen) {
      SPDLOG_INFO("glfwSetWindowMonitor as Fullscreen");
      glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
      glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0,
          nativeVideoMode->width, nativeVideoMode->height,
          nativeVideoMode->refreshRate);

    } else {
      handleError("Unrecognized window mode");
    }
    */
  }
}

void swapGLBuffers() {
  glfwSwapBuffers(window);
}

void swapCameras() {
  cameraBack = camera;
  uiCameraBack = uiCamera;
  shadowCameraBack = shadowCamera;
  skyboxCameraBack = skyboxCamera;
  weatherCameraBack = weatherCamera;
  mapCameraBack = mapCamera;
  captureCameraBack = captureCamera;
  nextWindowModeBack = nextWindowMode;
  perspective_r = perspective_g;
  perspective_g = MainPerspective;
}

void updateCamera(double duration) {

  float mapSize = getMapSize();
  float mapBuffer = c(CCityDistance)*15;
  if (cameraTarget.target.x < -mapBuffer)
    cameraTarget.target.x -= c(CCameraSpringSpeed) *
      (cameraTarget.target.x + mapBuffer);
  if (cameraTarget.target.y < -mapBuffer)
    cameraTarget.target.y -= c(CCameraSpringSpeed) *
      (cameraTarget.target.y + mapBuffer);
  if (cameraTarget.target.x > mapSize+mapBuffer)
    cameraTarget.target.x -= c(CCameraSpringSpeed) *
      (cameraTarget.target.x-mapSize-mapBuffer);
  if (cameraTarget.target.y > mapSize+mapBuffer)
    cameraTarget.target.y -= c(CCameraSpringSpeed) *
      (cameraTarget.target.y-mapSize-mapBuffer);

  double alpha = c(CCameraLag) == 0 ? 1 :
    clamp(duration/c(CCameraLag), 0., 0.2);

  camera.light = getLightInformation();
  if (isUndergroundView()) {
    if (isHeatMapIntense()) {
      if (getHeatMap() == TrafficHeatMap) {
        camera.clearColor = vec3(0.01f, 0.01f, 0.2f)*.25f; // traffic blue
      } else {
        float lightness = getHeatMap() == ZoneHeatMap ? .1f :
          getHeatMap() == RoadHeatMap ? .25f : 0.5;
        camera.clearColor = vec3(lightness, lightness, lightness);
      }
    } else {
      //camera.clearColor = vec3(0.376, 0.208, 0.168)*.1f; // dirt
      camera.clearColor = vec3(0.117, 0.032, 0.020)*.25f; // dirt
      //camera.clearColor *= camera.light.skyColor[2];
    }

  } else {
    //camera.clearColor = camera.light.skyColor[2];
  }

  //if (!freezeCamera) {
    if (abs(cameraTarget.pitch - c(CPitchClassic)) < 0.15) {
      cameraTarget.pitch = mix(cameraTarget.pitch,
          (double)c(CPitchClassic), alpha);
    }

    for (int i = 0; i <= 8; i++) {
      double p = pi_o*i/8;
      if (abs(cameraTarget.yaw - p) < 0.15) {
        cameraTarget.yaw = mix(cameraTarget.yaw, p, alpha);
      }
    }

    if (isGameLoaded() && c(CEnableCameraMovement)) {
      //double cameraLandZ = getCameraTargetZ(cameraTarget);
      //cameraTarget.target.z = mix(cameraTarget.target.z, cameraLandZ, alpha);
      cameraTarget.target.z = getCameraTargetZ(cameraTarget);
    }

    vec3 oldTarget = camera.target;
    camera.target = mix(camera.target, cameraTarget.target, alpha);
    camera.pitch = mix(camera.pitch, cameraTarget.pitch, alpha);
    camera.yaw = mix(camera.yaw, cameraTarget.yaw, alpha);
    camera.roll = mix(camera.roll, cameraTarget.roll, alpha);
    float oldDist = camera.distance;
    camera.distance = mix(camera.distance, cameraTarget.distance, alpha);

    vec2 shift = vec2(0,0);
    if (camera.pitch > 0.1) {
      vec3 targetDiff = vec3(camera.target) - oldTarget;
      vec3 cameraDirection2D = vec3(cos(cameraTarget.yaw),
          sin(cameraTarget.yaw), 0);
      vec3 cameraDirection2DNormal =
        vec3(cameraDirection2D.y, -cameraDirection2D.x, 0);
      cameraDirection2D *= 5*cos(cameraTarget.pitch);
      shift = vec2(dot(targetDiff, cameraDirection2DNormal),
          -dot(targetDiff, cameraDirection2D));
      shift *= 0.5f;
    }
    shift.y += (camera.distance - oldDist);
    shift /= camera.distance;
    moveRain(shift);
  //}

  //float durationError = clamp(getLastDurationError()+.5f, -.05f, .05f);
  //resDistMultTarget += durationError;
  //resDistMultTarget = clamp(resDistMultTarget, 0.01f, 10.f);
  //resolutionDistanceMult = mix(resolutionDistanceMult, resDistMultTarget,
      //duration);
  //SPDLOG_INFO("resDistMult {} durError {}", resolutionDistanceMult,
   //   durationError);

  positionCamera();
}

//int ko = 0;
//int yo = 0;

void moveCamera(double duration, InputEvent event) {
  if (freezeCamera) return;
  if (consoleIsOpen()) return;
  if (!c(CEnableCameraMovement)) return;
  if (duration > 0.2) duration = 0.2;

  vec2 cursor = event.cameraSpaceMouseLoc;
  vec2 movement = cursor - lastCursor;
  lastCursor = cursor;

  if (event.isButtonDown[MMB] || (event.isButtonDown[LMB] && (event.mods & GLFW_MOD_CONTROL))) {
    // Check for Tutorial updates
    if(movement.x > 0.0f)
      reportTutorialUpdate(TutorialUpdateCode::CameraRotateLeft);
    else if(movement.x < 0.0f)
      reportTutorialUpdate(TutorialUpdateCode::CameraRotateRight);
    else if(movement.y > 0.0f)
      reportTutorialUpdate(TutorialUpdateCode::CameraRotateDown);
    else if(movement.y < 0.0f)
      reportTutorialUpdate(TutorialUpdateCode::CameraRotateUp);

    cameraTarget.yaw   += c(CCameraYawSpeed) * movement.x;
    cameraTarget.pitch += c(CCameraPitchSpeed) * movement.y;
    if (cameraTarget.pitch > c(CMaxPitch)) {
      cameraTarget.pitch = c(CMaxPitch);
    } else if (cameraTarget.pitch < c(CMinPitch)) {
      cameraTarget.pitch = c(CMinPitch);
    }
  }

  // Capture mouse position when we start holding down
  // the RMB, if we haven't started tracking it already
  if(!event.isButtonDown[RMB]) {
    trackPos = vec2(0, 0);
    trackingMouse = false;
  } else if (!trackingMouse) {
    trackingMouse = true;
    trackPos = cursor;
  }

  vec3 cameraDirection2D = vec3(cos(cameraTarget.yaw),
      sin(cameraTarget.yaw), 0);
  vec3 cameraDirection2DNormal =
    vec3(cameraDirection2D.y, -cameraDirection2D.x, 0);
  cameraDirection2D /= std::max(0.1, cos(cameraTarget.pitch));
  float moveFactor = duration * cameraTarget.distance * (1 - .9f*getFOV());
  vec3 cameraDirection2Dm = cameraDirection2D * 
    moveFactor * c(CCameraSpeedY);
  vec3 cameraDirection2DNormalm = cameraDirection2DNormal *
    moveFactor * c(CCameraSpeedX);

  bool unmoddedRMB = event.isButtonDown[RMB] && !(event.mods & GLFW_MOD_CONTROL);

  if(unmoddedRMB && trackingMouse) {
    if(cameraClassicRMB) {
      vec2 flatVec = cursor - trackPos;
      cameraTarget.target += ((cameraDirection2D * -flatVec.y) +
        (cameraDirection2DNormal * flatVec.x)) *
        float(c(CCameraPanSpeedClassic) *
          camera.distance * 0.025f);
      setFollowingSelection(false);
    } else {
      vec2 flatVec = movement * float(camera.distance*c(CCameraPanSpeed));
      cameraTarget.target -= (cameraDirection2D * -flatVec.y) +
        (cameraDirection2DNormal * flatVec.x);
      float bounce = 1 - c(CCameraPanSmoothing);
      camera.target -= .5f* ((cameraDirection2D * -flatVec.y) +
        (cameraDirection2DNormal * flatVec.x));
    }
  }

  if (getKeyBind((int)InputAction::ActMoveUp).active ||
      (windowFocus == GLFW_TRUE && cameraEdgeScrolling && cursor.y > (1.0f * edgeScrollZoneY))) {
    cameraTarget.target -= cameraDirection2Dm;
    setFollowingSelection(false);
    reportTutorialUpdate(TutorialUpdateCode::CameraMoveUp);
  }

  if (getKeyBind((int)InputAction::ActMoveLeft).active ||
      (windowFocus == GLFW_TRUE && cameraEdgeScrolling && cursor.x < (-1.0f * edgeScrollZoneX))) {
    cameraTarget.target -= cameraDirection2DNormalm;
    setFollowingSelection(false);
    reportTutorialUpdate(TutorialUpdateCode::CameraMoveLeft);
  }

  if (getKeyBind((int)InputAction::ActMoveDown).active ||
      (windowFocus == GLFW_TRUE && cameraEdgeScrolling && cursor.y < (-1.0f * edgeScrollZoneY))) {
    cameraTarget.target += cameraDirection2Dm;
    setFollowingSelection(false);
    reportTutorialUpdate(TutorialUpdateCode::CameraMoveDown);
  }

  if (getKeyBind((int)InputAction::ActMoveRight).active ||
      (windowFocus == GLFW_TRUE && cameraEdgeScrolling && cursor.x > (1.0f * edgeScrollZoneX))) {
    cameraTarget.target += cameraDirection2DNormalm;
    setFollowingSelection(false);
    reportTutorialUpdate(TutorialUpdateCode::CameraMoveRight);
  }

  if (getKeyBind((int)InputAction::ActRotateLeft).active) {
    cameraTarget.yaw += c(CCameraYawKeySpeed) * float(duration);
    reportTutorialUpdate(TutorialUpdateCode::CameraRotateLeft);
    // Cheating for laptop users
    reportTutorialUpdate(TutorialUpdateCode::CameraRotateUp);
  }
  if (getKeyBind((int)InputAction::ActRotateRight).active) {
    cameraTarget.yaw -= c(CCameraYawKeySpeed) * float(duration);
    reportTutorialUpdate(TutorialUpdateCode::CameraRotateRight);
    // Cheating for laptop users
    reportTutorialUpdate(TutorialUpdateCode::CameraRotateDown);
  }

  float zoomAmount = 0;
  if (getKeyBind((int)InputAction::ActZoomIn).active) zoomAmount ++;
  if (getKeyBind((int)InputAction::ActZoomOut).active) zoomAmount --;
  zoomCamera(zoomAmount * c(CCameraKeyZoomSpeed));
}

bool inFrustrum(Camera c, vec3 eloc, float eSize) {
  Plane p;

  //for(int i = FrustumPlanes::BOTTOM; true; i ++) {
  //for(int i = yo; true; i ++) {
  for(int i = 0; i < FrustumPlanes::NUM_PLANES; i ++) {
  //for(int i = 1; i < 2; i++) {
    p = c.viewFrustum.planes[i];
    if(dot(p.norm, eloc) + p.D < -eSize) {
      return false;
    }
    //break;
  }

  return true;
}

float getHorizontalCameraAngle() {
  return camera.yaw;
}

void setHorizontalCameraAngle(float angle) {
  float diff = angle - cameraTarget.yaw;
  if (diff > pi_o) {
    diff -= 2*pi_o;
  }

  cameraTarget.yaw += diff*.01f;
}

float getVerticalCameraAngle() {
  return camera.pitch;
}

float getCameraDistance() {
  return camera.distance;
}

void setCameraDistance(float dist) {
  cameraTarget.distance = dist;
}

vec3 getCameraTarget() {
  return camera.target;
}

float getAspectRatio() {
  return camera.aspectRatio;
}

void setCameraTarget(vec3 location) {
  if (validate(location)) {
    cameraTarget.target = location;
  }
}

void setCameraYaw(float yaw) {
  cameraTarget.yaw = yaw;
}

void setCameraPitch(float pitch) {
  cameraTarget.pitch = pitch;
}

void setCameraRoll(float roll) {
  cameraTarget.roll = roll;
}

Camera getUICamera() {
  return uiCamera;
}

Camera getMainCamera() {
  return camera;
}

Camera getMainCameraBack() {
  return cameraBack;
}

Camera getMapCamera() {
  return mapCamera;
}

Camera getCaptureCamera() {
  return captureCamera;
}

Camera getCaptureCameraBack() {
  return captureCameraBack;
}

Camera getCurrentCamera_r() {
  switch (perspective_r) {
    case MainPerspective: return cameraBack;
    case SatMapPerspective: return mapCameraBack;
    case CapturePerspective: return captureCameraBack;
  }
  handleError("Invalid Perspective");
  return cameraBack;
}

Camera getCurrentCamera_g() {
  switch (perspective_g) {
    case MainPerspective: return camera;
    case SatMapPerspective: return mapCamera;
    case CapturePerspective: return captureCamera;
  }
  handleError("Invalid Perspective");
  return camera;
}

void setNextPerspective_g(item nextPerspective) {
  perspective_g = (Perspective) nextPerspective;
}

Perspective getPerspective_r() {
  return perspective_r;
}

Perspective getPerspective_g() {
  return perspective_g;
}

GLFWwindow* getWindow() {
  return window;
}

void setFreezeCamera(bool v) {
  freezeCamera = v;
}

void setFOV(float newFOV) { fov = newFOV; }
float getFOV() { return fov; }
float getFOVBias() { return fovBias; }
float getFOVMaxDegrees() { return maxFOVDegrees; }
float getFOVMaxRadians() { return maxFOVDegrees / 180.0f; }
