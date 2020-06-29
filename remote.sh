trap "killall -9 starcry" EXIT HUP INT QUIT TERM

typeset script="$1"
typeset params=""

./build/starcry $params -n 0 --expose-renderer 10000 -s $script -q 16 -Q 2 &

#sleep 5
#
#./build/starcry -w 10000 &
#./build/starcry -w 10001 &
#./build/starcry -w 10002 &
#./build/starcry -w 10003 &
#./build/starcry -w 10004 &
#./build/starcry -w 10005 &
#./build/starcry -w 10006 &
#./build/starcry -w 10007 &

wait
