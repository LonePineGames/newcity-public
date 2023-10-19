#!/bin/bash

cd ..
mkdir -p build-release
cd build-release && cmake -D CMAKE_BUILD_TYPE=Release .. &&
  make -j 16 newcity && cd .. || exit 1

