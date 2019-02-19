set -o errexit
set -o pipefail
set -o nounset
set -o xtrace

# NOTE use update-alternatives to configure for example clang++-7, etc.

typeset ARCH=
typeset UBUNTU15=
typeset CENTOS7=

apt install -y lsb-release || true

if [[ $(lsb_release -a | grep -i Ubuntu) ]]; then
    UBUNTU15=true
    ARCH=UBUNTU15
#elif [[ $CENTOS7 == true ]]; then
else
    CENTOS7=true
    ARCH=CENTOS7
fi

echo preparing $ARCH

# steps
typeset STEP="${1:-}"
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
#sudo apt-get remove libssl-dev -y
#BEGIN: UBUNTU15_initialize

if [[ $(lsb_release -a | grep -i bionic ) ]]; then
    sudo apt-get install libssl1.0-dev -y


    #clang
    apt-get install -y gnupg2
    echo deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-7 main >> /etc/apt/sources.list && \
    echo deb-src http://apt.llvm.org/bionic/ llvm-toolchain-bionic-7 main >> /etc/apt/sources.list && \
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add - ; \
    apt update -y && \
    apt-get install -y clang-7 lldb-7 lld-7 && \
    apt-get install -y libc++-7-dev libc++-7-dev

elif [[ $(lsb_release -a | grep -i xenial ) ]]; then
    sudo apt-get install libssl-dev -y
elif [[ $(lsb_release -a | grep -i artful ) ]]; then
    sudo apt-get install libssl-dev -y
fi

sudo apt-get install -y cmake git wget bzip2 python-dev libbz2-dev \
                        pkg-config curl \
                        liblzma-dev

update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-7 40

update-alternatives --install /usr/bin/cc cc /usr/bin/clang-7 40

update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-7 40

update-alternatives --install /usr/bin/clang clang /usr/bin/clang-7 40

update-alternatives --install /usr/bin/ld ld /usr/bin/ld.lld-7 40



# NOTE libssl-dev  was fine on Ubuntu 15, but on Ubuntu 18 the default is 1.1.0 which conflicts with crtmpserver
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
sudo apt-get install -y libgtk2.0-dev # Ubuntu 17.10
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
#sudo cp -prv ./v8/lib/lib* /usr/local/lib
#sudo ldconfig
# whatever, above shit isn't working anymore, the .a file is a thin archive and more bullshit
# let's create one ourselves that contains all the .o files
(cd ../../; rm -rf v8.a; ar rvs v8.a $(cat v8-include.txt |xargs -n 1 -I{} find {} -name '*.o'))

cd ../../
#END
# gave errors, but maybe thats "normal"

#if [[ $(find ./libs/v8pp/v8/lib/ -name '*.a'|wc -l) -lt 3 ]]; then
#    echo FAILED
#    exit 1
#fi

fi # STEP v8

if [[ $STEP_CRTMPSERVER == true ]]; then

# crtmpserver is also difficult to build...

#sudo apt-get remove -y libssl-dev
if [[ $(lsb_release -a | grep -i bionic ) ]]; then
    sudo apt-get install libssl1.0-dev -y
else
    # xenial, artful can just install libbsl-dev
    sudo apt-get install libssl-dev -y
fi

rm -rf libs/crtmpserver/builders/cmake/CMakeCache.txt
#BEGIN: crtmpserver_build

# OLD
#cd libs/crtmpserver/builders/cmake/
#gx=$(which c++)
#gx=$(which clang++-7) # seems like for now, build with g++ is broken
## for clang++-7 use:
#CXX=$gx CXXFLAGS="-Wno-reserved-user-defined-literal -Wno-deprecated-declarations -Wno-varargs" LDFLAGS="-fPIC" COMPILE_STATIC=1 cmake .
#make -j8
#cd ../../../../

# NEW
cd libs/
#mv crtmpserver crtmpserver_bak
#git clone https://github.com/shiretu/crtmpserver
cd crtmpserver/builders/cmake/
make clean || true
rm -rf CMakeCache.txt
gx=$(which c++)
gx=$(which clang++-7) # seems like for now, build with g++ is broken
# for clang++-7 use:
CXX=$gx CXXFLAGS="-Wno-reserved-user-defined-literal -Wno-deprecated-declarations -Wno-varargs" LDFLAGS="-fPIC" COMPILE_STATIC=1 cmake .
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
rm -rf build
gx=$(which clang++) # afaik c++ is not ok here..
#./configure --with-gcc=$gx --build-static \

