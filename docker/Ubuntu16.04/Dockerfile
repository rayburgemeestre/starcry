FROM ubuntu:16.04

MAINTAINER Ray Burgemeestre

RUN apt-get update && apt-get -y install g++ git cmake sudo

RUN g++ -v

WORKDIR /usr/local/src/

RUN git clone https://github.com/rayburgemeestre/starcry

WORKDIR /usr/local/src/starcry/

RUN git submodule update --init --recursive

RUN bash prepare_ubuntu15.sh

# some patching..
RUN cd libs/FastPFor && cmake . && make -j 8 && cd ../../
RUN cd libs/v8pp && ./build-v8.sh && sudo cp -prv ./v8/lib/lib* /usr/local/lib && sudo ldconfig && cd ../../

RUN git submodule update --init --recursive
RUN bash prepare_ubuntu15_2.sh

WORKDIR /projects/starcry

CMD "/bin/bash"
