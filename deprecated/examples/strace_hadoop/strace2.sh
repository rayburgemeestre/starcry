#!/bin/bash

echo preparing strace
rm -rf /tmp/joboutput.txt
rm -rf /tmp/job.strace 1>/dev/null 2>/dev/null
sleep 1
pdsh -g all killall -9 strace
sleep 1
echo preparing tcpdump
rm -rf /tmp/joboutput.txt
sleep 1
pdsh -g all killall -9 tcpdump
sleep 1
pdsh -g all rm -rf /tmp/tcpdump.txt
sleep 1

module load hadoop/hdfs1

kinit -k -t /etc/hadoop/hdfs1/sectest.keytab sectest

# test:
bash strace.sh

. tcpdump.sh

sleep 5

#typeset cmd1="yarn jar /cm/shared/apps/hadoop/Apache/2.7.2/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.7.2.jar pi 100 1000"
typeset cmd1="yarn jar /cm/shared/apps/hadoop/Apache/2.7.2/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.7.2.jar pi 10 1000"

#typeset cmd="bash cm-hadoop-tests.sh hdfs1 1"
#echo $cmd

function prefix
{
    typeset -A lines
    while read line; do
        echo "$(date +'%H:%M:%S:%3N000') @@job@@ $line";
    done < /dev/stdin
}

#echo monitoring started, executing job in 1 second | prefix > /tmp/joboutput.txt
echo straces started | prefix > /tmp/joboutput.txt
sleep 1
echo INFO cluster idle... | prefix >> /tmp/joboutput.txt
sleep 10
echo INFO yarn jar \<JAR\> pi example with 100 10000 | prefix >> /tmp/joboutput.txt
strace -f -F -ttt -e trace=network -s 80 -o /tmp/job.strace $cmd1 2>&1 | prefix | tee -a /tmp/joboutput.txt
sleep 10
echo INFO cluster idle... | prefix >> /tmp/joboutput.txt
#sleep 5
#echo INFO executing cm-hadoop-tests.sh with one iteration.. | prefix >> /tmp/joboutput.txt
#
#$cmd 2>&1 | prefix | tee -a /tmp/joboutput.txt
#
#echo done
#
sleep 3

echo INFO idle for 3 seconds, waiting 10 seconds.. | prefix >> /tmp/joboutput.txt
sleep 10

pdsh -g all killall -1 strace

kill -9 $spawned

sleep 2

echo INFO straces stopped | prefix >> /tmp/joboutput.txt

sleep 3

rm -rf /tmp/strace-output-parser
mkdir -p /tmp/strace-output-parser
scp root@node1:/tmp/strace-output-parser/* /tmp/strace-output-parser/
scp root@node2:/tmp/strace-output-parser/* /tmp/strace-output-parser/
scp root@node3:/tmp/strace-output-parser/* /tmp/strace-output-parser/

ls -althrs  /tmp/strace-output-parser/

## I know this contains a serious sort()ing flaw :)
#cat tcpdump1.txt tcpdump2.txt tcpdump3.txt | strings | grep tcp | sort > intermediate.txt
#cat intermediate.txt /tmp/joboutput.txt | sort > tcpdump_all.txt
#
#echo terminated.
#
#pdsh -g all killall -9 tcpdump
#pdsh -g all rm -rf /tmp/tcpdump.txt
#
#echo cleaned up.

#tcpdump
rm tcpdump.txt tcpdump_all.txt

scp root@node1:/tmp/tcpdump.txt tcpdump1.txt
scp root@node2:/tmp/tcpdump.txt tcpdump2.txt
scp root@node3:/tmp/tcpdump.txt tcpdump3.txt

# I know this contains a serious sort()ing flaw :)
cat tcpdump1.txt tcpdump2.txt tcpdump3.txt | strings | grep tcp | sort > intermediate.txt
cat intermediate.txt /tmp/joboutput.txt | sort > tcpdump_all.txt

pdsh -g all killall -9 tcpdump
pdsh -g all rm -rf /tmp/tcpdump.txt
