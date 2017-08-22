%dflat: -d td -n semi --lazy --tables --default-join -e vertex -e edge --no-empty-leaves --no-empty-root
#external childItem(X) : childNode(N), bag(N,X).
#external childCost(0..50).

item(X) :- childItem(X), current(X).
{ item(X) : introduced(X) }.
:- edge(X,Y), current(X), current(Y), not item(X), not item(Y).

cost(C) :- initial, C = #count{ X : item(X) }.
cost(CC + IC) :- childCost(CC), IC = #count{ X : item(X), introduced(X) }.
currentCost(C) :- C = #count{ X : item(X) }.
