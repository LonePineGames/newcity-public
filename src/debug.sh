mkdir -p ../build
cd ../build && make playground && cd ../playground && gdb -q -ex run playground
