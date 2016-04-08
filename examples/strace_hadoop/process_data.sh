rm -rf out.log 1>/dev/null 2>/dev/null

function h {
find $1 -type f | while read line; do
    typeset name="${line//\// }"
    name="${name//.*/}"
    cat $line | ./strace-output-parser "$name" >> out.log 2>/dev/null
    echo ret = $?
done < /dev/stdin
}

h node1
h node2
h node3

cat job.strace | ./strace-output-parser "headnode job" >> out.log 2>/dev/null
echo ret = $?
cat joboutput.txt >>  out.log
echo ret = $?
echo done
