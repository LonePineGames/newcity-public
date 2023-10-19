#cd ../build && make -j 24 newcity && cd ../src && \
export LD_PRELOAD="./libsteam_api.so ./libOpenGL.so.0"; \
  valgrind --suppressions=./.valgrind-suppressions \
  --redzone-size=4096 \
  --gen-suppressions=all --track-origins=yes \
  $1 ./newcity --debug
  #--gen-suppressions=all --db-attach=yes
  #--vgdb=yes --vgdb-error=1 \
