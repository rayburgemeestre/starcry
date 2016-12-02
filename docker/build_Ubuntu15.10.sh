#!/bin/bash

#docker build --no-cache -t sc_build_ubuntu:15.10 ./Ubuntu15.10

docker build -t sc_build_ubuntu:15.10 ./Ubuntu15.10

echo docker login
echo docker tag sc_build_ubuntu:15.10 rayburgemeestre/sc_build_ubuntu:15.10
echo docker push rayburgemeestre/sc_build_ubuntu:15.10

