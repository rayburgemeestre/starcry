trap ctrl_c INT

pids=""

function ctrl_c {
    echo "** Trapped CTRL-C"
    echo kill -9 $pids
    kill -9 $pids
}

#killall -9 starcry

rm -rf output.h264

sleep 0.2

make -j 8 starcry

if ! [[ $? -eq 0 ]]; then
    echo compiliation failed
    exit
fi

if ! [[ "$1" ]]; then
    echo please specify port-range, i.e. 1000, will result in 10001, 10002, etc.
    exit
fi

#./starcry --render-window 10002 &
sleep 1

export j=4 # num workers

for i in `seq 1 $j`
do
    echo ./starcry -w ${1}$i -c 1 -n 1  &
    ./starcry -w ${1}$i -c 1 -n 1  &
    pids="$pids $!"
done

sleep 0.2

./starcry -r ${1}1-${1}$j -c 1 -n 1 --dim 2560x1440 --no-video input/v8_test.js
pids="$pids $!"

echo kill -9 $pids
kill -9 $pids

ffplay test.h264
