#ifndef MAIN_H
#define MAIN_H

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <vector>
using namespace std;
using glm::clamp;

#include "game/constants.hpp"

const float splashScreenTime = 5;

float getLastDurationError();
bool isRenderLagging();
bool isGameLagging();
void quickDraw();
void renderMap_g();
void renderCapture_g();
bool isMultithreading();
float getCameraTime();
int getFPSCap();
void setFPSCap(int cap);
float getFPS();
float getFrameDuration();
double getTargetFrameDuration_g();
void endGame();
void restartGame();

#endif
