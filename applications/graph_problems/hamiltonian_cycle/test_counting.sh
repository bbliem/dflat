#!/bin/bash

numNodes=6
numEdges=18
export instanceGen="applications/graph_problems/instance_generator.py $numNodes $numEdges"
#export instanceGen="applications/graph_problems/tree_instance_generator.py $numNodes $numEdges"
export monolithicEncoding=applications/graph_problems/hamiltonian_cycle/monolithic.lp
export dflatArguments="-e edge applications/graph_problems/hamiltonian_cycle/dynamic.lp"

applications/test_counting.sh
