%dflat: --tables -e vertex -e edge -d td -n weak --no-empty-leaves --default-join
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
item(in(X)) :- extend(R), childItem(R,in(X)), current(X).
auxItem(dominated(X)) :- extend(R), childAuxItem(R,dominated(X)), current(X).
:- removed(X), extend(R), not childItem(R,in(X)), not childAuxItem(R,dominated(X)).
{ item(in(X)) : introduced(X) }.
auxItem(dominated(X)) :- item(in(Y)), edge(Y,X), current(X).
:- edge(X,Y), item(in(X)), item(in(Y)).

currentCost(C) :- C = #count { X : item(in(X)) }.
cost(C) :- initial, currentCost(C).
cost(CC+IC) :- not initial, extend(R), childCost(R,CC), IC = #count { X : item(in(X)), introduced(X) }.

#show extend/1.
#show item/1.
#show auxItem/1.
#show cost/1.
#show currentCost/1.
