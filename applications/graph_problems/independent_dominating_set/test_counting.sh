#!/bin/bash

numNodes=6
numEdges=18
export instanceGen="applications/graph_problems/instance_generator.py $numNodes $numEdges"
#export instanceGen="applications/graph_problems/tree_instance_generator.py $numNodes $numEdges"
export monolithicEncoding=applications/graph_problems/independent_dominating_set/monolithic.lp
export dflatArguments="-e edge -n semi -x applications/graph_problems/independent_dominating_set/dynamic_semi.lp -j applications/graph_problems/independent_dominating_set/dynamic_semi.lp"

applications/test_counting.sh
