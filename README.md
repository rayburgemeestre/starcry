[![C/C++ CI](https://github.com/rayburgemeestre/starcry/actions/workflows/ci.yml/badge.svg)](https://github.com/rayburgemeestre/starcry/actions/workflows/ci.yml) [![MPL 2.0 License](https://img.shields.io/badge/license-MPL2.0-blue.svg)](http://veldstra.org/2016/12/09/you-should-choose-mpl2-for-your-opensource-project.html)

<img src="https://cppse.nl/sc_logo.png?V3" width="404">

This is a rendering engine that aims to be a blend between photoshop and
code-based generative art.

The interface to the renderer is a declarative project file powered by the V8
Javascript engine.

The interface to the user is either the Javascript file or the VueJS based UI.

<img src="https://cppse.nl/sc.png" width="100%">

## Quickstart

Create `starcry` alias that uses podman (see docker version below):

    # With X11 support (image +/- 350 MiB)
    alias starcry='podman run --rm --name starcry -p 18080:18080 -i -t -v `pwd`:`pwd` -w `pwd` -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --entrypoint=/starcry docker.io/rayburgemeestre/starcry:v6'

    # Without X11 support (image +/- 50 MiB)
    alias starcry='podman run --rm --name starcry -p 18080:18080 -i -t -v `pwd`:`pwd` -w `pwd` --entrypoint=/starcry docker.io/rayburgemeestre/starcry-no-gui:v6'

Create `starcry` alias that uses docker (tested only with non-rootless docker):

    alias starcry='xhost + && docker run --rm --name starcry -i -t -v `pwd`:`pwd` -w `pwd` -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --entrypoint=/starcry docker.io/rayburgemeestre/starcry:v6'
    alias starcry='xhost + && docker run --rm --name starcry -i -t -v `pwd`:`pwd` -w `pwd` --entrypoint=/starcry docker.io/rayburgemeestre/starcry-no-gui:v6'

Use software rendering in case of OpenGL issues:

    alias starcry='docker run --rm --name starcry -i -t -v `pwd`:`pwd` -w `pwd` -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --device /dev/dri/card0:/dev/dri/card0 --entrypoint=/starcry docker.io/rayburgemeestre/starcry:v6'

Get a test project file:

    wget https://raw.githubusercontent.com/rayburgemeestre/starcry/master/input/script.js

Render it:

    starcry ./script.js

View the video:

    ffplay output*.h264

## Building

    git clone --recursive https://github.com/rayburgemeestre/starcry
    cd starcry
    make            # display all targets
    make build      # execute containerized clang build
    make build-gcc  # execute containerized gcc build
    make debug      # execute containerized clang debug build
    make dockerize  # create starcry runtime containers

The executable from the dockerized build will be in `build` and should run on
Ubuntu 20.04.


## Architecture

- `libs/piper` - a pipeline architecture is used for the main architecture.
- `libs/framer` - ffmpeg wrapper to generate video output.
- `libffmpeg` + `h264` - dependencies used by framer.
- `V8` + `v8pp` - used for scripting, starcry is tightly coupled with it.
- `VueJS` - used for the Web UI
- `Buefy` - used for the Web UI (vue + bulma)
- `Seasocks` - used for the Web UI
- `png++` - used for writing PNG files
- `fmt` - used for string formatting
- `SFML` - used for preview window
- `OpenEXR` - used for writing exr files
- `tvision` - used for the console UI
- `inotify-cpp` - used for monitoring changes to loaded js file on disk.

Almost everything is statically linked, resulting in a relatively portable binary.

The Web UI is based on Vue2. Currently the UI is using Buefy, which is not going to support Vue3.

Currently waiting for Vue3 to become more mature before switching to it.

## Notes

Currently experimenting with replacing the heavy V8-reliant generator component with a more native generator.

This is a work in progress.

    # native generator version
    make clean; make builddev && ./build/starcry input/native_test.js

    # non-native generator version
    make clean; make build && ./build/starcry input/js_test.js


