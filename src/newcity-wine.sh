#!/bin/sh

SCRIPT=$(readlink -f "$0")
GAMEPATH=$(dirname "$SCRIPT")
cd $GAMEPATH
wine newcity.exe
