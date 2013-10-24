#!/bin/bash

numElements=10
numOrderings=14
export instanceGen="applications/cyclic_ordering/instance_generator.py $numElements $numOrderings"
export monolithicEncoding=applications/cyclic_ordering/monolithic.lp
#export dflatArguments="-x applications/cyclic_ordering/exchange_decision.lp -e order -n semi"
export dflatArguments="-p applications/cyclic_ordering/dynamic_decision.lp -e order"

applications/test_decision.sh
