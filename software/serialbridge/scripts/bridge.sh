#!/bin/bash

if [[ $# -eq 3 ]]; then
	device=$1
	baudrate=$2
	port=$3
	cmdline="SerialBridge -d $1 -b $2 -p $3"
	
	while true; do
		if [[ -c "$1" ]]; then
			printf -- "\rStarting $1\n"
			exec $cmdline
		else
			printf -- "\rWaiting for $1";
			sleep 3;
		fi
	done


fi


