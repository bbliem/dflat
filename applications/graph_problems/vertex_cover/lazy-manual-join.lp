%dflat: -d td -n semi --lazy --tables -e vertex -e edge --no-empty-leaves --no-empty-root
#external childItem(in(X)) : childNode(N), bag(N,X).
#external childItem(out(X)) : childNode(N), bag(N,X).
#external childItem(X) : childNode(N), bag(N,X).
#external childCost(0..C): maxCost(C).

:- childItem(in(X)), childItem(out(X)).

item(in(X)) :- childItem(in(X)), current(X).
item(out(X)) :- childItem(out(X)), current(X).
1 { item(in(X)); item(out(X)) } 1 :- introduced(X).
:- edge(X,Y), current(X), current(Y), not item(in(X)), not item(in(Y)).

cost(C) :- initial, C = #count{ X : item(in(X)) }.
cost(CC + IC) :- childCost(CC), IC = #count{ X : item(in(X)), introduced(X) }.
currentCost(C) :- C = #count{ X : item(in(X)) }.

#show item/1. #show auxItem/1. #show childItem/1. #show childAuxItem/1. #show childCost/1.
