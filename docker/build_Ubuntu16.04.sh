#!/bin/bash

#docker build --no-cache -t sc_build_ubuntu:16.04 ./Ubuntu16.04

docker build "$@" -t sc_build_ubuntu:16.04 ./Ubuntu16.04

echo docker login
echo docker tag sc_build_ubuntu:16.04 rayburgemeestre/sc_build_ubuntu:16.04
echo docker push rayburgemeestre/sc_build_ubuntu:16.04

