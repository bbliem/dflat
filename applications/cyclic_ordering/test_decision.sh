#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)

numElements=10
numOrderings=14
export instanceGen="$DIR/instance_generator.py $numElements $numOrderings"
export monolithicEncoding=$DIR/monolithic.lp
export dflatArguments="-p $DIR/dynamic_decision.lp"

$DIR/../test_decision.sh
