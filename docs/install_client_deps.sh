#!/bin/bash

sudo apt-get update
sudo apt-get install -y libsdl2-dev
git clone https://github.com/emscripten-core/emsdk.git || true
pushd emsdk && \
./emsdk install latest && \
./emsdk activate latest && \
sudo cp -prv ./emsdk_env.sh /etc/profile.d/
