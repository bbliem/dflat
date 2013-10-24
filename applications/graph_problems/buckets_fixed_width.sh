#!/bin/bash
# generate graph problem instances and put them into buckets according to width of tree decomposition
DIR=$(cd "$( dirname "$0" )" && pwd)
ROOT=$DIR/../..
generator=$DIR/instance_generator.py
solver=$ROOT/build/release/dflat

# Observation for 3-COL: If the number of edges is roughly 2.5 times the number of nodes, hardly any instance has 3-colorings anymore. Up to 2.25, most instances have 3-colorings.

minNumNodes=40
maxNumNodes=80
stepNumNodes=2

maxEdgeFactor=3 # Produce up to numNodes * maxEdgeFactor edges

tw=12
numInstancesPerTw=20
giveUpAfter=1000 # unsuccessful tries

seed=$(date +%s) # current time

for numNodes in $(seq $minNumNodes $stepNumNodes $maxNumNodes); do
	echo $numNodes nodes:
#	maxNumEdges=$(echo "($numNodes * $maxEdgeFactor)/1" | bc) # if maxEdgeFactor were fractional...
	maxNumEdges=$(($numNodes * $maxEdgeFactor))
	tries=0
	numGenerated=0
	while [ $numGenerated -lt $numInstancesPerTw ]; do
		numEdges=$RANDOM
		let "numEdges %= ($maxNumEdges - $numNodes)"
		let "numEdges += $numNodes"

		let "seed += 1"

		instance=$(mktemp)
		trap "rm -f $instance" EXIT

		# write instance to file
		$generator $numNodes $numEdges $seed > $instance

		width=$($solver -e edge -s $seed --only-decompose --stats < $instance | awk '/Width:/ {print $2}' | head -n1)
		directory=$ROOT/instances/graphs/width${width}

		if [ $width -eq $tw ]; then
			echo -n "x"
			mkdir -p $directory
			mv $instance ${directory}/${numNodes}_${numGenerated}_${numEdges}_${seed}.lp
			let "numGenerated += 1"
		else
			echo -n "."
			rm -f $instance
			let "tries += 1"

			if [ $tries -ge $giveUpAfter ]; then
				echo "Exceeded tries for generating $numInstancesPerTw instances with tw=$tw and $numNodes nodes" 1>&2
				break
			fi
		fi

		trap - EXIT
	done
	echo
done
