#!/bin/bash
# 
# Main entry point to setup Ubuntu docker image and build project on it
# 

docker build . -f crossbuild/UbuntuDockerfile -t prosit_build_ubuntu:debug
docker run -v "$(pwd)":/data prosit_build_ubuntu:debug bash /data/crossbuild/ubuntu_build_script.sh
