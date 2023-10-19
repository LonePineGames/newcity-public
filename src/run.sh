cd ..
mkdir -p build
#ulimit -Sv 10000000
cd build && cmake -D CMAKE_BUILD_TYPE=Debug .. &&
  #make -j 4 game_test && ./game_test &&
  make -j 24 newcity && cd ../src &&
  #ASAN_OPTIONS=detect_leaks=1 \
  gdb -q -ex=run --args newcity --debug
  #gdb -q --statistics -ex=run --args newcity --debug

