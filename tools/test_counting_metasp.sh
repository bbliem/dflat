#!/bin/bash

numInstances=100
gringo=../gringo
clasp=../claspD-1.1.1-x86-linux
dflat=build/release/dflat
metaspDir=tools/metasp

if [[ -z "$instanceGen" || -z "$dflatArguments" || -z "$monolithicEncoding" ]]; then
	echo "Environment variables not set"
	exit 1
fi

for instance in $(seq 1 $numInstances); do
	seed=$RANDOM

	instance=$(mktemp)
	claspCountFile=$(mktemp)
	dflatCountFile=$(mktemp)
	trap "rm -f $instance $claspCountFile $dflatCountFile" EXIT

	$instanceGen $seed > $instance 2>/dev/null || exit

	$gringo --reify $monolithicEncoding $instance | $gringo - ${metaspDir}/{meta.lp,metaD.lp,metaO.lp} <(echo "optimize(1,1,incl).") | $clasp -q 0 | awk '/Models/ { print $3 }' > $claspCountFile
#	claspExit=${PIPESTATUS[2]}
	claspCount=$(<$claspCountFile)
	
	$dflat $dflatArguments -p counting -s $seed < $instance | tail -n1 | awk '{ print $2 }' > $dflatCountFile
#	dflatExit=${PIPESTATUS[0]}
	dflatCount=$(<$dflatCountFile)

#	if [ $claspExit -ne $dflatExit ]; then
#		cp $instance mismatch${seed}.lp
#		echo
#		echo "Exit code mismatch for seed $seed (dflat: ${dflatExit}, clasp: ${claspExit})"
#		exit 1
#	elif [ $claspCount -ne $dflatCount ]; then
	if [ $claspCount -ne $dflatCount ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Count mismatch for seed $seed (dflat: ${dflatCount}, clasp: ${claspCount})"
		exit 2
	else
#		echo -n "$dflatExit "
		echo -n "$dflatCount "
	fi

	# remove temp file
	rm -f $instance $claspCountFile $dflatCountFile
	trap - EXIT
done
echo
