#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
ROOT=$DIR/../..
MISMATCH_DIR=$ROOT/mismatch
numInstances=100
metaspDir=$DIR/metasp

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

	$instanceGen $seed > $instance || exit

	gringo --reify $monolithicEncoding $instance | gringo - ${metaspDir}/{meta.lp,metaD.lp,metaO.lp} <(echo "optimize(1,1,incl).") | clasp -q 0 | awk '/Models/ { print $3 }' > $claspCountFile
#	claspExit=${PIPESTATUS[2]}
	claspCount=$(<$claspCountFile)

	dflat $dflatArguments --depth 0 --seed $seed < $instance | tail -n1 | sed 's/\[\([0-9]*\)\]/\1/g' > $dflatCountFile
#	dflatExit=${PIPESTATUS[0]}
	dflatCount=$(<$dflatCountFile)

#	[ $claspExit -ne 30 ] || claspExit=10

#	if [ $claspExit -ne $dflatExit ]; then
#		cp $instance mismatch${seed}.lp
#		echo
#		echo "Exit code mismatch for seed $seed (dflat: ${dflatExit}, clasp: ${claspExit})"
#		exit 1
#	elif [ $claspCount -ne $dflatCount ]; then
	if [ $claspCount -ne $dflatCount ]; then
		mkdir -p $MISMATCH_DIR
		cp $instance $MISMATCH_DIR/mismatch${seed}.lp
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
