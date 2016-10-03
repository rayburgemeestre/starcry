typeset ARCH=
if [[ $UBUNTU15 == true ]]; then
    ARCH=UBUNTU15
elif [[ $CENTOS7 == true ]]; then
    ARCH=CENTOS7
fi

echo preparing $ARCH

if [[ $UBUNTU15 == true ]]; then
#BEGIN: UBUNTU15_initialize
sudo apt-get install -y cmake git wget bzip2 python-dev libbz2-dev
sudo apt-get install -y pkg-config
#END
elif [[ $CENTOS7 == true ]]; then
#BEGIN: CENTOS7_initialize
sudo yum install -y cmake git wget
#END
fi

#BEGIN: submodules_initialize
git submodule update --init --recursive
#END

if [[ $UBUNTU15 == true ]]; then
#BEGIN: UBUNTU15_allegro5_packages
sudo apt-get install -y freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev mesa-common-dev \
                     libxcursor-dev libavfilter-dev
sudo apt-get install -y libpng-dev libjpeg-dev libfreetype6-dev \
			libxrandr-dev libxinerama-dev libxi-dev
#END
elif [[ $CENTOS7 == true ]]; then
#BEGIN: CENTOS7_allegro5_packages
sudo yum install -y freeglut-devel mesa-libGL-devel mesa-libGLU-devel libXcursor-devel
#END

fi


if [[ $UBUNTU15 == true ]]; then
apt-get install -y libpng-dev libjpeg-dev libfreetype6-dev
fi

#BEGIN: allegro5_build
cd libs/allegro5
# static build
cmake -DCMAKE_BUILD_TYPE=release -DSHARED=off -DSTATIC=on -DWANT_FFMPEG=off -DWANT_TTF=on .
make -j 8
sudo make install
sudo cp -prv lib/pkgconfig/allegro* /usr/share/pkgconfig/
# shared build
cmake -DCMAKE_BUILD_TYPE=release -DSHARED=on -DSTATIC=on -DWANT_FFMPEG=off -DWANT_TTF=on .
make -j 8
sudo make install
sudo cp -prv lib/pkgconfig/allegro* /usr/share/pkgconfig/
cd ../../
#END

#BEGIN: caf_build
cd libs/caf
gx=$(which g++)
./configure --with-gcc=$gx --build-static \
    --no-examples \
    --no-unit-tests \
    --no-benchmarks \
    --no-tools
make -j 8
sudo make install
cd ../../
#END


mkdir -p /usr/local/src/starcry/boost_1_61_0/

#BEGIN: boost_build
wget -c "http://downloads.sourceforge.net/project/boost/boost/1.61.0/boost_1_61_0.tar.bz2"
tar -xvf boost_1_61_0.tar.bz2
cd boost_1_61_0
mkdir target
module load gcc
gx=$(which g++)
CXX=$gx ./bootstrap.sh --prefix=/usr/local/src/starcry/boost_1_61_0/target/
./b2 --prefix=/usr/local/src/starcry/boost_1_61_0/target/
module unload gcc
cd ..
#END

# no idea why, but boost doesn't use the target prefix.. it's actually here:
# /usr/local/src/starcry/boost_1_61_0/
# /usr/local/src/starcry/boost_1_61_0/stage/lib/

#BEGIN: benchmarklib_build
module load gcc
gx=$(which g++)
cd libs/benchmarklib
rm CMakeCache.txt
# Note: if this export CXX doesn't work, manually fix in CMakeCache (path of c++ / g++)
export CXX=$gx
echo $CXX
cmake -DSTATIC=1 -DBOOST_ROOT=/usr/local/src/starcry/boost_1_61_0/ .
make -j 8
sudo make install
cd ../../
#END


apt-get install -y curl

#BEGIN: v8_build
cd libs/v8pp
./build-v8.sh
# Now building is done, I always put everything in /usr/local/lib/
sudo cp -prv ./v8/lib/lib* /usr/local/lib
sudo ldconfig
cd ../../
#END
# gave errors, but maybe thats "normal"

#BEGIN: ffmpeg_build
# dependency for building ffmpeg is yasm (assembly compiler)
if [[ $UBUNTU15 == true ]]; then
	sudo apt-get install -y yasm
fi

# create temporary folder for building x264 and ffmpeg
mkdir -p tmp
cd tmp

# clone x264, build, install
git clone git://git.videolan.org/x264.git
cd x264
./configure --enable-static --enable-shared
make -j 8
sudo make install
cd ../

# clone ffmpeg, build, install
git clone git://source.ffmpeg.org/ffmpeg.git
cd ffmpeg
module load gcc
gx=$(which g++)
./configure --cxx=$gx --enable-shared --disable-swresample --enable-libx264 --enable-gpl
make -j 4
make install
module unload gcc
cd ../
#END

#BEGIN: fastpfor_build
cd libs/FastPFor
cmake .
make -j 8 
cd ../../
#END
