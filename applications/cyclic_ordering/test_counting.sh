#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
numElements=7
numOrderings=8
export instanceGen="$DIR/instance_generator.py $numElements $numOrderings"
export monolithicEncoding=$DIR/monolithic.lp
export dflatArguments="-p $DIR/dynamic_xp.lp"

$DIR/../test_counting.sh
