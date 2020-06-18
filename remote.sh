killall -9 starcry

./build/starcry -w 10000 &
./build/starcry -w 10001 &
./build/starcry -w 10002 &
./build/starcry -w 10003 &
./build/starcry -w 10004 &
./build/starcry -w 10005 &
./build/starcry -w 10006 &
./build/starcry -w 10007 &

sleep 2

# TODO: debug (already fixed this once before my laptop got stolen)
./build/starcry --no-video -r servers.txt -s input/motion.js --gui

# TODO: above should be faster than
#./build/starcry -s input/motion.js --gui -n 1 -c 1

