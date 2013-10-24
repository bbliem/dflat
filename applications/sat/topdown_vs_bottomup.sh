#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
gringo=gringo
clasp=clasp
exchangegen=$DIR/exchange_generator.py

numInstances=10

# Für 35 35 0 1000 ist bottom-up meistens ein bisschen schneller
# Für 30 30 0 1000 ist bottom-up manchmal schneller
# Für 25 25 0 1000 ist bottom-up selten schneller
# Für 20 20 0 1000 ist bottom-up fast nie schneller

# Für 35 35 0 3000 ist bottom-up schon deutlich langsamer

bagSize=32
childBagSize=32
numIntroduced=1
numChildTuples=100

for i in $(seq 1 $numInstances); do
	instance=$(mktemp)
	trap "rm -f $instance" EXIT
	$exchangegen $bagSize $childBagSize $numIntroduced $numChildTuples > $instance || exit

	\time -f "%U" bash -c "$gringo $DIR/exchange_decision.lp $instance 2>/dev/null | $clasp 0 >/dev/null" 2>&1 | tail -n1
	\time -f "%U" bash -c "$gringo $DIR/exchange.lp $instance 2>/dev/null | $clasp 0 >/dev/null" 2>&1 | tail -n1
	echo

	rm -f $instance
	trap - EXIT
done
