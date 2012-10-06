#!/bin/bash

numClauses=12
numVars=8
export instanceGen="tools/sat/instance_generator.py $numClauses $numVars"
export monolithicEncoding=asp_encodings/sat/monolithic.lp
export exchangeEncoding=asp_encodings/sat/exchange.lp
export joinEncoding=asp_encodings/sat/join.lp
export edgeArguments="-e pos -e neg"

tools/verify_counting.sh
