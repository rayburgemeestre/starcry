#/bin/bash

set -ex
set -o pipefail

sudo apt-get update
sudo apt install -y lsb-release software-properties-common ca-certificates curl

#sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 0xF64DD44383345E54
#sudo add-apt-repository "deb [arch=amd64] https://cppse.nl/repo/ $(lsb_release -cs) main"

curl -qs https://cppse.nl/public.pgp | sudo tee /etc/apt/keyrings/public.pgp
echo "deb [arch=amd64 signed-by=/etc/apt/keyrings/public.pgp] https://cppse.nl/repo/ $(lsb_release -cs) main" | sudo tee -a /etc/apt/sources.list
sudo apt update

sudo apt-get install -y cppse-ffmpeg cppse-v8 cppse-allegro5 cppse-allegro5sdl cppse-fastpfor cppse-boost cppse-sfml cppse-seasocks cppse-pngpp cppse-fmt cppse-openexr cppse-imagemagick cppse-tvision cppse-inotify-cpp cppse-redis-plus-plus cppse-vivid
sudo apt-get install -y coz-profiler
sudo apt-get install -y ccache

# removed for ubuntu 22.04: python-dev
sudo apt-get install -y cmake git wget bzip2 libbz2-dev \
            pkg-config \
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
sudo apt install -y imagemagick-6.q16 || sudo apt install imagemagick-7.q16

sudo apt install -y ninja-build

#ubuntu 22.04
sudo apt install -y libbz2-dev

# report.py integration tests
python3 -m pip install Pillow numpy
