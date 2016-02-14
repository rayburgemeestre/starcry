
killall -9 starcry 2>/dev/null

if ! [[ $? -eq 0 ]]; then
    echo starcry was not running
fi

sleep 0.2

time make -j 8 starcry

if ! [[ $? -eq 0 ]]; then
    echo compiliation failed
    exit
fi

time ./starcry $*

ffplay test.h264
