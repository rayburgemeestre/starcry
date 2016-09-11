#!/bin/bash

sudo apt-get install -y cmake git

git submodule update --init --recursive

##sudo apt-get install -y freeglut-devel mesa-libGL-devel mesa-libGLU-devel libXcursor-devel

cd libs/allegro5
cmake -DCMAKE_BUILD_TYPE=release -DSHARED=off -DWANT_FFMPEG=off -DWANT_TTF=on .
make -j 8
sudo make install
sudo cp -prv lib/pkgconfig/allegro* /usr/share/pkgconfig/
cd ../../

cd libs/caf
gx=$(which g++)
./configure --with-gcc=$gx --build-static
make -j 8
sudo make install
cd ../../




mkdir -p tmp
cd tmp

sudo apt-get install yasm
git clone git://git.videolan.org/x264.git
cd x264
./configure --enable-static --enable-shared
make -j 4
sudo make install
cd ../

git clone git://source.ffmpeg.org/ffmpeg.git
cd ffmpeg
gx=$(which g++)
./configure --cxx=$gx --enable-shared --disable-swresample --enable-libx264 --enable-gpl
make -j 4
sudo make install
cd ../

wget -c "http://downloads.sourceforge.net/project/boost/boost/1.61.0/boost_1_61_0.tar.bz2"

tar -xvf boost_1_61_0.tar.bz2

cd boost_1_61_0
mkdir target
module load gcc
gx=$(which g++)
CXX=$gx ./bootstrap.sh --prefix=/projects/starcry/tmp/boost_1_61_0/target/
./b2 --prefix=/projects/starcry/tmp/boost_1_61_0/target/
module unload gcc
cd ..
cd ..

gx=$(which g++)
cd libs/benchmarklib
rm CMakeCache.txt
# this crap didn't work, had to manually fix in CMakeCache path of c++ (g++)
export CXX=$gx
echo $CXX
cmake -DSTATIC=1 .
make -j 8
sudo make install
cd ../../

cd libs/v8pp

./build-v8.sh
sudo cp -prv ./v8/lib/lib* /usr/local/lib
sudo ldconfig

cd ../../
