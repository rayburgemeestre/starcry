
./starcry -w ${1}0 &
./starcry -w ${1}1 &
./starcry -w ${1}2 &
./starcry -w ${1}3 &

sleep 1

./starcry -r ${1}0-${1}3
