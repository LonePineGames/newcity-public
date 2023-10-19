#!/bin/bash

hexdump -cx -e '4 "%+012d " "\n"' "$1" | less

