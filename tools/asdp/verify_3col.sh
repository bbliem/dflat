#!/bin/bash

numInstances=100
numNodes=10
numEdges=20

instanceGen=tools/3col/instance_generator.py
gringo=../gringo
clasp=../clasp-2.0.2-st-x86-linux
monolithicEncoding=asp_encodings/3col/monolithic.lp
exchangeEncoding=asp_encodings/3col/exchange_decision.lp
asdp="build/release/asdp -e edge $exchangeEncoding -p decision"

for instance in $(seq 1 $numInstances); do
	seed=$RANDOM

	instance=$(mktemp)
	trap "rm -f $instance" EXIT

	$instanceGen $numNodes $numEdges $seed > $instance 2>/dev/null || exit

	$gringo $monolithicEncoding $instance 2>/dev/null | $clasp -q >/dev/null
	claspExit=$?
	$asdp -s $seed $@ < $instance &>/dev/null
	asdpExit=$?

	if [ $claspExit -ne $asdpExit ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Mismatch for seed $seed (asdp: ${asdpExit}, clasp: ${claspExit})"
	else
#		echo -n .
		echo -n "$asdpExit "
	fi

	# remove temp file
	rm -f $instance
	trap - EXIT
done
echo
