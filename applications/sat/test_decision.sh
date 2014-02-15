#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
numClauses=12
numVars=6
export instanceGen="$DIR/instance_generator.py $numClauses $numVars"
export monolithicEncoding=$DIR/monolithic.lp
export dflatArguments="-p $DIR/dynamic.lp"

$DIR/../test_decision.sh
