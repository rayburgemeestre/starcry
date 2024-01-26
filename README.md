[![C/C++ CI](https://github.com/rayburgemeestre/starcry/actions/workflows/ci.yml/badge.svg)](https://github.com/rayburgemeestre/starcry/actions/workflows/ci.yml) [![MPL 2.0 License](https://img.shields.io/badge/license-MPL2.0-blue.svg)](http://veldstra.org/2016/12/09/you-should-choose-mpl2-for-your-opensource-project.html)

<img src="https://cppse.nl/sc_logo3.png?V1" width="100%">

This is a rendering engine that aims to be a blend between photoshop and
code-based generative art.

The interface to the renderer is a declarative project file powered by the V8
Javascript engine.

The interface to the user is either the Javascript file or the VueJS based UI.

<img src="https://cppse.nl/screenshots.gif" width="100%">

## Quickstart

Create `starcry` alias that uses docker (see <a href="docs/docker.md">docker.md</a> for other versions, including with podman)

    alias starcry='xhost + && docker run --rm --name starcry -i -t -v `pwd`:`pwd` -w `pwd` -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --device /dev/dri/card0:/dev/dri/card0 --entrypoint=/starcry docker.io/rayburgemeestre/starcry:v8'

Get a test project file and render it (use `ctrl`+`t` to toggle a preview window, or provide `--gui` as an additional parameter):

    wget https://raw.githubusercontent.com/rayburgemeestre/starcry/master/input/web.js
    starcry ./web.js

View the video with `ffplay`, `mpv` or something else, e.g.:

    ffplay output*.h264

## Building

    git clone --recursive https://github.com/rayburgemeestre/starcry
    cd starcry
    make            # display all targets
    make build      # execute containerized clang build
    make build-gcc  # execute containerized gcc build
    make debug      # execute containerized clang debug build

The executable from the dockerized build will be in `build` and should run on
Ubuntu 22.04.

## Architecture

The architecture is only partially documented <a href="docs/tech_doc.md">here</a>.

## Components

- `libs/piper` - a pipeline architecture is used for the main architecture.
- `libs/framer` - ffmpeg wrapper to generate video output.
- `libffmpeg` + `h264` - dependencies used by framer.
- `V8` + `v8pp` - used for scripting, starcry is tightly coupled with it.
- `Quasar` - web UI framework (vue3, pinia, typescript)
- `Seasocks` - internal web server with websockets support
- `png++` - used for writing PNG files
- `fmt` - used for string formatting
- `SFML` - used for preview window
- `OpenEXR` - used for writing exr files
- `tvision` - used for the console UI (TUI)
- `inotify-cpp` - used for monitoring changes to loaded js file on disk.
- `Redis` - used for pub/sub between starcry and remote rendering workers
- `vivid` - color library used for hue etc.

Almost everything is statically linked, resulting in a relatively portable binary.

## TODO

* Introduce proper v8 javascript <> shape mapper and get rid of v8 code in a lot of places (except for the mappers).
* Getting rid of the old properties system we have for shapes.
* https://varun.ca/noise/#glossy-blobs
* Move more properties to "base" class object_bridge, some are needless duplication.

## Separate worker processes

Starcry can run with redis as a pub/sub. First install redis, run it, then:

    redis-server
    redis-cli config set client-output-buffer-limit "pubsub 512mb 256mb 120"

    make build
    ./build/starcry -i input/test.js -t 0 --server tcp://localhost:6379

    ./build/starcry --client tcp://localhost:6379 &
    ./build/starcry --client tcp://localhost:6379 &
    ./build/starcry --client tcp://localhost:6379 &
    ./build/starcry --client tcp://localhost:6379 &
