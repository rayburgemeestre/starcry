#!/bin/bash

# --no-cache
docker build -t sc_build_centos:7 ./CentOS7

echo docker login
echo docker tag sc_build_centos:7 rayburgemeestre/sc_build_centos:7
echo docker push rayburgemeestre/sc_build_centos:7

