#!/bin/bash

numInstances=100

if [[ -z "$instanceGen" || -z "$dflatArguments" || -z "$monolithicEncoding" ]]; then
	echo "Environment variables not set" 1>&2
	exit 8
fi

for instance in $(seq 1 $numInstances); do
	seed=$RANDOM

	instance=$(mktemp)
	claspOptValAndCountFile=$(mktemp)
	dflatOptValAndCountFile=$(mktemp)
	trap "rm -f $instance $claspOptValAndCountFile $dflatOptValAndCountFile" EXIT

	$instanceGen $seed > $instance 2>/dev/null || exit

	gringo $monolithicEncoding $instance | clasp -q 0 --opt-mode=optN | awk '/^ *Optimal/ { print $3 } /^Optimization/ { print $3 }' > $claspOptValAndCountFile
	claspExit=${PIPESTATUS[1]}
	OLDIFS=$IFS
	IFS=$'\n'
	claspOptValAndCount=($(<$claspOptValAndCountFile))
	IFS=$OLDIFS
	# If there is only one optimum model, clasp does not produce a line containing "Optimal".
	case ${#claspOptValAndCount[@]} in
		1)
			claspCount=1
			claspOptVal=${claspOptValAndCount[0]}
			;;
		2)
			claspCount=${claspOptValAndCount[0]}
			claspOptVal=${claspOptValAndCount[1]}
			;;
		*)
			echo "Clasp did report optimum value and number of optimum models" 1>&2
			exit 9
			;;
	esac

	dflat $dflatArguments --depth 0 --seed $seed < $instance | tail -n1 | awk '{ print substr($3,1,length($3)-1); print substr($1,2,length($1)-2) }' > $dflatOptValAndCountFile
	dflatExit=${PIPESTATUS[0]}
	OLDIFS=$IFS
	IFS=$'\n'
	dflatOptValAndCount=($(<$dflatOptValAndCountFile))
	IFS=$OLDIFS
	dflatOptVal=${dflatOptValAndCount[0]}
	dflatCount=${dflatOptValAndCount[1]}

	IFS=$OLDIFS

	[ $claspExit -ne 30 ] || claspExit=10

	if [ $claspExit -ne $dflatExit ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Exit code mismatch for seed $seed (dflat: ${dflatExit}, clasp: ${claspExit})"
		exit 1
	elif [ $claspOptVal -ne $dflatOptVal ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Optimum value mismatch for seed $seed (dflat: ${dflatOptVal}, clasp: ${claspOptVal})"
		exit 2
	elif [ $claspCount -ne $dflatCount ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Count mismatch for seed $seed (dflat: ${dflatCount}, clasp: ${claspCount})"
		exit 3
	else
#		echo -n "$dflatExit "
		echo -n "${dflatOptValAndCount[1]} "
	fi

	# remove temp file
	rm -f $instance $claspOptValAndCountFile $dflatOptValAndCountFile
	trap - EXIT
done
echo
