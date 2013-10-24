#!/bin/bash

numClauses=12
numVars=8
export instanceGen="applications/sat/instance_generator.py $numClauses $numVars"
export monolithicEncoding=applications/sat/monolithic.lp
#export dflatArguments="-x applications/sat/exchange.lp -j applications/sat/join.lp -e pos -e neg -n semi"
export dflatArguments="applications/sat/dynamic.lp -e pos -e neg"

applications/test_counting.sh
