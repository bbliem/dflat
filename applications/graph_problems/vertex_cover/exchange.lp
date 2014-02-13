%dflat: -d td --no-empty-leaves --no-empty-root --tables -e vertex -e edge -n semi --default-join
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
item(X) :- current(X), extend(R), childItem(R,X).

{ item(X) : introduced(X) }.
:- edge(X,Y), current(X;Y), not item(X;Y).

currentCost(C) :- C = #count{ X : item(X) }. % Cost of current vertices
cost(CC + IC) :- extend(R), childCost(R,CC), IC = #count{ X : item(X), introduced(X) }.
cost(C) :- initial, currentCost(C).

#show extend/1.
#show currentCost/1.
#show cost/1.
#show item/1.
