#!/bin/bash

docker build --no-cache -t sc_build_ubuntu:17.10 ./Ubuntu17.10

#docker build -t sc_build_ubuntu:17.10 ./Ubuntu17.10

echo docker login
echo docker tag sc_build_ubuntu:17.10 rayburgemeestre/sc_build_ubuntu:17.10
echo docker push rayburgemeestre/sc_build_ubuntu:17.10

