#!/bin/bash

numClauses=12
numVars=8
export instanceGen="tools/sat/instance_generator.py $numClauses $numVars"
export monolithicEncoding=asp_encodings/sat/monolithic.lp
export dflatArguments="-x asp_encodings/sat/exchange.lp -j asp_encodings/sat/join.lp -e pos -e neg -n semi"

tools/test_counting.sh
