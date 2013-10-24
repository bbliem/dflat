#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
numElements=10
numOrderings=14
export instanceGen="$DIR/instance_generator.py $numElements $numOrderings"
export monolithicEncoding=$DIR/monolithic.lp
export dflatArguments="-x $DIR/exchange.lp -j $DIR/join.lp -n semi -e order"

$DIR/../test_counting.sh
