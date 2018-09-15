[![Build Status](https://travis-ci.org/rayburgemeestre/starcry.svg?branch=master)](https://travis-ci.org/rayburgemeestre/starcry)

This is a new version of Starcry which was originally a rendering engine with a desktop UI (see screenshot).
It aimed to be the *Photoshop for Video*, albeit vector-based.
The first version was already quite advanced with features including Motion blur, Textures, Gradients, Polygons, Lines, Circles, Ellipses, Rectangles.
Gravity effects, all kinds of Motions, Behaviors, Custom Easing (Linear, Exponential, ..), etc., etc.
The code got a little unmaintainable which led to this rewrite.

![screenshot](https://bitbucket.org/rayburgemeestre/starcry/raw/master/docs/screenshot_v1.png)

One of the videos that could be rendered with this system can be found [here][https://vimeo.com/20206213].

## Project goal

The goal is to make scripting more first-class with reasonable V8 integration.
Streaming frames to video or online streaming service, as opposed to rendering individual BMP files first.
Web-based UI with less entanglement with the rest of the system.

The codebase should stay maintainable so a new rewrite won't be needed anytime soon.

## Building

For now visit http://cppse.nl/docs/developer\_manual.html for manually building.

In the near future docker can be used for building without having to worry about all the dependencies.

<!--
## Misc - Notes on static compiling

yum install libicu-devel mesa-libGLU-devel
-->
