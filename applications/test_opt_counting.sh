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

	gringo $monolithicEncoding $instance | clasp -q 0 --opt-mode=optN | awk '
			BEGIN { count = 1 }
			/^ *Optimal/ { count = $3 }
			/^Optimization/ { optval = $3 }
			END { print count; print optval }
		' > $claspOptValAndCountFile
	claspExit=${PIPESTATUS[1]}
	if [ $claspExit -ne 20 ]; then
		OLDIFS=$IFS
		IFS=$'\n'
		claspOptValAndCount=($(<$claspOptValAndCountFile))
		IFS=$OLDIFS
		claspCount=${claspOptValAndCount[0]}
		claspOptVal=${claspOptValAndCount[1]}
#		# If there is only one optimum model, clasp does not produce a line containing "Optimal".
#		case ${#claspOptValAndCount[@]} in
#			1)
#				claspCount=1
#				claspOptVal=${claspOptValAndCount[0]}
#				;;
#			2)
#				claspCount=${claspOptValAndCount[0]}
#				claspOptVal=${claspOptValAndCount[1]}
#				;;
#			*)
#				echo "Clasp did report optimum value and number of optimum models" 1>&2
#				cp $instance mismatch${seed}.lp
#				echo "Instance written to mismatch${seed}.lp" 1>&2
#				exit 9
#				;;
#		esac
	else
		claspCount=0
		claspOptVal=0
	fi

	dflat $dflatArguments --depth 0 --seed $seed < $instance | tail -n1 | awk '
		{
			print $3 ? substr($3,1,length($3)-1) : 0
			print substr($1,2,length($1)-2)
		}' > $dflatOptValAndCountFile
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
	elif [ "$claspOptVal" != "$dflatOptVal" ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Optimum value mismatch for seed $seed (dflat: ${dflatOptVal}, clasp: ${claspOptVal})"
		exit 2
	elif [ "$claspCount" != "$dflatCount" ]; then
		cp $instance mismatch${seed}.lp
		echo
		echo "Count mismatch for seed $seed (dflat: ${dflatCount}, clasp: ${claspCount})"
		exit 3
	else
#		echo -n "$dflatExit "
		echo -n "${dflatOptValAndCount[1]} (${dflatOptValAndCount[0]}) "
	fi

	# remove temp file
	rm -f $instance $claspOptValAndCountFile $dflatOptValAndCountFile
	trap - EXIT
done
echo
