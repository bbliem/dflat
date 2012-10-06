#!/bin/bash

# generate 3col instances and put them into buckets according to width of tree decomposition

generator=tools/3col/instance_generator.py
solver=build/release/3col

minNumNodes=30
maxNumNodes=60
stepNumNodes=2
minNumEdges=60
maxNumEdges=180
stepNumEdges=10

numInstances=4

seed=$(date +%s) # current time

for numNodes in $(seq $minNumNodes $stepNumNodes $maxNumNodes); do
	echo $numNodes nodes:
	for numEdges in $(seq $minNumEdges $stepNumEdges $maxNumEdges); do
		echo -n "	${numEdges} edges"
		for instanceNumber in $(seq 1 $numInstances); do
			echo -n "." # progress marker
			let "seed += 1"

			# write random instance into temp file
			instance=$(mktemp)
			trap "rm -f $instance" EXIT

			# write instance to file
			$generator $numNodes $numEdges $seed > $instance

			width=$($solver -s $seed --only-decompose --stats < $instance | awk '/Width:/ {print $2}' | head -n1)
			directory=instances/3col/width${width}

			# does an instance for this width/size already exist?
			if [ -e ${directory}/${numNodes}_${numEdges}_*.lp ]; then
				rm -f $instance
			else
				mkdir -p $directory
				basename=${directory}/${numNodes}_${numEdges}_${seed}
				mv $instance ${basename}.lp
			fi

			trap - EXIT
		done
		echo
	done
	echo
done
