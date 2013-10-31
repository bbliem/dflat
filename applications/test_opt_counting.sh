#!/bin/bash

numInstances=100
gringo=gringo
clasp=clasp
dflat=build/release/dflat

if [[ -z "$instanceGen" || -z "$dflatArguments" || -z "$monolithicEncoding" ]]; then
	echo "Environment variables not set"
	exit 1
fi

for instance in $(seq 1 $numInstances); do
	seed=$RANDOM

	instance=$(mktemp)
	claspOptValFile=$(mktemp)
	claspCountFile=$(mktemp)
	dflatOptValAndCountFile=$(mktemp)
	trap "rm -f $instance $claspOptValFile $claspCountFile $dflatOptValAndCountFile" EXIT

	$instanceGen $seed > $instance 2>/dev/null || exit

	$gringo $monolithicEncoding $instance | $clasp -q 0 | awk '/Optimization:/ { print $2 }' > $claspOptValFile
	claspExit=${PIPESTATUS[1]}
	claspOptVal=$(<$claspOptValFile)

	$dflat $dflatArguments --depth 0 --seed $seed < $instance | tail -n1 | awk '{ print substr($3,1,length($3)-1); print substr($1,2,length($1)-2) }' > $dflatOptValAndCountFile
	dflatExit=${PIPESTATUS[0]}
	OLDIFS=$IFS
	IFS=$'\n'
	dflatOptValAndCount=($(<$dflatOptValAndCountFile))
	IFS=$OLDIFS
	dflatOptVal=${dflatOptValAndCount[0]}
	dflatCount=${dflatOptValAndCount[1]}

	[ $claspExit -ne 30 ] || claspExit=10

	if [ $claspExit -ne $dflatExit ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Exit code mismatch for seed $seed (dflat: ${dflatExit}, clasp (run 1): ${claspExit})"
		exit 1
	elif [ $claspOptVal -ne $dflatOptVal ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Optimum value mismatch for seed $seed (dflat: ${dflatOptVal}, clasp: ${claspOptVal})"
		exit 2
	fi

	$gringo $monolithicEncoding $instance | $clasp -q 0 --opt-all=${claspOptVal} | awk '/Models/ { print $3 }' > $claspCountFile
	claspExit=${PIPESTATUS[1]}
	claspCount=$(<$claspCountFile)

	[ $claspExit -ne 30 ] || claspExit=10

	if [ $claspExit -ne $dflatExit ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Exit code mismatch for seed $seed (dflat: ${dflatExit}, clasp (run 2): ${claspExit})"
		exit 3
	elif [ $claspCount -ne $dflatCount ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Count mismatch for seed $seed (dflat: ${dflatCount}, clasp: ${claspCount})"
		exit 4
	else
#		echo -n "$dflatExit "
		echo -n "${dflatOptValAndCount[1]} "
	fi

	# remove temp file
	rm -f $instance $claspOptValFile $claspCountFile $dflatOptValAndCountFile
	trap - EXIT
done
echo
