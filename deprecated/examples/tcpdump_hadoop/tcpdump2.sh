#!/bin/bash

echo preparing..
rm -rf /tmp/joboutput.txt
sleep 1
pdsh -g all killall -9 tcpdump
sleep 1
pdsh -g all rm -rf /tmp/tcpdump.txt
sleep 1

module load hadoop/hdfs1

kinit -k -t /etc/hadoop/hdfs1/sectest.keytab sectest

# test:
typeset spawned=""
ssh root@node1 "stdbuf -i0 -o0 -e0 tcpdump -i any -q -nn '(port 50475 or port 10033 or port 8019 or port 8020 or port 40832 or port 8481 or port 50020 or port 10020 or port 8485 or port 50470 or port 8040 or port 3888 or port 8090 or port 13562 or port 38077 or port 8190 or port 16000 or port 8033 or port 16001 or port 10021 or port 2181) and (host 10.141.0.1 or host 127.0.0.1)' > /tmp/tcpdump.txt" &
spawned="$spawned $!"
ssh root@node2 "stdbuf -i0 -o0 -e0 tcpdump -i any -q -nn '(port 50475 or port 10033 or port 8019 or port 8020 or port 53824 or port 8481 or port 50020 or port 10020 or port 8485 or port 50470 or port 8040 or port 3888 or port 8090 or port 13562 or port 60347 or port 48156 or port 8030 or port 8190 or port 8031 or port 8033 or port 10021 or port 2181) and (host 10.141.0.2 or host 127.0.0.1)' > /tmp/tcpdump.txt" &
spawned="$spawned $!"
ssh root@node3 "stdbuf -i0 -o0 -e0 tcpdump -i any -q -nn '(port 50475 or port 8481 or port 50020 or port 8485 or port 8040 or port 2888 or port 8044 or port 3888 or port 13562 or port 55519 or port 2181) and (host 10.141.0.3 or host 127.0.0.1)' > /tmp/tcpdump.txt" &
spawned="$spawned $!"

echo spawned = $spawned

sleep 1

typeset cmd1="yarn jar /cm/shared/apps/hadoop/Apache/2.7.2/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.7.2.jar pi 100 1000"
#typeset cmd="yarn jar /cm/shared/apps/hadoop/Apache/2.7.2/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.7.2.jar pi 1 1"
typeset cmd="bash cm-hadoop-tests.sh hdfs1 1"
echo $cmd


function prefix
{
    typeset -A lines
    while read line; do
        echo "$(date +'%H:%M:%S:%3N000') @@job@@ $line";
    done < /dev/stdin
}

#echo monitoring started, executing job in 1 second | prefix > /tmp/joboutput.txt
echo tcpdumps started | prefix > /tmp/joboutput.txt
sleep 1
echo INFO Cluster idle... | prefix >> /tmp/joboutput.txt
sleep 10
echo INFO yarn jar \<JAR\> pi example with 100 10000 | prefix >> /tmp/joboutput.txt
$cmd1 2>&1 | prefix | tee -a /tmp/joboutput.txt
sleep 10
echo INFO Cluster idle... | prefix >> /tmp/joboutput.txt
sleep 5
echo INFO Executing cm-hadoop-tests.sh with one iteration.. | prefix >> /tmp/joboutput.txt

$cmd 2>&1 | prefix | tee -a /tmp/joboutput.txt

echo done

sleep 3

echo INFO Idle for 3 seconds, waiting 10 seconds.. | prefix >> /tmp/joboutput.txt
sleep 10

kill -9 $spawned

echo INFO tcpdumps stopped | prefix >> /tmp/joboutput.txt

sleep 3

rm tcpdump.txt tcpdump_all.txt

scp root@node1:/tmp/tcpdump.txt tcpdump1.txt
scp root@node2:/tmp/tcpdump.txt tcpdump2.txt
scp root@node3:/tmp/tcpdump.txt tcpdump3.txt

# I know this contains a serious sort()ing flaw :)
cat tcpdump1.txt tcpdump2.txt tcpdump3.txt | strings | grep tcp | sort > intermediate.txt
cat intermediate.txt /tmp/joboutput.txt | sort > tcpdump_all.txt

echo terminated.

pdsh -g all killall -9 tcpdump
pdsh -g all rm -rf /tmp/tcpdump.txt

echo cleaned up.

