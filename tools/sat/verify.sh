#!/bin/bash
# Arguments to this script will be passed to $sat additionally

numInstances=100
numClauses=12
numVars=8

satgen=tools/sat/instance_generator.py
clasp=../clasp-2.0.2-st-x86-linux
sat=build/release/sat

for instance in $(seq 1 $numInstances); do
	seed=$RANDOM

	lpInstance=$(mktemp)
	dimacsInstance=$(mktemp)
	trap "rm -f $lpInstance $dimacsInstance" EXIT

	$satgen $numClauses $numVars $seed --dimacs > $dimacsInstance 2>/dev/null || exit
	claspNum=$($clasp -q 0 -f $dimacsInstance | awk '/Models/ { print $4 }')

	$satgen $numClauses $numVars $seed > $lpInstance 2>/dev/null || exit
	satNum=$($sat -p counting -s $seed $@ < $lpInstance | tail -n1 | awk '{ print $2 }')

	if [ $satNum -ne $claspNum ]; then
		cp $dimacsInstance mismatch${seed}.dimacs
		cp $lpInstance mismatch${seed}.lp
		echo
		echo "Mismatch for seed $seed (sat: ${satNum}, clasp: ${claspNum})"
		exit 1
	else
		echo -n .
	fi

	# remove temp file
	rm -f $dimacsInstance $lpInstance
	trap - EXIT
done
echo
