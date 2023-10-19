cd ..
mkdir -p build
#ulimit -Sv 10000000
cd build && cmake -D CMAKE_BUILD_TYPE=Debug .. &&
  make -j 24 game_test && ./game_test

