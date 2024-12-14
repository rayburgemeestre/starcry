#!/bin/bash

echo wrapper start

set -ex

while true; do
    if [[ -f /workdir/build/starcry ]]; then
        cp -prv /workdir/build/starcry /starcry
        /starcry "$@"
    else
        echo no binary found yet...
    fi
    ls -al /workdir
    sleep 5
done