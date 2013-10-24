#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
numClauses=12
numVars=6
export instanceGen="$DIR/instance_generator.py $numClauses $numVars"
export monolithicEncoding=$DIR/monolithic.lp
export dflatArguments="-x $DIR/exchange_decision.lp -j $DIR/join.lp -e pos -e neg -n semi"

$DIR/../test_decision.sh
