#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
ROOT=$DIR/../..

dflat=$ROOT/build/release/dflat
gringo=gringo
clasp=clasp

exchangeDecEnc=$DIR/exchange_decision.lp
monolithicEnc=$DIR/monolithic.lp

echo "width,elements,num,extra_triples,seed,co_decision,monolithic_decision,consistent"

function printTime {
time=$1
exitCode=$2
case "$exitCode" in
	134 | 2)
		echo -n "memout,"
		;;
	137)
		echo -n "timeout,"
		;;
	*)
		echo -n "$time,"
		;;
esac
}

for f in $@; do
	suffix=${f##*width}
	instance_data=${suffix%%.lp}
	IFS="_/" read width elements num triples seed <<< "$instance_data"
	echo -n "$width,$elements,$num,$triples,$seed,"

	unset consistent

	# DECISION
	read time exitCode <<< $(\time -f "%U %S %x" bash -c "$dflat -p decision -e order -x $exchangeDecEnc -s $seed < $f" 2>&1 | tail -n1 | awk '{printf("%s %s", $1+$2, $3)}')
	printTime $time $exitCode

	if [[ $exitCode -eq 10 || $exitCode -eq 20 ]]; then
		consistent=$(($exitCode == 10))
	fi

	read time exitCode <<< $(\time -f "%U %S %x" bash -c "$gringo $f $monolithicEnc 2>/dev/null | $clasp -q" 2>&1 | tail -n1 | awk '{printf("%s %s", $1+$2, $3)}')
	printTime $time $exitCode

	if [[ $exitCode -eq 10 || $exitCode -eq 20 ]]; then
		newConsistent=$(($exitCode == 10))
		if [[ "$consistent" && $consistent -ne $newConsistent ]]; then
			echo mismatch
			exit 1
		fi
		consistent=$newConsistent
	fi

	if [[ "$consistent" ]]; then
		echo "$consistent"
	else
		echo unknown
	fi
done
