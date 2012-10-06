#!/bin/bash

# generate cyclic_ordering instances and put them into buckets according to width of tree decomposition

generator=tools/cyclic_ordering/instance_generator.py
solver=build/release/cyclic_ordering

minNumElements=10
maxNumElements=24
stepNumElements=1
minOrderingFactor=1
maxOrderingFactor=2 # Produce up to numElements * maxOrderingFactor orderings
stepNumOrderings=1

numInstances=100

seed=$(date +%s) # current time

for numElements in $(seq $minNumElements $stepNumElements $maxNumElements); do
	echo $numElements elements:
	minNumOrderings=$(($numElements * $minOrderingFactor))
	maxNumOrderings=$(($numElements * $maxOrderingFactor))
	for numOrderings in $(seq $minNumOrderings $stepNumOrderings $maxNumOrderings); do
		echo -n "	${numOrderings} orderings"
		for instanceNumber in $(seq 1 $numInstances); do
			echo -n "." # progress marker
			let "seed += 1"

			# write random instance into temp file
			instance=$(mktemp)
			trap "rm -f $instance" EXIT

			# write instance to file
			$generator $numElements $numOrderings $seed > $instance

			width=$($solver -s $seed --only-decompose --stats < $instance | awk '/Width:/ {print $2}' | head -n1)
			directory=instances/cyclic_ordering/width${width}

			# does an instance for this width/size already exist?
			if [ -e ${directory}/${numElements}_${numOrderings}_*.lp ]; then
				rm -f $instance
			else
				mkdir -p $directory
				basename=${directory}/${numElements}_${numOrderings}_${seed}
				mv $instance ${basename}.lp
			fi

			trap - EXIT
		done
		echo
	done
	echo
done
