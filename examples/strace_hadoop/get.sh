
typeset cmd1="yarn jar /cm/shared/apps/hadoop/Apache/2.7.2/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.7.2.jar pi 10 1000"
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
sleep 10
echo INFO cluster executing yarn jar \<JAR\> pi 100 10000 | prefix | tee -a joboutput.txt
strace -f -F -ttt -s 80 -o job.strace $cmd1 2>&1 | prefix | tee -a joboutput.txt
sleep 10
echo INFO cluster idle | prefix | tee -a joboutput.txt

sleep 3

echo INFO idle for 3 seconds, waiting 10 seconds.. | prefix | tee -a joboutput.txt
sleep 10
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

function h {
find $1 -type f | while read line; do
    typeset name="${line//\// }"
    name="${name//.*/}"
    cat $line | ./strace-output-parser "$name" | tee -a out.log
done < /dev/stdin
}

h node1
h node2
h node3

cat job.strace | ./strace-output-parser "headnode job" | tee -a out.log
cat joboutput.txt | tee -a out.log

