#!/bin/bash

# generate cyclic ordering problem instances and put them into buckets according to width of tree decomposition

generator=tools/cyclic_ordering/tree_instance_generator.py
solver=build/release/dflat

minNumElements=9
maxNumElements=60
stepNumElements=6

minExtraTriples=2
maxExtraTriples=18
#numExtraTriples=15

tw=7
numInstancesPerTw=28
giveUpAfter=1000 # unsuccessful tries

seed=$(date +%s) # current time

for numElements in $(seq $minNumElements $stepNumElements $maxNumElements); do
	echo $numElements elements:
	tries=0
	numGenerated=0
	while [ $numGenerated -lt $numInstancesPerTw ]; do
		numExtraTriples=$RANDOM
		let "numExtraTriples %= ($maxExtraTriples - $minExtraTriples)"
		let "numExtraTriples += $minExtraTriples"

		let "seed += 1"

		instance=$(mktemp)
		trap "rm -f $instance" EXIT

		# write instance to file
		$generator $numElements $numExtraTriples $seed > $instance || exit

		width=$($solver -e order -s $seed --only-decompose --stats < $instance | awk '/Width:/ {print $2}' | head -n1)
		directory=instances/cyclic_ordering/width${width}

		if [ $width -eq $tw ]; then
			echo -n "x"
			mkdir -p $directory
			mv $instance ${directory}/${numElements}_${numGenerated}_${numExtraTriples}_${seed}.lp
			let "numGenerated += 1"
		else
			echo -n "."
			rm -f $instance
			let "tries += 1"

			if [ $tries -ge $giveUpAfter ]; then
				echo "Exceeded tries for generating $numInstancesPerTw instances with tw=$tw and $numElements nodes" 1>&2
				break
			fi
		fi

		trap - EXIT
	done
	echo
done
