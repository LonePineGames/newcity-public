#!/bin/bash

FILE=modpacks/yours/data/constants.lua
rm $FILE

echo "------------------------------" >> $FILE
echo "-- NewCity Configuration --" >> $FILE
echo "------------------------------" >> $FILE
echo "-- To modify these values, remove the two dashes at the beginning (uncomment) and then modify the value." >> $FILE
echo "" >> $FILE

tail -n +6 data/constants.lua | sed 's/^C/-- C/g' - >> $FILE

