
![example1](https://bitbucket.org/rayburgemeestre/starcry/raw/master/docs/ex1.gif)
![example2](https://bitbucket.org/rayburgemeestre/starcry/raw/master/docs/ex2.gif)
![example3](https://bitbucket.org/rayburgemeestre/starcry/raw/master/docs/ex3.gif)

Source code for producing these videos / images, from left to right.

```
/**
 * Example 1: simple animation
 */
var fps           = 25;
var max_frames    = 10 * fps; // seconds
var realtime      = false;

function initialize() {
}

function next() {
    var radius      = current_frame > 100 ? current_frame - 100 : 0;
    var radius_size = 5.0 + current_frame / 2;
    add_circle(0, 0, 0, radius , radius_size);
    write_frame();
}
```

```
/**
 * Example 2: circle radius increases by time.
 */
var fps           = 25;
var max_frames    = 10 * fps; // seconds
var realtime      = true;
var seconds       = 0;
var begin         = +new Date();

function initialize() {
}

function next() {
    var diff = +new Date() - begin;
    seconds = (diff / 1000.0) % 60;
    add_circle(0, 0, 0, seconds, 5.0);
    write_frame();
}
```

```
/**
 * Example 3: read a file from stdin and visualize it.
 * run like: `cat /etc/passwd | starcry --stdin`
 */
var fps           = 25;
var max_frames    = 999999;
var realtime      = false;
var users         = [];

function initialize() {
}

function next() {
}

function input(line) {
    var s = line.split(':');
    if (!s.length) return;
    var x = Math.random() * 150 - 75;
    var y = Math.random() * 150 - 75;
    users.push([s[0], x, y]);

    for (var i=0; i<fps / 5; i++) {
        for (var j=0; j<users.length; j++) {
            var [user, x, y] = users[j];
            if (j > 0) {
                var [_, x2, y2] = users[j - 1];
                add_line(x, y, 0, x2, y2, 0, 3, 1, 0, 0);
            }
            add_text(x, y, 0, user, 'center');
        }
        write_frame();
    }
}
function close() {
    output('input stream closed');
    write_frame();
}
```

## Architecture overview

The architecture is really simple, the following diagram shows best Starcry rendering a video.

![sequencediagram](https://bitbucket.org/rayburgemeestre/starcry/raw/master/docs/seqdiag.png)

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

 ubuntu anyway..

sudo cp -prv ./libs/v8pp/v8/lib/lib* /usr/local/lib
sudo ldconfig

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
    job_generator->V8: next() javascript
    V8->job_generator: add shape **
    V8->job_generator: write_frame
    job_generator->job_storage: add_job *
    main->renderer: start
    main->streamer: start
    renderer->renderer: render_frame
    renderer->job_storage: get_job
    job_storage->renderer: <JOB>
    renderer->worker: <JOB>
    worker->renderer: job_ready
    renderer->streamer: frame **
    streamer->streamer: combine
    streamer->output: video frame
    streamer->job_storage: del_job
    job_generator->job_generator: next_frame
    job_generator->job_storage: num_jobs ?
    job_storage->job_generator: N
    job_generator->V8: [N < threshold] next() javascript
    job_generator->job_generator: [N >= threshold] sleep X;\nnext_frame

## Quick start

- ./starcry
- ./starcry --gui
- ffplay test.h264

or,
- ./starcry -w 10000 &
- ./starcry -w 10001 &
- ./starcry -w 10002 &
- ./starcry -w 10003 &
- ./starcry -r 10000-10003
- ffplay test.h264


## Problems

Need to check this out, something different accross different Linux distrubutions it seems..

/projects/starcry/src/ffmpeg/h264_encode.cpp:139:61: error: ‘av_rescale_q’ was not declared in this scope

## TODO

Not required now:

- apt-get install libsfml-dev

