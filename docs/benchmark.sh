#!/bin/bash

set -ex

make build

rm -rfv output/benchmark
mkdir -p output/benchmark

flags="-t 1 -c 1 --no-webserver"
./build/starcry $1 $flags --benchmark $PWD/output/benchmark/output1.h264
./build/starcry $1 $flags --benchmark --raw --no-video $PWD/output/benchmark/output2.h264
./build/starcry $1 $flags --benchmark --no-video $PWD/output/benchmark/output3.h264
./build/starcry $1 $flags --benchmark --no-video --no-render $PWD/output/benchmark/output4.h264

docs/parse_benchmark_results_in_output.py
