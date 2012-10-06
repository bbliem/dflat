#!/bin/bash

echo "width,clauses,vars,seed,sat_semi,sat_norm,monolithic,models"

function printTime {
time=$1
exitCode=$2
case "$exitCode" in
	134)
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
	IFS="_/" read width clauses vars seed <<< "$instance_data"
	echo -n "$width,$clauses,$vars,$seed,"

	unset count

	read count1 time exitCode <<< $(\time -f "%U %S %x" bash -c "build/release/sat -p counting -s $seed < $f" 2>&1 | tail -n2 | awk 'NR == 1 { printf("%s ", $2)} NR == 2 {printf("%s %s", $1+$2, $3)}')
	printTime $time $exitCode

	if [[ $exitCode -eq 0 ]]; then
		count=$count1
	fi

	read count2 time exitCode <<< $(\time -f "%U %S %x" bash -c "build/release/sat -p counting -n normalized -s $seed < $f" 2>&1 | tail -n2 | awk 'NR == 1 { printf("%s ", $2)} NR == 2 {printf("%s %s", $1+$2, $3)}')
	printTime $time $exitCode

	if [[ $exitCode -eq 0 ]]; then
		count=$count2
	fi

	read count3 time exitCode <<< $(\time -f "%U %S %x" bash -c "../gringo $f asp_encodings/sat/monolithic.lp 2>/dev/null | ../clasp-2.0.2-st-x86-linux -q 0" 2>&1 | tail -n5 | awk 'NR == 1 { if($0 ~ /^Models/) printf("%s ", $3); else printf("? ")} NR == 5 {printf("%s %s", $1+$2, $3)}')
	printTime $time $exitCode

	if [[ $exitCode -eq 10 || $exitCode -eq 20 ]]; then
		count=$count3
	fi

	if [[ "$count" ]]; then
		echo "$count"
	else
		echo unknown
	fi
done
