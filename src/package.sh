#!/bin/bash

#rm -rf ../build-release
#rm -rf ../build-windows

echo ""
echo "#################"
echo "#### WINDOWS ####"
echo "#################"
echo ""
unzip *.zip || exit 1
mv -v */newcity.* . || exit 1
#sh ./mingw.sh || exit 1

echo ""
echo "#################"
echo "####  LINUX  ####"
echo "#################"
echo ""
#sh ./chroot_and_build.sh
sh ./linux.sh || exit 1

echo ""
echo "#################"
echo "#### PACKAGE  ###"
echo "#################"
echo ""

cd ..
rm -rf NewCity NewCity.zip
mkdir NewCity

cp -r src/data src/designs src/shaders src/locale src/contributors src/docs NewCity/
cp NewCity/designs/{C1x1_cafe,autosave}.design

mkdir -p NewCity/textures/buildings
mkdir -p NewCity/textures/vehicles
mkdir -p NewCity/textures/stoplights
mkdir -p NewCity/textures/palette
cp src/textures/*.png NewCity/textures/
cp src/textures/stoplights/*.png NewCity/textures/stoplights/
cp src/textures/buildings/*.png NewCity/textures/buildings/
cp src/textures/palette/*.png NewCity/textures/palette/
cp src/textures/vehicles/*.png NewCity/textures/vehicles/

mkdir NewCity/models
rsync -avrm --include='*.obj' --include='*/' --exclude='*' \
  src/models/vehicles NewCity/models
rsync -avrm --include='*.obj' --include='*/' --exclude='*' \
  src/models/decorations NewCity/models
rsync -avrm --include='*.obj' --include='*/' --exclude='*' \
  src/models/statues NewCity/models

mkdir NewCity/fonts
cp src/fonts/font.ttf NewCity/fonts/

mkdir NewCity/sound
cp src/sound/*.ogg NewCity/sound/

mkdir NewCity/sound/music
cp src/sound/music/*.ogg NewCity/sound/music/

mkdir NewCity/sound/environment
cp src/sound/environment/*.ogg NewCity/sound/environment/

mkdir -p NewCity/newspaper/images
mkdir -p NewCity/newspaper/ads
cp src/newspaper/articles.lua NewCity/newspaper/
cp src/newspaper/*.md NewCity/newspaper/
cp src/newspaper/images/*.png NewCity/newspaper/images/
cp src/newspaper/ads/*.md NewCity/newspaper/ads/

mkdir -p NewCity/modpacks/yours/data
mkdir -p NewCity/modpacks/yours/designs
mkdir -p NewCity/modpacks/yours/fonts
mkdir -p NewCity/modpacks/yours/locale/us
mkdir -p NewCity/modpacks/yours/shaders
mkdir -p NewCity/modpacks/yours/sounds/environment
mkdir -p NewCity/modpacks/yours/sounds/music
mkdir -p NewCity/modpacks/yours/textures/buildings

cd src
echo "Making \"yours\" constants"
./make-your-constants.sh
cd ..
cp src/modpacks/yours/data/constants.lua NewCity/modpacks/yours/data/
cp src/modpacks/yours/preview.png NewCity/modpacks/yours/

cp -r src/modpacks/vapor NewCity/modpacks/
#cp -r src/modpacks/hard NewCity/modpacks/
cp -r src/modpacks/saturate NewCity/modpacks/
cp -r src/modpacks/performance NewCity/modpacks/
cp -r src/modpacks/Classic NewCity/modpacks/
cp -r src/modpacks/Sandbox NewCity/modpacks/

#cp -r src/sample-saves NewCity/saves
mkdir NewCity/saves
touch NewCity/saves/.keep
echo "This file intentionally left blank" > NewCity/game_log.log

cp src/newcity NewCity/newcity
chmod +x NewCity/newcity
cp src/newcity.exe NewCity/newcity.exe
cp src/newcity-wine.sh NewCity/newcity-wine.sh
chmod +x NewCity/newcity-wine.sh
cp src/newcity-linux.sh NewCity/newcity-linux.sh
chmod +x NewCity/newcity-linux.sh
cp src/binary_map NewCity/binary_map
cp src/eula.txt NewCity/eula.txt
cp lib/steam_api.dll NewCity/steam_api.dll
cp lib/steam_api64.dll NewCity/steam_api64.dll
cp lib/steam_api.lib NewCity/steam_api.lib
cp lib/OpenAL32.dll NewCity/OpenAL32.dll
cp src/libsteam_api.so NewCity/libsteam_api.so
cp src/libOpenGL.so.0 NewCity/libOpenGL.so.0
cp src/newcity.pdb NewCity/newcity.pdb
cp src/official_blueprints.txt NewCity/official_blueprints.txt

echo ""
echo "################"
echo "####   ZIP   ###"
echo "################"
echo ""

cd NewCity
zip -r ../NewCity.zip ./**
cd ..
#rm -rf NewCity

echo ""
echo "###############"
echo "#### UPLOAD ###"
echo "###############"
echo ""

while true; do
    read -p "Do you wish to upload to Steam? " yn
    case $yn in
        [Yy]* ) . cd .. ; ./upload_to_steam.sh; break;;
        [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
    esac
done

