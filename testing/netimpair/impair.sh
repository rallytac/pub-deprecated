#!/bin/bash


function show_help()
{
    echo "usage: impair.sh net_if delay_ms variance_ms [--help]"
}


echo "================================================================================"
echo "Linux Network Impairment Tool"
echo "Copyright (c) 2019 Rally Tactical Systems, Inc."
echo "================================================================================"


echo "***********UNDER DEVELOPMENT*********************"

INTF=ens33
RANGE=750
INTERVAL=0.5

tc qdisc del dev ${INTF} root handle 1 2> /dev/null

if [ "$1" == "-reset" ];
then
	exit 0
fi

tc qdisc add dev ${INTF} root handle 1: netem delay 0ms
while [ 1 ]
do
	sleep ${INTERVAL}
	DELAYMS=$((RANDOM % ${RANGE}))
	echo Delay: ${DELAYMS}
	tc qdisc change dev ${INTF} root handle 1: netem delay ${DELAYMS}ms
done