./configure --with-clang=$gx --build-static-only \
    --no-examples \
    --no-unit-tests \
    --no-benchmarks \
    --no-tools \
	--no-python # \
	#--no-auto-libc++ \
	#--no-compiler-check
CMAKE_CXX_FLAGS="-std=c++17 -stdlib=libc++" make -j 8
sudo make install
cd ../../
#END

fi #if [[ $STEP_CAF == true ]]; then

if [[ $STEP_BOOST == true ]]; then

mkdir -p /usr/local/src/starcry/boost_1_68_0/


#BEGIN: boost_build
if ! [[ -d boost_1_68_0 ]]; then
    wget -c "https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.bz2"
    tar -xvf boost_1_68_0.tar.bz2
fi
cd boost_1_68_0
mkdir -p target

gx=$(which c++)
CXX=$gx ./bootstrap.sh --prefix=/usr/local/src/starcry/boost_1_68_0/target/ --with-toolset=clang
./b2 clean
# || true because with clang I saw a few libraries not compile correctly, I hope it's safe if we ignore those failures.
./b2 --prefix=/usr/local/src/starcry/boost_1_68_0/target/ toolset=clang cxxflags="-stdlib=libc++" linkflags="-stdlib=libc++" || true
cd ..
#END

fi # if [[ $STEP_BOOST == true ]]; then

# no idea why, but boost doesn't use the target prefix.. it's actually here:
# /usr/local/src/starcry/boost_1_68_0/
# /usr/local/src/starcry/boost_1_68_0/stage/lib/

if [[ $STEP_BENCHMARKLIB == true ]]; then

#BEGIN: benchmarklib_build
gx=$(which c++)
cd libs/benchmarklib
if [[ -f CMakeCache.txt ]]; then rm CMakeCache.txt; fi
# Note: if this export CXX doesn't work, manually fix in CMakeCache (path of c++ / g++)
export CXX=$gx
echo $CXX
cmake -DSTATIC=1 -DBOOST_ROOT=/usr/local/src/starcry/boost_1_68_0/ .
make -j 8
sudo make install
cd ../../
#END

fi # if [[ $STEP_BENCHMARKLIB == true ]]; then

if [[ $STEP_FFMPEG == true ]]; then

#BEGIN: ffmpeg_build

# TODO move in separate block for documentation :-)
if [[ $UBUNTU15 == true ]]; then
	if [[ $(lsb_release -a | grep -i xenial ) ]]; then
		(wget https://www.nasm.us/pub/nasm/releasebuilds/2.13.01/nasm-2.13.01.tar.gz
		 tar xzvf nasm-2.13.01.tar.gz 
		 cd nasm-2.13.01
		 ./configure --prefix=/usr
		 make
		 sudo make install
		 # export PATH=/opt/nasm/bin/:$PATH
		 cd ..)
	else
		sudo apt-get install -y yasm
		sudo apt-get install -y nasm # apparently x264 switched to this
	fi
else
    yum install -y autoconf automake cmake freetype-devel gcc gcc-c++ git libtool make mercurial nasm pkgconfig zlib-devel
    git clone --depth 1 git://github.com/yasm/yasm.git && \
        cd yasm && \
        autoreconf -fiv && \
        CXX=$(which c++) ./configure && \
        make && \
        make install && \
        make distclean && cd ..
fi

# create temporary folder for building x264 and ffmpeg
mkdir -p tmp
cd tmp

# clone x264, build, install
if ! [[ -d x264 ]]; then
    git clone git://git.videolan.org/x264.git
fi
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
if ! [[ -d ffmpeg ]]; then
    git clone git://source.ffmpeg.org/ffmpeg.git
fi
cd ffmpeg
#git checkout 71052d85c16bd65fa1e3e01d9040f9a3925efd7a # or my muxing code won't work, they've modified the example since fba1592f35501bff0f28d7885f4128dfc7b82777
git checkout n3.1
gx=$(which c++)
#./configure --cxx=$gx --enable-shared --disable-swresample --enable-libx264 --enable-gpl
./configure --cxx=$gx --enable-shared --enable-libx264 --enable-gpl
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
