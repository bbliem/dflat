#!/bin/bash

dflat=build/release/dflat
gringo=../gringo
clasp=../clasp

exchangeEnc=asp_encodings/vertex_cover/exchange.lp
exchangeDecEnc=asp_encodings/vertex_cover/exchange_decision.lp
monolithicEnc=asp_encodings/vertex_cover/monolithic.lp

echo "width,nodes,num,edges,seed,vc_opt-value,monolithic_opt-value,vc_opt-counting,monolithic_opt-counting_value_given,vc_opt-enum,monolithic_opt-enum_value_given,optimum,optimal_vcs"

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
	IFS="_/" read width nodes num edges seed <<< "$instance_data"
	echo -n "$width,$nodes,$num,$edges,$seed,"

	unset optValue
	unset count

	# OPT-VALUE
	read curOptValue time exitCode <<< $(\time -f "TIME %U %S %x" bash -c "$dflat -p opt-value -e edge -x $exchangeEnc -s $seed < $f" 2>&1 | grep "^Minimum cost:\|^TIME" | awk 'BEGIN { cost = 0 } /^Minimum cost:/ { cost = $3 } /^TIME/ { time = $2+$3; code = $4 } END { printf("%s %s %s", cost, time, code) }')
	printTime $time $exitCode

	if [[ $exitCode -eq 10 || $exitCode -eq 20 ]]; then
		optValue=$curOptValue
	fi

	read curOptValue time exitCode <<< $(\time -f "%U %S %x" bash -c "$gringo $f $monolithicEnc 2>/dev/null | $clasp -q 0" 2>&1 | tail -n5 | awk 'NR == 1 { if($0 ~ /^Optimization/) printf("%s ", $2); else printf("? ")} NR == 5 {printf("%s %s", $1+$2, $3)}')
	printTime $time $exitCode

	if [[ $exitCode -eq 10 || $exitCode -eq 20 ]]; then
		if [[ "$optValue" && $optValue -ne $curOptValue ]]; then
			echo mismatch
			exit 1
		fi
		optValue=$curOptValue
	fi

	# If no optimal value was obtained, the other tests are pointless
	[ "$optValue" ] || (echo -n "?,?,?,?,?,?"; continue)

	# OPT-COUNTING
	read curCount time exitCode <<< $(\time -f "TIME %U %S %x" bash -c "$dflat -p opt-counting -e edge -x $exchangeEnc -s $seed < $f" 2>&1 | grep "^Optimal solutions:\|^TIME" | awk 'BEGIN { sol = 0 } /^Optimal solutions:/ { sol = $3 } /^TIME/ { time = $2+$3; code = $4 } END { printf("%s %s %s", sol, time, code) }')
	printTime $time $exitCode

	if [[ $exitCode -eq 10 || $exitCode -eq 20 ]]; then
		if [[ "$count" && $count -ne $curCount ]]; then
			echo mismatch
			exit 2
		fi
		count=$curCount
	fi

	read curCount time exitCode <<< $(\time -f "%U %S %x" bash -c "$gringo $f $monolithicEnc 2>/dev/null | $clasp -q 0 --opt-all=${optValue}" 2>&1 | tail -n7 | awk 'NR == 1 { if($0 ~ /^Models/) printf("%s ", $3); else printf("? ")} NR == 7 {printf("%s %s", $1+$2, $3)}')
	printTime $time $exitCode

	if [[ $exitCode -eq 10 || $exitCode -eq 20 ]]; then
		if [[ "$count" && $count -ne $curCount ]]; then
			echo mismatch
			exit 3
		fi
		count=$curCount
	fi

	# OPT-ENUM
	read curCount time exitCode <<< $(\time -f "TIME %U %S %x" bash -c "$dflat -p opt-enum -e edge -x $exchangeEnc -s $seed < $f" 2>&1 | grep "^Optimal solutions:\|^TIME" | awk 'BEGIN { sol = 0 } /^Optimal solutions:/ { sol = $3 } /^TIME/ { time = $2+$3; code = $4 } END { printf("%s %s %s", sol, time, code) }')
	printTime $time $exitCode

	if [[ $exitCode -eq 10 || $exitCode -eq 20 ]]; then
		if [[ "$count" && $count -ne $curCount ]]; then
			echo mismatch
			exit 4
		fi
		count=$curCount
	fi

	read curCount time exitCode <<< $(\time -f "%U %S %x" bash -c "$gringo $f $monolithicEnc 2>/dev/null | $clasp 0 --opt-all=${optValue}" 2>&1 | tail -n7 | awk 'NR == 1 { if($0 ~ /^Models/) printf("%s ", $3); else printf("? ")} NR == 7 {printf("%s %s", $1+$2, $3)}')
	printTime $time $exitCode

	if [[ $exitCode -eq 10 || $exitCode -eq 20 ]]; then
		if [[ "$count" && $count -ne $curCount ]]; then
			echo mismatch
			exit 5
		fi
		count=$curCount
	fi

	echo -n "$optValue,"

	if [[ "$count" ]]; then
		echo "$count"
	else
		echo unknown
	fi
done
