typeset ARCH=
if [[ $UBUNTU15 == true ]]; then
    ARCH=UBUNTU15
elif [[ $CENTOS7 == true ]]; then
    ARCH=CENTOS7
fi

echo preparing $ARCH

set -ex
set -o pipefail

# steps
typeset STEP="$1"
typeset STEP_INITIALIZE=false
typeset STEP_V8=false
typeset STEP_CRTMPSERVER=false
typeset STEP_ALLEGRO=false
typeset STEP_CAF=false
typeset STEP_BOOST=false
typeset STEP_BENCHMARKLIB=false
typeset STEP_FFMPEG=false
typeset STEP_FASTPFOR=false

if [[ $STEP == "" ]] || [[ $STEP == "initialize" ]]; then STEP_INITIALIZE=true; fi
if [[ $STEP == "" ]] || [[ $STEP == "v8" ]]; then STEP_V8=true; fi
if [[ $STEP == "" ]] || [[ $STEP == "crtmpserver" ]]; then STEP_CRTMPSERVER=true; fi
if [[ $STEP == "" ]] || [[ $STEP == "allegro" ]]; then STEP_ALLEGRO=true; fi
if [[ $STEP == "" ]] || [[ $STEP == "caf" ]]; then STEP_CAF=true; fi
if [[ $STEP == "" ]] || [[ $STEP == "boost" ]]; then STEP_BOOST=true; fi
if [[ $STEP == "" ]] || [[ $STEP == "benchmarklib" ]]; then STEP_BENCHMARKLIB=true; fi
if [[ $STEP == "" ]] || [[ $STEP == "ffmpeg" ]]; then STEP_FFMPEG=true; fi
if [[ $STEP == "" ]] || [[ $STEP == "fastpfor" ]]; then STEP_FASTPFOR=true; fi

if [[ $STEP_INITIALIZE == true ]]; then

if [[ $UBUNTU15 == true ]]; then
#BEGIN: UBUNTU15_initialize
sudo apt-get install -y cmake git wget bzip2 python-dev libbz2-dev \
                        pkg-config libssl-dev curl \
                        liblzma-dev
#END
elif [[ $CENTOS7 == true ]]; then
    yum install -y which  # I can get rid of the which usage in this script, but I like where I can plug in the compiler in all the scripts as well
    yum install -y sudo
    sed -ibak 's/Defaults    requiretty/#Defaults    requiretty/g' /etc/sudoers
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
sudo yum install -y freeglut-devel mesa-libGL-devel mesa-libGLU-devel libXcursor-devel \
                    freetype-devel
#END

fi


if [[ $UBUNTU15 == true ]]; then
    sudo apt-get install -y libpng-dev libjpeg-dev libfreetype6-dev
else
    sudo yum install -y xz-devel bzip2-devel libjpeg-turbo-devel libpng-devel \
                        openssl-devel # crtmpserver
fi

fi # if STEP_INITIALIZE

if [[ $STEP_V8 == true ]]; then

# apt-get install -y curl
# build v8 first, I think boost screws up the environment, causing v8 to fail..
#BEGIN: v8_build

cd libs/v8pp

./build-v8.sh
sudo cp -prv ./v8/lib/lib* /usr/local/lib
sudo ldconfig
# whatever, above shit isn't working anymore, the .a file is a thin archive and more bullshit
# let's create one ourselves that contains all the .o files
ar rvs v8.a $(find ./libs/v8pp/v8/out/x64.release/obj -name '*.o')

cd ../../
#END
# gave errors, but maybe thats "normal"

if [[ $(find ./libs/v8pp/v8/lib/ -name '*.a'|wc -l) -lt 3 ]]; then
    echo FAILED
    exit 1
fi

fi # STEP v8
if [[ $STEP_CRTMPSERVER == true ]]; then

# crtmpserver is also difficult to build...

#apt-get install -y libssl-dev
#BEGIN: crtmpserver_build
cd libs/crtmpserver/builders/cmake/
gx=$(which clang++-6.0)
CXX=$gx CXXFLAGS="-Wno-reserved-user-defined-literal -Wno-varargs" LDFLAGS="-fPIC" COMPILE_STATIC=1 cmake .
make -j8
cd ../../../../
#END

fi # if [[ $STEP_CRTMPSERVER == true ]]; then


if [[ $STEP_ALLEGRO == true ]]; then

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

