#!/bin/bash
# Arguments to this script will be passed to $co additionally

numInstances=100
numElements=14
numOrderings=21

instanceGen=tools/cyclic_ordering/instance_generator.py
gringo=../gringo
clasp=../clasp-2.0.2-st-x86-linux
monolithicEncoding=asp_encodings/cyclic_ordering/monolithic.lp
co=build/release/cyclic_ordering

for instance in $(seq 1 $numInstances); do
	seed=$RANDOM

	instance=$(mktemp)
	trap "rm -f $instance" EXIT

	$instanceGen $numElements $numOrderings $seed > $instance 2>/dev/null || exit

	$gringo $monolithicEncoding $instance 2>/dev/null | $clasp -q >/dev/null
	claspExit=$?
	$co -s $seed $@ < $instance &>/dev/null
	coExit=$?

	if [ $claspExit -ne $coExit ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Mismatch for seed $seed (cyclic_ordering: ${threeColNum}, clasp: ${claspNum})"
	else
#		echo -n .
		echo -n "$coExit "
	fi

	# remove temp file
	rm -f $instance
	trap - EXIT
done
echo
