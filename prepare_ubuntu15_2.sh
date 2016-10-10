#!/bin/bash

export UBUNTU15=true

apt-get install -y libssl-dev

#BEGIN: crtmpserver_build
cd libs/crtmpserver/builders/cmake/
cmake .
make clean
make -j 8 
cd ../../../../
#END
