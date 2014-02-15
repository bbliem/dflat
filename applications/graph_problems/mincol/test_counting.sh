#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)

numNodes=8
numEdges=16
export instanceGen="$DIR/../instance_generator.py $numNodes $numEdges"
export monolithicEncoding=$DIR/monolithic.lp
export dflatArguments="-p $DIR/dynamic.lp"

$DIR/../../test_counting_metasp.sh
