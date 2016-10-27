.. _developer_manual:

Building Starcry with Docker
============================

Prerequisites
-------------

- git (apt-get install git)
- docker (f.i. https://docs.docker.com/engine/installation/linux/ubuntulinux/)
- lzma (apt-get install lzma, dependency of starcry after compilation)
- Ubuntu 16 verified: libxcursor1 libjpeg8 libgl1  libxrandr2 libxinerama1 libxi6

Docker images are currently available for compiling on:

- Ubuntu 16.04 (rayburgemeestre/sc_build_ubuntu:16.04).
  Please note that executables compiled with this version will also run on Ubuntu 15.10 for example.
- CentOS 7 (rayburgemeestre/sc_build_centos:7)

It's possible to pull these docker images and build like this:
Simply install docker, pull in the image XYZ, go to the project folder root, and issue the build like this:

.. highlight:: bash
::

    git clone https://bitbucket.org/rayburgemeestre/starcry
    cd starcry

    bash build_docker_ubuntu1604.sh make -j8 starcry

    # The above script basically executes the following docker command
    docker run -i -t -v $PWD:/projects/starcry sc_build_ubuntu:16.04 sh -c \
      "cmake -DSTATIC=1 -DLIB_PREFIX_DIR=/usr/local/src/starcry . ;  make -j8 starcry"

Inside the docker image all dependencies have been prepared in ``/usr/local/src/starcry``., so you don't have to setup all the prerequisites. CMake just needs to know (inside the docker container) where to expect all the libraries. Note that a `git submodule update --init --recursive` is not needed outside the docker environment if you use this to compile starcry.

There are also a few helper scripts in the project folder root:

.. highlight:: bash
::

    build_docker_centos7.sh make -j8 starcry
    build_docker_ubuntu1604.sh make -j8 starcry


Building on Ubuntu or CentOS from scratch
=========================================

Currently there are no packages for Starcry and building requires quite a few steps as it is build on top of many libraries.

(Most of this documentation includes snippets from the ``prepare_*.sh`` script in the root of the project.)


Prerequisites
-------------

- git
- cmake
- g++ compiler >= C++11

Ubuntu 15:

.. highlight:: bash

.. literalinclude:: build_instructions/UBUNTU15_initialize

CentOS 7:

.. highlight:: bash

.. literalinclude:: build_instructions/CENTOS7_initialize

Getting the submodules
----------------------

The starcry github repository contains submodules (sub-repositories) in the ``libs/`` folder.

These were added using the following commands.

- git submodule add https://github.com/actor-framework/actor-framework.git libs/caf
- git submodule add https://bitbucket.org/rayburgemeestre/benchmarklib.git libs/benchmarklib
- git submodule add https://github.com/liballeg/allegro5.git libs/allegro5
- git submodule add https://github.com/pmed/v8pp libs/v8pp
- git submodule add https://github.com/USCiLab/cereal libs/cereal
- git submodule add https://github.com/ipkn/crow libs/crow
- git submodule add https://github.com/lemire/FastPFor libs/FastPFor
- git submodule add https://github.com/rayburgemeestre/crtmpserver libs/crtmpserver

When cloning the starcry repository you still need to check out all the submodules with the following command.

.. highlight:: bash

.. literalinclude:: build_instructions/submodules_initialize


Building all the dependencies
-----------------------------

Allegro 5
^^^^^^^^^

At the time of writing, in Ubuntu the packages are 5.0, and the external configuration file with ``allegro5.cfg`` is not supported.
This configuration file can be useful for debugging purposes. Other than that (if you don't care) you can probably just ``apt-get`` the following packages.

::

    apt-get install liballegro5-dev liballegro-image5-dev libavfilter-dev

In case you experience problems with Allegro, you are *not* on Ubuntu or just want to build for yourself, proceed with the following instructions.

On Ubuntu some dependencies are needed for building Allegro:

.. highlight:: bash

.. literalinclude:: build_instructions/UBUNTU15_allegro5_packages

On CentOS 7 you need to build from source, some dependencies needed before building:

.. highlight:: bash

.. literalinclude:: build_instructions/CENTOS7_allegro5_packages

Build Allegro now with CMake:

.. highlight:: bash

.. literalinclude:: build_instructions/allegro5_build

.. (Side-note: Tried static compiling too, you need to modify the find script, almost got it
.. to work (use --static for pkg-config, and allegro-static-5 instead of allegro-5, also append static to all lib names))

Please check if allegro-5 is recognized by pkg-config after the ``make install`` with:

    pkg-config --list-all | grep allegro-5

.. If this command yields nothing, like my experience with Ubuntu 15.10 and CentOS7u2, copy the .pc files like this:
.. 
..     trigen@FIREFLY2:/projects/starcry/libs/allegro5[(HEAD detached at dab3113)]> sudo cp -prv lib/pkgconfig/allegro* /usr/share/pkgconfig/
..     ‘lib/pkgconfig/allegro-5.pc’ -> ‘/usr/share/pkgconfig/allegro-5.pc’
..     ‘lib/pkgconfig/allegro_acodec-5.pc’ -> ‘/usr/share/pkgconfig/allegro_acodec-5.pc’
..     ‘lib/pkgconfig/allegro_audio-5.pc’ -> ‘/usr/share/pkgconfig/allegro_audio-5.pc’
..     ‘lib/pkgconfig/allegro_color-5.pc’ -> ‘/usr/share/pkgconfig/allegro_color-5.pc’
..     ‘lib/pkgconfig/allegro_font-5.pc’ -> ‘/usr/share/pkgconfig/allegro_font-5.pc’
..     ‘lib/pkgconfig/allegro_image-5.pc’ -> ‘/usr/share/pkgconfig/allegro_image-5.pc’
..     ‘lib/pkgconfig/allegro_main-5.pc’ -> ‘/usr/share/pkgconfig/allegro_main-5.pc’
..     ‘lib/pkgconfig/allegro_memfile-5.pc’ -> ‘/usr/share/pkgconfig/allegro_memfile-5.pc’
..     ‘lib/pkgconfig/allegro_primitives-5.pc’ -> ‘/usr/share/pkgconfig/allegro_primitives-5.pc’

.. This command should yield results now:

..     pkg-config --list-all | grep allegro-5 if needed

.. (Side-note: Please note that I edited FindAllegro5.cmake, namely removed allegro\_physfs, allegro\_dialog, by default it will search for these packages, but I don't use them anyway.)

C++ Actor Framework
^^^^^^^^^^^^^^^^^^^

.. highlight:: bash

.. literalinclude:: build_instructions/caf_build

The ``--build-static`` flag will build both shared and static in this case.

.. Note for CentOS7u2 I used a few extra flags for the configure step because default `g++` in path was not new enough, and to disable unused stuff that caused errors:
.. 
.. - `CXX=/usr/local/bin/c++ ./configure --no-examples --no-unit-tests --no-opencl --no-riac`

Boost
^^^^^

On Ubuntu:

- ``sudo apt-get install libboost1.58-all-dev`` should probably work.

On CentOS7 or if you prefer to compile use the following.

.. highlight:: bash
.. literalinclude:: build_instructions/boost_build


Benchmark lib
^^^^^^^^^^^^^

My own [benchmarklib][http://blog.cppse.nl/benchmarklib] which I wrote a few years ago.

.. highlight:: bash
.. literalinclude:: build_instructions/benchmarklib_build

Google's V8 Javascript engine
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In starcry [v8pp][https://github.com/pmed/v8pp] is used instead of raw-V8 code most of the time, which is really convenient and makes working with V8 less verbose.

.. highlight:: bash
.. literalinclude:: build_instructions/v8_build

ffmpeg with H264 support
^^^^^^^^^^^^^^^^^^^^^^^^

A few extra steps required for CentOS, which doesn't provide ffmpeg unfortunately.
Also there used to be a separate repo for it but it's no longer up.
Luckily it builds just fine.

.. highlight:: bash
.. literalinclude:: build_instructions/ffmpeg_build

.. crow webserver
.. ^^^^^^^^^^^^^^
.. 
.. I needed:
.. 
..     sudo apt-get install libboost-date-time-dev libboost-filesystem-dev libboost-thread-dev
.. 
.. Probably, optional (i ignored the SSL error):
.. 
..     trigen@zenbook:/projects/starcry[master]> apt-file search openssl/conf.h
..     libssl-dev: /usr/include/openssl/conf.h
..     libwolfssl-dev: /usr/include/cyassl/openssl/conf.h
..     libwolfssl-dev: /usr/include/wolfssl/openssl/conf.h
.. 
.. - cmake .
.. - make -j 8

LibPFor - fast vector compression
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. highlight:: bash
.. literalinclude:: build_instructions/fastpfor_build


Building Documentation
======================

You need ``sphinx`` and ``make`` to build the documentation.

::

    make -C docs html
    
Browsing locally using PHP webserver.

::

    php -S localhost:9999 -t $PWD/docs/_build/html/

Open ``http://localhost:9999/`` in browser.
