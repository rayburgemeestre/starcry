#!/bin/bash

echo Starting with Allegro...
if [[ -f "/etc/lsb-release" ]]; then
    # currently I use my shipped version of allegro5..
    #sudo apt-get install -y liballegro5-dev liballegro-image5-dev libavfilter-dev
    sudo apt-get -y install freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev mesa-common-dev libxcursor-dev && \
    cd libs/allegro5 && \
    sudo apt-get install libavfilter-dev && \
    cmake -DCMAKE_BUILD_TYPE=release -DSHARED=on -DWANT_FFMPEG=off -DWANT_TTF=on . && \
    make -j 8 && \
    sudo make install && \
    cd ../../
else
    echo TODO for CentOS
    cd libs/allegro5
    cd ../../
fi
exit 0

echo Starting with boost
if [[ -f "/etc/lsb-release" ]]; then
    sudo apt-get install -y libboost-dev libboost-system-dev libboost-program-options-dev
fi

echo Starting with CAF..
if [[ -f "/etc/lsb-release" ]]; then
    cd libs/caf && \
    ./configure && \
    make -j 8 && \
    sudo make install && \
    cd ../../
fi

echo Starting with V8 ..

cd libs/v8pp && \
./build-v8.sh && \
cd ../../ && \
sudo cp -prv ./libs/v8pp/v8/lib/lib* /usr/local/lib && \
sudo ldconfig

echo Starting with my benchmark lib..
cd libs/benchmarklib && \
sudo apt-get install -y libboost-chrono-dev && \
cmake . && \
make -j 8 && \
sudo make install && \
cd ../../
