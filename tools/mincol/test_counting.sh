#!/bin/bash

numNodes=8
numEdges=16
export instanceGen="tools/mincol/instance_generator.py $numNodes $numEdges"
export monolithicEncoding=asp_encodings/mincol/monolithic.lp
export dflatArguments="asp_encodings/mincol/dynamic.lp -e edge --multi-level"

tools/test_counting_metasp.sh
