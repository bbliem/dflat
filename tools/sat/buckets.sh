#!/bin/bash

# generate SAT instances and put them into buckets according to width of tree decomposition

satgen=tools/sat/instance_generator.py
sat=build/release/sat

minNumClauses=30
maxNumClauses=60
stepNumClauses=2
minNumVars=20
maxNumVars=80
stepNumVars=4

numInstances=4

seed=$(date +%s) # current time

for numClauses in $(seq $minNumClauses $stepNumClauses $maxNumClauses); do
	echo $numClauses clauses:
	for numVars in $(seq $minNumVars $stepNumVars $maxNumVars); do
		echo -n "	${numVars} variables"
		for instanceNumber in $(seq 1 $numInstances); do
			echo -n "." # progress marker
			let "seed += 1"

			# write random instance into temp file
			instance=$(mktemp)
			trap "rm -f $instance" EXIT

			# write instance to file
			$satgen $numClauses $numVars lp $seed > $instance

			width=$($sat -s $seed --only-decompose --stats < $instance | awk '/Width:/ {print $2}' | head -n1)
			directory=instances/width${width}

			# does an instance for this width/size already exist?
			if [ -e ${directory}/${numClauses}_${numVars}_*.lp ]; then
				rm -f $instance
			else
				mkdir -p $directory
				basename=${directory}/${numClauses}_${numVars}_${seed}
				mv $instance ${basename}.lp
				$satgen $numClauses $numVars dimacs $seed > ${basename}.dimacs
			fi

			trap - EXIT
		done
		echo
	done
	echo
done
