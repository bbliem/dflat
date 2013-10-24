#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
ROOT=$DIR/../..

# generate graph problem instances and put them into buckets according to width of tree decomposition

generator=$DIR/tree_instance_generator.py
solver=$ROOT/build/release/dflat

# Observation for 3-COL: If the number of edges is roughly 2.5 times the number of nodes, hardly any instance has 3-colorings anymore. Up to 2.25, most instances have 3-colorings.

minNumNodes=20
maxNumNodes=100
stepNumNodes=4
minNumRandomEdges=6
maxNumRandomEdges=30
stepNumRandomEdges=1

numInstances=4

seed=$(date +%s) # current time

for numNodes in $(seq $minNumNodes $stepNumNodes $maxNumNodes); do
	echo $numNodes nodes:
	for numRandomEdges in $(seq $minNumRandomEdges $stepNumRandomEdges $maxNumRandomEdges); do
		echo -n "	${numRandomEdges} random edges"
		for instanceNumber in $(seq 1 $numInstances); do
			echo -n "." # progress marker
			let "seed += 1"

			# write random instance into temp file
			instance=$(mktemp)
			trap "rm -f $instance" EXIT

			# write instance to file
			$generator $numNodes $numRandomEdges $seed > $instance

			width=$($solver -e edge -s $seed --only-decompose --stats < $instance | awk '/Width:/ {print $2}' | head -n1)
			directory=$ROOT/instances/graphs/width${width}

			# does an instance for this width/size already exist?
			if [ -e ${directory}/${numNodes}_${numRandomEdges}_*.lp ]; then
				rm -f $instance
			else
				mkdir -p $directory
				basename=${directory}/${numNodes}_${numRandomEdges}_${seed}
				mv $instance ${basename}.lp
			fi

			trap - EXIT
		done
		echo
	done
	echo
done
