
echo restarting all straces.

bash restart_allllll.sh

sleep 10

# yes this is the one :)

. ~/tools/inspect.sh
strace_all | tee info.txt

#typeset cmd1="yarn jar /cm/shared/apps/hadoop/Apache/2.7.2/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.7.2.jar pi 30 10000"
typeset cmd1="/bin/bash /projects/cluster-tools/cluster-tools_files/hadoop/cm-hadoop-tests.sh hdfs1 1"

function prefix
{
    typeset -A lines
    while read line; do
        echo "$(date +'%H:%M:%S:%3N000') @@job@@ $line";
    done < /dev/stdin
}

echo BEGIN | prefix | tee joboutput.txt
sleep 1
echo INFO cluster idle | prefix | tee -a joboutput.txt
sleep 0
#echo INFO cluster executing yarn jar \<JAR\> pi 50 1000 | prefix | tee -a joboutput.txt
echo INFO cluster executing cm-hadoop-tests.sh hdfs1 1 | prefix | tee -a joboutput.txt
strace -e trace=sendmmsg,sendto,write,recvfrom,read,sendfile,connect,socket,shutdown,close -f -F -ttt -s 80 -o job.strace $cmd1 2>&1 | prefix | tee -a joboutput.txt
sleep 10
echo INFO cluster idle | prefix | tee -a joboutput.txt

sleep 3

echo END | prefix | tee -a joboutput.txt

rm -rf node1
rm -rf node2
rm -rf node3
mkdir node1
mkdir node2
mkdir node3

scp root@node1:/tmp/*.strace node1/
scp root@node2:/tmp/*.strace node2/
scp root@node3:/tmp/*.strace node3/

rm -rf out.log 1>/dev/null 2>/dev/null

#IMMEDIATE APPROACH

bash process_data.sh

#ALTERNATIVE:
#tar -czf package.tar.gz node1 node2 node3 joboutput.txt job.strace
#echo delivered package 
