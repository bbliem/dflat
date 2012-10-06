#!/bin/bash

for f in $@; do
	echo $f
	suffix=${f##*_}
	seed=${suffix%%.lp}

	echo -en "\t3col: "
	result=$(\time -f "%U %S" build/release/3col -s $seed < $f | awk '/^Solutions/ {print $2}')
	echo -en "\tmono: "
	monolithicResult=$(\time -f "%U %S" bash -c "../gringo $f asp_encodings/3col/monolithic.lp 2>/dev/null | ../clasp-2.0.2-st-x86-linux -q 0 | awk '/^Models/ {print \$3}'")

	if [[ $result ]] && [[ $monolithicResult ]]; then
		if [[ $result -ne $monolithicResult ]]; then
			echo -e "\tMismatch (3col: ${result}, clasp: ${monolithicResult})"
		fi
	fi
	echo -e "\tresults 3col: ${result} mono: ${monolithicResult}"
done
