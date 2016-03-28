## Prerequisites

- cmake
- g++ compiler

## Compile

- git submodule update --init --recursive

### Allegro 5.1

At the time of writing, in Ubuntu the packages are 5.0, and allegro5.cfg is not
yet supported. But you may try if you prefer:

- apt-get install liballegro5-dev liballegro-image5-dev libavfilter-dev

Building yourself (it's already in the submodules at the correct branch):

- sudo apt-get install freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev mesa-common-dev libxcursor-dev
- sudo yum install freeglut-devel   mesa-libGL-devel mesa-libGLU-devel                libXcursor-devel
- maybe needed: sudo apt-get install libavfilter-dev

- cmake -DCMAKE_BUILD_TYPE=release -DSHARED=on -DWANT_FFMPEG=off -DWANT_TTF=on .
- make -j 8
- sudo make install

Tried static compiling too, you need to modify the find script, almost got it
to work (use --static for pkg-config, and allegro-static-5 instead of
allegro-5, also append static to all lib names)

notes:
trigen@FIREFLY2:/projects/starcry/libs/allegro5[(HEAD detached at dab3113)]> sudo cp -prv lib/pkgconfig/allegro* /usr/share/pkgconfig/
‘lib/pkgconfig/allegro-5.pc’ -> ‘/usr/share/pkgconfig/allegro-5.pc’
‘lib/pkgconfig/allegro_acodec-5.pc’ -> ‘/usr/share/pkgconfig/allegro_acodec-5.pc’
‘lib/pkgconfig/allegro_audio-5.pc’ -> ‘/usr/share/pkgconfig/allegro_audio-5.pc’
‘lib/pkgconfig/allegro_color-5.pc’ -> ‘/usr/share/pkgconfig/allegro_color-5.pc’
‘lib/pkgconfig/allegro_font-5.pc’ -> ‘/usr/share/pkgconfig/allegro_font-5.pc’
‘lib/pkgconfig/allegro_image-5.pc’ -> ‘/usr/share/pkgconfig/allegro_image-5.pc’
‘lib/pkgconfig/allegro_main-5.pc’ -> ‘/usr/share/pkgconfig/allegro_main-5.pc’
‘lib/pkgconfig/allegro_memfile-5.pc’ -> ‘/usr/share/pkgconfig/allegro_memfile-5.pc’
‘lib/pkgconfig/allegro_primitives-5.pc’ -> ‘/usr/share/pkgconfig/allegro_primitives-5.pc’

(above was ALSO necessary on CentOS7u2)

testable with this command (will not yield without):

pkg-config --list-all | grep allegro-5 if needed

also check the FindAllegro5 cmake file, I disabled some stuff
for allegro_ttf to work: sudo apt-get install libfreetype6-dev

### C++ Actor Framework

- cd libs/caf
- ./configure
- make
- sudo make install

on CentOS7u2 I used a few extra configure flags:

CXX=/usr/local/bin/c++ ./configure --no-examples --no-unit-tests --no-opencl --no-riac
make -j 4 
make install

### Boost

sudo apt-get install libboost1.58-all-dev

On CentOS7u2 I used:

g++ -v
mkdir target
CXX=/usr/local/bin/c++ ./bootstrap.sh --prefix=/projects/boost_1_60_0/target/
./b2 --prefix=/projects/boost_1_60_0/target/

ignored the prefix somehow, using this:
/projects/boost_1_60_0/
/projects/boost_1_60_0/stage/lib

### cd libs/benchmarklib

cmake .
make -j 8
sudo make install


### V8

cd libs/v8pp
./build-v8.sh

### ffmpeg (centos7)

1) first you need x264, 

- yum install yasm
- git clone git://git.videolan.org/x264.git
cd x264
./configure --enable-static --enable-shared
make -j 4
make install

2) root@rb-kerberos2:/projects/starcry/libs[master]> git clone git://source.ffmpeg.org/ffmpeg.git

./configure --cxx=/usr/local/bin/c++ --enable-shared --disable-swresample --enable-libx264 --enable-gpl
make -j 4
make install

[on Ubuntu I could just use the package(s)]

## How submodules were added

- git submodule add git@github.com:actor-framework/actor-framework.git libs/caf
- git submodule add git@bitbucket.org:rayburgemeestre/benchmarklib.git libs/benchmarklib
- git submodule add git@github.com:liballeg/allegro5.git libs/allegro5
- git submodule add https://github.com/pmed/v8pp libs/v8pp

    libs/hana[master]> cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++

## Websequencediagram for job flow

    main->job_generator: start
    job_generator->job_generator: prepare_frame
    job_generator->job_storage: num_jobs ?
    job_storage->job_generator: N
    job_generator->job_storage: add_job *
    main->renderer: start
    renderer->renderer: render_frame
    renderer->job_storage: get_job
    job_storage->renderer: <JOB>
    renderer->worker: <JOB>
    worker->renderer: job_ready
    renderer->job_storage: remove_job
    renderer->streamer: frame **
    streamer->streamer: combine
    streamer->output: video frame

## Quick start

- ./starcry
- ffplay test.h264

or,
- ./starcry -w 10000 &
- ./starcry -w 10001 &
- ./starcry -w 10002 &
- ./starcry -w 10003 &
- ./starcry -r 10000-10003
- ffplay test.h264

or,
- ./starcry --render-window 10001 &
- ./starcry --render-window-at 10001

## Problems

Need to check this out, something different accross different Linux distrubutions it seems..

/projects/starcry/src/ffmpeg/h264_encode.cpp:139:61: error: ‘av_rescale_q’ was not declared in this scope

## TODO

Not required now:

- apt-get install libsfml-dev

