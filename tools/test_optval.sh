#!/bin/bash

numInstances=100
gringo=../gringo
clasp=../clasp
dflat=build/release/dflat

if [[ -z "$instanceGen" || -z "$dflatArguments" || -z "$monolithicEncoding" ]]; then
	echo "Environment variables not set"
	exit 1
fi

for instance in $(seq 1 $numInstances); do
	seed=$RANDOM

	instance=$(mktemp)
	claspOptValFile=$(mktemp)
	dflatOptValFile=$(mktemp)
	trap "rm -f $instance $claspOptValFile $dflatOptValFile" EXIT

	$instanceGen $seed > $instance 2>/dev/null || exit

	$gringo $monolithicEncoding $instance | $clasp -q 0 | awk '/Optimization:/ { print $2 }' > $claspOptValFile
	claspExit=${PIPESTATUS[1]}
	claspOptVal=$(<$claspOptValFile)
	
	$dflat $dflatArguments -p opt-value -s $seed < $instance | tail -n1 | awk '{ print $3 }' > $dflatOptValFile
	dflatExit=${PIPESTATUS[0]}
	dflatOptVal=$(<$dflatOptValFile)

	if [ $claspExit -ne $dflatExit ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Exit code mismatch for seed $seed (dflat: ${dflatExit}, clasp: ${claspExit})"
		exit 1
	elif [ $claspOptVal -ne $dflatOptVal ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Optimum value mismatch for seed $seed (dflat: ${dflatOptVal}, clasp: ${claspOptVal})"
		exit 2
	else
#		echo -n "$dflatExit "
		echo -n "$dflatOptVal "
	fi

	# remove temp file
	rm -f $instance $claspOptValFile $dflatOptValFile
	trap - EXIT
done
echo
