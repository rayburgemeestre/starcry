[![Build Status](https://travis-ci.org/rayburgemeestre/starcry.svg?branch=master)](https://travis-ci.org/rayburgemeestre/starcry) [![MPL 2.0 License](https://img.shields.io/badge/license-MPL2.0-blue.svg)](http://veldstra.org/2016/12/09/you-should-choose-mpl2-for-your-opensource-project.html)

This is a rendering engine that aims to be a blend between photoshop and
code-based generative art.

The interface to the renderer is a declarative project file powered by the V8
Javascript engine.

The interface to the user is either the Javascript file or the VueJS based UI.


## Quickstart

    # create alias
    alias starcry='docker run -it -v `pwd`:`pwd` -w `pwd` rayburgemeestre/starcry:v2'

    # get a video definition file
    wget https://raw.githubusercontent.com/rayburgemeestre/starcry/master/input/test.js

    # render it
    starcry test.js

    # view the video
    ffplay output*.h264


## Building

    git clone --recursive https://github.com/rayburgemeestre/starcry
    cd starcry
    make        # will do a dockerized build
    make debug  # same, but debug build

The executable from the dockerized build will be in `build` and should run on
Ubuntu 20.04.


## Architecture

- `libs/piper` - a pipeline architecture is used for the main architecture.
- `libs/framer` - ffmpeg wrapper to generate video output.
- `libffmpeg` + `h264` - dependencies used by framer.
- `V8` + `v8pp` - used for scripting, starcry is tightly coupled with it.
- `VueJS` - used for the Web UI
- `Vuekit` - used for the Web UI
- `Seasocks` - used for the Web UI
- `png++` - used for writing PNG files
- `fmt` - used for string formatting
- `SFML` - used for preview window
- `ImageMagick` - used for writing 16-bit PNG, TIFF or EXR files
- `OpenEXR` - TODO: use for layered EXR files

Almost everything is statically linked, resulting in a relatively portable binary.
