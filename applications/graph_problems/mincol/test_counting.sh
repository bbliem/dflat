#!/bin/bash

numNodes=8
numEdges=16
export instanceGen="applications/graph_problems/mincol/instance_generator.py $numNodes $numEdges"
export monolithicEncoding=applications/graph_problems/mincol/monolithic.lp
export dflatArguments="applications/graph_problems/mincol/dynamic.lp -e edge --multi-level"

applications/test_counting_metasp.sh
