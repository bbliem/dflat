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
	trap "rm -f $instance" EXIT

	$instanceGen $seed > $instance 2>/dev/null || exit

	$gringo $monolithicEncoding $instance 2>/dev/null | $clasp -q >/dev/null
	claspExit=$?
	$dflat $edgeArguments -x $exchangeEncoding $joinEncodingArgument -p decision -s $seed < $instance &>/dev/null
	dflatExit=$?

	if [ $claspExit -ne $dflatExit ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Mismatch for seed $seed (dflat: ${dflatExit}, clasp: ${claspExit})"
	else
#		echo -n .
		echo -n "$dflatExit "
	fi

	# remove temp file
	rm -f $instance
	trap - EXIT
done
echo
