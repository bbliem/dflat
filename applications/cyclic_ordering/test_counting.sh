#!/bin/bash

numElements=10
numOrderings=14
export instanceGen="applications/cyclic_ordering/instance_generator.py $numElements $numOrderings"
export monolithicEncoding=applications/cyclic_ordering/monolithic.lp
export dflatArguments="-x applications/cyclic_ordering/exchange.lp -j applications/cyclic_ordering/join.lp -n semi -e order"

applications/test_counting.sh
