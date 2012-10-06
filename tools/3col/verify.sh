#!/bin/bash
# Arguments to this script will be passed to $threeCol additionally

numInstances=100
numNodes=10
numEdges=20

threeColGen=tools/3col/instance_generator.py
gringo=../gringo
clasp=../clasp-2.0.2-st-x86-linux
monolithicEncoding=monolithic/3col.lp
threeCol=build/release/3col

for instance in $(seq 1 $numInstances); do
	seed=$RANDOM

	instance=$(mktemp)
	trap "rm -f $instance" EXIT

	$threeColGen $numNodes $numEdges $seed > $instance 2>/dev/null || exit

	claspNum=$($gringo $monolithicEncoding $instance | $clasp -q 0 | awk '/^Models/ { print $3 }')
	threeColNum=$($threeCol -p counting -s $seed $@ < $instance | awk '/^Solutions/ { print $2 }')

#	echo "monolithic: $claspNum; decomposed: $threeColNum"

	if [ $threeColNum -ne $claspNum ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Mismatch for seed $seed (3col: ${threeColNum}, clasp: ${claspNum})"
	else
		echo -n .
	fi

	# remove temp file
	rm -f $instance
	trap - EXIT
done
echo
