FROM centos:7

MAINTAINER Ray Burgemeestre

RUN yum clean all && yum update -y && yum install -y g++ git cmake
# TODO FIX above
# TOO OLD...
#RUN yum install -y gcc-c++
# sudo
# also add sudo indeed


# src=https://www.vultr.com/docs/how-to-install-gcc-on-centos-6
# assumes you have g++ and svn and run with sudo, and nproc returns number of cores
# also ignores step to add additional swap
RUN yum -y install svn texinfo-tex flex zip libgcc.i686 glibc-devel.i686
RUN mkdir /usr/local/src/sourceInstallations
WORKDIR /usr/local/src/sourceInstallations
RUN svn co svn://gcc.gnu.org/svn/gcc/tags/gcc_6_2_0_release/
WORKDIR /usr/local/src/sourceInstallations/gcc_6_2_0_release/
RUN yum install -y wget bzip2
RUN ./contrib/download_prerequisites
WORKDIR /usr/local/src/sourceInstallations/
RUN mkdir gcc_6_2_0_release_build/
WORKDIR /usr/local/src/sourceInstallations/gcc_6_2_0_release_build/
RUN yum install -y gcc
RUN yum install -y gcc-c++ make glibc-devel
RUN ../gcc_6_2_0_release/configure && make -j $(nproc) && make install && echo "success"
RUN g++ -v
# now get rid of the old crap, only the new g++ will be in the path now.

WORKDIR /usr/local/src/

RUN git clone https://bitbucket.org/rayburgemeestre/starcry  ;
#ADD starcry /usr/local/src/starcry

WORKDIR /usr/local/src/starcry/

RUN git submodule update --init --recursive

RUN bash prepare_centos7.sh initialize 2>&1 | tee /tmp/install.log
# On CentOS7 build this project with the "old" g++ compiler.., so don't remove gcc-c++ just yet..
RUN bash prepare_centos7.sh crtmpserver 2>&1 | tee -a /tmp/install.log
# (it has some errors with the 6.2 compiler.)
RUN yum remove -y gcc-c++

#RUN bash prepare_centos7.sh v8 2>&1 | tee -a /tmp/install.log
#RUN bash prepare_centos7.sh allegro 2>&1 | tee -a /tmp/install.log
#RUN bash prepare_centos7.sh caf 2>&1 | tee -a /tmp/install.log
#RUN bash prepare_centos7.sh boost 2>&1 | tee -a /tmp/install.log
#RUN bash prepare_centos7.sh benchmarklib 2>&1 | tee -a /tmp/install.log
#RUN bash prepare_centos7.sh fastpfor 2>&1 | tee -a /tmp/install.log
#RUN bash prepare_centos7.sh ffmpeg 2>&1 | tee -a /tmp/install.log
## One of ffmpeg's dependencies pulled this package in again..
#RUN yum remove -y gcc-c++
#
## Just a bit of cleanup for now, to make the image smaller. (better make clean's are possible..)
#RUN rm -rf /usr/local/src/sourceInstallations && \
#    rm -rf /usr/local/src/starcry/boost_1_61_0.tar.bz2 && \
#    (cd /usr/local/src/starcry/boost_1_61_0/; ls -1 | egrep -v "^boost$|^stage$" |xargs -n 1 rm -rf) && \
#    (cd /usr/local/src/starcry/libs/v8pp/v8; ls -1 | egrep -v "^include$|^lib$" | xargs -n 1 rm -rf) && \
#    find /usr/local/src/starcry/ -name '*.o' | xargs -n 10 rm -rf && \
#    rm -rf /usr/local/src/starcry/tmp

# let's go for the one shot build
RUN bash prepare_centos7.sh 2>&1 | tee /tmp/install.log && \
    yum remove -y gcc-c++ && \
    rm -rf /usr/local/src/sourceInstallations && \
    rm -rf /usr/local/src/starcry/boost_1_61_0.tar.bz2 && \
    (cd /usr/local/src/starcry/boost_1_61_0/; ls -1 | egrep -v "^boost$|^stage$" |xargs -n 1 rm -rf) && \
    (cd /usr/local/src/starcry/libs/v8pp/v8; ls -1 | egrep -v "^include$|^lib$" | xargs -n 1 rm -rf) && \
    find /usr/local/src/starcry/ -name '*.o' | xargs -n 10 rm -rf && \
    rm -rf /usr/local/src/starcry/tmp

WORKDIR /projects/starcry

CMD "/bin/bash"
