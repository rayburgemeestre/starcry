#!/bin/bash

#docker build --no-cache -t sc_build_ubuntu:16.04 .

#docker build -t sc_build_ubuntu:16.04 .

docker run -i -t -v $PWD:/projects/starcry sc_build_ubuntu:16.04 sh -c "cmake -DSTATIC=1 -DLIB_PREFIX_DIR=/usr/local/src/starcry . ;  make -j8 starcry"

