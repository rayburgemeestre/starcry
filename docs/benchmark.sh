#!/bin/bash

set -ex

make build

rm -rfv output/benchmark
mkdir -p output/benchmark

flags="-t 1 -c 1 --no-webserver --tui"
./build/starcry $1 $flags --benchmark                        $PWD/output/benchmark/output1_video_______________.h264
./build/starcry $1 $flags --benchmark --raw                  $PWD/output/benchmark/output2_raw_frames_and_video.h264
./build/starcry $1 $flags --benchmark --raw --no-video       $PWD/output/benchmark/output3_raw_frames_no_video_.h264
./build/starcry $1 $flags --benchmark --no-video             $PWD/output/benchmark/output4_nothing_____________.h264
./build/starcry $1 $flags --benchmark --no-video --no-render $PWD/output/benchmark/output5_no_rendering________.h264

docs/parse_benchmark_results_in_output.py | sed "s/.*frame rendering.*/\x1b[1;31m&\x1b[0m/g"

