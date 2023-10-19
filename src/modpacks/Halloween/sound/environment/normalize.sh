#!/bin/bash

for f in *.wav; do
  echo "Normalizing $f..";
  ffmpeg-normalize $f -f -c:a libvorbis -e "-ac 1" -prf "lowpass=f=2000, afade=in:st=0:d=0.5" -of . -ext ogg
done

