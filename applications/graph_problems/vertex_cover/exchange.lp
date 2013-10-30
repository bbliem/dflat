1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
item(X) :- current(X), extend(R), childItem(R,X).

{ item(X) : introduced(X) }.
:- edge(X,Y), current(X;Y), not item(X;Y).

currentCost(C) :- C = #count{ item(_) }. % Cost of current vertices
cost(CC + IC) :- extend(R), childCost(R,CC), IC = #count{ item(X) : introduced(X) }.
-leaf :- childNode(_).
cost(C) :- not -leaf, currentCost(C).

#hide.
#show extend/1.
#show currentCost/1.
#show cost/1.
#show item/1.
