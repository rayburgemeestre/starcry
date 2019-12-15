#!/bin/bash

docker run -t -v $PWD:/projects/starcry rayburgemeestre/sc_build_ubuntu:18.04 sh -c "$*"

