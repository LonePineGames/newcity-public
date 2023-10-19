#!/bin/bash

#mkdir -p ../build
#cd ../build && make -j 24 newcity && cd ../src && \
export LD_PRELOAD="./libsteam_api.so ./libOpenGL.so.0"; \
  valgrind --tool=massif --xtree-memory=full -v ./newcity

  #--cache-sim=yes --branch-sim=yes --collect-bus=yes --dump-instr=yes \
#NOTE: use callgrind_control -i on

