#!/bin/bash

numElements=10
numOrderings=14
export instanceGen="tools/cyclic_ordering/instance_generator.py $numElements $numOrderings"
export monolithicEncoding=asp_encodings/cyclic_ordering/monolithic.lp
export dflatArguments="-x asp_encodings/cyclic_ordering/exchange_decision.lp -e order -n semi"

tools/test_decision.sh
