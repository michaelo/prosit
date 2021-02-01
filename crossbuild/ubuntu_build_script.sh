#!/bin/bash
#
# Basic script intended to be run on e.g. the docker-image for building for ubuntu
#

BUILDTYPE=${1:-debug}
echo "BuildType: $BUILDTYPE"

echo "Entering project dir"
cd /data

echo "pwd: $(pwd)"
echo "Setting up specific build dir"

if ! [ -d "xbuild/ubuntu_$BUILDTYPE" ]
then
# echo Reconfiguring
# meson --buildtype=$BUILDTYPE xbuild/ubuntu --reconfigure
# else
echo Setting up fresh build dir
meson --buildtype=$BUILDTYPE  xbuild/ubuntu_$BUILDTYPE
fi

cd xbuild/ubuntu_$BUILDTYPE
ninja test

if ! [ -d "../results" ]
then
mkdir ../results
fi
cp prosit ../results/prosit_ubuntu_x64_$BUILDTYPE