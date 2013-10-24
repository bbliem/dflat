#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
numNodes=10
numEdges=20
export instanceGen="$DIR/../instance_generator.py $numNodes $numEdges"
export monolithicEncoding=$DIR/monolithic.lp
#export dflatArguments="-x $DIR/exchange.lp -e edge -n semi"
export dflatArguments="--tables -p $DIR/dynamic.lp -e edge"

$DIR/../../test_counting.sh
