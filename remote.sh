trap "killall -9 starcry" EXIT HUP INT QUIT TERM

./build/starcry -w 10000 &
./build/starcry -w 10001 &
./build/starcry -w 10002 &
./build/starcry -w 10003 &
./build/starcry -w 10004 &
./build/starcry -w 10005 &
./build/starcry -w 10006 &
./build/starcry -w 10007 &

sleep 2

typeset script="$1"
typeset params=""
#typeset params="--no-video"
# TODO: debug (already fixed this once before my laptop got stolen)
./build/starcry $params -r servers.txt -s $script --gui --compress true

# TODO: above should be faster than
#./build/starcry $params -s $script --gui -n 1 -c 1 --compress true

