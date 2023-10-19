cd ../build && make -j 24 newcity && cd ../src && \
  valgrind --tool=helgrind
  $1 ./newcity --debug

  #valgrind --tool=drd --exclusive-threshold=10
