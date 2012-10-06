#!/bin/bash

numClauses=12
numVars=6
export instanceGen="tools/sat/instance_generator.py $numClauses $numVars"
export monolithicEncoding=asp_encodings/sat/monolithic.lp
export exchangeEncoding=asp_encodings/sat/exchange_decision.lp
export joinEncoding=asp_encodings/sat/join.lp
export edgeArguments="-e pos -e neg"

tools/verify_decision.sh
