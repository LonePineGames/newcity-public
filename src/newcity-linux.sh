#!/bin/sh

SCRIPT=$(readlink -f "$0")
GAMEPATH=$(dirname "$SCRIPT")
cd $GAMEPATH
echo "Launching NewCity for Linux from $GAMEPATH"
exec env "LD_LIBRARY_PATH=./:${LD_LIBRARY_PATH}" ./newcity 
#export LD_PRELOAD="./libsteam_api.so ./libOpenGL.so.0"; ./newcity

