#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
numClauses=12
numVars=8
export instanceGen="$DIR/instance_generator.py $numClauses $numVars"
export monolithicEncoding=$DIR/monolithic.lp
#export dflatArguments="-x $DIR/exchange.lp -j $DIR/join.lp -e pos -e neg -n semi"
export dflatArguments="$DIR/dynamic.lp -e pos -e neg"

$DIR/../test_counting.sh
