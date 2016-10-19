#!/bin/bash

export CENTOS7=true

bash prepare_linux.sh $1

exit 0



mkdir -p tmp
cd tmp

yum install yasm
git clone git://git.videolan.org/x264.git
cd x264
./configure --enable-static --enable-shared
make -j 4
sudo make install
cd ../

git clone git://source.ffmpeg.org/ffmpeg.git
cd ffmpeg
module load gcc
gx=$(which g++)
./configure --cxx=$gx --enable-shared --disable-swresample --enable-libx264 --enable-gpl
make -j 4
make install
module unload gcc
cd ../



