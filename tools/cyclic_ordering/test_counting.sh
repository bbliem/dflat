#!/bin/bash

numElements=10
numOrderings=14
export instanceGen="tools/cyclic_ordering/instance_generator.py $numElements $numOrderings"
export monolithicEncoding=asp_encodings/cyclic_ordering/monolithic.lp
export dflatArguments="-x asp_encodings/cyclic_ordering/exchange.lp -j asp_encodings/cyclic_ordering/join.lp -n semi -e order"

tools/test_counting.sh
