. $HOME/.kshrc

make -j 8 starcry

typeset -a processes=(sc_worker sc_streamer sc_renderer sc_gui)
killall -9 starcry ${processes[@]}

sleep 0.5

for process in ${processes[@]}; do
    cp -prv starcry $process
done
sync

typeset remote_renderer=false
typeset remote_streamer=false
#typeset remote_renderer=20000
#typeset remote_streamer=20001

typeset cli_args=

if [[ $remote_renderer != false ]]; then
    cli_args="$cli_args --use-remote-renderer localhost:$remote_renderer"
fi
if [[ $remote_streamer != false ]]; then
    cli_args="$cli_args --use-remote-streamer localhost:$remote_streamer"
fi

function worker
{
    ./sc_worker $cli_args -w $2 | tee worker_${2}.log 1>/dev/null 2>/dev/null
}

function launch_workers
{
    while read line; do
        launch worker $line
    done < /dev/stdin
}

function launch_renderer
{
    ./sc_renderer $cli_args --spawn-renderer $remote_renderer > renderer.log
}

function launch_streamer
{
    ./sc_streamer $cli_args --spawn-streamer $remote_streamer > streamer.log
}

function launch_gui
{
    ./sc_gui --spawn-gui > gui.log
}

function start
{
    cat servers.txt | egrep -v "^;|^#" | sed 's/:/ /' | launch_workers
}

if [[ $remote_renderer != false ]]; then
    echo launching renderer..
    launch launch_renderer
fi
if [[ $remote_streamer != false ]]; then
    echo launching streamer..
    launch launch_streamer
fi
#launch launch_gui
echo launching workers..
launch start

sleep 2

function cleanup
{
    killall -9 starcry ${processes[@]}
    exit 0
}

trap "cleanup" 2

cat sgi.log | ./starcry $cli_args --stdin -r servers.txt -s input/sgi.js --gui -c 1 2>&1 | tee ray.log
#./starcry $cli_args -r servers.txt -s input/test.js --gui -c 3 2>&1 | tee ray.log
#cat input.txt | ./starcry $cli_args -r servers.txt -s input/test2.js --stdin  -c 1 2>&1 | tee ray.log

echo starcry stopped.... | tee -a starcry.log

#killall -9 starcry ${processes[@]}
wait

