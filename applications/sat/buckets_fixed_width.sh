#!/bin/bash
# generate SAT problem instances and put them into buckets according to width of tree decomposition
DIR=$(cd "$( dirname "$0" )" && pwd)
ROOT=$DIR/../..
generator=$DIR/tree_instance_generator.py
solver=$ROOT/build/release/dflat

minNumVars=17
maxNumVars=44
stepNumVars=3

minNumExtraClauses=5
maxNumExtraClauses=20

tw=8
numInstancesPerTw=20
giveUpAfter=1000 # unsuccessful tries

seed=$(date +%s) # current time

for numVars in $(seq $minNumVars $stepNumVars $maxNumVars); do
	echo $numVars vars:
	tries=0
	numGenerated=0
	while [ $numGenerated -lt $numInstancesPerTw ]; do
		numExtraClauses=$RANDOM
		let "numExtraClauses %= ($maxNumExtraClauses - $minNumExtraClauses)"
		let "numExtraClauses += $minNumExtraClauses"

		let "seed += 1"

		instance=$(mktemp)
		trap "rm -f $instance" EXIT

		# write instance to file
		$generator $numVars $numExtraClauses $seed > $instance

		width=$($solver -e pos -e neg -s $seed --only-decompose --stats < $instance | awk '/Width:/ {print $2}' | head -n1)
		directory=$ROOT/instances/sat/width${width}

		if [ $width -eq $tw ]; then
			echo -n "x"
			mkdir -p $directory
			mv $instance ${directory}/${numVars}_${numGenerated}_${numExtraClauses}_${seed}.lp
			let "numGenerated += 1"
		else
			echo -n "."
			rm -f $instance
			let "tries += 1"

			if [ $tries -ge $giveUpAfter ]; then
				echo "Exceeded tries for generating $numInstancesPerTw instances with tw=$tw and $numVars vars" 1>&2
				break
			fi
		fi

		trap - EXIT
	done
	echo
done
