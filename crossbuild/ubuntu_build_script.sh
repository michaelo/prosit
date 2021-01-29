#!/bin/bash
#
# Basic script intended to be run on e.g. the docker-image for building for ubuntu
# 

echo "Entering project dir"
cd /data

echo "pwd: $(pwd)"
echo "Setting up specific build dir"

if [ -d "xbuild/ubuntu" ]
then
echo Reconfiguring
meson xbuild/ubuntu --reconfigure
else
echo Setting up fresh build dir
meson xbuild/ubuntu
fi

cd xbuild/ubuntu
ninja

if ! [ -d "../results" ]
then
mkdir ../results
fi
cp prosit ../results/prosit_ubuntu_x64