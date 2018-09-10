#!/bin/bash
cmake -DLIB_PREFIX_DIR=/usr/local/src/starcry/ . && make -j $(nproc)
