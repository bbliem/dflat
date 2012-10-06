#!/bin/bash

numNodes=10
numEdges=20
export instanceGen="tools/graphs/instance_generator.py $numNodes $numEdges"
export monolithicEncoding=asp_encodings/3col/monolithic.lp
#export dflatArguments="-x asp_encodings/3col/exchange.lp -e edge -n semi"
export dflatArguments="asp_encodings/3col/dynamic.lp -e edge"

tools/test_counting.sh
