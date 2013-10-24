#!/bin/bash

numNodes=10
numEdges=20
export instanceGen="applications/graph_problems/instance_generator.py $numNodes $numEdges"
export monolithicEncoding=applications/graph_problems/3col/monolithic.lp
#export dflatArguments="-x applications/graph_problems/3col/exchange.lp -e edge -n semi"
export dflatArguments="--tables -p applications/graph_problems/3col/dynamic.lp -e edge"

applications/test_counting.sh
