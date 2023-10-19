#!/bin/bash

cd ..
mkdir -p build-arch
cd build-arch && cmake -D CMAKE_BUILD_TYPE=Release .. &&
  make -j 16 newcity && cd .. || exit 1

