%dflat: -d td --lazy --tables -e vertex -e edge -n semi --default-join
#external childItem(in(X)) : childNode(N), bag(N,X).
#external childAuxItem(dominated(X)) : childNode(N), bag(N,X).

item(in(X)) :- childItem(in(X)), not removed(X).
auxItem(dominated(X)) :- childAuxItem(dominated(X)), not removed(X).

{ item(in(X)) : introduced(X) }.
auxItem(dominated(Y)) :- item(in(X)), edge(X,Y), current(X), current(Y).
:- removed(X), not childItem(in(X)), not childAuxItem(dominated(X)).

counterInc(cost,1,X) :- introduced(X), item(in(X)).
currentCounterInc(cost,1,X) :- introduced(X), item(in(X)).
currentCounterInc(cost,-1,X) :- removed(X), childItem(in(X)).

#show item/1. #show auxItem/1. #show childItem/1. #show childAuxItem/1.
