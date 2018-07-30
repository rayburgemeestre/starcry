FROM ubuntu:17.10

MAINTAINER Ray Burgemeestre

RUN apt-get update && apt-get -y install g++ git cmake sudo wget

RUN g++ -v

WORKDIR /usr/local/src/

RUN git clone https://bitbucket.org/rayburgemeestre/starcry

WORKDIR /usr/local/src/starcry/

RUN git submodule update --init --recursive

RUN echo deb http://apt.llvm.org/artful/ llvm-toolchain-artful-6.0 main >> /etc/apt/sources.list && \
    echo deb-src http://apt.llvm.org/artful/ llvm-toolchain-artful-6.0 main >> /etc/apt/sources.list && \
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add - ; \
    apt update -y && \
    apt-get install -y clang-6.0 lldb-6.0 lld-6.0

#RUN bash prepare_ubuntu15.sh initialize
#RUN bash prepare_ubuntu15.sh crtmpserver
#RUN bash prepare_ubuntu15.sh v8
#RUN bash prepare_ubuntu15.sh allegro
#RUN bash prepare_ubuntu15.sh caf
#RUN bash prepare_ubuntu15.sh boost
#RUN bash prepare_ubuntu15.sh benchmarklib
#RUN bash prepare_ubuntu15.sh fastpfor
#RUN bash prepare_ubuntu15.sh ffmpeg

# Use below to run all at once
RUN /bin/bash -c "set -euxo pipefail; bash prepare_ubuntu15.sh 2>&1 | tee /tmp/install.log"

# Use below as sanity check if image is correct
# TODO: is it using clang now or what??
RUN /bin/bash -c "set -euxo pipefail; cmake -DSTATIC=1 -DLIB_PREFIX_DIR=/usr/local/src/starcry . && make -j 8 starcry"

WORKDIR /projects/starcry

CMD "/bin/bash"
