#pragma once

#include "../main.hpp"

#include <string>

enum Framebuffer {
  MapFramebuffer, CaptureFramebuffer,
  ShadowFramebuffer, LightingFramebuffer, MainFramebuffer,
  numFramebuffers
};

void initFramebuffers();
void setupFramebuffer(Framebuffer f);
unsigned int getMapTexture();
void generateMapMipMap();
void captureFramebuffer(Framebuffer f, std::string filename);
void captureFramebuffer();
vec2 getCaptureDimensions();
void swapFramebuffers();

