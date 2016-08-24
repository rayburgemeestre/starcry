Here are some examples of videos that can be generated (or streamed) using simple javascript.

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

This is a new version of Starcry which was originally a rendering engine. Aiming to be the Photoshop for Video (albeit vector-based).
The first version was already quite advanced with Motion blur, Textures, Gradients, Polygons, Lines, Circles, Ellipses, Rectangles.
Gravity effects, all kinds of Motions, Behaviors, Easing (Linear, Exponential, ..), etc., etc.
The code got a little unmaintainable and I had more ideas that I wanted to support. So there was need for a "second" system. 

A screenshot from the first system:

![screenshot](https://bitbucket.org/rayburgemeestre/starcry/raw/master/docs/screenshot_v1.png)

A video that could be rendered with it [here][https://vimeo.com/20206213].

## Architecture overview

The architecture is really simple, the following sequence diagram shows best the typical messaging flow when rendering a video.

![sequencediagram](https://bitbucket.org/rayburgemeestre/starcry/raw/master/docs/seqdiag.png)

## Starcry dependencies

### Submodules

Below are the list of commands I used to add submodules.

- git submodule add https://github.com/actor-framework/actor-framework.git libs/caf
- git submodule add https://bitbucket.org/rayburgemeestre/benchmarklib.git libs/benchmarklib
- git submodule add https://github.com/liballeg/allegro5.git libs/allegro5
- git submodule add https://github.com/pmed/v8pp libs/v8pp
- git submodule add https://github.com/USCiLab/cereal libs/cereal
- git submodule add https://github.com/ipkn/crow libs/crow
- git submodule add https://github.com/lemire/FastPFor libs/FastPFor

You can have git check them all out with the `git submodule update --init --recursive`.

Some of the above submodules can be ignored depending on the platform you are on (i.e., you can use your package manager for some of the dependencies).

### Prerequisites

- cmake
- g++ compiler >= C++11
- git submodule update --init --recursive

### Building

- cmake .
- make -j 8

Note that CMake will throw an error for each dependency you are missing. 
One note: sometimes it may be necessary to `rm CMakeCache.txt` after you fixed a dependency, somehow the cache can get corrupted whenever your dependencies are not set up correctly.
So in case you installed a depenceny and CMake or compiling still complains, you may need to delete that file.

## Installing each dependency

### Allegro 5.X

At the time of writing, in Ubuntu the packages are 5.0, and allegro5.cfg is not
yet supported. This configuration file can be useful for debugging purposes. Other than that, you can probably just apt-get the following.

- `apt-get install liballegro5-dev liballegro-image5-dev libavfilter-dev`

In case you experience problems still, or are not on Ubuntu, continue with the rest (building from source)..

Building yourself from the submodule in `libs/allegro5`:

On Ubuntu some dependencies needed before building:

- `sudo apt-get install freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev mesa-common-dev libxcursor-dev`
- `sudo apt-get install libavfilter-dev`
- In case allegro\_ttf doesn't compile: `sudo apt-get install libfreetype6-dev`.

On CentOS 7 you need to build from source, some dependencies needed before building:

- `sudo yum install freeglut-devel mesa-libGL-devel mesa-libGLU-devel libXcursor-devel`

Build Allegro now with CMake:

- `make -DCMAKE_BUILD_TYPE=release -DSHARED=on -DWANT_FFMPEG=off -DWANT_TTF=on .`
- `make -j 8`
- `sudo make install`

(Side-note: Tried static compiling too, you need to modify the find script, almost got it
to work (use --static for pkg-config, and allegro-static-5 instead of allegro-5, also append static to all lib names))

Please check if allegro-5 is recognized by pkg-config after the make install with:

    pkg-config --list-all | grep allegro-5

If this command yields nothing, like my experience with Ubuntu 15.10 and CentOS7u2, copy the .pc files like this:

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

This command should yield results now:

    pkg-config --list-all | grep allegro-5 if needed

(Side-note: Please note that I edited FindAllegro5.cmake, namely removed allegro\_physfs, allegro\_dialog, by default it will search for these packages, but I don't use them anyway.)

### C++ Actor Framework

- `cd libs/caf`
- `./configure --build-static`
- `make`
- `sudo make install`

The `--build-static` flag will build both shared and static in this case.

Note for CentOS7u2 I used a few extra flags for the configure step because default `g++` in path was not new enough, and to disable unused stuff that caused errors:

- `CXX=/usr/local/bin/c++ ./configure --no-examples --no-unit-tests --no-opencl --no-riac`

### Boost

On Ubuntu:

- `sudo apt-get install libboost1.58-all-dev` should probably work.

On CentOS7u2 I used:

- `g++ -v`
- `mkdir target`
- `CXX=/usr/local/bin/c++ ./bootstrap.sh --prefix=/projects/boost_1_60_0/target/`
- `./b2 --prefix=/projects/boost_1_60_0/target/`

AFAIK `--prefix` normally worked fine, but it was ignored in my case, so I had to use the following two paths, currently hardcoded in the `CMakeLists.txt` file.

- `/projects/boost_1_60_0/`
- `/projects/boost_1_60_0/stage/lib`

### Benchmark lib

My own [benchmarklib][http://blog.cppse.nl/benchmarklib] I wrote a few years ago.

- `cd libs/benchmarklib`
- `cmake .`
- `make -j 8`
- `sudo make install`

### V8

I use [v8pp][https://github.com/pmed/v8pp] instead of raw-V8 code mostly, which is really convenient, makes working with V8 less verbose.

- `cd libs/v8pp`
- `./build-v8.sh`

For Ubuntu (and I think CentOS 7 too) I needed to move the libraries to folders where my system would find them with:

- `sudo cp -prv ./libs/v8pp/v8/lib/lib* /usr/local/lib`
- `sudo ldconfig`

### ffmpeg (centos7 only)

A few extra steps required for Centos, which doesn't provide ffmpeg unfortunately. Also there used to be a separate repo for it but it's no longer up.
Luckily it builds just fine.

#### 1) first you need x264

- `yum install yasm`
- `git clone git://git.videolan.org/x264.git`
- `cd x264`
- `./configure --enable-static --enable-shared`
- `make -j 4`
- ``make install`

#### 2) second you need ffmpeg

- `user@host:/projects/starcry/libs[master]> git clone git://source.ffmpeg.org/ffmpeg.git`
- `./configure --cxx=/usr/local/bin/c++ --enable-shared --disable-swresample --enable-libx264 --enable-gpl`
- `make -j 4`
- `make install`

### crow (webserver)

I needed:

    sudo apt-get install libboost-date-time-dev libboost-filesystem-dev libboost-thread-dev

Probably, optional (i ignored the SSL error):

    trigen@zenbook:/projects/starcry[master]> apt-file search openssl/conf.h
    libssl-dev: /usr/include/openssl/conf.h
    libwolfssl-dev: /usr/include/cyassl/openssl/conf.h
    libwolfssl-dev: /usr/include/wolfssl/openssl/conf.h

- cmake .
- make -j 8

### LibPFor

cd libs/FastPFor
cmake .
make -j 8 

## Quick start

- Render from `test.js` in current folder: `./starcry`
- Also open a GUI window: `./starcry --gui`
- Play the generated video `output.h264` (i.e., with ffplay, mplayer, ..)

## Quick start using remote workers

Launch four workers on ports 10000 - 10003:

- `./starcry -w 10000 &`
- `./starcry -w 10001 &`
- `./starcry -w 10002 &`
- `./starcry -w 10003 &`

Instruct starcry to use these remote workers:

- `./starcry -r 10000-10003`

## Misc

### Websequencediagram for job flow

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


## Notes on static compiling

yum install libicu-devel mesa-libGLU-devel
