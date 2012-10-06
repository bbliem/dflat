#!/bin/bash
# Arguments to this script will be passed to $sat additionally

numInstances=100
numClauses=12
numVars=8

satgen=./satgen.py
#minisat=/home/bernhard/Informatik/MiniSat_v1.14_linux
clasp=../clasp-2.0.2-st-x86-linux
sat=./build/release/sat

for instance in $(seq 1 $numInstances); do
	seed=$RANDOM

	lpInstance=$(mktemp)
	dimacsInstance=$(mktemp)
	trap "rm -f $lpInstance $dimacsInstance" EXIT

	$satgen $numClauses $numVars dimacs $seed > $dimacsInstance 2>/dev/null || exit
#	$minisat < $dimacsInstance &>/dev/null
#	miniSatExitCode=$?
	claspNum=$($clasp -q 0 -f $dimacsInstance | awk '/Models/ { print $4 }')

	$satgen $numClauses $numVars lp $seed > $lpInstance 2>/dev/null || exit
#	$sat -p decision -s $seed $@ < $lpInstance &>/dev/null
#	satExitCode=$?
	satNum=$($sat -p counting -s $seed $@ < $lpInstance | awk '/^Solutions/ { print $2 }')

#	if [[ ($miniSatExitCode -eq 10 && $satExitCode -ne 0) || ($miniSatExitCode -eq 20 && $satExitCode -ne 23) ]]; then
	if [ $satNum -ne $claspNum ]; then
		cp $dimacsInstance mismatch${seed}.dimacs
		cp $lpInstance mismatch${seed}.lp
		echo
		echo "Mismatch for seed $seed (sat: ${satNum}, clasp: ${claspNum})"
	else
		echo -n .
	fi

	# remove temp file
	rm -f $dimacsInstance $lpInstance
	trap - EXIT
done
echo
