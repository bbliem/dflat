#!/bin/bash

gringo=../gringo
clasp=../clasp-2.0.2-st-x86-linux
exchangegen=./exchangegen.py

numInstances=10

# Für 35 35 0 1000 ist bottom-up meistens ein bisschen schneller
# Für 30 30 0 1000 ist bottom-up manchmal schneller
# Für 25 25 0 1000 ist bottom-up selten schneller
# Für 20 20 0 1000 ist bottom-up fast nie schneller

# Für 35 35 0 3000 ist bottom-up schon deutlich langsamer

bagSize=15
childBagSize=15
numIntroduced=8
numChildTuples=5000

for i in $(seq 1 $numInstances); do
	instance=$(mktemp)
	trap "rm -f $instance" EXIT
	$exchangegen $bagSize $childBagSize $numIntroduced $numChildTuples > $instance || exit

	\time -f "%U" bash -c "$gringo exchange_decision.lp $instance 2>/dev/null | $clasp 0 >/dev/null" 2>&1 | tail -n1
	\time -f "%U" bash -c "$gringo exchange.lp $instance 2>/dev/null | $clasp 0 >/dev/null" 2>&1 | tail -n1
	echo

	rm -f $instance
	trap - EXIT
done
