#!/bin/bash

numElements=10
numOrderings=14
export instanceGen="tools/cyclic_ordering/instance_generator.py $numElements $numOrderings"
export monolithicEncoding=asp_encodings/cyclic_ordering/monolithic.lp
#export dflatArguments="-x asp_encodings/cyclic_ordering/exchange_decision.lp -e order -n semi"
export dflatArguments="asp_encodings/cyclic_ordering/dynamic_decision.lp -e order"

tools/test_decision.sh
