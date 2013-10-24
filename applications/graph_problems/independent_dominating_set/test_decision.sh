#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
numNodes=6
numEdges=18
export instanceGen="$DIR/../instance_generator.py $numNodes $numEdges"
#export instanceGen="$DIR/../tree_instance_generator.py $numNodes $numEdges"
export monolithicEncoding=$DIR/monolithic.lp
export dflatArguments="-e edge -n semi -x $DIR/dynamic_semi.lp -j $DIR/dynamic_semi.lp"

$DIR/../../test_decision.sh
