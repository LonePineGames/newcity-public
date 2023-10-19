#sh

cd ..
mkdir -p build-windows
cd build-windows && \
  cmake \
  -DCMAKE_TOOLCHAIN_FILE=../Toolchain-mingw32.cmake \
  -DCMAKE_BUILD_TYPE=Release .. \
  && make -j 16 newcity || exit 1
  #-DCMAKE_BUILD_TYPE=Release .. \
  #-DCMAKE_BUILD_TYPE=relwithdebinfo .. \

cd ../src
exec ./generate-map.sh

