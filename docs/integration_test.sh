#!/bin/bash

typeset output=output/observed
typeset cmd="./build/starcry -t 16 -c 16 --no-gui"

rm -rf $output
mkdir -p $output

$cmd input/toroidal.js -f 22 -o $output/toroidal1
$cmd input/toroidal.js -f 102 -o $output/toroidal2
$cmd input/toroidal.js -f 217 -o $output/toroidal3

for file in input/*.js; do
    if [[ $file =~ contrast.js ]]; then continue; fi
    if [[ $file =~ grid.js ]]; then continue; fi
    typeset out=${file/input/$output}
    out=${out/.js/}
    $cmd $file -f 1 -o ${out}1
done

for file in input/*.js; do
    if [[ $file =~ contrast.js ]]; then continue; fi
    if [[ $file =~ grid.js ]]; then continue; fi
    typeset out=${file/input/$output}
    out=${out/.js/}
    $cmd $file -f 100 -o ${out}100
done
