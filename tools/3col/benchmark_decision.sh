#!/bin/bash

for f in $@; do
	echo $f
	suffix=${f##*_}
	seed=${suffix%%.lp}

	echo -en "\t3col: "
	threeColTime=$(\time -f "%U %S" build/release/3col -p decision -s $seed < $f 2>&1)
	threeColExitCode=$?
	echo "$threeColTime" | tail -n1
	echo -en "\tmono: "
	monoTime=$(\time -f "%U %S" bash -c "../gringo $f asp_encodings/3col/monolithic.lp 2>/dev/null | ../clasp-2.0.2-st-x86-linux -q >/dev/null" 2>&1)
	monoExitCode=$?
	echo "$monoTime" | tail -n1

	if [[ $threeColExitCode -ne $monoExitCode ]]; then
		echo -e "\tMismatch"
	fi
	echo -e "\tresults 3col: ${threeColExitCode} mono: ${monoExitCode}"
done
