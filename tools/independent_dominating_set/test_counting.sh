#!/bin/bash

numNodes=6
numEdges=18
export instanceGen="tools/graphs/instance_generator.py $numNodes $numEdges"
#export instanceGen="tools/graphs/tree_instance_generator.py $numNodes $numEdges"
export monolithicEncoding=asp_encodings/independent_dominating_set/monolithic.lp
export dflatArguments="-e edge -n semi -x asp_encodings/independent_dominating_set/dynamic_semi.lp -j asp_encodings/independent_dominating_set/dynamic_semi.lp"

tools/test_counting.sh
