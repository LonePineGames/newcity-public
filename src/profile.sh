#!/bin/bash

#mkdir -p ../build
#cd ../build && make -j 24 newcities && cd ../src && \
export LD_PRELOAD="./libsteam_api.so ./libOpenGL.so.0"; \
  valgrind --tool=callgrind -v --instr-atstart=no \
  --cache-sim=yes --branch-sim=yes --collect-bus=yes --dump-instr=yes \
  --separate-threads=yes \
  ./newcity

rm vgcore.*

  #--callgrind-out-file=callgrind.out \
  #--cache-sim=yes --branch-sim=yes --collect-bus=yes --dump-instr=yes \
#NOTE: use callgrind_control -i on

