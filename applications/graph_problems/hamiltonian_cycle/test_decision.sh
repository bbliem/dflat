#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
numNodes=6
numEdges=18
export instanceGen="$DIR/../instance_generator.py $numNodes $numEdges"
#export instanceGen="$DIR/../tree_instance_generator.py $numNodes $numEdges"
export monolithicEncoding=$DIR/monolithic.lp
export dflatArguments="-e edge $DIR/dynamic.lp"

$DIR/../../test_decision.sh
