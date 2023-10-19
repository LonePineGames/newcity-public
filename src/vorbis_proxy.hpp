#pragma once

struct VorbisStream {
  unsigned int sampleRate;
  int channels;
};

int loadOgg(const char *filename, int *channels,
    int *sample_rate, short **output);
VorbisStream* startOggStream(int* error, const char* filename);
void stopOggStream(VorbisStream* stream);
int getNextOggFrame(VorbisStream* stream, short *buffer, int num_shorts);

