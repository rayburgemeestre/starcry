
killall -9 starcry

rm -rf test.h264

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

./starcry -w ${1}0 -c 1 -n 1  &
./starcry -w ${1}1 -c 1 -n 1  &
./starcry -w ${1}2 -c 1 -n 1  &
./starcry -w ${1}3 -c 1 -n 1  &
./starcry -w ${1}4 -c 1 -n 1  &
./starcry -w ${1}5 -c 1 -n 1  &
./starcry -w ${1}6 -c 1 -n 1  &

sleep 0.2

./starcry -r ${1}0-${1}6 -c 1 -n 1 #--render-window-at 10000

ffplay test.h264
