#!/bin/bash

numNodes=10
numEdges=20
export instanceGen="applications/graph_problems/instance_generator.py $numNodes $numEdges"
export monolithicEncoding=applications/graph_problems/vertex_cover/monolithic.lp
export dflatArguments="applications/graph_problems/vertex_cover/dynamic.lp -e edge"

applications/test_optval.sh
