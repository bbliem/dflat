#!/bin/bash

numClauses=12
numVars=6
export instanceGen="tools/sat/instance_generator.py $numClauses $numVars"
export monolithicEncoding=asp_encodings/sat/monolithic.lp
export dflatArguments="-x asp_encodings/sat/exchange_decision.lp -j asp_encodings/sat/join.lp -e pos -e neg -n semi"

tools/verify_decision.sh
