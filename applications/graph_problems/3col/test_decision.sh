#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
numNodes=10
numEdges=20
export instanceGen="$DIR/../instance_generator.py $numNodes $numEdges"
export monolithicEncoding=$DIR/monolithic.lp
export dflatArguments="-x $DIR/exchange_decision.lp -e edge -n semi"

$DIR/../../test_decision.sh
