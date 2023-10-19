#!/bin/bash

echo "Generating binary map for exe"
nm -na --defined-only --demangle newcity.exe |\
  awk -F " " '$2 == "T" || $2 == "t"' |\
  cut -d" " -f1,3- > binary_map

#nm -lgn --defined-only --demangle newcity.exe | grep 00000 | grep -v debug_ranges | grep -v debug_loc | cut -d" " -f1,3- | sort > binary_map
