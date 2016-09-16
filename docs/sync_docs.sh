#!/bin/bash

rsync -raPv $PWD/_build/html/ root@cppse.nl:/srv/www/vhosts/cppse.nl/docs
