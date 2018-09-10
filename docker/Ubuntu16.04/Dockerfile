FROM ubuntu:16.04

MAINTAINER Ray Burgemeestre

RUN apt-get update && apt-get -y install g++ git cmake sudo wget

RUN g++ -v

WORKDIR /usr/local/src/

RUN git clone https://bitbucket.org/rayburgemeestre/starcry

WORKDIR /usr/local/src/starcry/

RUN git checkout cmake-refactoring

RUN git submodule update --init --recursive

RUN apt-get install -y gnupg2

RUN echo deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-7 main | tee -a /etc/apt/sources.list && \
    echo deb-src http://apt.llvm.org/xenial/ llvm-toolchain-xenial-7 main |tee -a /etc/apt/sources.list && \
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add - ; \
    apt update -y && \
    apt-get install -y clang-7 lldb-7 lld-7

# while developing I maintain this copy
#COPY prepare_linux.sh /etc/prepare_linux.sh
#RUN /bin/bash -c "set -euxo pipefail; bash prepare_linux.sh initialize 2>&1 | tee /tmp/install.log"
#RUN /bin/bash -c "set -euxo pipefail; bash prepare_linux.sh crtmpserver 2>&1 | tee /tmp/install.log"
#RUN /bin/bash -c "set -euxo pipefail; bash prepare_linux.sh v8 2>&1 | tee /tmp/install.log"
#RUN /bin/bash -c "set -euxo pipefail; bash prepare_linux.sh allegro 2>&1 | tee /tmp/install.log"
#RUN /bin/bash -c "set -euxo pipefail; bash prepare_linux.sh caf 2>&1 | tee /tmp/install.log"
#RUN /bin/bash -c "set -euxo pipefail; bash prepare_linux.sh boost 2>&1 | tee /tmp/install.log"
#RUN /bin/bash -c "set -euxo pipefail; bash prepare_linux.sh benchmarklib 2>&1 | tee /tmp/install.log"
#RUN /bin/bash -c "set -euxo pipefail; bash prepare_linux.sh fastpfor 2>&1 | tee /tmp/install.log"
#RUN /bin/bash -c "set -euxo pipefail; bash prepare_linux.sh ffmpeg 2>&1 | tee /tmp/install.log"

# Use below to run all at once
RUN /bin/bash -c "set -euxo pipefail; bash prepare_linux.sh 2>&1 | tee /tmp/install.log"

WORKDIR /projects/starcry

CMD "/bin/bash"
