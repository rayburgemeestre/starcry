#!/bin/bash

typeset previous=""
function check_service {
    while read line; do
		typeset pid="$(echo $line | awk '{print $1}')"
		typeset cmd=""
        if ! [[ "$previous" = "$pid" ]]; then
			if ! [[ "$pid" = "-" ]]; then
				cmd="$(echo $line | awk '{print $1}' | xargs -I{} ssh root@$1 "ps ww --pid {}" | grep -v PID)"
				if [[ "$(echo $cmd | grep java)" ]]; then
					if ! [[ "$previous" == "" ]]; then
                        x=1
					#	echo ""
					fi
                    typeset proc=$(echo $cmd | awk -F '-Dproc_' '{print $2}' | awk '{print $1}')
					#echo -n command:
                    typeset label="$proc"
                    if [[ "$proc" ]]; then
                        x=1
                    elif [[ "$(echo $cmd | grep zookeeper )" ]]; then
                        label=zookeeper
                    elif [[ "$(echo $cmd | grep kms )" ]]; then
                        label=KMS
                    else
                        label=java
                    fi
				fi
			#else
				#echo "(command equals dash)"
			fi
        fi
        previous="$pid"
        typeset port=$(echo $line |sed s/:::/:/ |awk -F ':' '{print $2}')
		if [[ "$(echo $cmd | grep java)" ]]; then
            echo $(echo $label $line | awk '{print " - listen port: "$3" "$4" "$2" "$1}')   $(echo $(curl -L -k --connect-timeout 1 https://$1:$port 2>/dev/null | head -n 1) $(curl -L --connect-timeout 1 $1:$port 2>/dev/null | head -n 1) )
                                                                                            # extended info ^^
		fi
    done < /dev/stdin
}

function format {
	while read line;
	do  
	    typeset proto=${line// */}
	    if [[ $proto =~ tcp* ]] || [[ $proto = "udp" ]]; then
			typeset proto=$(echo $line | awk '{print $1}')
			typeset addr=$(echo $line | awk '{print $4}')
		fi
	    if [[ $proto =~ tcp* ]]; then
			typeset pid=$(echo $line | awk '{print $7}' | awk -F '/' '{print $1}')
	    elif [[ $proto = "udp" ]]; then
			typeset pid=$(echo $line | awk '{print $6}' | awk -F '/' '{print $1}')
		else
			continue
	    fi
		echo $pid $proto $addr
	done < /dev/stdin
}

function check {
    previous=""
    echo "-----------------------------------------------------------------------------------"
    printf "host: $1\n"
    echo "-----------------------------------------------------------------------------------"
    #ssh root@$1 "lsof -i -P -n| awk '/LISTEN/ {print \$2\" \"\$8\" \"\$9}'" | check_service $1 | cut -c-$(($COLUMNS-10))
    ssh root@$1 "netstat -lpn" | format | check_service $1 #| cut -c-$(($COLUMNS-10))
}

function check_all_impl {
    typeset cmds=""
    while read line; do
        cmds="check $line; "
    done < /dev/stdin
    echo $cmds
}

function check_all {
    cmsh -c 'device list' 2>/dev/null | awk '/PhysicalNode/ {print "check "$2";"}' | xargs -I{} sh -c ". /root/tools/inspect.sh && {}"
}

function check_tcpdump {
    previous=""
    typeset ips=$(ssh root@$1 "ifconfig" | grep "inet "|awk '{print $2}')
    ips="$(echo -n "dst host "; echo -n $ips | sed 's/ / or dst host /g')"
    typeset ports=$(ssh root@$1 "netstat -lpn" | format | check_service $1 | grep "listen port" | awk '{print $5}' | sed 's/.*://g')
    typeset cmd=$(echo "stdbuf -i0 -o0 -e0  tcpdump -i any -q -nn '(dst port $(echo -n $ports | sed 's/ / or dst port /g')) and ($ips)'")
    echo ssh root@$1 \"$cmd \> /tmp/tcpdump.txt\" \& | tee -a tcpdump.sh
    echo 'spawned="$spawned $!"' | tee -a tcpdump.sh
}

function postfix
{
    typeset -A lines
    while read line; do
        echo "$line$1";
    done < /dev/stdin
}

function check_strace {
    echo check strace = $*
    ssh root@$1 "netstat -lpn" | format | check_service $1 | grep "listen port" | awk '{print $6" "$7" bind"$5}'| sed 's/bind.*://g' | sort -u > /tmp/test.txt

    cat /tmp/test.txt |postfix " $1" | awk '{print "[\""$4"\", "$1", \""$2"\", "$3"],"}'

    echo "ssh root@$1 \"rm -rf /tmp/strace-output-parser 2>/dev/null\"" | tee -a strace.sh
    echo "ssh root@$1 \"mkdir -p /tmp/strace-output-parser\"" | tee -a strace.sh
    #cat /tmp/test.txt |postfix " $1" | awk '{print "nohup ssh root@"$4" \"strace -p "$1" -f -F -ttt -e trace=network -s 80 -o /tmp/strace-output-parser/"$4"_"$1".strace\" &"}' | sort -u | tee -a strace.sh
    #cat /tmp/test.txt |postfix " $1" | awk '{print "nohup ssh root@"$4" \"strace -p "$1" -f -F -ttt -s 80 -o /tmp/strace-output-parser/"$4"_"$1".strace\" &"}' | sort -u | tee -a strace.sh

    cat /tmp/test.txt |postfix " $1" | awk '{print "nohup ssh root@"$4" \"strace -p "$1" -f -F -ttt -s 80 -o /tmp/strace-output-parser/"$2".strace\" &"}' | sort -u | tee -a strace.sh

}

function test {
    #target tcpdump -i any -q -nn dst port 8020 or dst port 8080
    echo "tcpdump -i any -q -nn dst port $(cat test.txt |xargs echo | sed 's/ / or dst port /g')"
}

function tcpdump_all {
    echo 'typeset spawned=""' | tee tcpdump.sh
    cmsh -c 'device list' 2>/dev/null | awk '/PhysicalNode/ {print "check_tcpdump "$2";"}' | xargs -I{} sh -c ". /root/tools/inspect.sh && {}"
}

function strace_all {
    rm -rf strace.sh 1>/dev/null 2>/dev/null
    cmsh -c 'device list' 2>/dev/null | awk '/PhysicalNode/ {print "check_strace "$2";"}' | xargs -I{} sh -c ". /root/tools/inspect.sh && {}"
}

