#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
ROOT=$DIR/../..

# generate cyclic_ordering instances and put them into buckets according to width of tree decomposition

generator=$DIR/instance_generator.py
solver=$ROOT/build/release/cyclic_ordering

minNumElements=51
maxNumElements=100
stepNumElements=1
minOrderingFactor=1
maxOrderingFactor=1.25 # Produce up to numElements * maxOrderingFactor orderings
stepNumOrderings=1
minWidth=7
maxWidth=7

numInstances=100

seed=$(date +%s) # current time

for numElements in $(seq $minNumElements $stepNumElements $maxNumElements); do
	echo $numElements elements:
	minNumOrderings=$(bc <<< "($numElements * $minOrderingFactor)/1")
	maxNumOrderings=$(bc <<< "($numElements * $maxOrderingFactor)/1")
	for numOrderings in $(seq $minNumOrderings $stepNumOrderings $maxNumOrderings); do
		echo -n "	${numOrderings} orderings"
		for instanceNumber in $(seq 1 $numInstances); do
			echo -n "." # progress marker
			let "seed += 1"

			# write random instance into temp file
			instance=$(mktemp)
			trap "rm -f $instance" EXIT

			# write instance to file
			$generator $numElements $numOrderings $seed > $instance&
			wait

			width=$($solver -s $seed --only-decompose --stats < $instance | awk '/Width:/ {print $2}' | head -n1)
			echo -n "$width "
			[[ $width -ge $minWidth && $width -le $maxWidth ]] || continue
			directory=$ROOT/instances/cyclic_ordering/width${width}

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
