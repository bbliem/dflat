#!/bin/bash

# generate cyclic_ordering instances and put them into buckets according to width of tree decomposition

generator=tools/cyclic_ordering/tree_instance_generator.py
solver=build/release/cyclic_ordering

minN=6
maxN=98
minO=0
maxO=14
stepN=1
stepO=1

minWidth=0
maxWidth=9

numInstances=4

seed=$(date +%s) # current time

for n in $(seq $minN $stepN $maxN); do
	echo n=${n}:
	for o in $(seq $minO $stepO $maxO); do
		echo -n "	o=${o}"
		for instanceNumber in $(seq 1 $numInstances); do
			echo -n "." # progress marker
			let "seed += 1"

			# write random instance into temp file
			instance=$(mktemp)
			trap "rm -f $instance" EXIT

			# write instance to file
			$generator $n $o $seed > $instance&
			wait

			width=$($solver -s $seed --only-decompose --stats < $instance | awk '/Width:/ {print $2}' | head -n1)
			[[ $width -ge $minWidth && $width -le $maxWidth ]] || continue
			directory=instances/cyclic_ordering/width${width}

			# does an instance for this width/size already exist?
			if [ -e ${directory}/${n}_${o}_*.lp ]; then
				rm -f $instance
			else
				mkdir -p $directory
				basename=${directory}/${n}_${o}_${seed}
				mv $instance ${basename}.lp
			fi

			trap - EXIT
		done
		echo
	done
	echo
done
