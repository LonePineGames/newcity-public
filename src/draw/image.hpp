#ifndef IMAGE_H
#define IMAGE_H

#include "../main.hpp"

struct Image {
  int x;
  int y;
  int channels;
  unsigned char* data;
};

unsigned char* loadImage(const char* filename,
    int* x, int* y, int* n, int d);
void freeImage(unsigned char* image);

Image loadImage(const char* filename);
Image createImage(int x, int y);
void freeImage(Image img);

void blitImage(Image source, Image dest, int x, int y);
vec4 samplePixel(Image source, float x, float y);

#endif
