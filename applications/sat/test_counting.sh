#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
numClauses=12
numVars=8
export instanceGen="$DIR/instance_generator.py $numClauses $numVars"
export monolithicEncoding=$DIR/monolithic.lp
export dflatArguments="-p $DIR/dynamic.lp"

$DIR/../test_counting.sh
