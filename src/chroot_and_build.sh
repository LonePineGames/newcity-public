#!/bin/bash

sudo chroot ~/work/debootstrap/ubuntu /bin/su ubuntu "/home/ubuntu/work/newcity/src/cd_and_build.sh"
exit $?

