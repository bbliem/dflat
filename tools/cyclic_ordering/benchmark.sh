#!/bin/bash

#echo "width,elements,triples,seed,time_semi_normalized,time_normalized,time_monolithic,consistency"
echo "width,base_triples,additional_triples,instance_seed,seed,time_monolithic,consistency"

runsPerInstance=1

function printTime {
uTime=$1
sTime=$2
exitCode=$3
case "$exitCode" in
	10 | 20)
		echo -n "$(bc <<< "$uTime + $sTime"),"
		;;
	127 | 134)
		echo -n "memout,"
		;;
	137)
		echo -n "timeout,"
		;;
	*)
		echo -n "unexpected exit code ${exitCode},"
esac
}

for f in $@; do
	suffix=${f##*width}
	instance_data=${suffix%%.lp}
	IFS="_/" read width elements triples instanceSeed <<< "$instance_data"

	for run in $(seq $runsPerInstance); do
		# Try seeds until one leads to the desired width
		while true; do
			seed=$RANDOM

			effectiveWidth=$(build/release/cyclic_ordering -s $seed --only-decompose --stats < $f | grep Width | tail -n1 | awk '{print($2)}')
			[[ $effectiveWidth -ne $width ]] || break
		done

		echo -n "$width,$elements,$triples,$instanceSeed,$seed,"

		unset consistent

#		read uTime sTime exitCode <<< $(\time -f "%U %S %x" bash -c "build/release/cyclic_ordering -s $seed < $f" 2>&1 | tail -n1)
#		printTime $uTime $sTime $exitCode
#
#		if [[ $exitCode -eq 10 || $exitCode -eq 20 ]]; then
#			consistent=$exitCode
#		fi

		read uTime sTime exitCode <<< $(\time -f "%U %S %x" bash -c "../gringo $f asp_encodings/cyclic_ordering/monolithic.lp 2>/dev/null | ../clasp-2.0.2-st-x86-linux -q" 2>&1 | tail -n1)
		printTime $uTime $sTime $exitCode

		if [[ $exitCode -eq 10 || $exitCode -eq 20 ]]; then
			if [[ "$consistent" ]] && [[ $consistent -ne $exitCode ]]; then
				echo mismatch
				exit 2
			fi
			consistent=$exitCode
		fi

		if [[ "$consistent" ]]; then
			if [[ $consistent -eq 10 ]]; then echo y; elif [[ $consistent -eq 20 ]]; then echo n; else echo wtf; fi
		else
			echo unknown
		fi
	done
done
