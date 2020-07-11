[![Build Status](https://travis-ci.org/rayburgemeestre/starcry.svg?branch=master)](https://travis-ci.org/rayburgemeestre/starcry) [![MPL 2.0 License](https://img.shields.io/badge/license-MPL2.0-blue.svg)](http://veldstra.org/2016/12/09/you-should-choose-mpl2-for-your-opensource-project.html)

This is a new version of Starcry which was originally a rendering engine with a desktop UI (see screenshot).
It aimed to be the *Photoshop for Video*, albeit vector-based.
The first version was already quite advanced with features including Motion blur, Textures, Gradients, Polygons, Lines, Circles, Ellipses, Rectangles.
Gravity effects, all kinds of Motions, Behaviors, Custom Easing (Linear, Exponential, ..), etc., etc.
However, the code got a little unmaintainable which led to this rewrite.

![screenshot](https://bitbucket.org/rayburgemeestre/starcry/raw/master/docs/screenshot_v1.png)

One of the videos that could be rendered with the old system can be found [here][https://vimeo.com/20206213].

## Quickstart

    # create alias
    alias starcry='docker run -it -v `pwd`:`pwd` -w `pwd` rayburgemeestre/starcry:latest'  # 58 MiB
    
    # get a video definition file
    wget https://raw.githubusercontent.com/rayburgemeestre/starcry/master/input/test.js
    
    # render it
    starcry test.js
    
    # view the video
    ffplay output.h264
    
## Building

    git clone --recursive https://github.com/rayburgemeestre/starcry
    cd starcry
    make        # will do a dockerized build
    make debug  # same, but debug build
    
The executable produced will be in `build` and due to static linking should run on systems such as Ubuntu 18.04.

## Current state

The risk of this project has been a bit of the second-system effect, however now five years of slow development a workable architecture became clear.
Now a huge refactoring and implementation spree is in progress to make it happen.

## Project goal

The goal is to make scripting more first-class with reasonable V8 integration.
Streaming frames to video or online streaming service, as opposed to rendering individual BMP files first.
Web-based UI with less entanglement with the rest of the system.

## Design decisions

Some design decisions made so far have been:

* Pipeline architecture
* V8 as scripting engine
* Web interface based on VueJS and Vuikit
* Web server with websockets support using Seasocks.
* Video generation using ffmpeg
* Render workers should be plug& play for scaling easily
* Deploy in Kubernetes

## Documentation

For now visit http://cppse.nl/docs/.

