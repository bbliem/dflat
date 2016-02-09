%dflat: --tables -e vertex -e edge -d td --no-empty-leaves -n semi --default-join --cardinality-cost
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).

item(in(X)) :- extend(R), childItem(R,in(X)), not removed(X).
auxItem(dominated(X)) :- extend(R), childAuxItem(R,dominated(X)), not removed(X).

{ item(in(X)) : introduced(X) }.
auxItem(dominated(Y)) :- item(in(X)), edge(X,Y), current(X), current(Y).
:- removed(X), extend(R), not childItem(R,in(X)), not childAuxItem(R,dominated(X)).

%currentCost(C) :- C = #count { X : item(in(X)) }.
%cost(C) :- initial, currentCost(C).
%cost(CC+IC) :- not initial, extend(R), childCost(R,CC), IC = #count { X : item(in(X)), introduced(X) }.

#show extend/1.
#show item/1.
#show auxItem/1.
%#show cost/1.
%#show currentCost/1.
