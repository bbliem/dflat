#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
ROOT=$DIR/../..
MISMATCH_DIR=$ROOT/mismatch
numInstances=100
# Note that blocksqbf generates QBFs with \exists as innermost quantifier
#instanceGen="$ROOT/../blocksqbf/blocksqbf -c 10 -b 1 -bs 4 -bc 2"
#instanceGen="$ROOT/../blocksqbf/blocksqbf -c 10 -b 3 -bs 2 -bs 2 -bs 2 -bc 1 -bc 1 -bc 1"
#instanceGen="$ROOT/../blocksqbf/blocksqbf -c 20 -b 4 -bs 1 -bs 2 -bs 1 -bs 2 -bc 1 -bc 1 -bc 1 -bc 1"
blocks=4
instanceGen="blocksqbf -c 10 -b $blocks $(for i in `seq 1 $blocks`; do echo -n '-bs 1 '; done) $(for i in `seq 1 $blocks`; do echo -n '-bc 1 '; done)"
qdimacs2lp=$DIR/qdimacs2lp.awk
dflatArguments="-p $DIR/dynamic.lp"

for instance in $(seq 1 $numInstances); do
	seed=$RANDOM

	instance=$(mktemp)
	trap "rm -f $instance" EXIT

	$instanceGen -s $seed > $instance || exit

	depqbf $instance >/dev/null
	depQbfExit=$?
	awk -f $qdimacs2lp $instance | dflat $dflatArguments --depth 0 --seed $seed >/dev/null
	dflatExit=$?

	if [ $depQbfExit -ne $dflatExit ]; then
		mkdir -p $MISMATCH_DIR
		cp $instance $MISMATCH_DIR/mismatch${seed}.qdimacs
		awk -f $qdimacs2lp $instance > $MISMATCH_DIR/mismatch${seed}.lp
		echo
		echo "Mismatch for seed $seed (dflat: ${dflatExit}, DepQBF: ${depQbfExit})"
		exit 1
	else
#		echo -n .
		echo -n "$dflatExit "
	fi

	# remove temp file
	rm -f $instance
	trap - EXIT
done
echo
