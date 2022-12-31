#/bin/bash

set -ex
set -o pipefail

sudo apt-get update
sudo apt install -y lsb-release software-properties-common ca-certificates

sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 5CE16B7B
sudo add-apt-repository "deb [arch=amd64] https://cppse.nl/repo/ $(lsb_release -cs) main"
sudo apt-get install -y cppseffmpeg=1.1.1 v8pp=1.1.1 allegro5=1.1.1 allegro5sdl=1.1.1 fastpfor=1.1.1 boost=1.1.1 sfml=1.1.1 seasocks=1.1.1 pngpp=1.1.1 fmt=1.1.1 cppseopenexr=1.1.1 cppseimagemagick=1.1.1 cppse-tvision=1.1.1 inotify-cpp=1.1.1 redis-plus-plus
sudo apt-get install -y coz-profiler
sudo apt-get install -y ccache

# removed for ubuntu 22.04: python-dev
sudo apt-get install -y cmake git wget bzip2 libbz2-dev \
            pkg-config curl \
            liblzma-dev libomp5

# ubuntu 18.04: sudo apt-get install -y libssl1.0-dev
sudo apt-get install -y libssl-dev
sudo apt-get install -y freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev mesa-common-dev \
                libxcursor-dev libavfilter-dev
sudo apt-get install -y libpng-dev libjpeg-dev libfreetype6-dev \
            libxrandr-dev libxinerama-dev libxi-dev
sudo apt-get install -y libgtk2.0-dev
sudo apt-get install -y libpng-dev libjpeg-dev libfreetype6-dev
# sfml (system 2.4)
#sudo apt-get install -y libbz2-dev liblzma-dev libz-dev
#sudo pt-get install -y libsfml-dev
# sfml self-compiled (2.5)
sudo apt-get install -y libudev-dev libopenal-dev libflac-dev libvorbis-dev

# image magick and openexr
sudo apt-get install -y libwebp-dev libarchive-dev libzstd-dev libjbig-dev libtiff-dev
# ubuntu runtime deps
# libwebp6: ..
# libarchive13: /usr/lib/x86_64-linux-gnu/libarchive.so.13
# libzstd1: /usr/lib/x86_64-linux-gnu/libzstd.so.1.4.4
# ncurses
sudo apt-get install -y libncurses-dev

# clion dependencies
sudo apt install -y libxtst6

# integration test dependencies
sudo apt install -y imagemagick-6.q16

sudo apt install -y ninja-build

#ubuntu 22.04
sudo apt install -y libbz2-dev
