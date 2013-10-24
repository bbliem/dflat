#!/bin/bash

numNodes=10
numEdges=20
export instanceGen="applications/graph_problems/instance_generator.py $numNodes $numEdges"
export monolithicEncoding=applications/graph_problems/3col/monolithic.lp
export dflatArguments="-x applications/graph_problems/3col/exchange_decision.lp -e edge -n semi"

applications/test_decision.sh
