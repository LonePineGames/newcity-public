#!/bin/bash

SAVE_VERSION="58"
PATCH_VERSION="17"
PATCH_LETTER=""

VERSION_STRING="Version 0.0.$SAVE_VERSION.$PATCH_VERSION$PATCH_LETTER ALPHA"
echo "Setting Version $VERSION_STRING"

rm src/game/version_generated.hpp
echo "const int saveVersion = $SAVE_VERSION;" >> src/game/version_generated.hpp
echo "const int patchVersion = $PATCH_VERSION;" >> src/game/version_generated.hpp
echo "const char* const patchLetter = \"$PATCH_LETTER\";" >> src/game/version_generated.hpp

cat > newcity_build.vdf << EndOfMessage
"appbuild"
{
  "appid" "1067860"
  "desc" "NewCity Build $VERSION_STRING" // description for this build
  "buildoutput" "./output/" // build output folder for .log, .csm & .csd files, relative to location of this file
  "contentroot" "./NewCity/" // root content folder, relative to location of this file
  "setlive" "developer" // branch to set live after successful build, non if empty
  "preview" "0" // to enable preview builds
  "local" "" // set to file path of local content server 

  "depots"
  {
    "1067861" "depot_build_newcity.vdf"
  }
}
EndOfMessage