fi # if [[ $STEP_ALLEGRO == true ]]; then
if [[ $STEP_CAF == true ]]; then

#BEGIN: caf_build
cd libs/caf
gx=$(which g++)
gx=$(which clang++-6.0)
./configure --with-gcc=$gx --build-static \
    --no-examples \
    --no-unit-tests \
    --no-benchmarks \
    --no-tools
make -j 8
sudo make install
cd ../../
#END

fi #if [[ $STEP_CAF == true ]]; then

if [[ $STEP_BOOST == true ]]; then

mkdir -p /usr/local/src/starcry/boost_1_61_0/

#BEGIN: boost_build
wget -c "http://downloads.sourceforge.net/project/boost/boost/1.61.0/boost_1_61_0.tar.bz2"
tar -xvf boost_1_61_0.tar.bz2
cd boost_1_61_0
mkdir target
gx=$(which g++)
gx=$(which clang++-6.0)
CXX=$gx ./bootstrap.sh --prefix=/usr/local/src/starcry/boost_1_61_0/target/
./b2 --prefix=/usr/local/src/starcry/boost_1_61_0/target/
cd ..
#END

fi # if [[ $STEP_BOOST == true ]]; then

# no idea why, but boost doesn't use the target prefix.. it's actually here:
# /usr/local/src/starcry/boost_1_61_0/
# /usr/local/src/starcry/boost_1_61_0/stage/lib/

if [[ $STEP_BENCHMARKLIB == true ]]; then

#BEGIN: benchmarklib_build
gx=$(which g++)
gx=$(which clang++-6.0)
cd libs/benchmarklib
if [[ -f CMakeCache.txt ]]; then rm CMakeCache.txt; fi
# Note: if this export CXX doesn't work, manually fix in CMakeCache (path of c++ / g++)
export CXX=$gx
echo $CXX
cmake -DSTATIC=1 -DBOOST_ROOT=/usr/local/src/starcry/boost_1_61_0/ .
make -j 8
sudo make install
cd ../../
#END

fi # if [[ $STEP_BENCHMARKLIB == true ]]; then

if [[ $STEP_FFMPEG == true ]]; then

#BEGIN: ffmpeg_build

# TODO move in separate block for documentation :-)
if [[ $UBUNTU15 == true ]]; then
	sudo apt-get install -y yasm
	sudo apt-get install -y nasm # apparently x264 switched to this
else
    yum install -y autoconf automake cmake freetype-devel gcc gcc-c++ git libtool make mercurial nasm pkgconfig zlib-devel
    git clone --depth 1 git://github.com/yasm/yasm.git && \
        cd yasm && \
        autoreconf -fiv && \
        ./configure && \
        make && \
        make install && \
        make distclean && cd ..
fi

# create temporary folder for building x264 and ffmpeg
mkdir -p tmp
cd tmp

# clone x264, build, install
git clone git://git.videolan.org/x264.git
cd x264
git checkout 2451a7282463f68e532f2eee090a70ab139bb3e7 # parent of 71ed44c7312438fac7c5c5301e45522e57127db4, which is where they dropped support for:
#libavcodec/libx264.c:282:9: error: 'x264_bit_depth' undeclared (first use in this function); did you mean 'x264_picture_t'?
#     if (x264_bit_depth > 8)
#                  ^~~~~~~~~~~~~~


./configure --enable-static --enable-shared
make -j 8
sudo make install
cd ../

# clone ffmpeg, build, install
git clone git://source.ffmpeg.org/ffmpeg.git
cd ffmpeg
#git checkout 71052d85c16bd65fa1e3e01d9040f9a3925efd7a # or my muxing code won't work, they've modified the example since fba1592f35501bff0f28d7885f4128dfc7b82777
git checkout n3.1
gx=$(which g++)
gx=$(which clang++-6.0)
./configure --cxx=$gx --enable-shared --disable-swresample --enable-libx264 --enable-gpl
make -j 4
sudo make install
cd ../
#END

cd ../ # leave "tmp"

fi # if [[ $STEP_FFMPEG == true ]]; then

if [[ $STEP_FASTPFOR == true ]]; then

#BEGIN: fastpfor_build
cd libs/FastPFor
cmake .
make -j 8 
cd ../../
#END

fi # if [[ $STEP_FASTPFOR == true ]]; then
