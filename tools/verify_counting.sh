#!/bin/bash

numInstances=100
gringo=../gringo
clasp=../clasp-2.0.2-st-x86-linux
asdp=build/release/asdp

if [[ -z "$instanceGen" || -z "$monolithicEncoding" || -z "$exchangeEncoding" || -z "$edgeArguments" ]]; then
	echo "Environment variables not set"
	exit 1
fi

[ -z "$joinEncoding" ] || joinEncodingArgument="-j $joinEncoding"

for instance in $(seq 1 $numInstances); do
	seed=$RANDOM

	instance=$(mktemp)
	claspCountFile=$(mktemp)
	asdpCountFile=$(mktemp)
	trap "rm -f $instance $claspCountFile $asdpCountFile" EXIT

	$instanceGen $seed > $instance 2>/dev/null || exit

	$gringo $monolithicEncoding $instance | $clasp -q 0 | awk '/Models/ { print $3 }' > $claspCountFile
	claspExit=${PIPESTATUS[1]}
	claspCount=$(<$claspCountFile)
	
	$asdp $edgeArguments -x $exchangeEncoding $joinEncodingArgument -p counting -s $seed < $instance | tail -n1 | awk '{ print $2 }' > $asdpCountFile
	asdpExit=${PIPESTATUS[0]}
	asdpCount=$(<$asdpCountFile)

	if [ $claspExit -ne $asdpExit ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Exit code mismatch for seed $seed (asdp: ${asdpExit}, clasp: ${claspExit})"
	elif [ $claspCount -ne $asdpCount ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Count mismatch for seed $seed (asdp: ${asdpCount}, clasp: ${claspCount})"
	else
#		echo -n .
		echo -n "$asdpExit "
	fi

	# remove temp file
	rm -f $instance $claspCountFile $asdpCountFile
	trap - EXIT
done
echo
