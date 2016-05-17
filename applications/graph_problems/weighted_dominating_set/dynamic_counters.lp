%dflat: --tables -e vertex -e edge -d td -n semi --default-join
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).

{ item(in(X)) : introduced(X) }.
item(in(X)) :- extend(R), not removed(X), childItem(R,in(X)).
auxItem(dominated(X)) :- extend(R), not removed(X), childAuxItem(R,dominated(X)).
auxItem(dominated(Y)) :- item(in(X)), current(X), current(Y), edge(X,Y).

:- extend(R), removed(X), not childItem(R,in(X)), not childAuxItem(R,dominated(X)).

counterInc(cost,W,X) :- introduced(X), item(in(X)), weight(X,W).
currentCounterInc(cost,W,X) :- introduced(X), item(in(X)), weight(X,W).
currentCounterInc(cost,-W,X) :- extend(R), removed(X), childItem(R,in(X)), weight(X,W).