#!/bin/sh

./package.sh || exit 1
cd ../NewCity && ./newcity &
git add . ../NewCity.zip
git co -m "Package $1"
git tag $1
git push && g push --tags

