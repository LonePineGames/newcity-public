#!/bin/bash

ffmpeg-normalize *.ogg -f -c:a libvorbis -e "-ac 1" -of . -ext ogg
#ffmpeg-normalize traffic-*.ogg -f -c:a libvorbis -e "-ac 1" -prf "lowpass=f=2000, afade=in:st=0:d=0.5" -of . -ext ogg
