.. _developer_manual:

Building Starcry with Docker
============================

Prerequisites
-------------


.. highlight:: bash
::

    git clone --recursive https://github.com/rayburgemeestre/starcry
    cd starcry
    make        # will do a dockerized build
    make debug  # same, but debug build

The artifacts will end up in the ``build`` directory.

See the ``Makefile`` and ``Dockerfile`` for details if you want to build on your local machine.
Only Ubuntu 18.04 is currently supported, and the build process involves installing a bunch of packages from my own apt repository.
The docker approach also produces a binary that is perfectly runnable outside of docker.

Building the dependencies takes a few hours, especially V8 and boost, and we need static builds of everything because we aim
to simplify the artifact as much as possible.


Building Documentation
======================

You need ``sphinx`` and ``make`` to build the documentation.

::

    pip install sphinx
    pip install sphinx_rtd_theme
    make -C docs html
    
Browsing locally:

::

    php -S localhost:9999 -t $PWD/docs/_build/html/

Open ``http://localhost:9999/`` in browser.
