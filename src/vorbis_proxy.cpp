#include "vorbis_proxy.hpp"

#include "stb_vorbis.c"

int loadOgg(const char *filename, int *channels,
    int *sample_rate, short **output) {
  return stb_vorbis_decode_filename(filename, channels, sample_rate, output);
}

VorbisStream* startOggStream(int* error, const char* filename) {
  return (VorbisStream*) stb_vorbis_open_filename(filename, error, NULL);
}

int getNextOggFrame(VorbisStream* stream, short *buffer, int num_shorts) {
  return stb_vorbis_get_frame_short_interleaved(
      (stb_vorbis*)stream, stream->channels, buffer, num_shorts);
}

void stopOggStream(VorbisStream* stream) {
  stb_vorbis_close((stb_vorbis*)stream);
}

