#!/bin/bash

numClauses=12
numVars=6
export instanceGen="applications/sat/instance_generator.py $numClauses $numVars"
export monolithicEncoding=applications/sat/monolithic.lp
export dflatArguments="-x applications/sat/exchange_decision.lp -j applications/sat/join.lp -e pos -e neg -n semi"

applications/test_decision.sh
