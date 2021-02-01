#!/bin/bash
# 
# Main entry point to setup Ubuntu docker image and build project on it
# 
BUILDTYPE=${1:-debug}

docker build . -f crossbuild/UbuntuDockerfile -t prosit_build_ubuntu:debug
docker run -v "$(pwd)":/data prosit_build_ubuntu:debug /data/crossbuild/ubuntu_build_script.sh $BUILDTYPE
