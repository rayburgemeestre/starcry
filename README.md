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

- cmake -DCMAKE_BUILD_TYPE=release -DSHARED=on -DWANT_FFMPEG=off .
- make -j 8
- sudo make install

Tried static compiling too, you need to modify the find script, almost got it
to work (use --static for pkg-config, and allegro-static-5 instead of
allegro-5, also append static to all lib names)

### C++ Actor Framework

- cd libs/caf
- ./configure
- make
- sudo make install

## How submodules were added

- git submodule add git@github.com:actor-framework/actor-framework.git libs/caf
- git submodule add git@bitbucket.org:rayburgemeestre/benchmarklib.git libs/benchmarklib
- git submodule add git@github.com:liballeg/allegro5.git libs/allegro5
- git submodule add https://github.com/boostorg/hana.git libs/hana
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

