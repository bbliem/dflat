#!/bin/bash
DIR=$(cd "$( dirname "$0" )" && pwd)
$DIR/instance_generator.py $@ | cat - <(echo 'occursIn(A,C) :- pos(C,A). occursIn(A,C) :- neg(C,A). edge(X,Y) :- occursIn(X,C), occursIn(Y,C), X != Y.') | gringo -t
