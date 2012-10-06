#!/bin/bash

numElements=10
numOrderings=14
export instanceGen="tools/cyclic_ordering/instance_generator.py $numElements $numOrderings"
export monolithicEncoding=asp_encodings/cyclic_ordering/monolithic.lp
export exchangeEncoding=asp_encodings/cyclic_ordering/exchange_decision.lp
export edgeArguments="-e order"

tools/verify_decision.sh
