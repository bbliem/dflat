#!/bin/bash

numNodes=6
numEdges=18
export instanceGen="tools/graphs/instance_generator.py $numNodes $numEdges"
#export instanceGen="tools/graphs/tree_instance_generator.py $numNodes $numEdges"
export monolithicEncoding=asp_encodings/hamiltonian_cycle/monolithic.lp
export dflatArguments="-e edge -l 2 asp_encodings/hamiltonian_cycle/dynamic.lp"

tools/verify_counting.sh
