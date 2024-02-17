#!/bin/bash

while read file; do
    echo trying $file
    SECONDS=0
    timeout 300 time ./build/starcry --stdout -f 1 $file
    ret=$?
    echo timing: $file was: $SECONDS seconds.
    if [[ $ret -ne 0 ]]; then
        echo broken: $file
    fi
done < <(find ./input -type f)


