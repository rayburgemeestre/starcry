#!/bin/bash

#docker run -i -t -v $PWD:/projects/starcry rayburgemeestre/sc_build_ubuntu:15.10 sh -c "rm -rf CMakeCache.txt; rm -rf CMakeFiles; cmake -DSTATIC=1 -DLIB_PREFIX_DIR=/usr/local/src/starcry . ; $*"
docker run -i -t -v $PWD:/projects/starcry rayburgemeestre/sc_build_ubuntu:15.10 sh -c "$*"
#docker run -i -t -v $PWD:/projects/starcry rayburgemeestre/sc_build_ubuntu:15.10 sh -c "$*"

