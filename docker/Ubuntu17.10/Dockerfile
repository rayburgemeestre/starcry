FROM ubuntu:17.10

MAINTAINER Ray Burgemeestre

RUN apt-get update && apt-get -y install g++ git cmake sudo

RUN g++ -v

WORKDIR /usr/local/src/

RUN git clone https://bitbucket.org/rayburgemeestre/starcry

WORKDIR /usr/local/src/starcry/

RUN git submodule update --init --recursive

RUN bash prepare_ubuntu15.sh initialize 2>&1 | tee /tmp/install.log
RUN bash prepare_ubuntu15.sh crtmpserver 2>&1 | tee -a /tmp/install.log
RUN bash prepare_ubuntu15.sh v8 2>&1 | tee -a /tmp/install.log
RUN bash prepare_ubuntu15.sh allegro 2>&1 | tee -a /tmp/install.log
RUN bash prepare_ubuntu15.sh caf 2>&1 | tee -a /tmp/install.log
RUN bash prepare_ubuntu15.sh boost 2>&1 | tee -a /tmp/install.log
RUN bash prepare_ubuntu15.sh benchmarklib 2>&1 | tee -a /tmp/install.log
RUN bash prepare_ubuntu15.sh fastpfor 2>&1 | tee -a /tmp/install.log
RUN bash prepare_ubuntu15.sh ffmpeg 2>&1 | tee -a /tmp/install.log

# let's switch back to all at once..
#RUN bash prepare_ubuntu15.sh 2>&1 | tee /tmp/install.log

WORKDIR /projects/starcry

CMD "/bin/bash"
