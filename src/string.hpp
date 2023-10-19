#ifndef STRING_H
#define STRING_H

#include "draw/mesh.hpp"
#include <string>

void parseFont();
void renderString(Mesh* mesh, const char* string, vec3 start, float size);
void renderString(Mesh* mesh, const char* string, vec3 start, float size,
    float firstLineIndent);
void renderString(Mesh* mesh, const char* string, vec3 start, vec3 along);
void renderString(Mesh* mesh, const char* string, vec3 start, vec3 along,
  vec3 down);
void renderStringCentered(Mesh* mesh, const char* string,
  vec3 center, vec3 along, vec3 down);
float characterWidth(char c);
float characterWidth(char c,char next);
float stringWidth(const char* string);
vec2 stringDimensions(const char* string);
uint32_t getMaxStrLen();

#endif
