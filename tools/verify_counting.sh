#!/bin/bash

numInstances=100
gringo=../gringo
clasp=../clasp-2.0.2-st-x86-linux
dflat=build/release/dflat

if [[ -z "$instanceGen" || -z "$monolithicEncoding" || -z "$exchangeEncoding" || -z "$edgeArguments" ]]; then
	echo "Environment variables not set"
	exit 1
fi

[ -z "$joinEncoding" ] || joinEncodingArgument="-j $joinEncoding"

for instance in $(seq 1 $numInstances); do
	seed=$RANDOM

	instance=$(mktemp)
	claspCountFile=$(mktemp)
	dflatCountFile=$(mktemp)
	trap "rm -f $instance $claspCountFile $dflatCountFile" EXIT

	$instanceGen $seed > $instance 2>/dev/null || exit

	$gringo $monolithicEncoding $instance | $clasp -q 0 | awk '/Models/ { print $3 }' > $claspCountFile
	claspExit=${PIPESTATUS[1]}
	claspCount=$(<$claspCountFile)
	
	$dflat $edgeArguments -x $exchangeEncoding $joinEncodingArgument -p counting -s $seed < $instance | tail -n1 | awk '{ print $2 }' > $dflatCountFile
	dflatExit=${PIPESTATUS[0]}
	dflatCount=$(<$dflatCountFile)

	if [ $claspExit -ne $dflatExit ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Exit code mismatch for seed $seed (dflat: ${dflatExit}, clasp: ${claspExit})"
	elif [ $claspCount -ne $dflatCount ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Count mismatch for seed $seed (dflat: ${dflatCount}, clasp: ${claspCount})"
	else
#		echo -n .
		echo -n "$dflatExit "
	fi

	# remove temp file
	rm -f $instance $claspCountFile $dflatCountFile
	trap - EXIT
done
echo
