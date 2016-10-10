#!/bin/bash

export UBUNTU15=true

#BEGIN: crtmpserver_build
cd libs/crtmpserver/builders
cmake .
make clean
make -j 8 
cd ../../../
#END
