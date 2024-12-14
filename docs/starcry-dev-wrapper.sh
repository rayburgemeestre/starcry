#!/bin/bash

echo wrapper start

set -ex

while true; do
    cp -prv /workdir/build/starcry /starcry
    /starcry "$@"
    ls -al /workdir
    sleep 5
done
