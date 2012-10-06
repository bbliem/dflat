#!/bin/bash

numNodes=10
numEdges=20
export instanceGen="tools/graphs/instance_generator.py $numNodes $numEdges"
export monolithicEncoding=asp_encodings/vertex_cover/monolithic.lp
export dflatArguments="asp_encodings/vertex_cover/dynamic.lp -e edge"

tools/test_opt_counting.sh
