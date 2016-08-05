. $HOME/.kshrc

make -j 8 starcry
make -j 8 starcry_worker


pdsh -g all killall -9 starcry starcry_worker
sleep 0.5

cp -prv starcry_worker /cm/shared/
cp -prv Monaco_Linux-Powerline.ttf /cm/shared/

sleep 2

# required this:
#pdsh -g computenode yum install -y mesa-libGLU

function worker
{
    ssh root@$1 "module load shared gcc; cd /cm/shared/; ./starcry_worker $2" | tee worker_${2}.log 1>/dev/null 2>/dev/null
}

function launch_workers
{
    while read line; do
        launch worker $line
    done < /dev/stdin
}

function launch_sc
{
	module load gcc
    cat sgi.log | ./starcry --stdin -r servers.txt -s input/sgi.js 2>&1 | tee ray.log 1>/dev/null 2>/dev/null
}

function start
{
    cat servers.txt | egrep -v "^;|^#" | sed 's/:/ /' | launch_workers
}

launch start

sleep 10

launch launch_sc

#wait


trap "killall -9 starcry; exit" 2

echo Running..
module load gcc
cat sgi.log | ./starcry --stdin -r servers.txt -s input/sgi.js 2>&1 | tee ray.log

echo starcry stopped.... | tee -a ray.log

