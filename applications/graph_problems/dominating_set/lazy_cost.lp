%dflat: -d td --lazy --tables -e vertex -e edge --no-empty-leaves -n semi --default-join
#external childItem(in(X)) : childNode(N), bag(N,X).
#external childAuxItem(dominated(X)) : childNode(N), bag(N,X).
#external childCost(0..C): maxCost(C).

item(in(X)) :- childItem(in(X)), not removed(X).
auxItem(dominated(X)) :- childAuxItem(dominated(X)), not removed(X).

{ item(in(X)) : introduced(X) }.
auxItem(dominated(Y)) :- item(in(X)), edge(X,Y), current(X), current(Y).
:- removed(X), not childItem(in(X)), not childAuxItem(dominated(X)).

cost(C) :- initial, C = #count{ X : item(in(X)) }.
cost(CC + IC) :- childCost(CC), IC = #count{ X : item(in(X)), introduced(X) }.
currentCost(C) :- C = #count{ X : item(in(X)) }.

#show item/1. #show auxItem/1. #show childItem/1. #show childAuxItem/1. #show childCost/1.
