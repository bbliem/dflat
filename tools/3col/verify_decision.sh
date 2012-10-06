#!/bin/bash

numNodes=10
numEdges=20
export instanceGen="tools/graphs/instance_generator.py $numNodes $numEdges"
export monolithicEncoding=asp_encodings/3col/monolithic.lp
export exchangeEncoding=asp_encodings/3col/exchange_decision.lp
export edgeArguments="-e edge"

tools/verify_decision.sh
