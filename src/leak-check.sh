#cd ../build && make -j 24 newcity && cd ../src &&

  #--show-leak-kinds=all \
  #--vgdb=yes --vgdb-error=0 \
./linux.sh && export LD_PRELOAD="./libsteam_api.so ./libOpenGL.so.0"; \
  valgrind \
  --track-origins=yes --leak-check=full \
  $1 ./newcity --debug 2>&1 | tee leak-check.out
