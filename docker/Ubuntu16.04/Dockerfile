FROM ubuntu:16.04

MAINTAINER Ray Burgemeestre

RUN apt-get update && apt-get -y install g++ git cmake sudo

RUN g++ -v

WORKDIR /usr/local/src/

RUN git clone https://github.com/rayburgemeestre/starcry

WORKDIR /usr/local/src/starcry/

RUN git submodule update --init --recursive

RUN bash prepare_ubuntu15.sh 2>&1 | tee /tmp/install.log

WORKDIR /projects/starcry

CMD "/bin/bash"
